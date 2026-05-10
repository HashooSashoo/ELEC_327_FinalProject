// characters.c -- 36-glyph vector font library.
//
// Glyph data layout:
//   For each character, two static const arrays in flash:
//     glyph_verts_<name>[]      Point3D array, all z = 0.0f
//     glyph_adj_<name>[]        AdjacencyEntry array
//   These are wrapped in a `const GlyphData char_<name>` struct.
//
// Total flash cost: roughly 5-6 KB across all 36 characters.
// RAM cost: one Object3D workspace (~600 bytes), shared across all calls.

#include "characters.h"
#include "point3d.h"
#include "graphics3d.h"
#include <stdint.h>

// ---- Per-glyph data (flash-resident) ---------------------------------------

static const Point3D glyph_verts_char_0[8] = {
    { 0.0000f, 1.0000f, 0.0000f },
    { 0.0000f, 4.0000f, 0.0000f },
    { 1.0000f, 5.0000f, 0.0000f },
    { 2.0000f, 5.0000f, 0.0000f },
    { 3.0000f, 4.0000f, 0.0000f },
    { 3.0000f, 1.0000f, 0.0000f },
    { 2.0000f, 0.0000f, 0.0000f },
    { 1.0000f, 0.0000f, 0.0000f },
};

static const AdjacencyEntry glyph_adj_char_0[8] = {
    { { 1, 7, 0, 0, 0, 0 }, 2 },
    { { 0, 2, 0, 0, 0, 0 }, 2 },
    { { 1, 3, 0, 0, 0, 0 }, 2 },
    { { 2, 4, 0, 0, 0, 0 }, 2 },
    { { 3, 5, 0, 0, 0, 0 }, 2 },
    { { 4, 6, 0, 0, 0, 0 }, 2 },
    { { 5, 7, 0, 0, 0, 0 }, 2 },
    { { 0, 6, 0, 0, 0, 0 }, 2 },
};

const GlyphData char_0 = {
    .vertices     = glyph_verts_char_0,
    .num_vertices = 8,
    .adjacency    = glyph_adj_char_0,
};

static const Point3D glyph_verts_char_1[5] = {
    { 1.5000f, 0.0000f, 0.0000f },
    { 1.5000f, 5.0000f, 0.0000f },
    { 0.5000f, 4.0000f, 0.0000f },
    { 0.0000f, 0.0000f, 0.0000f },
    { 3.0000f, 0.0000f, 0.0000f },
};

static const AdjacencyEntry glyph_adj_char_1[5] = {
    { { 1, 0, 0, 0, 0, 0 }, 1 },
    { { 0, 2, 0, 0, 0, 0 }, 2 },
    { { 1, 0, 0, 0, 0, 0 }, 1 },
    { { 4, 0, 0, 0, 0, 0 }, 1 },
    { { 3, 0, 0, 0, 0, 0 }, 1 },
};

const GlyphData char_1 = {
    .vertices     = glyph_verts_char_1,
    .num_vertices = 5,
    .adjacency    = glyph_adj_char_1,
};

static const Point3D glyph_verts_char_2[6] = {
    { 0.0000f, 4.0000f, 0.0000f },
    { 1.0000f, 5.0000f, 0.0000f },
    { 2.0000f, 5.0000f, 0.0000f },
    { 3.0000f, 4.0000f, 0.0000f },
    { 0.0000f, 0.0000f, 0.0000f },
    { 3.0000f, 0.0000f, 0.0000f },
};

static const AdjacencyEntry glyph_adj_char_2[6] = {
    { { 1, 0, 0, 0, 0, 0 }, 1 },
    { { 0, 2, 0, 0, 0, 0 }, 2 },
    { { 1, 3, 0, 0, 0, 0 }, 2 },
    { { 2, 4, 0, 0, 0, 0 }, 2 },
    { { 3, 5, 0, 0, 0, 0 }, 2 },
    { { 4, 0, 0, 0, 0, 0 }, 1 },
};

const GlyphData char_2 = {
    .vertices     = glyph_verts_char_2,
    .num_vertices = 6,
    .adjacency    = glyph_adj_char_2,
};

static const Point3D glyph_verts_char_3[6] = {
    { 0.0000f, 5.0000f, 0.0000f },
    { 3.0000f, 5.0000f, 0.0000f },
    { 3.0000f, 2.5000f, 0.0000f },
    { 1.5000f, 2.5000f, 0.0000f },
    { 3.0000f, 0.0000f, 0.0000f },
    { 0.0000f, 0.0000f, 0.0000f },
};

static const AdjacencyEntry glyph_adj_char_3[6] = {
    { { 1, 0, 0, 0, 0, 0 }, 1 },
    { { 0, 2, 0, 0, 0, 0 }, 2 },
    { { 1, 3, 4, 0, 0, 0 }, 3 },
    { { 2, 0, 0, 0, 0, 0 }, 1 },
    { { 2, 5, 0, 0, 0, 0 }, 2 },
    { { 4, 0, 0, 0, 0, 0 }, 1 },
};

const GlyphData char_3 = {
    .vertices     = glyph_verts_char_3,
    .num_vertices = 6,
    .adjacency    = glyph_adj_char_3,
};

static const Point3D glyph_verts_char_4[4] = {
    { 2.5000f, 5.0000f, 0.0000f },
    { 2.5000f, 0.0000f, 0.0000f },
    { 0.0000f, 2.0000f, 0.0000f },
    { 3.0000f, 2.0000f, 0.0000f },
};

static const AdjacencyEntry glyph_adj_char_4[4] = {
    { { 1, 2, 0, 0, 0, 0 }, 2 },
    { { 0, 0, 0, 0, 0, 0 }, 1 },
    { { 0, 3, 0, 0, 0, 0 }, 2 },
    { { 2, 0, 0, 0, 0, 0 }, 1 },
};

const GlyphData char_4 = {
    .vertices     = glyph_verts_char_4,
    .num_vertices = 4,
    .adjacency    = glyph_adj_char_4,
};

static const Point3D glyph_verts_char_5[7] = {
    { 3.0000f, 5.0000f, 0.0000f },
    { 0.0000f, 5.0000f, 0.0000f },
    { 0.0000f, 2.5000f, 0.0000f },
    { 2.0000f, 2.5000f, 0.0000f },
    { 3.0000f, 1.5000f, 0.0000f },
    { 2.0000f, 0.0000f, 0.0000f },
    { 0.0000f, 0.0000f, 0.0000f },
};

static const AdjacencyEntry glyph_adj_char_5[7] = {
    { { 1, 0, 0, 0, 0, 0 }, 1 },
    { { 0, 2, 0, 0, 0, 0 }, 2 },
    { { 1, 3, 0, 0, 0, 0 }, 2 },
    { { 2, 4, 0, 0, 0, 0 }, 2 },
    { { 3, 5, 0, 0, 0, 0 }, 2 },
    { { 4, 6, 0, 0, 0, 0 }, 2 },
    { { 5, 0, 0, 0, 0, 0 }, 1 },
};

const GlyphData char_5 = {
    .vertices     = glyph_verts_char_5,
    .num_vertices = 7,
    .adjacency    = glyph_adj_char_5,
};

static const Point3D glyph_verts_char_6[10] = {
    { 3.0000f, 5.0000f, 0.0000f },
    { 1.0000f, 5.0000f, 0.0000f },
    { 0.0000f, 4.0000f, 0.0000f },
    { 0.0000f, 1.0000f, 0.0000f },
    { 1.0000f, 0.0000f, 0.0000f },
    { 2.0000f, 0.0000f, 0.0000f },
    { 3.0000f, 1.0000f, 0.0000f },
    { 3.0000f, 2.0000f, 0.0000f },
    { 2.0000f, 2.5000f, 0.0000f },
    { 0.0000f, 2.5000f, 0.0000f },
};

static const AdjacencyEntry glyph_adj_char_6[10] = {
    { { 1, 0, 0, 0, 0, 0 }, 1 },
    { { 0, 2, 0, 0, 0, 0 }, 2 },
    { { 1, 3, 0, 0, 0, 0 }, 2 },
    { { 2, 4, 0, 0, 0, 0 }, 2 },
    { { 3, 5, 0, 0, 0, 0 }, 2 },
    { { 4, 6, 0, 0, 0, 0 }, 2 },
    { { 5, 7, 0, 0, 0, 0 }, 2 },
    { { 6, 8, 0, 0, 0, 0 }, 2 },
    { { 7, 9, 0, 0, 0, 0 }, 2 },
    { { 8, 0, 0, 0, 0, 0 }, 1 },
};

const GlyphData char_6 = {
    .vertices     = glyph_verts_char_6,
    .num_vertices = 10,
    .adjacency    = glyph_adj_char_6,
};

static const Point3D glyph_verts_char_7[3] = {
    { 0.0000f, 5.0000f, 0.0000f },
    { 3.0000f, 5.0000f, 0.0000f },
    { 1.0000f, 0.0000f, 0.0000f },
};

static const AdjacencyEntry glyph_adj_char_7[3] = {
    { { 1, 0, 0, 0, 0, 0 }, 1 },
    { { 0, 2, 0, 0, 0, 0 }, 2 },
    { { 1, 0, 0, 0, 0, 0 }, 1 },
};

const GlyphData char_7 = {
    .vertices     = glyph_verts_char_7,
    .num_vertices = 3,
    .adjacency    = glyph_adj_char_7,
};

static const Point3D glyph_verts_char_8[18] = {
    { 1.0000f, 2.5000f, 0.0000f },
    { 0.3000f, 3.0000f, 0.0000f },
    { 0.0000f, 4.0000f, 0.0000f },
    { 0.3000f, 4.5000f, 0.0000f },
    { 1.0000f, 5.0000f, 0.0000f },
    { 2.0000f, 5.0000f, 0.0000f },
    { 2.7000f, 4.5000f, 0.0000f },
    { 3.0000f, 4.0000f, 0.0000f },
    { 2.7000f, 3.0000f, 0.0000f },
    { 2.0000f, 2.5000f, 0.0000f },
    { 0.3000f, 2.0000f, 0.0000f },
    { 0.0000f, 1.0000f, 0.0000f },
    { 0.3000f, 0.5000f, 0.0000f },
    { 1.0000f, 0.0000f, 0.0000f },
    { 2.0000f, 0.0000f, 0.0000f },
    { 2.7000f, 0.5000f, 0.0000f },
    { 3.0000f, 1.0000f, 0.0000f },
    { 2.7000f, 2.0000f, 0.0000f },
};

static const AdjacencyEntry glyph_adj_char_8[18] = {
    { { 1, 9, 10, 0, 0, 0 }, 3 },
    { { 0, 2, 0, 0, 0, 0 }, 2 },
    { { 1, 3, 0, 0, 0, 0 }, 2 },
    { { 2, 4, 0, 0, 0, 0 }, 2 },
    { { 3, 5, 0, 0, 0, 0 }, 2 },
    { { 4, 6, 0, 0, 0, 0 }, 2 },
    { { 5, 7, 0, 0, 0, 0 }, 2 },
    { { 6, 8, 0, 0, 0, 0 }, 2 },
    { { 7, 9, 0, 0, 0, 0 }, 2 },
    { { 0, 8, 17, 0, 0, 0 }, 3 },
    { { 0, 11, 0, 0, 0, 0 }, 2 },
    { { 10, 12, 0, 0, 0, 0 }, 2 },
    { { 11, 13, 0, 0, 0, 0 }, 2 },
    { { 12, 14, 0, 0, 0, 0 }, 2 },
    { { 13, 15, 0, 0, 0, 0 }, 2 },
    { { 14, 16, 0, 0, 0, 0 }, 2 },
    { { 15, 17, 0, 0, 0, 0 }, 2 },
    { { 9, 16, 0, 0, 0, 0 }, 2 },
};

const GlyphData char_8 = {
    .vertices     = glyph_verts_char_8,
    .num_vertices = 18,
    .adjacency    = glyph_adj_char_8,
};

static const Point3D glyph_verts_char_9[10] = {
    { 3.0000f, 2.5000f, 0.0000f },
    { 1.0000f, 2.5000f, 0.0000f },
    { 0.0000f, 3.0000f, 0.0000f },
    { 0.0000f, 4.0000f, 0.0000f },
    { 1.0000f, 5.0000f, 0.0000f },
    { 2.0000f, 5.0000f, 0.0000f },
    { 3.0000f, 4.0000f, 0.0000f },
    { 3.0000f, 1.0000f, 0.0000f },
    { 2.0000f, 0.0000f, 0.0000f },
    { 0.0000f, 0.0000f, 0.0000f },
};

static const AdjacencyEntry glyph_adj_char_9[10] = {
    { { 1, 0, 0, 0, 0, 0 }, 1 },
    { { 0, 2, 0, 0, 0, 0 }, 2 },
    { { 1, 3, 0, 0, 0, 0 }, 2 },
    { { 2, 4, 0, 0, 0, 0 }, 2 },
    { { 3, 5, 0, 0, 0, 0 }, 2 },
    { { 4, 6, 0, 0, 0, 0 }, 2 },
    { { 5, 7, 0, 0, 0, 0 }, 2 },
    { { 6, 8, 0, 0, 0, 0 }, 2 },
    { { 7, 9, 0, 0, 0, 0 }, 2 },
    { { 8, 0, 0, 0, 0, 0 }, 1 },
};

const GlyphData char_9 = {
    .vertices     = glyph_verts_char_9,
    .num_vertices = 10,
    .adjacency    = glyph_adj_char_9,
};

static const Point3D glyph_verts_char_A[5] = {
    { 0.0000f, 0.0000f, 0.0000f },
    { 1.5000f, 5.0000f, 0.0000f },
    { 3.0000f, 0.0000f, 0.0000f },
    { 0.7500f, 2.5000f, 0.0000f },
    { 2.2500f, 2.5000f, 0.0000f },
};

static const AdjacencyEntry glyph_adj_char_A[5] = {
    { { 1, 0, 0, 0, 0, 0 }, 1 },
    { { 0, 2, 0, 0, 0, 0 }, 2 },
    { { 1, 0, 0, 0, 0, 0 }, 1 },
    { { 4, 0, 0, 0, 0, 0 }, 1 },
    { { 3, 0, 0, 0, 0, 0 }, 1 },
};

const GlyphData char_A = {
    .vertices     = glyph_verts_char_A,
    .num_vertices = 5,
    .adjacency    = glyph_adj_char_A,
};

static const Point3D glyph_verts_char_B[10] = {
    { 0.0000f, 0.0000f, 0.0000f },
    { 0.0000f, 2.5000f, 0.0000f },
    { 0.0000f, 5.0000f, 0.0000f },
    { 2.5000f, 5.0000f, 0.0000f },
    { 3.0000f, 4.5000f, 0.0000f },
    { 3.0000f, 3.0000f, 0.0000f },
    { 2.5000f, 2.5000f, 0.0000f },
    { 3.0000f, 2.0000f, 0.0000f },
    { 3.0000f, 0.5000f, 0.0000f },
    { 2.5000f, 0.0000f, 0.0000f },
};

static const AdjacencyEntry glyph_adj_char_B[10] = {
    { { 1, 9, 0, 0, 0, 0 }, 2 },
    { { 0, 2, 6, 0, 0, 0 }, 3 },
    { { 1, 3, 0, 0, 0, 0 }, 2 },
    { { 2, 4, 0, 0, 0, 0 }, 2 },
    { { 3, 5, 0, 0, 0, 0 }, 2 },
    { { 4, 6, 0, 0, 0, 0 }, 2 },
    { { 1, 5, 7, 0, 0, 0 }, 3 },
    { { 6, 8, 0, 0, 0, 0 }, 2 },
    { { 7, 9, 0, 0, 0, 0 }, 2 },
    { { 0, 8, 0, 0, 0, 0 }, 2 },
};

const GlyphData char_B = {
    .vertices     = glyph_verts_char_B,
    .num_vertices = 10,
    .adjacency    = glyph_adj_char_B,
};

static const Point3D glyph_verts_char_C[6] = {
    { 3.0000f, 5.0000f, 0.0000f },
    { 1.0000f, 5.0000f, 0.0000f },
    { 0.0000f, 4.0000f, 0.0000f },
    { 0.0000f, 1.0000f, 0.0000f },
    { 1.0000f, 0.0000f, 0.0000f },
    { 3.0000f, 0.0000f, 0.0000f },
};

static const AdjacencyEntry glyph_adj_char_C[6] = {
    { { 1, 0, 0, 0, 0, 0 }, 1 },
    { { 0, 2, 0, 0, 0, 0 }, 2 },
    { { 1, 3, 0, 0, 0, 0 }, 2 },
    { { 2, 4, 0, 0, 0, 0 }, 2 },
    { { 3, 5, 0, 0, 0, 0 }, 2 },
    { { 4, 0, 0, 0, 0, 0 }, 1 },
};

const GlyphData char_C = {
    .vertices     = glyph_verts_char_C,
    .num_vertices = 6,
    .adjacency    = glyph_adj_char_C,
};

static const Point3D glyph_verts_char_D[6] = {
    { 0.0000f, 0.0000f, 0.0000f },
    { 0.0000f, 5.0000f, 0.0000f },
    { 2.0000f, 5.0000f, 0.0000f },
    { 3.0000f, 4.0000f, 0.0000f },
    { 3.0000f, 1.0000f, 0.0000f },
    { 2.0000f, 0.0000f, 0.0000f },
};

static const AdjacencyEntry glyph_adj_char_D[6] = {
    { { 1, 5, 0, 0, 0, 0 }, 2 },
    { { 0, 2, 0, 0, 0, 0 }, 2 },
    { { 1, 3, 0, 0, 0, 0 }, 2 },
    { { 2, 4, 0, 0, 0, 0 }, 2 },
    { { 3, 5, 0, 0, 0, 0 }, 2 },
    { { 0, 4, 0, 0, 0, 0 }, 2 },
};

const GlyphData char_D = {
    .vertices     = glyph_verts_char_D,
    .num_vertices = 6,
    .adjacency    = glyph_adj_char_D,
};

static const Point3D glyph_verts_char_E[6] = {
    { 3.0000f, 5.0000f, 0.0000f },
    { 0.0000f, 5.0000f, 0.0000f },
    { 0.0000f, 0.0000f, 0.0000f },
    { 3.0000f, 0.0000f, 0.0000f },
    { 0.0000f, 2.5000f, 0.0000f },
    { 2.0000f, 2.5000f, 0.0000f },
};

static const AdjacencyEntry glyph_adj_char_E[6] = {
    { { 1, 0, 0, 0, 0, 0 }, 1 },
    { { 0, 2, 0, 0, 0, 0 }, 2 },
    { { 1, 3, 0, 0, 0, 0 }, 2 },
    { { 2, 0, 0, 0, 0, 0 }, 1 },
    { { 5, 0, 0, 0, 0, 0 }, 1 },
    { { 4, 0, 0, 0, 0, 0 }, 1 },
};

const GlyphData char_E = {
    .vertices     = glyph_verts_char_E,
    .num_vertices = 6,
    .adjacency    = glyph_adj_char_E,
};

static const Point3D glyph_verts_char_F[5] = {
    { 3.0000f, 5.0000f, 0.0000f },
    { 0.0000f, 5.0000f, 0.0000f },
    { 0.0000f, 0.0000f, 0.0000f },
    { 0.0000f, 2.5000f, 0.0000f },
    { 2.0000f, 2.5000f, 0.0000f },
};

static const AdjacencyEntry glyph_adj_char_F[5] = {
    { { 1, 0, 0, 0, 0, 0 }, 1 },
    { { 0, 2, 0, 0, 0, 0 }, 2 },
    { { 1, 0, 0, 0, 0, 0 }, 1 },
    { { 4, 0, 0, 0, 0, 0 }, 1 },
    { { 3, 0, 0, 0, 0, 0 }, 1 },
};

const GlyphData char_F = {
    .vertices     = glyph_verts_char_F,
    .num_vertices = 5,
    .adjacency    = glyph_adj_char_F,
};

static const Point3D glyph_verts_char_G[8] = {
    { 3.0000f, 5.0000f, 0.0000f },
    { 1.0000f, 5.0000f, 0.0000f },
    { 0.0000f, 4.0000f, 0.0000f },
    { 0.0000f, 1.0000f, 0.0000f },
    { 1.0000f, 0.0000f, 0.0000f },
    { 3.0000f, 0.0000f, 0.0000f },
    { 3.0000f, 2.0000f, 0.0000f },
    { 1.5000f, 2.0000f, 0.0000f },
};

static const AdjacencyEntry glyph_adj_char_G[8] = {
    { { 1, 0, 0, 0, 0, 0 }, 1 },
    { { 0, 2, 0, 0, 0, 0 }, 2 },
    { { 1, 3, 0, 0, 0, 0 }, 2 },
    { { 2, 4, 0, 0, 0, 0 }, 2 },
    { { 3, 5, 0, 0, 0, 0 }, 2 },
    { { 4, 6, 0, 0, 0, 0 }, 2 },
    { { 5, 7, 0, 0, 0, 0 }, 2 },
    { { 6, 0, 0, 0, 0, 0 }, 1 },
};

const GlyphData char_G = {
    .vertices     = glyph_verts_char_G,
    .num_vertices = 8,
    .adjacency    = glyph_adj_char_G,
};

static const Point3D glyph_verts_char_H[6] = {
    { 0.0000f, 0.0000f, 0.0000f },
    { 0.0000f, 5.0000f, 0.0000f },
    { 3.0000f, 0.0000f, 0.0000f },
    { 3.0000f, 5.0000f, 0.0000f },
    { 0.0000f, 2.5000f, 0.0000f },
    { 3.0000f, 2.5000f, 0.0000f },
};

static const AdjacencyEntry glyph_adj_char_H[6] = {
    { { 1, 0, 0, 0, 0, 0 }, 1 },
    { { 0, 0, 0, 0, 0, 0 }, 1 },
    { { 3, 0, 0, 0, 0, 0 }, 1 },
    { { 2, 0, 0, 0, 0, 0 }, 1 },
    { { 5, 0, 0, 0, 0, 0 }, 1 },
    { { 4, 0, 0, 0, 0, 0 }, 1 },
};

const GlyphData char_H = {
    .vertices     = glyph_verts_char_H,
    .num_vertices = 6,
    .adjacency    = glyph_adj_char_H,
};

static const Point3D glyph_verts_char_I[6] = {
    { 0.0000f, 5.0000f, 0.0000f },
    { 3.0000f, 5.0000f, 0.0000f },
    { 1.5000f, 5.0000f, 0.0000f },
    { 1.5000f, 0.0000f, 0.0000f },
    { 0.0000f, 0.0000f, 0.0000f },
    { 3.0000f, 0.0000f, 0.0000f },
};

static const AdjacencyEntry glyph_adj_char_I[6] = {
    { { 1, 0, 0, 0, 0, 0 }, 1 },
    { { 0, 0, 0, 0, 0, 0 }, 1 },
    { { 3, 0, 0, 0, 0, 0 }, 1 },
    { { 2, 0, 0, 0, 0, 0 }, 1 },
    { { 5, 0, 0, 0, 0, 0 }, 1 },
    { { 4, 0, 0, 0, 0, 0 }, 1 },
};

const GlyphData char_I = {
    .vertices     = glyph_verts_char_I,
    .num_vertices = 6,
    .adjacency    = glyph_adj_char_I,
};

static const Point3D glyph_verts_char_J[6] = {
    { 0.0000f, 5.0000f, 0.0000f },
    { 3.0000f, 5.0000f, 0.0000f },
    { 3.0000f, 1.0000f, 0.0000f },
    { 2.0000f, 0.0000f, 0.0000f },
    { 1.0000f, 0.0000f, 0.0000f },
    { 0.0000f, 1.0000f, 0.0000f },
};

static const AdjacencyEntry glyph_adj_char_J[6] = {
    { { 1, 0, 0, 0, 0, 0 }, 1 },
    { { 0, 2, 0, 0, 0, 0 }, 2 },
    { { 1, 3, 0, 0, 0, 0 }, 2 },
    { { 2, 4, 0, 0, 0, 0 }, 2 },
    { { 3, 5, 0, 0, 0, 0 }, 2 },
    { { 4, 0, 0, 0, 0, 0 }, 1 },
};

const GlyphData char_J = {
    .vertices     = glyph_verts_char_J,
    .num_vertices = 6,
    .adjacency    = glyph_adj_char_J,
};

static const Point3D glyph_verts_char_K[5] = {
    { 0.0000f, 0.0000f, 0.0000f },
    { 0.0000f, 5.0000f, 0.0000f },
    { 3.0000f, 5.0000f, 0.0000f },
    { 0.0000f, 2.5000f, 0.0000f },
    { 3.0000f, 0.0000f, 0.0000f },
};

static const AdjacencyEntry glyph_adj_char_K[5] = {
    { { 1, 0, 0, 0, 0, 0 }, 1 },
    { { 0, 0, 0, 0, 0, 0 }, 1 },
    { { 3, 0, 0, 0, 0, 0 }, 1 },
    { { 2, 4, 0, 0, 0, 0 }, 2 },
    { { 3, 0, 0, 0, 0, 0 }, 1 },
};

const GlyphData char_K = {
    .vertices     = glyph_verts_char_K,
    .num_vertices = 5,
    .adjacency    = glyph_adj_char_K,
};

static const Point3D glyph_verts_char_L[3] = {
    { 0.0000f, 5.0000f, 0.0000f },
    { 0.0000f, 0.0000f, 0.0000f },
    { 3.0000f, 0.0000f, 0.0000f },
};

static const AdjacencyEntry glyph_adj_char_L[3] = {
    { { 1, 0, 0, 0, 0, 0 }, 1 },
    { { 0, 2, 0, 0, 0, 0 }, 2 },
    { { 1, 0, 0, 0, 0, 0 }, 1 },
};

const GlyphData char_L = {
    .vertices     = glyph_verts_char_L,
    .num_vertices = 3,
    .adjacency    = glyph_adj_char_L,
};

static const Point3D glyph_verts_char_M[5] = {
    { 0.0000f, 0.0000f, 0.0000f },
    { 0.0000f, 5.0000f, 0.0000f },
    { 1.5000f, 2.5000f, 0.0000f },
    { 3.0000f, 5.0000f, 0.0000f },
    { 3.0000f, 0.0000f, 0.0000f },
};

static const AdjacencyEntry glyph_adj_char_M[5] = {
    { { 1, 0, 0, 0, 0, 0 }, 1 },
    { { 0, 2, 0, 0, 0, 0 }, 2 },
    { { 1, 3, 0, 0, 0, 0 }, 2 },
    { { 2, 4, 0, 0, 0, 0 }, 2 },
    { { 3, 0, 0, 0, 0, 0 }, 1 },
};

const GlyphData char_M = {
    .vertices     = glyph_verts_char_M,
    .num_vertices = 5,
    .adjacency    = glyph_adj_char_M,
};

static const Point3D glyph_verts_char_N[4] = {
    { 0.0000f, 0.0000f, 0.0000f },
    { 0.0000f, 5.0000f, 0.0000f },
    { 3.0000f, 0.0000f, 0.0000f },
    { 3.0000f, 5.0000f, 0.0000f },
};

static const AdjacencyEntry glyph_adj_char_N[4] = {
    { { 1, 0, 0, 0, 0, 0 }, 1 },
    { { 0, 2, 0, 0, 0, 0 }, 2 },
    { { 1, 3, 0, 0, 0, 0 }, 2 },
    { { 2, 0, 0, 0, 0, 0 }, 1 },
};

const GlyphData char_N = {
    .vertices     = glyph_verts_char_N,
    .num_vertices = 4,
    .adjacency    = glyph_adj_char_N,
};

static const Point3D glyph_verts_char_O[8] = {
    { 0.0000f, 1.0000f, 0.0000f },
    { 0.0000f, 4.0000f, 0.0000f },
    { 1.0000f, 5.0000f, 0.0000f },
    { 2.0000f, 5.0000f, 0.0000f },
    { 3.0000f, 4.0000f, 0.0000f },
    { 3.0000f, 1.0000f, 0.0000f },
    { 2.0000f, 0.0000f, 0.0000f },
    { 1.0000f, 0.0000f, 0.0000f },
};

static const AdjacencyEntry glyph_adj_char_O[8] = {
    { { 1, 7, 0, 0, 0, 0 }, 2 },
    { { 0, 2, 0, 0, 0, 0 }, 2 },
    { { 1, 3, 0, 0, 0, 0 }, 2 },
    { { 2, 4, 0, 0, 0, 0 }, 2 },
    { { 3, 5, 0, 0, 0, 0 }, 2 },
    { { 4, 6, 0, 0, 0, 0 }, 2 },
    { { 5, 7, 0, 0, 0, 0 }, 2 },
    { { 0, 6, 0, 0, 0, 0 }, 2 },
};

const GlyphData char_O = {
    .vertices     = glyph_verts_char_O,
    .num_vertices = 8,
    .adjacency    = glyph_adj_char_O,
};

static const Point3D glyph_verts_char_P[7] = {
    { 0.0000f, 0.0000f, 0.0000f },
    { 0.0000f, 5.0000f, 0.0000f },
    { 2.0000f, 5.0000f, 0.0000f },
    { 3.0000f, 4.0000f, 0.0000f },
    { 3.0000f, 3.0000f, 0.0000f },
    { 2.0000f, 2.0000f, 0.0000f },
    { 0.0000f, 2.0000f, 0.0000f },
};

static const AdjacencyEntry glyph_adj_char_P[7] = {
    { { 1, 0, 0, 0, 0, 0 }, 1 },
    { { 0, 2, 0, 0, 0, 0 }, 2 },
    { { 1, 3, 0, 0, 0, 0 }, 2 },
    { { 2, 4, 0, 0, 0, 0 }, 2 },
    { { 3, 5, 0, 0, 0, 0 }, 2 },
    { { 4, 6, 0, 0, 0, 0 }, 2 },
    { { 5, 0, 0, 0, 0, 0 }, 1 },
};

const GlyphData char_P = {
    .vertices     = glyph_verts_char_P,
    .num_vertices = 7,
    .adjacency    = glyph_adj_char_P,
};

static const Point3D glyph_verts_char_Q[10] = {
    { 0.0000f, 1.0000f, 0.0000f },
    { 0.0000f, 4.0000f, 0.0000f },
    { 1.0000f, 5.0000f, 0.0000f },
    { 2.0000f, 5.0000f, 0.0000f },
    { 3.0000f, 4.0000f, 0.0000f },
    { 3.0000f, 1.0000f, 0.0000f },
    { 2.0000f, 0.0000f, 0.0000f },
    { 1.0000f, 0.0000f, 0.0000f },
    { 2.0000f, 1.0000f, 0.0000f },
    { 3.0000f, 0.0000f, 0.0000f },
};

static const AdjacencyEntry glyph_adj_char_Q[10] = {
    { { 1, 7, 0, 0, 0, 0 }, 2 },
    { { 0, 2, 0, 0, 0, 0 }, 2 },
    { { 1, 3, 0, 0, 0, 0 }, 2 },
    { { 2, 4, 0, 0, 0, 0 }, 2 },
    { { 3, 5, 0, 0, 0, 0 }, 2 },
    { { 4, 6, 0, 0, 0, 0 }, 2 },
    { { 5, 7, 0, 0, 0, 0 }, 2 },
    { { 0, 6, 0, 0, 0, 0 }, 2 },
    { { 9, 0, 0, 0, 0, 0 }, 1 },
    { { 8, 0, 0, 0, 0, 0 }, 1 },
};

const GlyphData char_Q = {
    .vertices     = glyph_verts_char_Q,
    .num_vertices = 10,
    .adjacency    = glyph_adj_char_Q,
};

static const Point3D glyph_verts_char_R[8] = {
    { 0.0000f, 0.0000f, 0.0000f },
    { 0.0000f, 5.0000f, 0.0000f },
    { 2.0000f, 5.0000f, 0.0000f },
    { 3.0000f, 4.0000f, 0.0000f },
    { 3.0000f, 3.0000f, 0.0000f },
    { 2.0000f, 2.0000f, 0.0000f },
    { 0.0000f, 2.0000f, 0.0000f },
    { 3.0000f, 0.0000f, 0.0000f },
};

static const AdjacencyEntry glyph_adj_char_R[8] = {
    { { 1, 0, 0, 0, 0, 0 }, 1 },
    { { 0, 2, 0, 0, 0, 0 }, 2 },
    { { 1, 3, 0, 0, 0, 0 }, 2 },
    { { 2, 4, 0, 0, 0, 0 }, 2 },
    { { 3, 5, 0, 0, 0, 0 }, 2 },
    { { 4, 6, 7, 0, 0, 0 }, 3 },
    { { 5, 0, 0, 0, 0, 0 }, 1 },
    { { 5, 0, 0, 0, 0, 0 }, 1 },
};

const GlyphData char_R = {
    .vertices     = glyph_verts_char_R,
    .num_vertices = 8,
    .adjacency    = glyph_adj_char_R,
};

static const Point3D glyph_verts_char_S[8] = {
    { 3.0000f, 5.0000f, 0.0000f },
    { 1.0000f, 5.0000f, 0.0000f },
    { 0.0000f, 4.0000f, 0.0000f },
    { 1.0000f, 2.5000f, 0.0000f },
    { 2.0000f, 2.5000f, 0.0000f },
    { 3.0000f, 1.0000f, 0.0000f },
    { 2.0000f, 0.0000f, 0.0000f },
    { 0.0000f, 0.0000f, 0.0000f },
};

static const AdjacencyEntry glyph_adj_char_S[8] = {
    { { 1, 0, 0, 0, 0, 0 }, 1 },
    { { 0, 2, 0, 0, 0, 0 }, 2 },
    { { 1, 3, 0, 0, 0, 0 }, 2 },
    { { 2, 4, 0, 0, 0, 0 }, 2 },
    { { 3, 5, 0, 0, 0, 0 }, 2 },
    { { 4, 6, 0, 0, 0, 0 }, 2 },
    { { 5, 7, 0, 0, 0, 0 }, 2 },
    { { 6, 0, 0, 0, 0, 0 }, 1 },
};

const GlyphData char_S = {
    .vertices     = glyph_verts_char_S,
    .num_vertices = 8,
    .adjacency    = glyph_adj_char_S,
};

static const Point3D glyph_verts_char_T[4] = {
    { 0.0000f, 5.0000f, 0.0000f },
    { 3.0000f, 5.0000f, 0.0000f },
    { 1.5000f, 5.0000f, 0.0000f },
    { 1.5000f, 0.0000f, 0.0000f },
};

static const AdjacencyEntry glyph_adj_char_T[4] = {
    { { 1, 0, 0, 0, 0, 0 }, 1 },
    { { 0, 0, 0, 0, 0, 0 }, 1 },
    { { 3, 0, 0, 0, 0, 0 }, 1 },
    { { 2, 0, 0, 0, 0, 0 }, 1 },
};

const GlyphData char_T = {
    .vertices     = glyph_verts_char_T,
    .num_vertices = 4,
    .adjacency    = glyph_adj_char_T,
};

static const Point3D glyph_verts_char_U[6] = {
    { 0.0000f, 5.0000f, 0.0000f },
    { 0.0000f, 1.0000f, 0.0000f },
    { 1.0000f, 0.0000f, 0.0000f },
    { 2.0000f, 0.0000f, 0.0000f },
    { 3.0000f, 1.0000f, 0.0000f },
    { 3.0000f, 5.0000f, 0.0000f },
};

static const AdjacencyEntry glyph_adj_char_U[6] = {
    { { 1, 0, 0, 0, 0, 0 }, 1 },
    { { 0, 2, 0, 0, 0, 0 }, 2 },
    { { 1, 3, 0, 0, 0, 0 }, 2 },
    { { 2, 4, 0, 0, 0, 0 }, 2 },
    { { 3, 5, 0, 0, 0, 0 }, 2 },
    { { 4, 0, 0, 0, 0, 0 }, 1 },
};

const GlyphData char_U = {
    .vertices     = glyph_verts_char_U,
    .num_vertices = 6,
    .adjacency    = glyph_adj_char_U,
};

static const Point3D glyph_verts_char_V[3] = {
    { 0.0000f, 5.0000f, 0.0000f },
    { 1.5000f, 0.0000f, 0.0000f },
    { 3.0000f, 5.0000f, 0.0000f },
};

static const AdjacencyEntry glyph_adj_char_V[3] = {
    { { 1, 0, 0, 0, 0, 0 }, 1 },
    { { 0, 2, 0, 0, 0, 0 }, 2 },
    { { 1, 0, 0, 0, 0, 0 }, 1 },
};

const GlyphData char_V = {
    .vertices     = glyph_verts_char_V,
    .num_vertices = 3,
    .adjacency    = glyph_adj_char_V,
};

static const Point3D glyph_verts_char_W[5] = {
    { 0.0000f, 5.0000f, 0.0000f },
    { 0.7500f, 0.0000f, 0.0000f },
    { 1.5000f, 2.5000f, 0.0000f },
    { 2.2500f, 0.0000f, 0.0000f },
    { 3.0000f, 5.0000f, 0.0000f },
};

static const AdjacencyEntry glyph_adj_char_W[5] = {
    { { 1, 0, 0, 0, 0, 0 }, 1 },
    { { 0, 2, 0, 0, 0, 0 }, 2 },
    { { 1, 3, 0, 0, 0, 0 }, 2 },
    { { 2, 4, 0, 0, 0, 0 }, 2 },
    { { 3, 0, 0, 0, 0, 0 }, 1 },
};

const GlyphData char_W = {
    .vertices     = glyph_verts_char_W,
    .num_vertices = 5,
    .adjacency    = glyph_adj_char_W,
};

static const Point3D glyph_verts_char_X[4] = {
    { 0.0000f, 0.0000f, 0.0000f },
    { 3.0000f, 5.0000f, 0.0000f },
    { 0.0000f, 5.0000f, 0.0000f },
    { 3.0000f, 0.0000f, 0.0000f },
};

static const AdjacencyEntry glyph_adj_char_X[4] = {
    { { 1, 0, 0, 0, 0, 0 }, 1 },
    { { 0, 0, 0, 0, 0, 0 }, 1 },
    { { 3, 0, 0, 0, 0, 0 }, 1 },
    { { 2, 0, 0, 0, 0, 0 }, 1 },
};

const GlyphData char_X = {
    .vertices     = glyph_verts_char_X,
    .num_vertices = 4,
    .adjacency    = glyph_adj_char_X,
};

static const Point3D glyph_verts_char_Y[4] = {
    { 0.0000f, 5.0000f, 0.0000f },
    { 1.5000f, 2.5000f, 0.0000f },
    { 3.0000f, 5.0000f, 0.0000f },
    { 1.5000f, 0.0000f, 0.0000f },
};

static const AdjacencyEntry glyph_adj_char_Y[4] = {
    { { 1, 0, 0, 0, 0, 0 }, 1 },
    { { 0, 2, 3, 0, 0, 0 }, 3 },
    { { 1, 0, 0, 0, 0, 0 }, 1 },
    { { 1, 0, 0, 0, 0, 0 }, 1 },
};

const GlyphData char_Y = {
    .vertices     = glyph_verts_char_Y,
    .num_vertices = 4,
    .adjacency    = glyph_adj_char_Y,
};

static const Point3D glyph_verts_char_Z[4] = {
    { 0.0000f, 5.0000f, 0.0000f },
    { 3.0000f, 5.0000f, 0.0000f },
    { 0.0000f, 0.0000f, 0.0000f },
    { 3.0000f, 0.0000f, 0.0000f },
};

static const AdjacencyEntry glyph_adj_char_Z[4] = {
    { { 1, 0, 0, 0, 0, 0 }, 1 },
    { { 0, 2, 0, 0, 0, 0 }, 2 },
    { { 1, 3, 0, 0, 0, 0 }, 2 },
    { { 2, 0, 0, 0, 0, 0 }, 1 },
};

const GlyphData char_Z = {
    .vertices     = glyph_verts_char_Z,
    .num_vertices = 4,
    .adjacency    = glyph_adj_char_Z,
};

// ---- Shared rendering workspace and dispatcher -----------------------------

// One Object3D shared across all glyph_render calls. obj3d_init is called
// for every glyph_render -- this is intentionally O(work). The init copies
// vertex/adjacency from flash, computes the centroid, and runs the BFS
// triangulation (which is unused for wireframe rendering but harmless).
// For ~36 characters per frame at modest scales, the cost is negligible
// compared to the SPI transfer.
static Object3D _glyph_workspace;

// ---- Tunables for layout ---------------------------------------------------

// Glyph local bounding box. Vertex data spans [0, GLYPH_W] x [0, GLYPH_H].
#define GLYPH_W   3.0f
#define GLYPH_H   5.0f

// Letter advance in glyph_render_word (local-space units, before scale):
// glyph width plus a 1-unit gap. Scaled by `scale` at use time.
#define GLYPH_ADVANCE   (GLYPH_W + 1.0f)   // = 4.0f

void glyph_render(const GlyphData *glyph, float x, float y, float z, float scale) {
    if (glyph == 0) return;

    // Materialize the glyph into the workspace from flash data.
    obj3d_init(&_glyph_workspace,
               glyph->vertices,
               glyph->num_vertices,
               glyph->adjacency);

    // Scale every vertex about local origin (0, 0, 0). z is already 0 so the
    // multiplication is a no-op for it; we do it anyway for consistency.
    for (int8_t i = 0; i < _glyph_workspace.num_vertices; i++) {
        _glyph_workspace.vertices[i].x *= scale;
        _glyph_workspace.vertices[i].y *= scale;
        _glyph_workspace.vertices[i].z *= scale;
    }
    // The centroid stored by obj3d_init was computed from unscaled vertices,
    // so it must scale too. (Origin is currently unused for wireframe but
    // keeping it correct in case we add per-glyph rotation later.)
    _glyph_workspace.origin.x *= scale;
    _glyph_workspace.origin.y *= scale;
    _glyph_workspace.origin.z *= scale;

    // Translate so the glyph's bounding-box center lands at (x, y, z).
    // Local center is (1.5, 2.5, 0) before scale, (1.5*scale, 2.5*scale, 0)
    // after scale. Subtract that and add the user's target.
    float dx = x - (GLYPH_W * 0.5f) * scale;
    float dy = y - (GLYPH_H * 0.5f) * scale;
    float dz = z;
    obj3d_translate(&_glyph_workspace, dx, dy, dz);

    obj3d_render_wireframe(&_glyph_workspace);
}

const GlyphData *glyph_for_char(char c) {
    switch (c) {
        case '0': return &char_0;
        case '1': return &char_1;
        case '2': return &char_2;
        case '3': return &char_3;
        case '4': return &char_4;
        case '5': return &char_5;
        case '6': return &char_6;
        case '7': return &char_7;
        case '8': return &char_8;
        case '9': return &char_9;
        case 'A': return &char_A;
        case 'B': return &char_B;
        case 'C': return &char_C;
        case 'D': return &char_D;
        case 'E': return &char_E;
        case 'F': return &char_F;
        case 'G': return &char_G;
        case 'H': return &char_H;
        case 'I': return &char_I;
        case 'J': return &char_J;
        case 'K': return &char_K;
        case 'L': return &char_L;
        case 'M': return &char_M;
        case 'N': return &char_N;
        case 'O': return &char_O;
        case 'P': return &char_P;
        case 'Q': return &char_Q;
        case 'R': return &char_R;
        case 'S': return &char_S;
        case 'T': return &char_T;
        case 'U': return &char_U;
        case 'V': return &char_V;
        case 'W': return &char_W;
        case 'X': return &char_X;
        case 'Y': return &char_Y;
        case 'Z': return &char_Z;
        default:  return 0;
    }
}

void glyph_render_char(char c, float x, float y, float z, float scale) {
    const GlyphData *g = glyph_for_char(c);
    if (g != 0) glyph_render(g, x, y, z, scale);
}

// Render a string centered as a whole around (x, y, z). Letters advance by
// GLYPH_ADVANCE * scale (= 4 * scale) to the right of the previous one.
//
// Spaces (' ') consume a full advance worth of horizontal space but render
// nothing -- so "A B" is exactly as wide as "ABC", with a blank slot in
// the middle. Other unsupported characters are skipped silently AND consume
// no space (glyph_render_char with an unknown char is a no-op, but cur_x
// advances anyway because the centering math assumed it would). If you want
// unsupported chars to also be invisible-and-blank like spaces, that's the
// current behavior; if you want them squeezed out, we'd need a two-pass
// width calc.
//
// Computes string length manually rather than calling strlen() to avoid
// pulling in <string.h>; the loop runs once for length, once for render.
void glyph_render_word(const char *str, float x, float y, float z, float scale) {
    if (str == 0) return;

    // Count characters
    int n = 0;
    while (str[n] != '\0') n++;
    if (n == 0) return;

    // Total width = N glyphs plus (N-1) gaps = N*GLYPH_W + (N-1)*1
    //             = N * (GLYPH_W + 1) - 1
    //             = N * GLYPH_ADVANCE - 1
    // (the trailing gap doesn't exist since there's no letter after the last)
    float total_width_local = (float)n * GLYPH_ADVANCE - 1.0f;
    float total_width = total_width_local * scale;

    // Center of leftmost glyph: starts at -total_width/2 + (glyph half-width)
    // because glyph_render takes the CENTER of each glyph, not its left edge.
    float first_center_x = x - total_width * 0.5f + (GLYPH_W * 0.5f) * scale;

    float cur_x = first_center_x;
    for (int i = 0; i < n; i++) {
        // Skip rendering for spaces, but still advance horizontally so the
        // gap appears in the right place. Any other character goes through
        // glyph_render_char (which silently does nothing for unknown chars).
        if (str[i] != ' ') {
            glyph_render_char(str[i], cur_x, y, z, scale);
        }
        cur_x += GLYPH_ADVANCE * scale;
    }
}