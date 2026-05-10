#ifndef shapes_include
#define shapes_include

#include "graphics3d.h"

// ---- Built-in shape factories ----------------------------------------------
//
// Each factory initializes a caller-provided Object3D with the topology and
// geometry of a built-in shape. The shape is centered at the local origin
// (0, 0, 0) -- the caller is responsible for translating it to wherever
// it should appear.
//
// To create multiple instances, just declare multiple Object3D variables
// and pass each to the factory:
//
//     Object3D cube1, cube2, arrow1;
//     cube_init(&cube1);
//     cube_init(&cube2);
//     arrow_init(&arrow1);
//     obj3d_translate(&cube1,  -1.5f, 0, 4);   // left
//     obj3d_translate(&cube2,   1.5f, 0, 4);   // right
//     obj3d_translate(&arrow1,  0,    2, 4);   // above
//
// Each instance has its own vertex/adjacency storage (~600 bytes per
// Object3D), so they can be transformed and rendered independently.

// Unit cube, 8 vertices at (+/-1, +/-1, +/-1), 12 edges. Centroid at (0,0,0).
void cube_init(Object3D *cube);

// 2D arrow lying in the XY plane (z = 0), pointing along +X.
// 4 vertices, 3 edges (shaft + two fins). Centroid at (0, 0, 0):
//
//      (0.5, 1) *
//                \
//                 \
//   (-2.5, 0) *----* (1.5, 0)        <- tip points right
//                 /
//                /
//      (0.5,-1) *
//
// Local bounding box: x in [-2.5, 1.5], y in [-1, 1]. Total width 4, height 2.
void arrow_init(Object3D *arrow);

#endif
