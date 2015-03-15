#include <pebble.h>

static Window *window;
static TextLayer *text_layer;
static char *message;
static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Received");
  // Get the first pair
  Tuple *t = dict_read_first(iterator);

  char* sentiment = "";
  // Process all pairs present
  while(t != NULL) {
    // Process this pair's key
    switch (t->key) {
      case 1:
        APP_LOG(APP_LOG_LEVEL_INFO, "key %d received with value %s", (int)t->key, (char*)t->value);
        message = (char*)t->value;
      case 2:
        APP_LOG(APP_LOG_LEVEL_INFO, "key %d received with value %s", (int)t->key, (char*)t->value);
        sentiment = (char*)t->value;
    }

    // Get next pair, if any
    t = dict_read_next(iterator);
  }
  text_layer_set_text(text_layer, sentiment);
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
  text_layer_set_text(text_layer, "Dropped");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
  text_layer_set_text(text_layer, "Failed");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
    APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
  text_layer_set_text(text_layer, "Sent");
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(text_layer, "Select");
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "up");
  text_layer_set_text(text_layer, message);
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "down");
  text_layer_set_text(text_layer, "Down");
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  text_layer = text_layer_create((GRect) { .origin = { 0, 72 }, .size = { bounds.size.w, 20 } });
  text_layer_set_text(text_layer, "bullshit");
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text_layer));
}

static void window_unload(Window *window) {
  text_layer_destroy(text_layer);
}

static void init(void) {
  window = window_create();
  window_set_click_config_provider(window, click_config_provider);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
  window_stack_push(window, animated);
  APP_LOG(APP_LOG_LEVEL_INFO, "setting up callbacks");
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());

}

static void deinit(void) {
  window_destroy(window);
}

int main(void) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "starting");
  APP_LOG(APP_LOG_LEVEL_INFO, "starting");
  init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

  app_event_loop();
  deinit();
}
