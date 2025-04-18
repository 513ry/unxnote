#include "unxnote/font.h"

#include <xcb/xcb.h>
#include <freetype2/ft2build.h>
#include FT_FREETYPE_H

static FT_Library ft = NULL;

static uint32_t utf8_decode(const char **str);

// --- API Definitions

bool
init_font()
{

  /* Initialize FreeType */

  if (FT_Init_FreeType(&ft))
    return false;
  return true;
}

FT_Face
new_font(char *font_path)
{
  FT_Face face;
  if (FT_New_Face(ft, font_path, 0, &face))
    return NULL;
  FT_Set_Pixel_Sizes(face, 0, FONT_SIZE);
  return face;
}

void
free_font(unsigned int n, ...)
{
  va_list ap;
  va_start(ap, n);

  for (int i = 0; i < n; ++i) {
    FT_Face face = va_arg(ap, FT_Face);
    if (face != NULL)
      FT_Done_Face((FT_Face)face);
  }

  va_end(ap);
  FT_Done_FreeType(ft);
}

/*
 * Render UTF8 encoded text and draw FT glyph bitmap
 */

void
draw_text(xcb_connection_t *conn, xcb_window_t window, xcb_gcontext_t gc,
	  FT_Face face, const char *text, uint32_t x, uint32_t y)
{
  const char *p = text;
  while (*p) {
    uint32_t codepoint = utf8_decode(&p);
    if (codepoint < 0 || FT_Load_Char(face, codepoint, FT_LOAD_RENDER))
      continue;

    FT_Bitmap *bitmap = &face->glyph->bitmap;
    for (uint32_t row = 0; row < bitmap->rows; ++row) {
      for (uint32_t col = 0; col < bitmap->width; ++col) {
        unsigned char intensity = bitmap->buffer[row * bitmap->width + col];
	uint32_t color = (intensity << 16) | (intensity << 8) | intensity;
	uint32_t values[] = {color};
	xcb_change_gc(conn, gc, XCB_GC_FOREGROUND, values);

	xcb_rectangle_t pixel_rect = {x + face->glyph->bitmap_left + col,
				      y - face->glyph->bitmap_top + row, 1,
				      1};
	xcb_poly_fill_rectangle(conn, window, gc, 1, &pixel_rect);
      }
    }

    /* Move forward by glyph's advance width */

    x += face->glyph->advance.x >> 6;
  }
}

const char *utf8_encode(uint32_t glyph) {
  static char encode_buffer[5];
  if (!(glyph & ~0x7F)) {           // 0xxxxxxx — 1 byte
    encode_buffer[0] = glyph;
    encode_buffer[1] = '\0';
  } else if (!(glyph & ~0x7FF)) {   // 110xxxxx 10xxxxxx — 2 bytes
    encode_buffer[0] = 0xC0 | (glyph >> 6);
    encode_buffer[1] = 0x80 | (glyph & 0x3F);
    encode_buffer[2] = '\0';
  } else if (!(glyph & ~0xFFFF)) {  // 1110xxxx ... — 3 bytes
    encode_buffer[0] = 0xE0 | (glyph >> 12);
    encode_buffer[1] = 0x80 | ((glyph >> 6) & 0x3F);
    encode_buffer[2] = 0x80 | (glyph & 0x3F);
    encode_buffer[3] = '\0';
  } else if (!(glyph & ~0x10FFFF)) { // 11110xxx ... — 4 bytes
    encode_buffer[0] = 0xF0 | (glyph >> 18);
    encode_buffer[1] = 0x80 | ((glyph >> 12) & 0x3F);
    encode_buffer[2] = 0x80 | ((glyph >> 6) & 0x3F);
    encode_buffer[3] = 0x80 | (glyph & 0x3F);
    encode_buffer[4] = '\0';
  }
  return encode_buffer;
}

// --- Static Definitions

/*
 * UTF-8 to Unicode code point decoder
 */

static uint32_t
utf8_decode(const char **str)
{
  const unsigned char *s = (const unsigned char *)*str;
  uint32_t code = 0;
  uint32_t bytes = 1;

  if ((*s & 0x80) == 0) {              // 1-byte character
    code = *s;
  } else if ((*s & 0xe0) == 0xc0) {    // 2-byte character
    code = *s & 0x1f;
    bytes = 2;
  } else if ((*s & 0xf0) == 0xe0) {    // 3-byte character
    code = *s & 0x0f;
    bytes = 3;
  } else if ((*s & 0xf8) == 0xf0) {    // 4-byte character
    code = *s & 0x07;
    bytes = 4;
  } else {
    return -1;                         // Invalid UTF-8
  }

  for (uint32_t i = 1; i < bytes; i++) {
    code = (code << 6) | (s[i] & 0x3f);
  }
  *str += bytes;
  return code;
}
