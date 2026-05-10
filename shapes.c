// shapes.c -- factory functions for built-in 3D shapes.
//
// Each factory packages up a fixed vertex list and adjacency list, then
// hands them off to obj3d_init. The caller-provided Object3D ends up fully
// initialized: vertices populated, adjacency populated, origin (centroid)
// computed.
//
// The shape data is declared `static const` so it lives in flash, not RAM.
// Each call to cube_init/arrow_init reads from the same flash data and
// copies into the caller's Object3D, so there's no per-instance flash cost.

#include "shapes.h"
#include "point3d.h"
#include "graphics3d.h"

// ============================================================================
// Cube
// ============================================================================
//
// Vertex layout (centroid at origin):
//
//        4-------5            y
//       /|      /|            |
//      0-+-----1 |            |
//      | 7-----+-6     z------+------(no axis -- y goes up, x right, z out)
//      |/      |/            /
//      3-------2            x
//
// Indices in the convention used by graphics3d:
//   0,1,2,3 = front face (z = -1)   (closer to camera if camera at +z, but
//   4,5,6,7 = back face  (z = +1)    we project from origin so the sign of z
//                                    just sets which face is "front" visually)

static const Point3D _cube_vertices[8] = {
    { 1.0f,  1.0f, -1.0f},   // 0: front top-right
    {-1.0f,  1.0f, -1.0f},   // 1: front top-left
    {-1.0f, -1.0f, -1.0f},   // 2: front bottom-left
    { 1.0f, -1.0f, -1.0f},   // 3: front bottom-right
    { 1.0f,  1.0f,  1.0f},   // 4: back  top-right
    {-1.0f,  1.0f,  1.0f},   // 5: back  top-left
    {-1.0f, -1.0f,  1.0f},   // 6: back  bottom-left
    { 1.0f, -1.0f,  1.0f},   // 7: back  bottom-right
};

// Each vertex is connected to 3 neighbors -- one along each axis.
// Trailing zeros in the neighbor array are unused (count says how many are
// valid). Layout matches the original cube_init in main.c:
//   0 -- 1 (along -x), 0 -- 3 (along -y), 0 -- 4 (along +z)
static const AdjacencyEntry _cube_adjacency[8] = {
    {{1, 3, 4, 0, 0, 0}, 3},   // 0 connects to 1, 3, 4
    {{0, 2, 5, 0, 0, 0}, 3},   // 1 connects to 0, 2, 5
    {{1, 3, 6, 0, 0, 0}, 3},   // 2 connects to 1, 3, 6
    {{0, 2, 7, 0, 0, 0}, 3},   // 3 connects to 0, 2, 7
    {{0, 5, 7, 0, 0, 0}, 3},   // 4 connects to 0, 5, 7
    {{1, 4, 6, 0, 0, 0}, 3},   // 5 connects to 1, 4, 6
    {{2, 5, 7, 0, 0, 0}, 3},   // 6 connects to 2, 5, 7
    {{3, 4, 6, 0, 0, 0}, 3},   // 7 connects to 3, 4, 6
};

void cube_init(Object3D *cube) {
    obj3d_init(cube, _cube_vertices, 8, _cube_adjacency);
}

// ============================================================================
// 2D Arrow
// ============================================================================
//
// Lies in the XY plane (z = 0), pointing along +X. Four vertices:
//
//         (0.5, 1, 0)
//             \
//              \
//   (-2.5,0,0)--(1.5,0,0)   <- tip
//              /
//             /
//         (0.5,-1, 0)
//
// Adjacency (3 undirected edges):
//   shaft: vertex 0 (-2.5, 0) <--> vertex 1 (1.5, 0)
//   fin1:  vertex 1 (1.5, 0)  <--> vertex 2 (0.5, +1)
//   fin2:  vertex 1 (1.5, 0)  <--> vertex 3 (0.5, -1)
//
// The tip (vertex 1) has 3 neighbors; the other three have 1 each.

static const Point3D _arrow_vertices[4] = {
    {-2.5f, 0.0f, 0.0f},   // 0: back of shaft
    { 1.5f, 0.0f, 0.0f},   // 1: tip
    { 0.5f, 1.0f, 0.0f},   // 2: top fin
    { 0.5f,-1.0f, 0.0f},   // 3: bottom fin
};

static const AdjacencyEntry _arrow_adjacency[4] = {
    {{1, 0, 0, 0, 0, 0},       1},   // 0 -> 1
    {{0, 2, 3, 0, 0, 0},       3},   // 1 -> 0, 2, 3
    {{1, 0, 0, 0, 0, 0},       1},   // 2 -> 1
    {{1, 0, 0, 0, 0, 0},       1},   // 3 -> 1
};

void arrow_init(Object3D *arrow) {
    obj3d_init(arrow, _arrow_vertices, 4, _arrow_adjacency);
}
