// COLS

// PWM PWM08A - PWM21A
// 2ty transistors PNP driven high
// base      - GPIO
// collector - LED Col pins
// emitter   - VDD

// GPIO
// (E)     (B)
// |       |
// |
// (C)

// ROWS RGB
// PWM PWM08B - PWM23B
// C 0-15
// j3y transistors NPM driven low
// base      - GPIO
// collector - LED RGB row pins
// emitter   - GND
// (C)
// |
// |       |
// (B)     (E)
//

// #include <SN32F260.h>
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

volatile SN_CT16B1_Type * ct_b1 = (SN_CT16B1_Type *) SN_CT16B1_BASE;

extern volatile matrix_row_t raw_matrix[MATRIX_ROWS];  // raw values
static const pin_t row_pins[MATRIX_ROWS] = MATRIX_ROW_PINS;

uint32_t led_pwm_values[16] =
{
    12000, 12000, 1200, 12000, 12000, 12000, 12000, 12000, 12000, 12000, 12000, 12000, 12000, 12000, 12000, 12000
};                                     // Extra for the boot pin not in use

static struct
{
    uint8_t g;
    uint8_t b;
    uint8_t r;
} __attribute__((packed)) led_state[DRIVER_LED_TOTAL];

static const pin_t col_pins[MATRIX_COLS] = MATRIX_COL_PINS;

static uint8_t col_index = 0;

// set counter reset on MRn (setting MRn to the full period) and set the other MRs to the PWM duty cycle you want for that pin
// on period interrupt update all the PWM MRs to the values for the next LED
// the only issue is that when you do that, the timer has reset and may count during the ISR, so you'll have to detect low or 0 values and set the pin accordingly

void init (void) {
    // Enable Timer Clock
    SN_SYS1->AHBCLKEN_b.CT16B1CLKEN = 1;

    //// PFPA - Set PWM to port B pins
    // SN_PFPA->CT16B1 = 0xFFFF00;        // 8-9, 11-23 = top half 16 bits
    // 1111 1111 1111 1011 0000 0000

    ////Set MR23 value for 1ms PWM period ==> count value = 1000*12 = 12000
    //// SN_CT16B1->MR22 = 12000;
    //// base 0x40002000
    //// MR0 at Offset:0x20
    //// MR8 at Offset:0x40
    //// MR24 at Offset:0x80
    //// set all match registers the same for now
    // memcpy((void*) 0x40002040, led_pwm_values, sizeof(led_pwm_values));

    // 16 bits - max = 65535
    SN_CT16B1->MR[0] = 0xFFFF;
    SN_CT16B1->MR[1] = 0;
    SN_CT16B1->MR[2] = 0;

    SN_CT16B1->MR[3] = 0xFFFF;
    SN_CT16B1->MR[4] = 0;
    SN_CT16B1->MR[5] = 0;

    SN_CT16B1->MR[6] = 0xFFFF;
    SN_CT16B1->MR[7] = 0;
    SN_CT16B1->MR[8] = 0;

    SN_CT16B1->MR[9]  = 0xFFFF;
    SN_CT16B1->MR[10] = 0;
    SN_CT16B1->MR[11] = 0;

    SN_CT16B1->MR[12] = 0xFFFF;
    SN_CT16B1->MR[13] = 0;
    SN_CT16B1->MR[14] = 0;

    // Enable PWM function, IOs and select the PWM modes
    // SN_CT16B1->PWMENB   = 0xFFFB00;     //Enable PWM8-PWM9, PWM11-PWM23 function
    // |mskCT16_PWM10EN_EN
    SN_CT16B1->PWMENB = (mskCT16_PWM0EN_EN |  \
                         mskCT16_PWM1EN_EN |  \
                         mskCT16_PWM2EN_EN |  \
                         mskCT16_PWM3EN_EN |  \
                         mskCT16_PWM4EN_EN |  \
                         mskCT16_PWM5EN_EN |  \
                         mskCT16_PWM6EN_EN |  \
                         mskCT16_PWM7EN_EN |  \
                         mskCT16_PWM8EN_EN |  \
                         mskCT16_PWM9EN_EN |  \
                         mskCT16_PWM10EN_EN | \
                         mskCT16_PWM11EN_EN | \
                         mskCT16_PWM12EN_EN | \
                         mskCT16_PWM13EN_EN | \
                         mskCT16_PWM14EN_EN);

    // SN_CT16B1->PWMIOENB = 0xFFFB00;     //Enable PWM8-PWM9 PWM12-PWM23 IO
    // |mskCT16_PWM0IOEN_EN
    SN_CT16B1->PWMIOENB = (mskCT16_PWM0IOEN_EN |  \
                           mskCT16_PWM1IOEN_EN |  \
                           mskCT16_PWM2IOEN_EN |  \
                           mskCT16_PWM3IOEN_EN |  \
                           mskCT16_PWM4IOEN_EN |  \
                           mskCT16_PWM5IOEN_EN |  \
                           mskCT16_PWM6IOEN_EN |  \
                           mskCT16_PWM7IOEN_EN |  \
                           mskCT16_PWM8IOEN_EN |  \
                           mskCT16_PWM9IOEN_EN |  \
                           mskCT16_PWM10IOEN_EN | \
                           mskCT16_PWM11IOEN_EN | \
                           mskCT16_PWM12IOEN_EN | \
                           mskCT16_PWM13IOEN_EN | \
                           mskCT16_PWM14IOEN_EN);

    // SN_CT16B1->PWMCTRL  = 0x55650000;   //PWM08-PWM9 , PWM11-PWM15 select as PWM mode 2
    // |mskCT16_PWM10MODE_2
    SN_CT16B1->PWMCTRL = (mskCT16_PWM0MODE_2 |  \
                          mskCT16_PWM1MODE_2 |  \
                          mskCT16_PWM2MODE_2 |  \
                          mskCT16_PWM3MODE_2 |  \
                          mskCT16_PWM4MODE_2 |  \
                          mskCT16_PWM5MODE_2 |  \
                          mskCT16_PWM6MODE_2 |  \
                          mskCT16_PWM7MODE_2 |  \
                          mskCT16_PWM8MODE_2 |  \
                          mskCT16_PWM9MODE_2 |  \
                          mskCT16_PWM10MODE_2 | \
                          mskCT16_PWM11MODE_2 | \
                          mskCT16_PWM12MODE_2 | \
                          mskCT16_PWM13MODE_2 | \
                          mskCT16_PWM14MODE_2);

    // 01 1011 0000 0000 0000 0000 0000 0000
    // SN_CT16B1->MCTRL2 = 0x1B6DB6D8; // PWM11-PWM19
    // mskCT16_MR10RST_EN|mskCT16_MR10IE_EN

#if 1
    SN_CT16B1->MR[15] = 0xFF;
    SN_CT16B1->MCTRL2 = (mskCT16_MR15IE_EN);
#endif

    // 011 0110 1101 1011

    // Green 14 11 8 5 1

    //// Testing individual PWMs
    // SN_PFPA->CT16B1 = ((1<<16)|(1<<17));
    // SN_CT16B1->MR16 = 1200;
    // SN_CT16B1->MR17 = 1200;

    // SN_CT16B1->PWMIOENB = ((1<<16)|(1<<17)); // Enable PWM16 IO
    // SN_CT16B1->PWMENB   = ((1<<16)|(1<<17)); // Enable PWM16 function
    // SN_CT16B1->PWMCTRL2 = ((1<<0) | 1<<2);   // PWM16 select as PWM mode 2

    // SN_CT16B1->MCTRL2 = ((1<<18)|(1<<19)); // PWM16 TC and RESET
    // SN_CT16B1->MCTRL2 = (mskCT16_MR16IE_EN|mskCT16_MR16RST_EN);

    // Set prescale value
    SN_CT16B1->PRE = 0x4;

    // Vector84();

    // Let TC start counting.
    SN_CT16B1->TMRCTRL |= mskCT16_CEN_EN;

    //Set col, read rows
    for (uint8_t current_col = 0; current_col < MATRIX_COLS; current_col++)
    {
        setPinOutput(col_pins[current_col]);
    }
    writePinLow(col_pins[col_index]);

    // for (uint8_t current_col = col_first; current_col < col_last; current_col++)
    // {
    //     writePinLow(col_pins[current_col]);
    // }

    SN_CT16B1->MCTRL2_b.MR15RST = 1;

    // Wait until timer reset done.
    while (SN_CT16B1->TMRCTRL & mskCT16_CRST)
    {
        ;
    }

    // SN_CT16B1->PRE = 0x3;

    NVIC_ClearPendingIRQ(CT16B1_IRQn);
    nvicEnableVector(CT16B1_IRQn, 4);
}

static void flush (void) {
}

static void set_color (int index, uint8_t r, uint8_t g, uint8_t b) {
    led_state[index].r = r;
    led_state[index].g = g;
    led_state[index].b = b;
}

static void set_color_all (uint8_t r, uint8_t g, uint8_t b) {
    for (int i = 0; i < DRIVER_LED_TOTAL; i++)
    {
        set_color(i, r, g, b);
    }
}

const rgb_matrix_driver_t rgb_matrix_driver =
{
    .init          = init,
    .flush         = flush,
    .set_color     = set_color,
    .set_color_all = set_color_all,
};
static void select_col (uint8_t col) {
    // setPinOutput(col_pins[col]);
}

static void unselect_col (uint8_t col) {
}

/**
 * @brief   TIM2 interrupt handler.
 *
 * @isr
 */
extern volatile bool matrix_changed;

// OSAL_IRQ_HANDLER(Vector84)
void RgbIsr ()
{
    // OSAL_IRQ_PROLOGUE();

    volatile uint32_t shit = SN_CT16B1->TC;

    // unselect_col(col);
    writePinHigh(col_pins[col_index]);

    col_index = (col_index + 1) % MATRIX_COLS;

    // volatile uint32_t shit =
    writePinLow(col_pins[col_index]);

    SN_CT16B1->MR[0] = led_state[(col_index) + 0].g;
    SN_CT16B1->MR[1] = led_state[(col_index) + 0].b;
    SN_CT16B1->MR[2] = led_state[(col_index) + 0].r;

    SN_CT16B1->MR[3] = led_state[(col_index) + 1].g;
    SN_CT16B1->MR[4] = led_state[(col_index) + 1].b;
    SN_CT16B1->MR[5] = led_state[(col_index) + 1].r;

    SN_CT16B1->MR[6] = led_state[(col_index) + 2].g;
    SN_CT16B1->MR[7] = led_state[(col_index) + 2].b;
    SN_CT16B1->MR[8] = led_state[(col_index) + 2].r;

    SN_CT16B1->MR[9]  = led_state[(col_index) + 3].g;
    SN_CT16B1->MR[10] = led_state[(col_index) + 3].b;
    SN_CT16B1->MR[11] = led_state[(col_index) + 3].r;

    SN_CT16B1->MR[12] = led_state[(col_index) + 4].g;
    SN_CT16B1->MR[13] = led_state[(col_index) + 4].b;
    SN_CT16B1->MR[14] = led_state[(col_index) + 4].r;
#if 1
    // if(0 == col_index)
    // {
    //     matrix_changed = 0;
    // }

    for (uint8_t row_index = 0; row_index < MATRIX_ROWS; row_index++)
    {
        matrix_row_t last_row_value = raw_matrix[row_index];
        // Check row pin state
        if (readPin(row_pins[row_index]) == 0)
        {
            // Pin LO, set col bit
            raw_matrix[row_index] |= (MATRIX_ROW_SHIFTER << col_index);
        }
        else
        {
            // Pin HI, clear col bit
            raw_matrix[row_index] &= ~(MATRIX_ROW_SHIFTER << col_index);
        }

        // Determine if the matrix changed state
        if ((last_row_value != raw_matrix[row_index]))
        {
            matrix_changed |= true;
        }
    }
    #endif

    // select_col(col);

    SN_CT16B1->IC = SN_CT16B1->RIS;    // Clear all for now
    // SN_CT16B1->TC = 0;

    // OSAL_IRQ_EPILOGUE();
}
