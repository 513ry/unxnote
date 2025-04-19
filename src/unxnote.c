#include "unxnote.h"
#include "unxnote/window.h"
#include "unxnote/font.h"

#include <errno.h>
#include <malloc.h>
#include <stdarg.h>
#include <string.h>
#include <poll.h>

#include "unxnote/hash.h"
#include "unxnote/hash_config.h"

#define TIMEOUT_MS 20

/**
 * Window data
 */

typedef struct {
  const char *cookie_name;
  char *title;
  char *msg;
  xcb_window_t window;
} UNXNoteWindow;

/**
 * Window manager
 */

typedef struct {
  size_t count;
  UNXNoteWindow *windows;
  PT_Hash *contexts;
  PT_Hash *cookies;
  FT_Face font;
} UNXNoteManager;

static UNXNoteManager unxnote_manager = {
  .count = 0,
  .windows = NULL,
  .contexts = NULL,
  .cookies = NULL,
  .font = NULL,
};

#define BUTTON_WIDTH UINT32_C(20)
#define BUTTON_HEIGHT UINT32_C(20)
#define BUTTON_X_POS WINDOW_WIDTH - BUTTON_WIDTH
#define BUTTON_Y_POS UINT32_C(0)
#define UNXNOTE_FONT "../ttf/VictorMonoNerdFont-Medium.ttf"

static void handle_event(xcb_generic_event_t *event);
static void draw_context(xcb_window_t window_id);
static void unxnote_free_window(UNXNoteWindow window);
static void unxnote_close_window(xcb_window_t window_id);
static const int find_window_index(xcb_window_t window_handler);
static UNXNoteWindow *find_window(xcb_window_t window_handler);

// --- API Definitions

void
unxnote_init(unsigned int n, ...)
{
  if (!init_window())
    unxnote_bug("init_window", errno);

  init_font();
  unxnote_manager.font = new_font(UNXNOTE_FONT);

  /* Create empty context hash */

  unxnote_manager.contexts = pt_hash_new(0);

  /* Create cookie hash */

  if (n == 0) {
    HASH_UNION_TYPE value;
    value.cookie = (UNXNoteCookie){
      .glyph = 0xeb54, .title = "UNXNote", .bg_color = 0x222233, .fg_color = 0x222233
    };
    unxnote_manager.cookies = pt_hash_new(0);
    pt_hash_set(unxnote_manager.cookies, "default", value);
  } else {
    unxnote_manager.cookies = pt_hash_new(0);
    va_list ap;
    va_start(ap, n);
    for (unsigned int i = 0; i < n; ++i) {
      char *key = va_arg(ap, char *);
      HASH_UNION_TYPE value;
      value.cookie = va_arg(ap, UNXNoteCookie);
      pt_hash_set(unxnote_manager.cookies, key, value);
    }
    va_end(ap);
  }
  unxnote_manager.windows = malloc(0);
}

void
unxnote_free(void)
{
  for (size_t i = 0; i < unxnote_manager.count; i++)
    unxnote_free_window(unxnote_manager.windows[i]);
  free(unxnote_manager.windows);
  unxnote_manager.count = 0;
  free_font(1, unxnote_manager.font);
  free_window();
  pt_hash_free(unxnote_manager.cookies);
  pt_hash_free(unxnote_manager.contexts);
}

/*
 * Register a UNXNote notification window
 */

void
unxnote_open_window(const char *cookie_name, char *from, char *msg)
{

  /* Get the window cookie */

  UNXNoteCookie cookie;
  if (!({HASH_UNION_TYPE tmp;
         bool f = pt_hash_get(unxnote_manager.cookies, cookie_name, &tmp);
         cookie = tmp.cookie;
	 f;
       }))
    unxnote_bug("cookie could not be retrived", EXIT_FAILURE);

  /* Reallocate windows array with one more index */

  UNXNoteWindow *new_array = realloc(unxnote_manager.windows,
				     (unxnote_manager.count + 1) *
				     sizeof(UNXNoteWindow));
  if (!new_array)
    unxnote_bug("realloc", EXIT_FAILURE);

  UNXNoteWindow *new_win = &new_array[unxnote_manager.count];

  /* Create header string */

  const char *cookie_glyph = utf8_encode(cookie.glyph);
  const uint16_t header_size =
    UINT16_C(6) + UINT16_C(strlen(cookie.title)) +
    UINT16_C(strlen(from)) + UINT16_C(strlen(cookie_glyph));
  char header[header_size];
  snprintf(header, header_size, "%s  %s - %s",
	   cookie_glyph, cookie.title, from);
  new_win->cookie_name = cookie_name;
  new_win->title = strdup(header);
  new_win->msg = strdup(msg);
  new_win->window = new_window(unxnote_manager.count);

  unxnote_log("new title: %s\n", new_win->title);
  unxnote_log("new msg: %s\n", new_win->msg);

  /* Define new graphical contexts if none is matching */

  HASH_UNION_TYPE value;

  if (!pt_hash_get(unxnote_manager.contexts, cookie_name, &value)) {
    value.contexts[0] = xcb_generate_id(x_conn.conn);
    const uint32_t bg_values[] = {cookie.bg_color};
    xcb_create_gc(x_conn.conn, value.contexts[0], new_win->window,
		  XCB_GC_FOREGROUND, bg_values);
    value.contexts[1] = xcb_generate_id(x_conn.conn);
    const uint32_t text_values[] = {cookie.fg_color};
    xcb_create_gc(x_conn.conn, value.contexts[1], new_win->window,
		  XCB_GC_FOREGROUND, text_values);
    pt_hash_set(unxnote_manager.contexts, cookie_name, value);
  }

  unxnote_manager.windows = new_array;
  ++unxnote_manager.count;
}

/*
 * Display UNXNote window manager
 */

void
unxnote_update()
{
  bool update = false;
  struct pollfd pfd = {
    .fd = xcb_get_file_descriptor(x_conn.conn),
    .events = POLLIN
  };

  while (true) {
    xcb_generic_event_t *event;
    while ((event = xcb_poll_for_event(x_conn.conn)) != NULL) {
      update = true;
      handle_event(event);
    }
    int ret = poll(&pfd, 1, TIMEOUT_MS);
    if (ret < 0) {
      unxnote_log("pool failed\n");
      break;
    } else if (ret == 0) {
      break;
    } else {
      // POLLIN
      unxnote_log("polling\n");
    }
  };

  if (update)
    xcb_flush(x_conn.conn);
}

// -- Static Definitions

static void
handle_event(xcb_generic_event_t *event)
{
  switch (event->response_type & ~0x80) {
  case XCB_EXPOSE:
    draw_context(((xcb_expose_event_t *)event)->window);
    break;
  case XCB_BUTTON_PRESS:
    unxnote_close_window(((xcb_button_press_event_t *)event)->event);
  }
  free(event);
}

static void
 draw_context(xcb_window_t window_id)
{
  UNXNoteWindow *current_window = find_window(window_id);
  if (current_window == NULL)
    unxnote_bug("find_window", EXIT_FAILURE);

  /* Get context data */

  HASH_UNION_TYPE value;
  pt_hash_get(unxnote_manager.contexts, current_window->cookie_name, &value);

  /* Clear window background */

  xcb_rectangle_t rect = {0, 0, WINDOW_WIDTH, WINDOW_MIN_HEIGHT};
  xcb_poly_fill_rectangle(x_conn.conn, current_window->window,
			  value.contexts[0], 1, &rect);

  /* Render title text */

  draw_text(x_conn.conn, current_window->window, value.contexts[1],
	    unxnote_manager.font, current_window->title, 3, FONT_SIZE);
}

static void
unxnote_free_window(UNXNoteWindow window)
{
  free(window.msg);
  window.msg = NULL;
  free(window.title);
  window.title = NULL;
  xcb_destroy_window(x_conn.conn, window.window);
}

static void
unxnote_close_window(xcb_window_t window_id)
{
  const size_t index =
    find_window_index(window_id);
  if (index == -1) {
    unxnote_log("Bug: find_window_index\n");
    return;
  }
  unxnote_free_window(unxnote_manager.windows[index]);
  if (unxnote_manager.count == 1) {
    unxnote_manager.windows = NULL;
    unxnote_manager.count = 0;
    return;
  }

  /* Shift elements left */

  for (size_t i = index; i < unxnote_manager.count - 1; ++i) {
    unxnote_manager.windows[i] = unxnote_manager.windows[i + 1];
    xcb_destroy_window(x_conn.conn, unxnote_manager.windows[i].window);
    unxnote_manager.windows[i].window = new_window(i);
  }

  /* Shrink the array */

  UNXNoteWindow *new_array = realloc(
    unxnote_manager.windows,
    (unxnote_manager.count - 1) * sizeof(UNXNoteWindow)
  );
  if (!new_array) unxnote_bug("unregister realloc", ENOMEM);
  unxnote_manager.windows = new_array;
  --unxnote_manager.count;
}

static const int
find_window_index(xcb_window_t window_handler)
{
  for (int i = unxnote_manager.count - 1; i >= 0; --i)
    if (unxnote_manager.windows[(size_t)i].window == window_handler)
      return i;
  return -1;
}

static UNXNoteWindow *
find_window(xcb_window_t window_handler)
{
  for (int i = unxnote_manager.count - 1; i >= 0; --i)
    if (unxnote_manager.windows[(size_t)i].window == window_handler)
      return &unxnote_manager.windows[(size_t)i];
  return NULL;
}
