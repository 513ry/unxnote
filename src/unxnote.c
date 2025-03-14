#include "unxnote.h"
#include "unxnote/window.h"
#include "unxnote/font.h"
#include "unxnote/hash.h"

#include <malloc.h>
#include <stdarg.h>
#include <string.h>

#define BUTTON_WIDTH UINT32_C(20)
#define BUTTON_HEIGHT UINT32_C(20)
#define BUTTON_X_POS WINDOW_WIDTH - BUTTON_WIDTH
#define BUTTON_Y_POS UINT32_C(0)

static FT_Face font = NULL;
static PT_Hash *cookies = NULL;
static bool initialized = false;
static uint16_t window_n = 0;

// --- API Definitions

bool
unxnote_init(unsigned int n, ...)
{
  /* Define application font */
  init_font();
  font = new_font(UNXNOTE_FONT);

  /* Initialize X session */
  if (!init_window())
    return false;

  /* Create cookie hash */
  if (n == 0) {
    UNXNoteCookie cookie = (UNXNoteCookie){
      .glyph = 0xf4f6, .title = "UNXNote", 0xff0000, 0x222233
    };
    cookies = pt_hash_new(1, "default", cookie);
  } else {
    va_list ap;
    va_start(ap, n);
    for (unsigned int i = 0; i < n; ++i) {
      char *key = va_arg(ap, char *);
      UNXNoteCookie *cookie = va_arg(ap, UNXNoteCookie *);
      pt_hash_set(cookies, key, *cookie);
    }
    va_end(ap);
  }

  return initialized = true;
}

bool
unxnote_free(void)
{
  if (!initialized)
    return false;
  
  free_font(1, font);
  free_window();
  pt_hash_free(cookies);
  initialized = false;
  return true;
}

/*
 * Display UNXNote message in X
 */
bool
unxnote_msg(char *cookie_name, char *from, char *msg)
{
  /* Create a new X window */
  xcb_window_t window = new_window(window_n);
  ++window_n;
  
  /* Get the window cookie */
  UNXNoteCookie cookie = pt_hash_get(cookies, cookie_name);
  
  /* Define graphical contexts */
  const xcb_gcontext_t bg_gc = xcb_generate_id(x_conn.conn);
  const uint32_t bg_values[] = {cookie.bg_color};
  xcb_create_gc(x_conn.conn, bg_gc, window, XCB_GC_FOREGROUND, bg_values);
  const xcb_gcontext_t text_gc = xcb_generate_id(x_conn.conn);
  const uint32_t text_values[] = {cookie.fg_color};
  xcb_create_gc(x_conn.conn, text_gc, window, XCB_GC_FOREGROUND, text_values);
  const xcb_gcontext_t button_gc = xcb_generate_id(x_conn.conn);
  const uint32_t button_values[] = {0x333333};
  xcb_create_gc(x_conn.conn, button_gc, window, XCB_GC_FOREGROUND, button_values);

  char *glyph_string = NULL;
  encode_utf8(cookie.glyph, &glyph_string);

  bool done = false;
  const uint16_t header_size = (uint16_t)strlen(glyph_string) + (uint16_t)strlen(cookie.title) + UINT16_C(2) + 1;
  char header[header_size];
  snprintf(header, header_size, "%s  %s", glyph_string, cookie.title);

  xcb_generic_event_t *event;
  while (!done && (event = xcb_wait_for_event(x_conn.conn))) {
    switch (event->response_type & ~0x80) {
    case XCB_EXPOSE: {
      // Clear window background
      xcb_rectangle_t rect = {0, 0, WINDOW_WIDTH, WINDOW_MIN_HEIGHT};
      xcb_poly_fill_rectangle(x_conn.conn, window, bg_gc, 1, &rect);

      // Render title text
      draw_text(x_conn.conn, window, text_gc, font, header, 3,
		FONT_SIZE);

      // Draw button background
      xcb_rectangle_t button_rect = {BUTTON_X_POS, BUTTON_Y_POS, BUTTON_WIDTH,
                                     BUTTON_HEIGHT};
      xcb_poly_fill_rectangle(x_conn.conn, window, button_gc, 1, &button_rect);

      // Render "x" text in the center of the button
      int x_text_pos = BUTTON_X_POS + BUTTON_WIDTH / 2 - FONT_SIZE / 4;
      int y_text_pos = BUTTON_Y_POS + BUTTON_HEIGHT / 2 + FONT_SIZE / 4;
      draw_text(x_conn.conn, window, text_gc, font, "x", x_text_pos, y_text_pos);

      xcb_flush(x_conn.conn);
      break;
    }
    case XCB_BUTTON_PRESS: {
      xcb_button_press_event_t *bp = (xcb_button_press_event_t *)event;
      if (bp->event_x >= BUTTON_X_POS &&
          bp->event_x <= BUTTON_X_POS + BUTTON_WIDTH &&
          bp->event_y >= BUTTON_Y_POS &&
          bp->event_y <= BUTTON_Y_POS + BUTTON_HEIGHT) {
        // Exit if the button is clicked
        done = true;
      }
      break;
    }
    }
    free(event);
  }

  free(glyph_string);
  xcb_free_gc(x_conn.conn, bg_gc);
  xcb_free_gc(x_conn.conn, text_gc);
  xcb_free_gc(x_conn.conn, button_gc);
  xcb_destroy_window(x_conn.conn, window);

  return true;
}

// -- Static Definitions
