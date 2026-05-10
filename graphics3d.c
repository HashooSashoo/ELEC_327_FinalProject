// graphics3d.c -- Object3D, triangulation, projection, rendering.
//
// Architecture notes (vs the Python original):
//
// * No dynamic allocation. Every "list" is a fixed-size array with a count.
//   The Python pointList used by Line3D and PlaneTriangle3D is NOT
//   materialized -- we generate parameterized points one at a time and
//   stream them straight into Screen_SetPixel. (One PlaneTriangle3D's full
//   pointList would be ~5 KB per triangle -- with 18 triangles in a cube
//   that would blow the 32 KB SRAM budget by itself.)
//
// * Triangulation (BFS spanning tree -> fundamental cycles -> ear-clipping)
//   depends only on topology, so we compute it once in obj3d_init and reuse
//   the triangle list every frame after rotation.
//
// * Python's `dict` for adjacency becomes a fixed-size struct array
//   (AdjacencyEntry[]), and the `set` of visited vertices becomes a
//   bool[OBJ3D_MAX_VERTICES] flag array.
//
// * The Python BFS queue is reproduced as a small static circular buffer
//   below.

#include "graphics3d.h"
#include "screen.h"
#include "drawmethods.h"   // BITMAP_COLS, BITMAP_ROWS
#include "point3d.h"
#include <math.h>
#include <stdbool.h>

// ============================================================================
// Tunables
// ============================================================================

// 1 / step size for parameterized lines. 40 -> 41 points per line. Bumped
// from 20 (Python's t = 0.05) so that lines don't look dotted on the
// 120x160 logical grid -- the longest projected cube edge spans ~26 logical
// pixels, and 41 points keeps spacing at ~0.65 px/step.
#define LINE_T_INV   40

// 1 / step size for triangle sweep. Same reasoning.
#define TRI_T_INV    40

// Field-of-view: 60 degrees. Half-angle is 30 degrees, and 1/tan(30 deg) =
// sqrt(3). Pre-computed to avoid calling tanf at runtime.
#define FOV_SCALE    4.0f

// Maps cartesian range [-CART_RANGE, +CART_RANGE] to logical pixels. The
// Python Map class hardcoded 2.0 via the `(coord + 2)` and `(2 - coord)`
// transforms in cartesian_to_poxels.
#define CART_RANGE   2.0f

// Cull range -- points with |projected x or y| greater than this are
// discarded by project(). Decoupled from CART_RANGE: it's safe to cull
// only what's clearly off-screen rather than at the same boundary used
// for screen-mapping. With CART_SCALE=30 and SCREEN_OFFSET_Y=20, screen
// row 0 corresponds to projected y = 2.667, so a cull range of 3.0
// guarantees nothing visible on screen is dropped, and Screen_SetPixel's
// bounds check cleanly handles anything past the edge.
#define CART_CULL    3.0f

// Uniform pixel-per-cartesian-unit scale. We use the smaller of the two
// dimensions so the cube renders square instead of stretched -- the screen
// is 120x160 (taller than wide) so we use 120/4 = 30 for both axes and
// center vertically.
#define CART_SCALE   30

// Where (cartesian 0, 0) ends up on the screen. Centered by construction.
#define SCREEN_OFFSET_X  ((BITMAP_COLS - 4 * CART_SCALE) / 2)   // = 0
#define SCREEN_OFFSET_Y  ((BITMAP_ROWS - 4 * CART_SCALE) / 2)   // = 20

// ============================================================================
// 3D -> 2D screen projection
// ============================================================================

typedef struct {
    int16_t col;
    int16_t row;
    bool    valid;   // false if behind camera or outside the cartesian frustum
} ScreenPoint;

static ScreenPoint project(Point3D p) {
    ScreenPoint sp = { 0, 0, false };

    // Match the Python pipeline: cull anything behind the camera (z < 0)
    // and anything where the projection denominator (z + 1) goes nonpositive.
    if (p.z < 0.0f) return sp;
    float denom = p.z + 1.0f;
    if (denom <= 0.0f) return sp;

    float nx = (p.x / denom) * FOV_SCALE;
    float ny = (p.y / denom) * FOV_SCALE;

    // Frustum cull: drop points clearly outside the screen. Uses CART_CULL
    // (3.0) rather than CART_RANGE (2.0) so we don't drop points that would
    // still land on visible screen rows -- with CART_SCALE=30 and
    // SCREEN_OFFSET_Y=20, the screen extends from projected y=+2.667
    // (row 0) down to y=-3.0 (row 150). Anything past the actual screen
    // edge is cleanly discarded by Screen_SetPixel's bounds check.
    if (nx < -CART_CULL || nx > CART_CULL) return sp;
    if (ny < -CART_CULL || ny > CART_CULL) return sp;

    // Screen mapping. Y-axis flipped (high Y = top of screen) like the Python.
    float fcol = (nx + CART_RANGE) * (float)CART_SCALE + (float)SCREEN_OFFSET_X;
    float frow = (CART_RANGE - ny) * (float)CART_SCALE + (float)SCREEN_OFFSET_Y;

    // Round-half-up for nonnegative values
    sp.col = (int16_t)(fcol + 0.5f);
    sp.row = (int16_t)(frow + 0.5f);
    sp.valid = true;
    return sp;
}

// ============================================================================
// Streaming line and triangle rendering
// ============================================================================
//
// Both routines walk a parameter t in N+1 steps and project each generated
// point. No intermediate point lists -- O(1) memory per primitive.

static void render_line(Point3D p1, Point3D p2) {
    Point3D step = scalePoint3D(subtractPoint3D(p2, p1), 1.0f / (float)LINE_T_INV);
    Point3D cur  = p1;
    for (int i = 0; i <= LINE_T_INV; i++) {
        ScreenPoint sp = project(cur);
        if (sp.valid) {
            Screen_SetPixel(sp.col, sp.row, true);
        }
        cur = addPoint3D(cur, step);
    }
}

static void render_triangle(Point3D p1, Point3D p2, Point3D p3) {
    // Sweep boundary along edge p2->p3, draw a line from p1 to each.
    Point3D edge_step = scalePoint3D(subtractPoint3D(p3, p2), 1.0f / (float)TRI_T_INV);
    Point3D boundary  = p2;
    for (int i = 0; i <= TRI_T_INV; i++) {
        render_line(p1, boundary);
        boundary = addPoint3D(boundary, edge_step);
    }
}

// ============================================================================
// BFS Queue (FIFO, circular, capacity = OBJ3D_MAX_VERTICES)
// ============================================================================
//
// The Python BFS uses queue = [...]; queue.pop(0); queue.append(x). We
// reproduce that behavior with a fixed-size circular buffer. Small enough
// that we just use a single static instance shared across triangulation
// calls -- triangulation runs at obj3d_init time only, never reentrantly.

typedef struct {
    int8_t  buf[OBJ3D_MAX_VERTICES];
    uint8_t head;
    uint8_t tail;
    uint8_t count;
} Queue;

static void queue_init(Queue *q) {
    q->head = 0;
    q->tail = 0;
    q->count = 0;
}

static void queue_push(Queue *q, int8_t v) {
    if (q->count >= OBJ3D_MAX_VERTICES) return;   // shouldn't happen
    q->buf[q->tail] = v;
    q->tail = (uint8_t)((q->tail + 1) % OBJ3D_MAX_VERTICES);
    q->count++;
}

static int8_t queue_pop(Queue *q) {
    if (q->count == 0) return -1;
    int8_t v = q->buf[q->head];
    q->head = (uint8_t)((q->head + 1) % OBJ3D_MAX_VERTICES);
    q->count--;
    return v;
}

static bool queue_empty(const Queue *q) {
    return q->count == 0;
}

// ============================================================================
// Triangulation: BFS spanning tree -> non-tree edges -> cycles -> ear-clip
// ============================================================================
//
// This is a faithful port of find_cycles_from_spanning_tree() and
// output_triangles() from graphics_helper_funcs.py.
//
// IMPORTANT FAITHFULNESS NOTE: the Python algorithm has a subtle
// duplicate-detection quirk where the normalized edge `edge` (a min/max
// tuple) is compared against `non_tree_edges` which stores UNNORMALIZED
// (v, neighbor) tuples. As a result, the same edge can be added from both
// directions. For a cube this produces 7 cycles -> 18 triangles instead of
// the topologically minimal 5 cycles. We reproduce this behavior here
// because that's what your current Python code does; if you want to fix
// the dedup, normalize the comparison in the inner if-check below.

typedef struct { int8_t u, v; } Edge;

#define MAX_TREE_EDGES        (OBJ3D_MAX_VERTICES - 1 + 8)
#define MAX_NON_TREE_EDGES    16

// All triangulation state is static because triangulation runs at init
// only, never concurrently. Keeps obj3d_init's stack small.
static bool   _visited[OBJ3D_MAX_VERTICES];
static int8_t _parent[OBJ3D_MAX_VERTICES];
static Edge   _tree_edges[MAX_TREE_EDGES];
static int    _num_tree_edges;
static Edge   _non_tree_edges[MAX_NON_TREE_EDGES];
static int    _num_non_tree_edges;
static Queue  _bfs_q;

static void normalize_edge(int8_t a, int8_t b, int8_t *out_u, int8_t *out_v) {
    if (a < b) { *out_u = a; *out_v = b; }
    else       { *out_u = b; *out_v = a; }
}

// BFS from vertex 0 through the adjacency graph, classifying edges as
// tree-edges or non-tree-edges. Output goes into the static buffers above.
static void bfs_spanning_tree(const AdjacencyEntry *adj, int8_t num_vertices) {
    for (int8_t i = 0; i < num_vertices; i++) {
        _visited[i] = false;
        _parent[i]  = -1;
    }
    _num_tree_edges     = 0;
    _num_non_tree_edges = 0;
    queue_init(&_bfs_q);

    _visited[0] = true;
    queue_push(&_bfs_q, 0);

    while (!queue_empty(&_bfs_q)) {
        int8_t v = queue_pop(&_bfs_q);
        for (int i = 0; i < adj[v].count; i++) {
            int8_t neighbor = adj[v].neighbors[i];
            if (!_visited[neighbor]) {
                _visited[neighbor] = true;
                _parent[neighbor]  = v;
                if (_num_tree_edges < MAX_TREE_EDGES) {
                    int8_t a, b;
                    normalize_edge(v, neighbor, &a, &b);
                    _tree_edges[_num_tree_edges].u = a;
                    _tree_edges[_num_tree_edges].v = b;
                    _num_tree_edges++;
                }
                queue_push(&_bfs_q, neighbor);
            } else if (_parent[v] != neighbor) {
                // Non-tree edge candidate. Replicates the Python's flawed
                // dedup: compare normalized `edge` against tree_edges (which
                // is normalized -> works) and against non_tree_edges (which
                // stores UNNORMALIZED tuples -> often misses).
                int8_t a, b;
                normalize_edge(v, neighbor, &a, &b);

                // Check tree edges (normalized comparison)
                bool in_tree = false;
                for (int j = 0; j < _num_tree_edges; j++) {
                    if (_tree_edges[j].u == a && _tree_edges[j].v == b) {
                        in_tree = true;
                        break;
                    }
                }
                if (in_tree) continue;

                // Check non-tree edges (Python compares (a,b) tuple to
                // stored (orig_v, orig_neighbor) tuple; succeeds only when
                // the stored pair happens to already be in normalized form)
                bool in_non_tree = false;
                for (int j = 0; j < _num_non_tree_edges; j++) {
                    if (_non_tree_edges[j].u == a && _non_tree_edges[j].v == b) {
                        in_non_tree = true;
                        break;
                    }
                }
                if (in_non_tree) continue;

                if (_num_non_tree_edges < MAX_NON_TREE_EDGES) {
                    // Store UNNORMALIZED, like the Python
                    _non_tree_edges[_num_non_tree_edges].u = v;
                    _non_tree_edges[_num_non_tree_edges].v = neighbor;
                    _num_non_tree_edges++;
                }
            }
        }
    }
}

// Find the cycle in the spanning tree induced by adding non-tree edge (u,v):
// trace u to root, trace v to root, find LCA, build u..lca..v.
// Returns cycle length, writes vertices to cycle_out[].
static int find_cycle_from_edge(int8_t u, int8_t v,
                                int8_t cycle_out[OBJ3D_MAX_CYCLE_LEN]) {
    int8_t path_u[OBJ3D_MAX_VERTICES];
    int    path_u_len = 0;
    int8_t cur = u;
    while (cur != -1 && path_u_len < OBJ3D_MAX_VERTICES) {
        path_u[path_u_len++] = cur;
        cur = _parent[cur];
    }

    int8_t path_v[OBJ3D_MAX_VERTICES];
    int    path_v_len = 0;
    cur = v;
    while (cur != -1 && path_v_len < OBJ3D_MAX_VERTICES) {
        path_v[path_v_len++] = cur;
        cur = _parent[cur];
    }

    // LCA = first node in path_v that also appears in path_u
    int8_t lca = -1;
    for (int i = 0; i < path_v_len && lca == -1; i++) {
        for (int j = 0; j < path_u_len; j++) {
            if (path_v[i] == path_u[j]) { lca = path_v[i]; break; }
        }
    }
    if (lca == -1) return 0;  // disconnected - shouldn't happen

    // Build cycle: u, parent(u), parent(parent(u)), ..., lca
    int len = 0;
    cur = u;
    while (cur != lca && len < OBJ3D_MAX_CYCLE_LEN) {
        cycle_out[len++] = cur;
        cur = _parent[cur];
    }
    if (len < OBJ3D_MAX_CYCLE_LEN) cycle_out[len++] = lca;

    // Then the v-side, reversed (so the cycle reads u..lca..v..(back to u))
    int8_t path_v_to_lca[OBJ3D_MAX_CYCLE_LEN];
    int    pvtl_len = 0;
    cur = v;
    while (cur != lca && pvtl_len < OBJ3D_MAX_CYCLE_LEN) {
        path_v_to_lca[pvtl_len++] = cur;
        cur = _parent[cur];
    }
    for (int i = pvtl_len - 1; i >= 0 && len < OBJ3D_MAX_CYCLE_LEN; i--) {
        cycle_out[len++] = path_v_to_lca[i];
    }
    return len;
}

// Ear-clip a single cycle into triangles, matching Python output_triangles().
// The Python algorithm interleaves cycle[i] with cycle[-i] (front and back),
// dedupes preserving order, then takes consecutive 3-tuples as triangles.
//
// Returns number of triangles written; appends them to obj->triangles.
static int ear_clip_cycle(Object3D *obj,
                          const int8_t cycle[], int cycle_len) {
    // Build interleaved [cycle[0], cycle[-0], cycle[1], cycle[-1], ...]
    int8_t interleaved[OBJ3D_MAX_CYCLE_LEN * 2];
    int    inter_len = 0;
    for (int i = 0; i < cycle_len && inter_len + 2 <= OBJ3D_MAX_CYCLE_LEN * 2; i++) {
        interleaved[inter_len++] = cycle[i];
        // Python cycle[-i]: when i==0 this is cycle[0], else cycle[len-i]
        int reverse_idx = (i == 0) ? 0 : (cycle_len - i);
        interleaved[inter_len++] = cycle[reverse_idx];
    }

    // Order-preserving dedup (matches Python list(dict.fromkeys(...)))
    int8_t dedup[OBJ3D_MAX_CYCLE_LEN * 2];
    int    dedup_len = 0;
    for (int i = 0; i < inter_len; i++) {
        bool seen = false;
        for (int j = 0; j < dedup_len; j++) {
            if (dedup[j] == interleaved[i]) { seen = true; break; }
        }
        if (!seen) dedup[dedup_len++] = interleaved[i];
    }

    // Consecutive 3-tuples become triangles
    int added = 0;
    for (int i = 0; i + 2 < dedup_len && obj->num_triangles < OBJ3D_MAX_TRIANGLES; i++) {
        obj->triangles[obj->num_triangles][0] = dedup[i];
        obj->triangles[obj->num_triangles][1] = dedup[i + 1];
        obj->triangles[obj->num_triangles][2] = dedup[i + 2];
        obj->num_triangles++;
        added++;
    }
    return added;
}

// Run the whole pipeline: BFS -> non-tree edges -> cycles -> ear-clip.
static void compute_triangulation(Object3D *obj) {
    obj->num_triangles = 0;
    bfs_spanning_tree(obj->adjacency, obj->num_vertices);

    for (int i = 0; i < _num_non_tree_edges; i++) {
        int8_t cycle[OBJ3D_MAX_CYCLE_LEN];
        int cycle_len = find_cycle_from_edge(_non_tree_edges[i].u,
                                             _non_tree_edges[i].v,
                                             cycle);
        if (cycle_len >= 3) {
            ear_clip_cycle(obj, cycle, cycle_len);
        }
    }
}

// ============================================================================
// Object3D lifecycle
// ============================================================================

void obj3d_init(Object3D *obj,
                const Point3D *vertices,
                int8_t num_vertices,
                const AdjacencyEntry *adjacency) {
    if (num_vertices > OBJ3D_MAX_VERTICES) num_vertices = OBJ3D_MAX_VERTICES;
    obj->num_vertices = num_vertices;

    // Copy vertices
    for (int8_t i = 0; i < num_vertices; i++) {
        obj->vertices[i] = vertices[i];
    }

    // Copy adjacency
    for (int8_t i = 0; i < num_vertices; i++) {
        obj->adjacency[i] = adjacency[i];
    }

    // Compute centroid (matches `sum(vertex_list) / len(vertex_list)`)
    Point3D sum = makePoint3D(0.0f, 0.0f, 0.0f);
    for (int8_t i = 0; i < num_vertices; i++) {
        sum = addPoint3D(sum, vertices[i]);
    }
    obj->origin = scalePoint3D(sum, (1 / (float)num_vertices));

    // Cache topology-derived triangulation (used by render_filled)
    // compute_triangulation(obj);
}

void obj3d_translate(Object3D *obj, float dx, float dy, float dz) {
    translatePoint3D(&obj->origin, dx, dy, dz);
    for (int8_t i = 0; i < obj->num_vertices; i++) {
        translatePoint3D(&obj->vertices[i], dx, dy, dz);
    }
}

void obj3d_rotate_about_origin(Object3D *obj,
                               float theta, float phi, float rho) {
    Point3D o = obj->origin;
    for (int8_t i = 0; i < obj->num_vertices; i++) {
        translatePoint3D(&obj->vertices[i], -o.x, -o.y, -o.z);
        rotatePoint3D(&obj->vertices[i], theta, phi, rho);
        translatePoint3D(&obj->vertices[i], o.x, o.y, o.z);
    }
}

void obj3d_rotate_about_world(Object3D *obj,
                              float theta, float phi, float rho) {
    for (int8_t i = 0; i < obj->num_vertices; i++) {
        rotatePoint3D(&obj->vertices[i], theta, phi, rho);
    }
}

void obj3d_scale(Object3D *obj, float s) {
    for (int8_t i = 0; i < obj->num_vertices; i++) {
        obj->vertices[i].x *= s;
        obj->vertices[i].y *= s;
        obj->vertices[i].z *= s;
    }
    obj->origin.x *= s;
    obj->origin.y *= s;
    obj->origin.z *= s;
}

// ============================================================================
// Rendering
// ============================================================================

void obj3d_render_wireframe(const Object3D *obj) {
    // For each vertex, draw a line to each of its neighbors. Each undirected
    // edge is rendered twice (once from each endpoint) -- harmless on a
    // monochrome buffer because pixels are idempotent set-operations.
    for (int8_t i = 0; i < obj->num_vertices; i++) {
        const AdjacencyEntry *e = &obj->adjacency[i];
        for (int j = 0; j < e->count; j++) {
            int8_t k = e->neighbors[j];
            if (k >= 0 && k < obj->num_vertices) {
                render_line(obj->vertices[i], obj->vertices[k]);
            }
        }
    }
}

void obj3d_render_filled(const Object3D *obj) {
    for (int8_t i = 0; i < obj->num_triangles; i++) {
        int8_t a = obj->triangles[i][0];
        int8_t b = obj->triangles[i][1];
        int8_t c = obj->triangles[i][2];
        render_triangle(obj->vertices[a], obj->vertices[b], obj->vertices[c]);
    }
}

void obj3d_render(const Object3D *obj) {
    obj3d_render_wireframe(obj);
    obj3d_render_filled(obj);
}