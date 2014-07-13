#include <pebble.h>
#include <pebble_fonts.h>

#include "timeutils.h"

static Window *window;
static BitmapLayer *logo;
static GBitmap *logo_bitmap = NULL;
static TextLayer* countdown;

enum {
    KEY_UTC_OFFSET  = 0,
    KEY_NEXT_LAUNCH = 1,
    KEY_FLASH       = 2
};

static time_t utc_offset     = -1;
static time_t next_launch_ts = -1;

static int lock = 0;

void flash_text(const char* text, int duration) {
    text_layer_set_background_color(countdown, GColorWhite);
    text_layer_set_text_color(countdown, GColorBlack);
    text_layer_set_text(countdown, text);
    vibes_long_pulse();

    lock += duration;
}

void in_received_handler(DictionaryIterator* received, void* ctx) {
    Tuple* data = dict_read_first(received);

    while(data) {
        switch(data->key) {
            case KEY_UTC_OFFSET:
                utc_offset = (time_t) data->value->int32;
                persist_write_int(KEY_UTC_OFFSET, utc_offset);
                break;

            case KEY_NEXT_LAUNCH:
                next_launch_ts = (time_t) data->value->int32;
                persist_write_int(KEY_NEXT_LAUNCH, next_launch_ts);
                break;

            case KEY_FLASH: 
                flash_text(data->value->cstring, 5);
                break;
        }

        data = dict_read_next(received);
    }
}

void update_countdown(struct tm* tick_time, TimeUnits change) {
    if(utc_offset == -1 || next_launch_ts == -1) return;
    if(lock > 0) {
        lock--;
        return;
    }

    struct tm diff;
    time_t unix_ts = calc_unix_seconds(tick_time) + utc_offset;
    static char buffer[] = "99:99:99";

    calc_diff(&diff, unix_ts, next_launch_ts);

    if(diff.tm_hour == 0 && diff.tm_min == 0 && diff.tm_sec == 0) {
        flash_text("LIFTOFF", 0);
    } else {
        snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d", diff.tm_hour, diff.tm_min, diff.tm_sec);

        text_layer_set_background_color(countdown, GColorBlack);
        text_layer_set_text_color(countdown, GColorWhite);
        text_layer_set_text(countdown, buffer);
    }

    APP_LOG(APP_LOG_LEVEL_DEBUG, buffer);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);

  logo = bitmap_layer_create(GRect(0, 0, 144, 144));
  layer_add_child(window_layer, bitmap_layer_get_layer(logo));

  logo_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SPACEX);
  bitmap_layer_set_bitmap(logo, logo_bitmap);

  countdown = text_layer_create(GRect(0, 130, 144, 38));
  text_layer_set_background_color(countdown, GColorBlack);
  text_layer_set_text_color(countdown, GColorWhite);
  text_layer_set_text_alignment(countdown, GTextAlignmentCenter);
  text_layer_set_font(countdown, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_MONO_36)));

  if(persist_exists(KEY_UTC_OFFSET)) {
      utc_offset = (time_t) persist_read_int(KEY_UTC_OFFSET);
  }
  if(persist_exists(KEY_NEXT_LAUNCH)) {
      next_launch_ts = (time_t) persist_read_int(KEY_NEXT_LAUNCH); 
  }

  text_layer_set_text(countdown, "NO TELE");
  layer_add_child(window_layer, text_layer_get_layer(countdown));
}

static void window_unload(Window *window) {
    bitmap_layer_destroy(logo);
    text_layer_destroy(countdown);
    tick_timer_service_unsubscribe();
}

static void init(void) {
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });

  window_set_fullscreen(window, true);
  window_stack_push(window, true);
  window_set_background_color(window, GColorBlack);

  tick_timer_service_subscribe(SECOND_UNIT, update_countdown);

  app_message_register_inbox_received(in_received_handler);
  app_message_open(APP_MESSAGE_INBOX_SIZE_MINIMUM, 0);
}

static void deinit(void) {
  window_destroy(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
