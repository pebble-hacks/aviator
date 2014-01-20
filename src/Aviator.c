#include <pebble.h>

static int mSeconds;        //Show seconds on clock (0/1)
static int mInvert;         //Invert colours (0/1)
static int mBluetoothVibe;  //Vibrate on bluetooth disconnect (0/1)
static int mVibeMinutes;    //Vibrate every X minutes
static int mHands;          //Show clock hands (0/1)
static int mStyle;          //Date International (0), Date US (1), Local/Zulu (2)
static int mBackground;     //Background Image: Full (0), Simple (1), Minimal (2), None (3)

static int mTimezoneOffset = 0;
static int mVibeMinutesTimer = 0;

static bool mAppStarted = false;
static bool mTimeLayerShifted = false;

static int mAngleMinute = 0;

enum {
  SECONDS_KEY = 0x0,
  INVERT_KEY = 0x1,
  BLUETOOTHVIBE_KEY = 0x2,
  VIBEMINUTES_KEY = 0x3,
  HANDS_KEY = 0x4,
  STYLE_KEY = 0x5,
  BACKGROUND_KEY = 0x6,
  TIMEZONE_OFFSET_KEY = 0x7
};

static AppSync sync;
static uint8_t sync_buffer[128];

static uint8_t batteryPercent;

static Window *window;

static GFont *tiny_font;

static TextLayer *tiny_top_text;
static TextLayer *tiny_bottom_text;
static TextLayer *tiny_alarm_text;

static GBitmap *background_image;
static BitmapLayer *background_layer;

static InverterLayer *inverter_layer;

static GBitmap *battery_image;
static BitmapLayer *battery_image_layer;

const int BATTERY_IMAGE_RESOURCE_IDS[] = {
  RESOURCE_ID_IMAGE_BATTERY_100,
  RESOURCE_ID_IMAGE_BATTERY_75,
  RESOURCE_ID_IMAGE_BATTERY_50,
  RESOURCE_ID_IMAGE_BATTERY_25,
  RESOURCE_ID_IMAGE_BATTERY_0,
  RESOURCE_ID_IMAGE_BATTERY_CHARGE
};

static Layer *time_layer;

#define TOTAL_TIME_DIGITS 8 //00:00:00
static GBitmap *time_digits_images[TOTAL_TIME_DIGITS];
static BitmapLayer *time_digits_layers[TOTAL_TIME_DIGITS];

static Layer *zulu_time_layer;
static GBitmap *zulu_time_digits_images[TOTAL_TIME_DIGITS];
static BitmapLayer *zulu_time_digits_layers[TOTAL_TIME_DIGITS];

static Layer *date_layer;

#define TOTAL_DATE_DIGITS 10 //00-00-0000
static GBitmap *date_digits_images[TOTAL_DATE_DIGITS];
static BitmapLayer *date_digits_layers[TOTAL_DATE_DIGITS];

const int TINY_IMAGE_RESOURCE_IDS[] = {
  RESOURCE_ID_IMAGE_TINY_0,
  RESOURCE_ID_IMAGE_TINY_1,
  RESOURCE_ID_IMAGE_TINY_2,
  RESOURCE_ID_IMAGE_TINY_3,
  RESOURCE_ID_IMAGE_TINY_4,
  RESOURCE_ID_IMAGE_TINY_5,
  RESOURCE_ID_IMAGE_TINY_6,
  RESOURCE_ID_IMAGE_TINY_7,
  RESOURCE_ID_IMAGE_TINY_8,
  RESOURCE_ID_IMAGE_TINY_9,
  RESOURCE_ID_IMAGE_TINY_COLON,
  RESOURCE_ID_IMAGE_TINY_DIVIDE
};

const int MED_IMAGE_RESOURCE_IDS[] = {
  RESOURCE_ID_IMAGE_MED_0,
  RESOURCE_ID_IMAGE_MED_1,
  RESOURCE_ID_IMAGE_MED_2,
  RESOURCE_ID_IMAGE_MED_3,
  RESOURCE_ID_IMAGE_MED_4,
  RESOURCE_ID_IMAGE_MED_5,
  RESOURCE_ID_IMAGE_MED_6,
  RESOURCE_ID_IMAGE_MED_7,
  RESOURCE_ID_IMAGE_MED_8,
  RESOURCE_ID_IMAGE_MED_9,
  RESOURCE_ID_IMAGE_MED_COLON,
  RESOURCE_ID_IMAGE_MED_DIVIDE
};


bool isSpace(char c) {
    switch (c) {
        case ' ':
        case '\n':
        case '\t':
        case '\f':
        case '\r':
            return true;
            break;
        default:
            return false;
            break;
    }
}

char* trim(char* input) {
    char* start = input;
    while (isSpace(*start)) { //trim left
        start++;
    }

    char* ptr = start;
    char* end = start;
    while (*ptr++ != '\0') { //trim right
        if (!isSpace(*ptr)) { //only move end pointer if char isn't a space
            end = ptr;
        }
    }

    *end = '\0'; //terminate the trimmed string with a null
    return start;
}


static void remove_invert() {
  if(inverter_layer!=NULL) {
    layer_remove_from_parent(inverter_layer_get_layer(inverter_layer));
    inverter_layer_destroy(inverter_layer);
    inverter_layer = NULL;
  }
}
static void set_invert() {
  if(!inverter_layer) {
    inverter_layer = inverter_layer_create(GRect(0, 0, 144, 168));  
    layer_add_child(window_get_root_layer(window), inverter_layer_get_layer(inverter_layer));
    layer_mark_dirty(inverter_layer_get_layer(inverter_layer));
  }
}
void change_background() {
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "change_background()");
  if(background_image) {
    gbitmap_destroy(background_image);
    background_image = NULL;
  }
  switch(mBackground) {
    case 0:
      background_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);
      break;
    case 1:
      background_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND_SIMPLE);
      break;
    case 2:
      background_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND_MINIMAL);
      break;
    case 3:
      background_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND_NONE);
      break;
  }
  if(background_image != NULL) {
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "change_background() not null");
    bitmap_layer_set_bitmap(background_layer, background_image);
    layer_set_hidden(bitmap_layer_get_layer(background_layer), false);
    layer_mark_dirty(bitmap_layer_get_layer(background_layer));
  }

  
}
static void toggleSeconds(bool hidden) {
	
  layer_set_hidden(bitmap_layer_get_layer(time_digits_layers[5]), hidden);
  layer_set_hidden(bitmap_layer_get_layer(time_digits_layers[6]), hidden);
  layer_set_hidden(bitmap_layer_get_layer(time_digits_layers[7]), hidden);
  
  layer_set_hidden(bitmap_layer_get_layer(zulu_time_digits_layers[5]), hidden);
  layer_set_hidden(bitmap_layer_get_layer(zulu_time_digits_layers[6]), hidden);
  layer_set_hidden(bitmap_layer_get_layer(zulu_time_digits_layers[7]), hidden);

  if(hidden) {
    //seconds aren't visible
    if(mTimeLayerShifted) {
      //12hr clock, single digit
      layer_set_frame(time_layer, GRect(9, 0, 144, 168)); 
      layer_set_frame(zulu_time_layer, GRect(9, 0, 144, 168));
    }
    else {
      layer_set_frame(time_layer, GRect(16, 0, 144, 168)); 
      layer_set_frame(zulu_time_layer, GRect(16, 0, 144, 168));
    }
  }
  else {
    //seconds are visible
    if(mTimeLayerShifted) {
      //12hr clock, single digit
      layer_set_frame(time_layer, GRect(-7, 0, 144, 168));
      layer_set_frame(zulu_time_layer, GRect(-7, 0, 144, 168));
    }
    else {
      layer_set_frame(time_layer, GRect(0, 0, 144, 168));
      layer_set_frame(zulu_time_layer, GRect(0, 0, 144, 168));
    }
  }
}

static void handle_tick(struct tm *tick_time, TimeUnits units_changed);
static void update_zulu_hours(struct tm *tick_time);
static void update_zulu_minutes(struct tm *tick_time);
static void update_zulu_seconds(struct tm *tick_time);

static void sync_tuple_changed_callback(const uint32_t key, const Tuple* new_tuple, const Tuple* old_tuple, void* context) {
  
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "TUPLE! %lu : %d", key, new_tuple->value->uint8);
  
  switch (key) {
    case SECONDS_KEY:
      mSeconds = new_tuple->value->uint8;
      tick_timer_service_unsubscribe();
      if(mSeconds) {
        toggleSeconds(false);
        tick_timer_service_subscribe(SECOND_UNIT, handle_tick);
      }
      else {
        toggleSeconds(true);
        tick_timer_service_subscribe(MINUTE_UNIT, handle_tick);
      }
      break;
    case INVERT_KEY:
      remove_invert();
      mInvert = new_tuple->value->uint8;
      if(mInvert) {
        set_invert();
      }
      break;
    case BLUETOOTHVIBE_KEY:
      mBluetoothVibe = new_tuple->value->uint8;
      break;      
    case VIBEMINUTES_KEY:
      mVibeMinutes = new_tuple->value->int32;
      mVibeMinutesTimer = mVibeMinutes;
      if(mVibeMinutes>0) {
        static char label_text[20] = "";  
        snprintf(label_text, 
                 sizeof(label_text), 
                 "CR.AL:%d", mVibeMinutes);
        text_layer_set_text(tiny_alarm_text, trim(label_text));
        layer_set_hidden(text_layer_get_layer(tiny_alarm_text), false);
      }
      else {
        layer_set_hidden(text_layer_get_layer(tiny_alarm_text), true);
      }
      break;
    case HANDS_KEY:
      mHands = new_tuple->value->uint8;
      ///////////////
      break;
    case STYLE_KEY:
      mStyle = new_tuple->value->uint8;
      time_t now = time(NULL);
      struct tm *tick_time = localtime(&now); 
      if(mStyle<2) {
        layer_set_hidden(zulu_time_layer, true);
        layer_set_hidden(text_layer_get_layer(tiny_bottom_text), true);
        layer_set_hidden(date_layer, false); 
        handle_tick(tick_time, DAY_UNIT);
      }
      else {
        layer_set_hidden(zulu_time_layer, false);
        layer_set_hidden(text_layer_get_layer(tiny_bottom_text), false);
        layer_set_hidden(date_layer, true);
        if(mSeconds) {
          handle_tick(tick_time, HOUR_UNIT+MINUTE_UNIT+SECOND_UNIT);
        } 
        else {
          handle_tick(tick_time, HOUR_UNIT+MINUTE_UNIT);
        }
      } 
      break;
    case BACKGROUND_KEY:
      mBackground = new_tuple->value->uint8;
      change_background();
      break;
    case TIMEZONE_OFFSET_KEY:
      mTimezoneOffset = new_tuple->value->int32;

      time_t tnow = time(NULL);
      struct tm *ttick_time = localtime(&tnow);  
      handle_tick(ttick_time, SECOND_UNIT+MINUTE_UNIT+HOUR_UNIT+DAY_UNIT);
	  
      break;
  }
}

void bluetooth_connection_callback(bool connected) {
  if(mAppStarted && !connected && mBluetoothVibe) {
    //vibe!
    vibes_long_pulse();
  }
}

static void set_container_image(GBitmap **bmp_image, BitmapLayer *bmp_layer, const int resource_id, GPoint origin) {
  GBitmap *old_image = *bmp_image;
  *bmp_image = gbitmap_create_with_resource(resource_id);
  GRect frame = (GRect) {
    .origin = origin,
    .size = (*bmp_image)->bounds.size
  };
  bitmap_layer_set_bitmap(bmp_layer, *bmp_image);
  layer_set_frame(bitmap_layer_get_layer(bmp_layer), frame);
  if (old_image != NULL) {
    gbitmap_destroy(old_image);
    old_image = NULL;
  }
}

static void update_battery(BatteryChargeState charge_state) {
  
  batteryPercent = charge_state.charge_percent;
  
  if(charge_state.is_charging && batteryPercent<100) {
    set_container_image(&battery_image, battery_image_layer, BATTERY_IMAGE_RESOURCE_IDS[5], GPoint(69, 84));
    return;
  }
  
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "Battery Level: %d", batteryPercent);

  if(batteryPercent<30) {
    set_container_image(&battery_image, battery_image_layer, BATTERY_IMAGE_RESOURCE_IDS[3], GPoint(69, 84));
  }
  else if(batteryPercent<60) {
    set_container_image(&battery_image, battery_image_layer, BATTERY_IMAGE_RESOURCE_IDS[2], GPoint(69, 84));
  }
  else if(batteryPercent<90) {
    set_container_image(&battery_image, battery_image_layer, BATTERY_IMAGE_RESOURCE_IDS[1], GPoint(69, 84));
  }
  else {
    set_container_image(&battery_image, battery_image_layer, BATTERY_IMAGE_RESOURCE_IDS[0], GPoint(69, 84));
  }

}


unsigned short get_display_hour(unsigned short hour) {
  if (clock_is_24h_style()) {
    return hour;
  }
  unsigned short display_hour = hour % 12;
  // Converts "0" to "12"
  return display_hour ? display_hour : 12;
}

static void update_days(struct tm *tick_time) {
  int month = tick_time->tm_mon + 1;
  int year = tick_time->tm_year + 1900;

  //APP_LOG(APP_LOG_LEVEL_DEBUG, "year: %d", year%10);
  
  int first_digit = tick_time->tm_mday/10;
  int second_digit = tick_time->tm_mday%10;
  int third_digit = month/10;
  int fourth_digit = month%10;
  
  if(mStyle == 1) {
    //US date
    first_digit = month/10;
    second_digit = month%10;
    third_digit = tick_time->tm_mday/10;
    fourth_digit = tick_time->tm_mday%10;
  }
  
  
  set_container_image(&date_digits_images[0], date_digits_layers[0], TINY_IMAGE_RESOURCE_IDS[first_digit], GPoint(27, 94));
  set_container_image(&date_digits_images[1], date_digits_layers[1], TINY_IMAGE_RESOURCE_IDS[second_digit], GPoint(37, 94));
  //-2
  set_container_image(&date_digits_images[3], date_digits_layers[3], TINY_IMAGE_RESOURCE_IDS[third_digit], GPoint(53, 94));
  set_container_image(&date_digits_images[4], date_digits_layers[4], TINY_IMAGE_RESOURCE_IDS[fourth_digit], GPoint(63, 94));
  //-5
  set_container_image(&date_digits_images[6], date_digits_layers[6], TINY_IMAGE_RESOURCE_IDS[year/1000%10], GPoint(79, 94));
  set_container_image(&date_digits_images[7], date_digits_layers[7], TINY_IMAGE_RESOURCE_IDS[year/100%10], GPoint(89, 94));
  set_container_image(&date_digits_images[8], date_digits_layers[8], TINY_IMAGE_RESOURCE_IDS[year/10%10], GPoint(99, 94));
  set_container_image(&date_digits_images[9], date_digits_layers[9], TINY_IMAGE_RESOURCE_IDS[year%10], GPoint(109, 94));
 
  
  
}

static void update_hours(struct tm *tick_time) {

  static char top_text[20] = "";
  
  unsigned short display_hour = get_display_hour(tick_time->tm_hour);

  set_container_image(&time_digits_images[0], time_digits_layers[0], MED_IMAGE_RESOURCE_IDS[display_hour/10], GPoint(26, 62));
  set_container_image(&time_digits_images[1], time_digits_layers[1], MED_IMAGE_RESOURCE_IDS[display_hour%10], GPoint(40, 62));

  if (!clock_is_24h_style()) { 
    if (display_hour/10 == 0) {
      layer_set_frame(time_layer, GRect(-6, 0, 144, 168));
      mTimeLayerShifted = true;
      layer_set_hidden(bitmap_layer_get_layer(time_digits_layers[0]), true);
    }
    else {
      layer_set_frame(time_layer, GRect(0, 0, 144, 168));
      mTimeLayerShifted = false;
      layer_set_hidden(bitmap_layer_get_layer(time_digits_layers[0]), false);
    }
    
    if(tick_time->tm_hour<12) {
      snprintf(top_text, 
        sizeof(top_text), 
        "LOCAL %s", 
        "AM");
    }
    else {
      snprintf(top_text, 
        sizeof(top_text), 
        "LOCAL %s", 
        "PM");
    }
  }
  else {
      snprintf(top_text, 
        sizeof(top_text), 
        "LOCAL %s", 
        "24");
  }
  
  text_layer_set_text(tiny_top_text, trim(top_text)); 
  
}
static void update_minutes(struct tm *tick_time) {
  if(mVibeMinutes>0) {
    if(mVibeMinutesTimer<=0) {
      vibes_double_pulse();
      mVibeMinutesTimer = mVibeMinutes;
    }
    else {
      mVibeMinutesTimer--;
    }
  }
  set_container_image(&time_digits_images[3], time_digits_layers[3], MED_IMAGE_RESOURCE_IDS[tick_time->tm_min/10], GPoint(59, 62));
  set_container_image(&time_digits_images[4], time_digits_layers[4], MED_IMAGE_RESOURCE_IDS[tick_time->tm_min%10], GPoint(73, 62));
}
static void update_seconds(struct tm *tick_time) {
  set_container_image(&time_digits_images[6], time_digits_layers[6], MED_IMAGE_RESOURCE_IDS[tick_time->tm_sec/10], GPoint(92, 62));
  set_container_image(&time_digits_images[7], time_digits_layers[7], MED_IMAGE_RESOURCE_IDS[tick_time->tm_sec%10], GPoint(106, 62));
  
  /*
  mAngleMinute+=6;
  if(mAngleMinute>=360) {
    mAngleMinute = 0;
  }
  */
  
  //rot_bitmap_layer_increment_angle(minute_hand_layer, 6);

}
static void update_zulu_hours(struct tm *tick_time) {

  static char label_text[20] = "";  
  unsigned short display_hour = get_display_hour(tick_time->tm_hour);

  set_container_image(&zulu_time_digits_images[0], zulu_time_digits_layers[0], MED_IMAGE_RESOURCE_IDS[display_hour/10], GPoint(26, 94));
  set_container_image(&zulu_time_digits_images[1], zulu_time_digits_layers[1], MED_IMAGE_RESOURCE_IDS[display_hour%10], GPoint(40, 94));

  if (!clock_is_24h_style()) { 
    if (display_hour/10 == 0) {
      layer_set_frame(zulu_time_layer, GRect(-7, 0, 144, 168));
      layer_set_hidden(bitmap_layer_get_layer(zulu_time_digits_layers[0]), true);
    }
    else {
      layer_set_frame(zulu_time_layer, GRect(0, 0, 144, 168));
      layer_set_hidden(bitmap_layer_get_layer(zulu_time_digits_layers[0]), false);
    }
    
    if(tick_time->tm_hour<12) {
      snprintf(label_text, 
        sizeof(label_text), 
        "ZULU %s", 
        "AM");
    }
    else {
      snprintf(label_text, 
        sizeof(label_text), 
        "ZULU %s", 
        "PM");
    }
  }
  else {
      snprintf(label_text, 
        sizeof(label_text), 
        "ZULU %s", 
        "24");
  }
  
  text_layer_set_text(tiny_bottom_text, trim(label_text)); 
  
}
static void update_zulu_minutes(struct tm *tick_time) {
  set_container_image(&zulu_time_digits_images[3], zulu_time_digits_layers[3], MED_IMAGE_RESOURCE_IDS[tick_time->tm_min/10], GPoint(59, 94));
  set_container_image(&zulu_time_digits_images[4], zulu_time_digits_layers[4], MED_IMAGE_RESOURCE_IDS[tick_time->tm_min%10], GPoint(73, 94));
}
static void update_zulu_seconds(struct tm *tick_time) {
  set_container_image(&zulu_time_digits_images[6], zulu_time_digits_layers[6], MED_IMAGE_RESOURCE_IDS[tick_time->tm_sec/10], GPoint(92, 94));
  set_container_image(&zulu_time_digits_images[7], zulu_time_digits_layers[7], MED_IMAGE_RESOURCE_IDS[tick_time->tm_sec%10], GPoint(106, 94));
}


static void handle_tick(struct tm *tick_time, TimeUnits units_changed) {

  time_t utc = time(NULL) + mTimezoneOffset;
  struct tm zulu_tick_time = *localtime(&utc);	
	
  //// BEGIN SECTION - Remove this section and the zulu/local time are the same value? ////
  static char local_date_test[] = "XXXXXXXXXXXXXXXXXXXXX";
  
  strftime(local_date_test,
             sizeof(local_date_test),
             "%a %d %b - %H:%M",
             tick_time);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Local Time: %s", local_date_test);
  
  if(mStyle==2) {      

      static char date_test[] = "XXXXXXXXXXXXXXXXXXXXX";
      
      strftime(date_test,
                 sizeof(date_test),
                 "%a %d %b - %H:%M",
                 &zulu_tick_time);

      APP_LOG(APP_LOG_LEVEL_DEBUG, "Offset: %d - Zulu Time: %s", mTimezoneOffset, date_test);
  }
  //// END SECTION ////

  if (units_changed & DAY_UNIT && mStyle<2) {
    update_days(tick_time);
  }
  if (units_changed & HOUR_UNIT) {
    update_hours(tick_time);
    if(mStyle==2) {
      update_zulu_hours(&zulu_tick_time);
    }
    if(mSeconds) {
      toggleSeconds(false);
    }
    else {
      toggleSeconds(true);
    }
  }
  if (units_changed & MINUTE_UNIT) {
    update_minutes(tick_time);
    if(mStyle==2) {
      update_zulu_minutes(&zulu_tick_time);
    }
  }	
  if (units_changed & SECOND_UNIT && mSeconds==1) {
    update_seconds(tick_time);
    if(mStyle==2) {
      update_zulu_seconds(&zulu_tick_time);
    }
  }		
}

static void window_load(Window *window) {
  
  memset(&time_digits_layers, 0, sizeof(time_digits_layers));
  memset(&time_digits_images, 0, sizeof(time_digits_images));
  memset(&date_digits_layers, 0, sizeof(date_digits_layers));
  memset(&date_digits_images, 0, sizeof(date_digits_images));
  
  Layer *window_layer = window_get_root_layer(window);
  
  //BACKGROUND
  background_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);
  background_layer = bitmap_layer_create(layer_get_frame(window_layer));
  bitmap_layer_set_bitmap(background_layer, background_image);
  layer_add_child(window_layer, bitmap_layer_get_layer(background_layer));  
  
  // TIME LAYER //  
  time_layer = layer_create(layer_get_frame(window_layer));
  layer_add_child(window_layer, time_layer);
  
  //TIME
  GRect dummy_frame = { {0, 0}, {0, 0} };
  for (int i = 0; i < TOTAL_TIME_DIGITS; ++i) {
    time_digits_layers[i] = bitmap_layer_create(dummy_frame);
    layer_add_child(time_layer, bitmap_layer_get_layer(time_digits_layers[i]));
  }  
  
  //TIME COLONS
  set_container_image(&time_digits_images[2], time_digits_layers[2], MED_IMAGE_RESOURCE_IDS[10], GPoint(54, 68));  
  set_container_image(&time_digits_images[5], time_digits_layers[5], MED_IMAGE_RESOURCE_IDS[10], GPoint(87, 68));  
  
  // ZULU TIME LAYER //  
  zulu_time_layer = layer_create(layer_get_frame(window_layer));
  layer_add_child(window_layer, zulu_time_layer);
  
  //ZULU TIME
  for (int i = 0; i < TOTAL_TIME_DIGITS; ++i) {
    zulu_time_digits_layers[i] = bitmap_layer_create(dummy_frame);
    layer_add_child(zulu_time_layer, bitmap_layer_get_layer(zulu_time_digits_layers[i]));
  }  
  
  //ZULU TIME COLONS
  set_container_image(&zulu_time_digits_images[2], zulu_time_digits_layers[2], MED_IMAGE_RESOURCE_IDS[10], GPoint(54, 100));  
  set_container_image(&zulu_time_digits_images[5], zulu_time_digits_layers[5], MED_IMAGE_RESOURCE_IDS[10], GPoint(87, 100));  
  
  //HIDE ZULU INITIALLY
  layer_set_hidden(zulu_time_layer, true);
  
  // DATE LAYER //  
  date_layer = layer_create(layer_get_frame(window_layer));
  layer_add_child(window_layer, date_layer);  
  
  //DATE
  for (int i = 0; i < TOTAL_DATE_DIGITS; ++i) {
    date_digits_layers[i] = bitmap_layer_create(dummy_frame);
    layer_add_child(date_layer, bitmap_layer_get_layer(date_digits_layers[i]));
  }  
  
  //DATE SEPARATORS
  set_container_image(&date_digits_images[2], date_digits_layers[2], TINY_IMAGE_RESOURCE_IDS[11], GPoint(47, 94));
  set_container_image(&date_digits_images[5], date_digits_layers[5], TINY_IMAGE_RESOURCE_IDS[11], GPoint(73, 94));
  
  
  //BATTERY
  GRect BatteryFrame = (GRect) {
    .origin = { .x = 69, .y = 84 },
    .size = {.w = 6, .h = 6}
  };  
  battery_image = gbitmap_create_with_resource(BATTERY_IMAGE_RESOURCE_IDS[4]);
  battery_image_layer = bitmap_layer_create(BatteryFrame);
  bitmap_layer_set_bitmap(battery_image_layer, battery_image);
  layer_add_child(window_layer, bitmap_layer_get_layer(battery_image_layer));  
  update_battery(battery_state_service_peek());

  //TINY TEXT LABELS
  tiny_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_TINY_12));
  
  tiny_top_text = text_layer_create(GRect(0, 46, 144, 14));  
  text_layer_set_background_color(tiny_top_text, GColorClear);
  text_layer_set_text_color(tiny_top_text, GColorWhite);
  text_layer_set_font(tiny_top_text, tiny_font);
  text_layer_set_text_alignment(tiny_top_text, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(tiny_top_text)); 
  
  tiny_bottom_text = text_layer_create(GRect(0, 108, 144, 14));  
  text_layer_set_background_color(tiny_bottom_text, GColorClear);
  text_layer_set_text_color(tiny_bottom_text, GColorWhite);
  text_layer_set_font(tiny_bottom_text, tiny_font);
  text_layer_set_text_alignment(tiny_bottom_text, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(tiny_bottom_text)); 

  tiny_alarm_text = text_layer_create(GRect(0, 117, 144, 14));  
  text_layer_set_background_color(tiny_alarm_text, GColorClear);
  text_layer_set_text_color(tiny_alarm_text, GColorWhite);
  text_layer_set_font(tiny_alarm_text, tiny_font);
  text_layer_set_text_alignment(tiny_alarm_text, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(tiny_alarm_text));   
  
  // Avoids a blank screen on watch start.
  time_t now = time(NULL);
  struct tm *tick_time = localtime(&now);  
  handle_tick(tick_time, SECOND_UNIT+MINUTE_UNIT+HOUR_UNIT+DAY_UNIT);
	
  Tuplet initial_values[] = {
    TupletInteger(SECONDS_KEY, 1),
    TupletInteger(INVERT_KEY, 0),
    TupletInteger(BLUETOOTHVIBE_KEY, 1),
    TupletInteger(VIBEMINUTES_KEY, 0),
    TupletInteger(HANDS_KEY, 1),
    TupletInteger(STYLE_KEY, 0),
    TupletInteger(BACKGROUND_KEY, 0),
    TupletInteger(TIMEZONE_OFFSET_KEY, 0)
  };
  
  app_sync_init(&sync, sync_buffer, sizeof(sync_buffer), initial_values, ARRAY_LENGTH(initial_values),
      sync_tuple_changed_callback, NULL, NULL);

  mAppStarted = true;  

  tick_timer_service_subscribe(SECOND_UNIT, handle_tick);
  battery_state_service_subscribe(&update_battery);
  bluetooth_connection_service_subscribe(bluetooth_connection_callback);
}

static void window_unload(Window *window) {

  app_sync_deinit(&sync);
  
  tick_timer_service_unsubscribe();
  battery_state_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();
  

  //layer_destroy((Layer*)minute_hand_layer);
  
  layer_remove_from_parent(bitmap_layer_get_layer(background_layer));
  bitmap_layer_destroy(background_layer);
  if(background_image != NULL) {
    gbitmap_destroy(background_image);
    background_image = NULL;
  }
  layer_remove_from_parent(bitmap_layer_get_layer(battery_image_layer));
  bitmap_layer_destroy(battery_image_layer);
  gbitmap_destroy(battery_image);
  battery_image = NULL;
  
  for (int i = 0; i < TOTAL_TIME_DIGITS; i++) {
    layer_remove_from_parent(bitmap_layer_get_layer(time_digits_layers[i]));
    gbitmap_destroy(time_digits_images[i]);
    bitmap_layer_destroy(time_digits_layers[i]);
    time_digits_layers[i] = NULL;
  }
  
  for (int i = 0; i < TOTAL_TIME_DIGITS; i++) {
    layer_remove_from_parent(bitmap_layer_get_layer(zulu_time_digits_layers[i]));
    gbitmap_destroy(zulu_time_digits_images[i]);
    bitmap_layer_destroy(zulu_time_digits_layers[i]);
    zulu_time_digits_layers[i] = NULL;
  }
  
  for (int i = 0; i < TOTAL_DATE_DIGITS; i++) {
    layer_remove_from_parent(bitmap_layer_get_layer(date_digits_layers[i]));
    gbitmap_destroy(date_digits_images[i]);
    bitmap_layer_destroy(date_digits_layers[i]);
    date_digits_layers[i] = NULL;
  }
  
  fonts_unload_custom_font(tiny_font);

  text_layer_destroy(tiny_top_text);
  text_layer_destroy(tiny_bottom_text);
  text_layer_destroy(tiny_alarm_text);
  inverter_layer_destroy(inverter_layer);
  layer_destroy(time_layer);
  layer_destroy(zulu_time_layer);
  layer_destroy(date_layer);
}

static void init(void) {
  app_message_open(128, 128); 
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(window, true);
}

static void deinit(void) {
  window_destroy(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
