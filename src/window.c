/*
** window.c - System window and screen backend
**
** See Copyright Notice in window.h
*/

#include "unxnote/window.h"
#include "unxnote/font.h"

#include <malloc.h>
#include <string.h>
#include <errno.h>
#include <xcb/randr.h>

#define WINDOW_TYPE "DIALOG"

/**
 * Struct for Motif Hinst (Atom)
 */

typedef struct {
  uint32_t flags;
  uint32_t functions;
  uint32_t decorations;
  int32_t input_mode;
  uint32_t status;
} MotifWmHints;

static bool set_window_type(xcb_connection_t *conn, xcb_window_t window);
static bool get_primary_monitor_geometry(xcb_connection_t *conn, xcb_window_t root,
					 uint16_t *width, uint16_t *height,
					 uint16_t *x, uint16_t *y);

XConnection x_conn = {NULL, 0};

// --- API Definitions

bool
init_window()
{
  xcb_connection_t     *conn;
  xcb_screen_t         *screen;
  int                   screen_n;
  xcb_screen_iterator_t iter;
  
  /* Connect to X server. Use the DISPLAY environment variable */

  conn = xcb_connect(NULL, &screen_n);
  if (xcb_connection_has_error(conn)) {
    errno = EACCES;
    return false;
  }

  /* Get the screen returned by DISPLAY environment variable */

  iter = xcb_setup_roots_iterator(xcb_get_setup(conn));
  for (; iter.rem; --screen_n, xcb_screen_next(&iter))
    if (screen_n == 0) {
      screen = iter.data;
      break;
    }

  uint16_t screen_width, screen_height, screen_x, screen_y;
  if (!get_primary_monitor_geometry(conn, screen->root, &screen_width,
                                    &screen_height, &screen_x, &screen_y)) {
    errno = ECANCELED;
    return false;
  }

#if !defined (UNX_NO_STDIO)
  putchar('\n');
  printf("Informations of screen %i:\n", screen_n);
  printf("  width..........: %" PRIu16 "\n", screen_width);
  printf("  height.........: %" PRIu16 "\n", screen_height);
  printf("  white pixel....: %" PRIu32 "\n", screen->white_pixel);
  printf("  black pixel....: %" PRIu32 "\n", screen->black_pixel);
  putchar('\n');
#endif

  x_conn = (XConnection)
    {conn,
     screen_width, screen_height,
     screen_x, screen_y,
     screen->root, screen->root_visual,
     {0, XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_BUTTON_PRESS}};

  return true;
}

xcb_window_t
new_window(size_t id)
{
  xcb_window_t window = xcb_generate_id(x_conn.conn);
  xcb_create_window(
    x_conn.conn, XCB_COPY_FROM_PARENT, window, x_conn.screen_root,
    x_conn.screen_x + x_conn.screen_width - WINDOW_WIDTH - WINDOW_MARGIN,
    x_conn.screen_y + x_conn.screen_height - (WINDOW_MIN_HEIGHT * (id + 1)) -
    (WINDOW_MARGIN * (id + 1)) - 37, WINDOW_WIDTH, WINDOW_MIN_HEIGHT, 0,
    XCB_WINDOW_CLASS_INPUT_OUTPUT, x_conn.screen_visualid,
    XCB_CW_BACK_PIXMAP | XCB_BACK_PIXMAP_NONE | XCB_CW_EVENT_MASK, x_conn.mask
  );

  set_window_type(x_conn.conn, window);
  xcb_map_window(x_conn.conn, window);
  xcb_flush(x_conn.conn);

  return window;
}

void free_window()
{
  xcb_disconnect(x_conn.conn);
}

// --- Static Definitions

/*
 * Get atom from X to XCB
 */

static xcb_atom_t
get_atom(xcb_connection_t *conn, const char *name)
{
  xcb_intern_atom_cookie_t cookie =
      xcb_intern_atom(conn, 0, strlen(name), name);
  xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(conn, cookie, NULL);
  if (!reply) {
    fprintf(stderr, "Failed to get atom for %s\n", name);
    return XCB_NONE;
  }
  xcb_atom_t atom = reply->atom;
  free(reply);
  return atom;
}

/*
 * Set window type and motif atoms
 */

static bool
set_window_type(xcb_connection_t *conn, xcb_window_t window)
{
  xcb_atom_t net_wm_window_type = get_atom(conn, "_NET_WM_WINDOW_TYPE");
  xcb_atom_t net_wm_window_type_name =
      get_atom(conn, "_NET_WM_WINDOW_TYPE_" WINDOW_TYPE);
  if (net_wm_window_type == XCB_NONE || net_wm_window_type_name == XCB_NONE) {
    errno = EBADR;
    return false;
  }
  xcb_change_property(conn, XCB_PROP_MODE_REPLACE, window, net_wm_window_type,
                      XCB_ATOM_ATOM, 32, 1, &net_wm_window_type_name);

  /* Set explicit Motif Hints if Motif is handled by wm */

  xcb_atom_t motif_wm_hints = get_atom(conn, "_MOTIF_WM_HINTS");
  if (motif_wm_hints == XCB_NONE) {
    errno = EBADR;
    return false;
  }

  /* Set decorations to 0 to remove them */

  MotifWmHints hints = {0x3, 0x3e, 0x0, 0x0, 0x0};
  xcb_change_property(conn, XCB_PROP_MODE_REPLACE, window, motif_wm_hints,
                      motif_wm_hints, 32, sizeof(hints) / 4, &hints);

  return true;
}

/*
 * Get primary screen space aviable for the notification to draw on
 */

static bool
get_primary_monitor_geometry(xcb_connection_t *conn, xcb_window_t root,
			     uint16_t *width, uint16_t *height,
			     uint16_t *x, uint16_t *y)
{
  xcb_randr_get_screen_resources_current_cookie_t res_cookie =
      xcb_randr_get_screen_resources_current(conn, root);
  xcb_randr_get_screen_resources_current_reply_t *res_reply =
      xcb_randr_get_screen_resources_current_reply(conn, res_cookie, NULL);

  if (!res_reply)
    return false;

  xcb_randr_get_output_primary_cookie_t primary_cookie =
      xcb_randr_get_output_primary(conn, root);
  xcb_randr_get_output_primary_reply_t *primary_reply =
      xcb_randr_get_output_primary_reply(conn, primary_cookie, NULL);

  if (!primary_reply) {
    free(res_reply);
    return false;
  }

  xcb_randr_output_t primary_output = primary_reply->output;
  free(primary_reply);

  xcb_randr_get_output_info_cookie_t output_info_cookie =
      xcb_randr_get_output_info(conn, primary_output, XCB_CURRENT_TIME);
  xcb_randr_get_output_info_reply_t *output_info =
      xcb_randr_get_output_info_reply(conn, output_info_cookie, NULL);

  if (!output_info || output_info->crtc == XCB_NONE) {
    free(res_reply);
    free(output_info);
    return false;
  }

  xcb_randr_get_crtc_info_cookie_t crtc_info_cookie =
      xcb_randr_get_crtc_info(conn, output_info->crtc, XCB_CURRENT_TIME);
  xcb_randr_get_crtc_info_reply_t *crtc_info =
      xcb_randr_get_crtc_info_reply(conn, crtc_info_cookie, NULL);

  if (!crtc_info) {
    free(res_reply);
    free(output_info);
    return false;
  }

  *width = crtc_info->width;
  *height = crtc_info->height;
  *x = crtc_info->x;
  *y = crtc_info->y;

  free(res_reply);
  free(output_info);
  free(crtc_info);

  return true;
}
