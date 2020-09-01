#include QMK_KEYBOARD_H

/*
 *
 * brettmiller's Womier K66 layout - lovingly handcrafted
 *
 */

#define _BL 0   // Layer - Base
#define _L1 1   // Layer - 1
#define _CRGB 2 // Layer - Chage RGB

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
    _______        , _______ , _______ , _______ , _______ , _______  , _______  , _______ , KC_PSCR , KC_SLCK , KC_PGUP , KC_DEL  , _______ , _______ , KC_VOLD ,
    _______        , _______ , _______ , _______ , _______ , _______  , KC_LEFT  , KC_DOWN , KC_UP   , KC_RIGHT, KC_PGDN , KC_BSPC , _______ ,
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

// Suspend - requires '#define RGB_DISABLE_WHEN_USB_SUSPENDED true' in config.h
void suspend_power_down_user(void) {
    rgb_matrix_set_suspend_state(true);
}

void suspend_wakeup_init_user(void) {
    rgb_matrix_set_suspend_state(false);
}
