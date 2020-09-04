#include QMK_KEYBOARD_H

// variable for rgb_matrix config
extern rgb_config_t rgb_matrix_config;

/*
 *
 * brettmiller's Womier K66 layout - lovingly handcrafted
 *
 */

#define _BL 0   // Layer - Base
#define _L1 1   // Layer - 1
#define _CRGB 2 // Layer - Change RGB

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
  [_BL] = LAYOUT(
    KC_GRV         , KC_1    , KC_2    , KC_3    , KC_4    , KC_5     , KC_6     , KC_7    , KC_8    , KC_9    , KC_0    , KC_MINS , KC_EQL  , KC_BSPC , KC_INS ,
    KC_TAB         , KC_Q    , KC_W    , KC_E    , KC_R    , KC_T     , KC_Y     , KC_U    , KC_I    , KC_O    , KC_P    , KC_LBRC , KC_RBRC , KC_BSLS , KC_DEL ,
    LCTL_T(KC_ESC) , KC_A    , KC_S    , KC_D    , KC_F    , KC_G     , KC_H     , KC_J    , KC_K    , KC_L    , KC_SCLN , KC_QUOT , KC_ENT  ,
    KC_LSFT        , KC_Z    , KC_X    , KC_C    , KC_V    , KC_B     , KC_N     , KC_M    , KC_COMM , KC_DOT  , KC_SLSH , KC_RSFT , KC_UP   ,
    MO(_CRGB)      , KC_LALT , KC_LGUI ,                   LT(_L1,KC_SPC) ,                  KC_RGUI , KC_RALT , KC_RCTL , KC_LEFT , KC_DOWN , KC_RGHT
    ),
  [_L1] = LAYOUT(
    KC_GRV         , KC_F1   , KC_F2   , KC_F3   , KC_F4   , KC_F5    , KC_F6    , KC_F7   , KC_F8   , KC_F9   , KC_F10  , _______ , _______ , KC_MUTE , KC_VOLU ,
    _______        , _______ , _______ , _______ , _______ , _______  , _______  , _______ , KC_PSCR , KC_SLCK , KC_DEL  , KC_PGUP , _______ , _______ , KC_VOLD ,
    _______        , _______ , _______ , KC_DEL  , _______ , _______  , KC_LEFT  , KC_DOWN , KC_UP   , KC_RIGHT, KC_BSPC , KC_PGDN , _______ ,
    _______        , _______ , _______ , _______ , _______ , _______  , _______  , _______ , _______ , _______ , _______ , _______ , KC_PGUP ,
    MO(_CRGB)      , _______ , _______ ,                     _______  ,                      _______ , MO(_CRGB), MO(_L1), KC_HOME , KC_PGDN , KC_END
    ),
  [_CRGB] = LAYOUT(
    RGB_MOD       , RGB_RMOD , RGB_M_P , RGB_M_B , RGB_M_R , RGB_M_SW , RGB_M_SN , RGB_M_K , RGB_M_X , RGB_M_G , RGB_M_T , KC_F11  , KC_F12  , RGB_TOG , RGB_VAI ,
    _______       , _______  , _______ , _______ , RESET   , _______  , _______  , _______ , _______ , _______ , _______ , _______ , _______ , _______ , RGB_VAD ,
    _______       , _______  , _______ , _______ , _______ , _______  , KC_LEFT  , KC_DOWN , KC_UP   , KC_RIGHT, _______ , _______ , _______ ,
    _______       , _______  , _______ , _______ , _______ , _______  , _______  , _______ , _______ , _______ , _______ , _______ , RGB_SAI ,
    MO(2)         , _______  , _______ ,                     _______  ,                      _______ , _______ , _______ , RGB_HUD , RGB_SAD , RGB_HUI
    )
};

// Layers RGB map
const uint8_t PROGMEM ledcolors[][DRIVER_LED_TOTAL][4] = {
  [_L1] ={
    {34, HSV_GREEN},  // Light LEDs starting with LED 34 GREEN - hjkl
    {35, HSV_GREEN},  // Light LEDs starting with LED 34 GREEN - hjkl
    {36, HSV_GREEN},  // Light LEDs starting with LED 34 GREEN - hjkl
    {37, HSV_GREEN},  // Light LEDs starting with LED 34 GREEN - hjkl
    {24, HSV_RED},    // Light LED 38 RED - p
    {38, HSV_RED}     // Light LED 38 RED - ;
 },
  [_CRGB] = {
    {0, HSV_GREEN},   // Light LEDs 0-12 -  ~ to Backspace
    {1, HSV_GREEN},   // Light LEDs 0-12 -  ~ to Backspace
    {2, HSV_GREEN},   // Light LEDs 0-12 -  ~ to Backspace
    {3, HSV_GREEN},   // Light LEDs 0-12 -  ~ to Backspace
    {4, HSV_GREEN},   // Light LEDs 0-12 -  ~ to Backspace
    {5, HSV_GREEN},   // Light LEDs 0-12 -  ~ to Backspace
    {6, HSV_GREEN},   // Light LEDs 0-12 -  ~ to Backspace
    {7, HSV_GREEN},   // Light LEDs 0-12 -  ~ to Backspace
    {8, HSV_GREEN},   // Light LEDs 0-12 -  ~ to Backspace
    {9, HSV_GREEN},   // Light LEDs 0-12 -  ~ to Backspace
    {10, HSV_GREEN},  // Light LEDs 0-12 -  ~ to Backspace
    {11, HSV_GREEN},  // Light LEDs 0-12 -  ~ to Backspace
    {12, HSV_GREEN}   // Light LEDs 0-12 -  ~ to Backspace
  }
};

void matrix_init_user(void) {
  rgb_matrix_config.raw = eeprom_read_dword(EECONFIG_RGB_MATRIX);
}

// Function to set individual LED colors
void set_leds_color( int layer) {
  for (int i = 0; i < DRIVER_LED_TOTAL; i++) {
    uint8_t val = pgm_read_byte(&ledcolors[layer][i][2]);
    // if the brightness of the led is set to 0 in the map,
    // the value is not overriden with global controls, allowing the led
    // to appear turned off
    HSV hsv = { .h = pgm_read_byte(&ledcolors[layer][i][1]), .s = pgm_read_byte(&ledcolors[layer][i][2]), .v = val == 0 ? 0 : rgb_matrix_config.hsv.v};
    int led = pgm_read_byte(&ledcolors[layer][1][0]);
    RGB rgb = hsv_to_rgb(hsv);
    rgb_matrix_set_color( led, rgb.r, rgb.g, rgb.b );
  }
};

void rgb_matrix_indicators_user(void) {
  uint32_t mode = rgblight_get_mode();
  // assign colors if the matrix is on and the current mode
  // is SOLID COLORS => No animations running
  if(rgb_matrix_config.enable == 1 && mode == 1) {
    uint8_t layer = biton32(layer_state);
    switch (layer) {
      case _L1:
        //set_leds_color(_L1);
        rgb_matrix_set_color(28, 0, 0, 255);
        rgb_matrix_set_color(34, 0, 0, 255);
        rgb_matrix_set_color(35, 0, 0, 255);
        rgb_matrix_set_color(36, 0, 0, 255);
        rgb_matrix_set_color(37, 0, 0, 255);
        break;
      case _CRGB:
        //set_leds_color(_CRGB);
        rgb_matrix_set_color(0, 0, 255, 0);
        rgb_matrix_set_color(1, 0, 255, 0);
        rgb_matrix_set_color(2, 0, 255, 0);
        break;
    }
  }
};
