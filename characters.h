#ifndef characters_include
#define characters_include

#include "point3d.h"
#include "graphics3d.h"

// ---- Character glyph library -----------------------------------------------
//
// 36 vector-font glyphs (digits 0-9 and uppercase A-Z), each defined as a
// 3-wide x 5-tall planar wireframe at z = 0.0. Glyph data lives in flash as
// `const`; rendering materializes one glyph at a time into a single shared
// Object3D workspace.
//
// Local coordinate system: each glyph's vertex data spans
//   x in [0.0, 3.0]
//   y in [0.0, 5.0]
//   z = 0.0
// but glyph_render() applies a centroid offset so that, from the caller's
// perspective, the (x, y) translation places the glyph's CENTER at world
// (x, y, z). For example, glyph_render(&char_A, 0, 0, 4, 0.25f) puts A's
// visual center at world (0, 0, 4), scaled to 25% of its default size.
//
// Typical usage:
//
//     // each frame, after Screen_Clear():
//     glyph_render_word("HELLO", 0.0f, 0.0f, 4.0f, 0.25f);
//     Screen_Display();
//
// Notes:
//   - All glyphs share one Object3D workspace inside characters.c, so you
//     must call glyph_render() for each character in sequence (it does init
//     + scale + translate + render in one shot). Do NOT cache pointers to
//     the workspace between calls; subsequent calls will overwrite it.
//   - Glyphs are rendered as wireframe only (filled rendering doesn't make
//     sense for 1D shapes).
//   - The `scale` parameter is a multiplier: 1.0 = default 3x5 size,
//     0.25 = quarter size (0.75 wide x 1.25 tall), etc.

typedef struct {
    const Point3D *vertices;          // points into a const flash array
    int8_t num_vertices;
    const AdjacencyEntry *adjacency;  // points into a const flash array
} GlyphData;

// Materialize a glyph into the shared workspace, scale by `scale`, translate
// so the glyph's center is at (x, y, z), and render its wireframe into the
// screen framebuffer. Caller must call Screen_Clear() before the first glyph
// and Screen_Display() after the last.
//
// scale = 1.0 -> default 3-wide x 5-tall.
// scale = 0.25 -> 0.75 wide x 1.25 tall, etc.
void glyph_render(const GlyphData *glyph, float x, float y, float z, float scale);

// Look up a glyph by character. Returns NULL for unsupported characters
// (anything other than 0-9 and A-Z; lowercase is not supported).
const GlyphData *glyph_for_char(char c);

// Convenience wrapper around glyph_render that takes a char.
void glyph_render_char(char c, float x, float y, float z, float scale);

// Render a null-terminated string of supported characters, centered as a
// whole around (x, y, z). Letters are spaced one bounding-box width plus a
// 1-unit gap apart (4 units of local space, scaled by `scale`).
// Unsupported characters are skipped silently but still consume their slot.
void glyph_render_word(const char *str, float x, float y, float z, float scale);

// All 36 glyph globals (declared extern; defined in characters.c).
extern const GlyphData char_0;
extern const GlyphData char_1;
extern const GlyphData char_2;
extern const GlyphData char_3;
extern const GlyphData char_4;
extern const GlyphData char_5;
extern const GlyphData char_6;
extern const GlyphData char_7;
extern const GlyphData char_8;
extern const GlyphData char_9;
extern const GlyphData char_A;
extern const GlyphData char_B;
extern const GlyphData char_C;
extern const GlyphData char_D;
extern const GlyphData char_E;
extern const GlyphData char_F;
extern const GlyphData char_G;
extern const GlyphData char_H;
extern const GlyphData char_I;
extern const GlyphData char_J;
extern const GlyphData char_K;
extern const GlyphData char_L;
extern const GlyphData char_M;
extern const GlyphData char_N;
extern const GlyphData char_O;
extern const GlyphData char_P;
extern const GlyphData char_Q;
extern const GlyphData char_R;
extern const GlyphData char_S;
extern const GlyphData char_T;
extern const GlyphData char_U;
extern const GlyphData char_V;
extern const GlyphData char_W;
extern const GlyphData char_X;
extern const GlyphData char_Y;
extern const GlyphData char_Z;

#endif