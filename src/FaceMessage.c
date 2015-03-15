#include <pebble.h>

static Window *window;
static TextLayer *text_layer;
static char *message;
static Layer *window_layer;
static GBitmap *s_image;
static Layer *s_image_layer;


static void  layer_update_callback(Layer *layer, GContext* ctx) {
  // We make sure the dimensions of the GRect to draw into
  // are equal to the size of the bitmap--otherwise the image
  // will automatically tile. Which might be what *you* want.

#ifdef PBL_PLATFORM_BASALT
  GSize image_size = gbitmap_get_bounds(s_image).size;
#else
  GSize image_size = s_image->bounds.size;
#endif

  graphics_draw_bitmap_in_rect(ctx, s_image, GRect(0, 0, image_size.w, image_size.h));
}

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


  // Check our sentiment, then add a corresponding image layer with the
  //  the appropriate image. Before adding additional layers, remember to remove
  // all children layer from the window layer.
  if (strcmp(sentiment, "sad")) {
    layer_remove_child_layers(window_layer);
    layer_set_update_proc(s_image_layer, layer_update_callback);
    layer_add_child(window_layer, s_image_layer);
    s_image = gbitmap_create_with_resource(RESOURCE_ID_FROWN);
  } else if (strcmp(sentiment, "happy")) {
    layer_remove_child_layers(window_layer);
    layer_set_update_proc(s_image_layer, layer_update_callback);
    layer_add_child(window_layer, s_image_layer);
    s_image = gbitmap_create_with_resource(RESOURCE_ID_SMILEY);
  } else  {
    layer_remove_child_layers(window_layer);
    layer_set_update_proc(s_image_layer, layer_update_callback);
    layer_add_child(window_layer, s_image_layer);
    s_image = gbitmap_create_with_resource(RESOURCE_ID_NEUTRAL);
  }
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

  // Remove the any children layer of the window, then add a fresh text layer to
  // show our message
  layer_remove_child_layers(window_layer);
  text_layer_set_text(text_layer, message);
  layer_add_child(window_layer, text_layer_get_layer(text_layer));
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
  window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  s_image_layer = layer_create(bounds);
  text_layer = text_layer_create((GRect) { .origin = { 0, 72 }, .size = { bounds.size.w, 20 } });

  text_layer_set_text(text_layer, "Welcome");
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
