#ifndef graphics3d_include
#define graphics3d_include

#include <stdint.h>
#include "point3d.h"

// ---- Capacity limits -------------------------------------------------------
// All Object3D storage is fixed-size (no malloc). Bump these if you build
// objects more complex than a cube/dodecahedron/etc.
//
// Bumped from 16 to 20 to accommodate the figure-8 character (18 vertices).
#define OBJ3D_MAX_VERTICES   20
#define OBJ3D_MAX_NEIGHBORS  6     // edges incident to one vertex
#define OBJ3D_MAX_TRIANGLES  32    // triangles after BFS->cycles->ear-clipping
#define OBJ3D_MAX_CYCLE_LEN  20    // length of any single cycle

// ---- Adjacency -------------------------------------------------------------
// Replaces the Python tuple-of-lists ([1,3,4],[0,2,5],...) used as a "dict"
// of vertex -> neighbor indices. One entry per vertex; `count` says how many
// of `neighbors[]` are valid.
typedef struct {
    int8_t neighbors[OBJ3D_MAX_NEIGHBORS];
    int8_t count;
} AdjacencyEntry;

// ---- Object3D --------------------------------------------------------------
// Mirrors the Python Object3D. The triangulation (faces/triangles derived
// from BFS spanning tree + ear-clipping) is computed once at init time, since
// it depends only on topology (adjacency), not vertex positions.
typedef struct {
    Point3D        vertices[OBJ3D_MAX_VERTICES];
    AdjacencyEntry adjacency[OBJ3D_MAX_VERTICES];
    int8_t         num_vertices;
    Point3D        origin;            // centroid of vertices

    // Cached triangulation -- triangles[i] = three vertex indices
    int8_t         triangles[OBJ3D_MAX_TRIANGLES][3];
    int8_t         num_triangles;
} Object3D;

// ---- Object lifecycle ------------------------------------------------------
// Initialize an Object3D from a vertex list and per-vertex adjacency list.
// Computes the centroid (origin) and runs BFS->cycles->ear-clip.
void obj3d_init(Object3D *obj,
                const Point3D *vertices,
                int8_t num_vertices,
                const AdjacencyEntry *adjacency);

// Translate origin and every vertex (matches Object3D.translate).
void obj3d_translate(Object3D *obj, float dx, float dy, float dz);

// Rotate every vertex about the object's own origin (translate to origin,
// rotate, translate back). Matches Object3D.rotate_amount.
void obj3d_rotate_about_origin(Object3D *obj,
                               float theta, float phi, float rho);

// Rotate every vertex about world origin (matches rotate_around_axis).
void obj3d_rotate_about_world(Object3D *obj,
                              float theta, float phi, float rho);

// Scale every vertex (and the origin) about world origin (0, 0, 0).
// Multiplies x, y, z of every vertex by `s`. Use BEFORE translating, since
// scaling about world origin means a translated object would also move.
void obj3d_scale(Object3D *obj, float s);

// ---- Rendering -------------------------------------------------------------
// Each renders into the screen.h framebuffer; call Screen_Clear() before
// and Screen_Display() after.

// Draw all edges as 3D parameterized lines (matches generate_line_list()
// followed by the Python pixel-plotting pipeline).
void obj3d_render_wireframe(const Object3D *obj);

// Draw all cached triangles as filled planes (sweep-line method, matches
// PlaneTriangle3D + generate_plane_list()).
void obj3d_render_filled(const Object3D *obj);

// Both wireframe and filled. Matches output_display_map().
void obj3d_render(const Object3D *obj);

#endif