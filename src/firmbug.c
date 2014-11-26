#include <pebble.h>

//  constants

const int kToastShow = 500;
const int kToastHide = 3500;
const int kToastMargin = 10;
const char* kToastText = "why is this text uncentered?";

//  variables

static Window* window;
static PropertyAnimation* animation_toast = NULL;
static TextLayer* text_main, * text_toast = NULL;
static GSize size_toast;

//  toast functions

void toast_hidden(struct Animation* animation, bool finished, void* context) {
  animation_toast = NULL;
}

void toast_shown(struct Animation* animation, bool finished, void* context) {
  animation_toast = property_animation_create_layer_frame(text_layer_get_layer(text_toast), NULL, &GRect((144 - size_toast.w) / 2, -size_toast.h, size_toast.w, size_toast.h));
  animation_set_duration(&animation_toast->animation, kToastHide);
  animation_set_curve(&animation_toast->animation, AnimationCurveEaseIn);
  animation_set_handlers(&animation_toast->animation, (AnimationHandlers) {
    .stopped = toast_hidden
  }, NULL);
  animation_schedule(&animation_toast->animation);
}

void toast_unload() {
  if (animation_toast) {
    if (animation_is_scheduled(&animation_toast->animation))
      animation_unschedule(&animation_toast->animation);
    property_animation_destroy(animation_toast);
    animation_toast = NULL;
  }
}

//  button select

void button_select(ClickRecognizerRef recognizer, void* context) {
  //  stop any partially-started animation
  if (animation_toast) {
    if (animation_is_scheduled(&animation_toast->animation))
      animation_unschedule(&animation_toast->animation);
    property_animation_destroy(animation_toast);
  }
  //  relocate toast
  layer_set_frame(text_layer_get_layer(text_toast), GRect(0, 0, 144, 168));
  //  set toast text; get size
  text_layer_set_text(text_toast, kToastText);
  size_toast = text_layer_get_content_size(text_toast);
  size_toast.w += 2 * kToastMargin;
  if (size_toast.w > 144)
    size_toast.w = 144;
  size_toast.h += kToastMargin;
  //  setup status display
  layer_set_frame(text_layer_get_layer(text_toast), GRect((144 - size_toast.w) / 2, -size_toast.h, size_toast.w, size_toast.h));
  //text_layer_set_size(text_toast, size_toast);    //  makes no difference
  //  animate
  toast_unload();
  animation_toast = property_animation_create_layer_frame(text_layer_get_layer(text_toast), NULL, &GRect((144 - size_toast.w) / 2, 0, size_toast.w, size_toast.h));
  animation_set_duration(&animation_toast->animation, kToastShow);
  animation_set_curve(&animation_toast->animation, AnimationCurveEaseOut);
  animation_set_handlers(&animation_toast->animation, (AnimationHandlers) {
    .stopped = toast_shown
  }, NULL);
  animation_schedule(&animation_toast->animation);
}

void config_provider(void* context) {
  //  set select handlers
  window_single_click_subscribe(BUTTON_ID_SELECT, button_select);
}

//  window functions

static void window_load(Window* window) {
  //  get root layer
  Layer* layer_root = window_get_root_layer(window);
  //  main
  text_main = text_layer_create(GRect(0, 100, 144, 36));
  text_layer_set_text_color(text_main, GColorBlack);
  text_layer_set_background_color(text_main, GColorClear);
  text_layer_set_font(text_main, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(text_main, GTextAlignmentCenter);
  text_layer_set_text(text_main, "Press SELECT Button");
  layer_add_child(layer_root, text_layer_get_layer(text_main));
  //  toast
  text_toast = text_layer_create(GRect(0, -36, 144, 36));
  text_layer_set_text_color(text_toast, GColorWhite);
  text_layer_set_background_color(text_toast, GColorBlack);
  text_layer_set_font(text_toast, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(text_toast, GTextAlignmentCenter);    //  or GTextAlignmentRight
  text_layer_set_overflow_mode(text_toast, GTextOverflowModeWordWrap);
  layer_add_child(layer_root, text_layer_get_layer(text_toast));
}

static void window_unload(Window* window) {
  //  unload toast animation
  toast_unload();
  //  free layers
  text_layer_destroy(text_main);
  text_layer_destroy(text_toast);
}

//  main function

int main(void) {
  //  create window
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_set_background_color(window, GColorWhite);
  window_set_fullscreen(window, true);
  window_set_click_config_provider(window, config_provider);
  window_stack_push(window, true);
  //  main event loop
  app_event_loop();
  //  destroy window
  window_destroy(window);
}
