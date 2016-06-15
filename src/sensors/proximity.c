#include "proximity.h"

#include "ch.h"
#include "hal.h"
#include <stdio.h>
#include <string.h>
#include "analogic.h"

#define PWM_CLK_FREQ 42000000
#define PWM_FREQUENCY 1000
#define PWM_CYCLE (PWM_CLK_FREQ / PWM_FREQUENCY)
/* Max duty cycle is 0.071, 2x safety margin. */
#define TCRT1000_DC 0.03
#define ON_MEASUREMENT_POS 0.02
#define OFF_MEASUREMENT_POS 0.5
#define NUM_IR_SENSORS 13

#define PROXIMITY_NB_CHANNELS_ADC3 12
#define PROXIMITY_ADC_SAMPLE_TIME ADC_SAMPLE_112
#define DMA_BUFFER_SIZE (1)

unsigned int values[PROXIMITY_NB_CHANNELS_ADC3] = {0};
static BSEMAPHORE_DECL(measurement_ready, true);

static void adc3_proximity_cb(ADCDriver *adcp, adcsample_t *samples, size_t n)
{
    (void) adcp;
    (void) samples;
    (void) n;

    memset(values, 0, sizeof(values));

    for (int i = 0; i < PROXIMITY_NB_CHANNELS_ADC3; i++) {
        values[i] = samples[i];
    }



//j    for (size_t i = 0; i < n; i++) {
//j        for (size_t j = 0; j < PROXIMITY_NB_CHANNELS_ADC3; j++) {
//j            values[j] = samples[j * n + i];
//j        }
//j    }

//    for (size_t i = 0; i < PROXIMITY_NB_CHANNELS_ADC3; i++) {
//        values[i] /= n;
//    }
//
    palTogglePad(GPIOE, GPIOE_LED_STATUS);

    chSysLockFromISR();
    chBSemSignalI(&measurement_ready);
    chSysUnlockFromISR();
}

static THD_FUNCTION(proximity_thd, arg)
{
    (void) arg;
    chRegSetThreadName(__FUNCTION__);

    static adcsample_t adc3_proximity_samples[PROXIMITY_NB_CHANNELS_ADC3 * DMA_BUFFER_SIZE];
    static const ADCConversionGroup adcgrpcfg3 = {
        .circular=true,
        .num_channels=PROXIMITY_NB_CHANNELS_ADC3,
        .end_cb=adc3_proximity_cb,
        .error_cb=NULL,

        /* Discontinuous mode with 8 conversion per trigger. */
        .cr1=ADC_CR1_DISCEN | ADC_CR1_DISCNUM_0,

        /* Trigger on timer 8 CC1. */
        .cr2= ADC_CR2_EXTEN_1 | ADC_CR2_EXTSEL_SRC(0xd),

        .smpr1= ADC_SMPR1_SMP_AN10(PROXIMITY_ADC_SAMPLE_TIME) |  // PC0 - IR_AN8
        ADC_SMPR1_SMP_AN11(PROXIMITY_ADC_SAMPLE_TIME) |  // PC1 - IR_AN9
        ADC_SMPR1_SMP_AN12(PROXIMITY_ADC_SAMPLE_TIME) |  // PC2 - IR_AN10
        ADC_SMPR1_SMP_AN13(PROXIMITY_ADC_SAMPLE_TIME) |  // PC3 - IR_AN11
        ADC_SMPR1_SMP_AN14(PROXIMITY_ADC_SAMPLE_TIME) |  // PF4 - IR_AN1
        ADC_SMPR1_SMP_AN15(PROXIMITY_ADC_SAMPLE_TIME),   // PF5 - IR_AN2

        .smpr2=
        ADC_SMPR2_SMP_AN0(PROXIMITY_ADC_SAMPLE_TIME) |   // PF6 - IR_AN3
        ADC_SMPR2_SMP_AN1(PROXIMITY_ADC_SAMPLE_TIME) |   // PF6 - IR_AN3
        ADC_SMPR2_SMP_AN2(PROXIMITY_ADC_SAMPLE_TIME) |   // PF6 - IR_AN3
        ADC_SMPR2_SMP_AN3(PROXIMITY_ADC_SAMPLE_TIME) |   // PF6 - IR_AN3
        ADC_SMPR2_SMP_AN4(PROXIMITY_ADC_SAMPLE_TIME) |   // PF6 - IR_AN3
        ADC_SMPR2_SMP_AN5(PROXIMITY_ADC_SAMPLE_TIME) |   // PF7 - IR_AN4
        ADC_SMPR2_SMP_AN6(PROXIMITY_ADC_SAMPLE_TIME) |   // PF8 - IR_AN5
        ADC_SMPR2_SMP_AN7(PROXIMITY_ADC_SAMPLE_TIME) |   // PF9 - IR_AN6
        ADC_SMPR2_SMP_AN8(PROXIMITY_ADC_SAMPLE_TIME) |   // PF10 - IR_AN7
        ADC_SMPR2_SMP_AN9(PROXIMITY_ADC_SAMPLE_TIME),    // PF3 - IR_AN0

        .sqr1=ADC_SQR1_NUM_CH(PROXIMITY_NB_CHANNELS_ADC3),
        /*SQR2*/
        .sqr2=ADC_SQR2_SQ7_N(7) | ADC_SQR2_SQ8_N(8) | ADC_SQR2_SQ9_N(10) | ADC_SQR2_SQ10_N(11) | ADC_SQR2_SQ11_N(12) | ADC_SQR2_SQ12_N(13),
        /*SQR3*/
        .sqr3=ADC_SQR3_SQ1_N(9) | ADC_SQR3_SQ2_N(14) |
        ADC_SQR3_SQ3_N(15) | ADC_SQR3_SQ4_N(4) |
        ADC_SQR3_SQ5_N(5) | ADC_SQR3_SQ6_N(6),
    };


    adcStart(&ADCD3, NULL);

    adcStartConversion(&ADCD3, &adcgrpcfg3, adc3_proximity_samples, DMA_BUFFER_SIZE);

    while (true) {
        unsigned int myvalues[PROXIMITY_NB_CHANNELS_ADC3];
        chBSemWait(&measurement_ready);

        chSysLock();
        memcpy(myvalues, values, sizeof(myvalues));
        chSysUnlock();

        printf("proximity: ");
        for (int i = 0; i < PROXIMITY_NB_CHANNELS_ADC3; i++) {
            if (myvalues[i] > 400) {
                printf("%0d:%04d ", i, myvalues[i]);
            } else {
                printf("         ");
            }
        }

        printf("\r\n");

        chThdSleepMilliseconds(500);
    }
}

void pwm_cb(void *p)
{
    (void) p;
}

void proximity_start(void)
{
    static const PWMConfig pwmcfg_proximity = {
        /* timer clock frequency */
        .frequency = PWM_CLK_FREQ,
        /* timer period */
        .period = PWM_CYCLE,
        .cr2 = 0,

        /* Enable DMA event generation on channel 1. */
        .dier = TIM_DIER_CC1DE,
        .callback = NULL,
        .channels = {
            /* Channel 1 is used to generate ADC triggers. It must be in output
             * mode, although it is not routed to any pin. */
            {.mode = PWM_OUTPUT_ACTIVE_HIGH, .callback = NULL},

            /* Channel 2N is used to generate TCRT1000 drive signals. */
            {.mode = PWM_COMPLEMENTARY_OUTPUT_ACTIVE_HIGH, .callback = NULL},
            {.mode = PWM_OUTPUT_DISABLED, .callback = NULL},
            {.mode = PWM_OUTPUT_DISABLED, .callback = NULL},
        },
    };

    /* Init PWM */
    pwmStart(&PWMD8, &pwmcfg_proximity);

    /* Set duty cycle for TCRT1000 drivers. */
//    pwmEnableChannel(&PWMD8, 1, (pwmcnt_t) (PWM_CYCLE * TCRT1000_DC));

    /* Set measurement time for ADC. */
    pwmEnableChannel(&PWMD8, 0, (pwmcnt_t) (PWM_CYCLE * OFF_MEASUREMENT_POS));

    static THD_WORKING_AREA(proximity_thd_wa, 2048);
    chThdCreateStatic(proximity_thd_wa, sizeof(proximity_thd_wa), NORMALPRIO, proximity_thd, NULL);
}
