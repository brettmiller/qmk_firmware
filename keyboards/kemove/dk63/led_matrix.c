#include <SN32F240B.h>
#include <string.h>
#include "CT16.h"
#include "ch.h"
#include "hal.h"
#include "rgb.h"
#include "rgb_matrix.h"
#include "rgb_matrix_types.h"
#include "led.h"
#include "color.h"
#include "matrix.h"

/*
    COLS key / led
    PWM PWM08A - PWM21A
    2ty transistors PNP driven high
    base      - GPIO
    collector - LED Col pins
    emitter   - VDD

    VDD     GPIO
    (E)     (B)
     |  PNP  |
     |_______|
         |
         |
        (C)
        LED

    ROWS RGB
    PWM PWM08B - PWM23B
    C 0-15
    j3y transistors NPM driven low
    base      - GPIO
    collector - LED RGB row pins
    emitter   - GND

        LED
        (C)
         |
         |
      _______
     |  NPM  |
     |       |
    (B)     (E)
    GPIO    GND
*/


static const pin_t row_pins[MATRIX_ROWS] = MATRIX_ROW_PINS;
static const pin_t col_pins[MATRIX_COLS] = MATRIX_COL_PINS;

static uint8_t current_col = 0;

extern volatile matrix_row_t raw_matrix[MATRIX_ROWS]; //raw values

LED_TYPE led_state[DRIVER_LED_TOTAL];

void init(void){
    // Enable Timer Clock
    SN_SYS1->AHBCLKEN_b.CT16B1CLKEN = 1;

    // PFPA - Set PWM to port B pins
    SN_PFPA->CT16B1 = 0xFFFF00;         // 8-9, 11-23 = top half 16 bits

    // Enable PWM function, IOs and select the PWM modes
    // Enable PWM8-PWM9, PWM11-PWM23 function
    SN_CT16B1->PWMENB   =   (mskCT16_PWM8EN_EN  \
                            |mskCT16_PWM9EN_EN  \
                            |mskCT16_PWM11EN_EN \
                            |mskCT16_PWM12EN_EN \
                            |mskCT16_PWM13EN_EN \
                            |mskCT16_PWM14EN_EN \
                            |mskCT16_PWM15EN_EN \
                            |mskCT16_PWM16EN_EN \
                            |mskCT16_PWM17EN_EN \
                            |mskCT16_PWM18EN_EN \
                            |mskCT16_PWM19EN_EN \
                            |mskCT16_PWM20EN_EN \
                            |mskCT16_PWM21EN_EN \
                            |mskCT16_PWM22EN_EN \
                            |mskCT16_PWM23EN_EN);

    // Enable PWM8-PWM9 PWM12-PWM23 IO
    SN_CT16B1->PWMIOENB =   (mskCT16_PWM8IOEN_EN  \
                            |mskCT16_PWM9IOEN_EN  \
                            |mskCT16_PWM11IOEN_EN \
                            |mskCT16_PWM12IOEN_EN \
                            |mskCT16_PWM13IOEN_EN \
                            |mskCT16_PWM14IOEN_EN \
                            |mskCT16_PWM15IOEN_EN \
                            |mskCT16_PWM16IOEN_EN \
                            |mskCT16_PWM17IOEN_EN \
                            |mskCT16_PWM18IOEN_EN \
                            |mskCT16_PWM19IOEN_EN \
                            |mskCT16_PWM20IOEN_EN \
                            |mskCT16_PWM21IOEN_EN \
                            |mskCT16_PWM22IOEN_EN \
                            |mskCT16_PWM23IOEN_EN);

    // Select as PWM mode 2
    SN_CT16B1->PWMCTRL =    (mskCT16_PWM8MODE_2  \
                            |mskCT16_PWM9MODE_2  \
                            |mskCT16_PWM11MODE_2 \
                            |mskCT16_PWM12MODE_2 \
                            |mskCT16_PWM13MODE_2 \
                            |mskCT16_PWM14MODE_2 \
                            |mskCT16_PWM15MODE_2);
    SN_CT16B1->PWMCTRL2 =   (mskCT16_PWM16MODE_2 \
                            |mskCT16_PWM17MODE_2 \
                            |mskCT16_PWM18MODE_2 \
                            |mskCT16_PWM19MODE_2 \
                            |mskCT16_PWM20MODE_2 \
                            |mskCT16_PWM21MODE_2 \
                            |mskCT16_PWM22MODE_2 \
                            |mskCT16_PWM23MODE_2);

    // Set match interrupts and TC rest
    SN_CT16B1->MCTRL = (mskCT16_MR1IE_EN);
    // SN_CT16B1->MCTRL_b.MR0RST = 1;
    SN_CT16B1->MCTRL_b.MR1RST = 1;

    // COL match register
    SN_CT16B1->MR1 = 0xFF;

    // Set prescale value
    SN_CT16B1->PRE = 0x24;

    //Set CT16B1 as the up-counting mode.
	SN_CT16B1->TMRCTRL = (mskCT16_CRST);

    // Wait until timer reset done.
    while (SN_CT16B1->TMRCTRL & mskCT16_CRST);

    // Let TC start counting.
    SN_CT16B1->TMRCTRL |= mskCT16_CEN_EN;

    NVIC_ClearPendingIRQ(CT16B1_IRQn);
    nvicEnableVector(CT16B1_IRQn, 6);
}

static void flush(void) {}

void set_color(int index, uint8_t r, uint8_t g, uint8_t b) {
    led_state[index].r = r * 255;
    led_state[index].g = g * 255;
    led_state[index].b = b * 255;
}

static void set_color_all(uint8_t r, uint8_t g, uint8_t b) {
    for (int i=0; i<DRIVER_LED_TOTAL; i++)
        set_color(i, r, g, b);
}

const rgb_matrix_driver_t rgb_matrix_driver = {
    .init          = init,
    .flush         = flush,
    .set_color     = set_color,
    .set_color_all = set_color_all,
};

// byte order: R,B,G
static uint8_t caps_lock_color[3] = { 0x00, 0x00, 0xFF };

void led_set(uint8_t usb_led) {
    // dk63 has only CAPS indicator
    if (usb_led >> USB_LED_CAPS_LOCK & 1) {
        set_color(11, caps_lock_color[0], caps_lock_color[2], caps_lock_color[1]);
    } else {
        set_color(11, 0x00, 0x00, 0x00);
    }
}

/**
 * @brief   TIM2 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(Vector80) {

    OSAL_IRQ_PROLOGUE();

    SN_CT16B1->IC = mskCT16_MR1IC; // Clear match interrupt status

    // Turn COL off
    setPinInput(col_pins[current_col]);
    writePinHigh(col_pins[current_col]);

    for (uint8_t row_index = 0; row_index < MATRIX_ROWS; row_index++) {

        writePinLow(row_pins[row_index]);

        // Check row pin state
        if (readPin(col_pins[current_col]) == 0) {
            // Pin LO, set col bit
            raw_matrix[row_index] |= (MATRIX_ROW_SHIFTER << current_col);
        } else {
            // Pin HI, clear col bit
            raw_matrix[row_index] &= ~(MATRIX_ROW_SHIFTER << current_col);
        }

        writePinHigh(row_pins[row_index]);
    }

    current_col = (current_col + 1) % MATRIX_COLS;

    // Turn COL ON
    setPinOutput(col_pins[current_col]);
    writePinLow(col_pins[current_col]);

    SN_CT16B1->MR23 = led_state[(current_col) + 0].r;
    SN_CT16B1->MR8  = led_state[(current_col) + 0].b;
    SN_CT16B1->MR9  = led_state[(current_col) + 0].g;

    SN_CT16B1->MR11 = led_state[(current_col) + 1].r;
    SN_CT16B1->MR12 = led_state[(current_col) + 1].b;
    SN_CT16B1->MR13 = led_state[(current_col) + 1].g;

    SN_CT16B1->MR15 = led_state[(current_col) + 2].b;
    SN_CT16B1->MR16 = led_state[(current_col) + 2].g;
    SN_CT16B1->MR14 = led_state[(current_col) + 2].r;

    SN_CT16B1->MR17 = led_state[(current_col) + 3].r;
    SN_CT16B1->MR18 = led_state[(current_col) + 3].b;
    SN_CT16B1->MR19 = led_state[(current_col) + 3].g;

    SN_CT16B1->MR20 = led_state[(current_col) + 4].r;
    SN_CT16B1->MR21 = led_state[(current_col) + 4].b;
    SN_CT16B1->MR22 = led_state[(current_col) + 4].g;

    OSAL_IRQ_EPILOGUE();
}
