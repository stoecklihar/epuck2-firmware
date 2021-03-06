#ifndef SKEL_USER_H
#define SKEL_USER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "common/types.h"
#include "vm/vm.h"
#include "vm/natives.h"
#include "parameter/parameter.h"
#include "sensors/proximity.h"

#include "body_leds.h"
#include "madgwick.h"

/** Number of variables usable by the Aseba script. */
#define VM_VARIABLES_FREE_SPACE 256

/** Maximum number of args an Aseba event can use. */
#define VM_VARIABLES_ARG_SIZE 32

#define SETTINGS_COUNT 32

/** Enum containing all the possible events. */
enum AsebaLocalEvents {
    EVENT_RANGE=0, // New range measurement
    EVENT_PROXIMITY, // New proximity sensor measurement
    EVENT_ENCODERS, // New motor encoders measurement
    EVENT_IMU, // New acceleration and gyro measurement
    EVENT_TIMER, // Timer was fired
    EVENT_SOUND_PLAY_FINISHED, // A sound finished playing
    EVENT_SOUND_ERROR, // An error occurred during sound playback
};


/** Struct type defining the variables to expose to the VM.
 *
 * @note This should be kept in sync with the variable descriptions.
 */
struct _vmVariables {
    sint16 id;                          // NodeID
    sint16 source;                      // Source
    sint16 args[VM_VARIABLES_ARG_SIZE]; // Args
    sint16 fwversion[2];                // Firmware version
    sint16 productId;                   // Product ID

    // Variables
    uint16 battery_mv;
    uint16 range;
    sint16 proximity_delta[PROXIMITY_NB_CHANNELS];
    sint16 proximity_ambient[PROXIMITY_NB_CHANNELS];
    sint16 proximity_reflected[PROXIMITY_NB_CHANNELS];

    sint16 motor_left_pwm;
    sint16 motor_right_pwm;

    sint16 motor_left_current;
    sint16 motor_right_current;
    sint16 motor_left_velocity;
    sint16 motor_right_velocity;
    sint16 motor_left_position;
    sint16 motor_right_position;

    /* 32 bit encoders are stored in an MSB, LSB tuple. */
    sint16 motor_left_enc[2];
    sint16 motor_right_enc[2];

    sint16 acceleration[3];
    sint16 gyro[3];
    sint16 theta;
    sint16 phi;
    sint16 psi;

    /* Led values as percentages. */
    uint16 leds[BODY_LED_COUNT];

    /* Setpoints */
    sint16 motor_left_current_setpoint;
    sint16 motor_right_current_setpoint;
    sint16 motor_left_velocity_setpoint;
    sint16 motor_right_velocity_setpoint;
    sint16 motor_left_position_setpoint;
    sint16 motor_right_position_setpoint;

    /* Control parameters */
    sint16 control_left_current_kp;
    sint16 control_left_current_ki;
    sint16 control_left_current_kd;
    sint16 control_left_current_ilimit;
    sint16 control_left_velocity_kp;
    sint16 control_left_velocity_ki;
    sint16 control_left_velocity_kd;
    sint16 control_left_velocity_ilimit;
    sint16 control_left_position_kp;
    sint16 control_left_position_ki;
    sint16 control_left_position_kd;
    sint16 control_left_position_ilimit;

    sint16 control_right_current_kp;
    sint16 control_right_current_ki;
    sint16 control_right_current_kd;
    sint16 control_right_current_ilimit;
    sint16 control_right_velocity_kp;
    sint16 control_right_velocity_ki;
    sint16 control_right_velocity_kd;
    sint16 control_right_velocity_ilimit;
    sint16 control_right_position_kp;
    sint16 control_right_position_ki;
    sint16 control_right_position_kd;
    sint16 control_right_position_ilimit;


    // Free space
    sint16 freeSpace[VM_VARIABLES_FREE_SPACE];
};

/** Declares the parameters and variables required by the Aseba application. */
void aseba_variables_init(parameter_namespace_t *aseba_ns);

/** Updates the Aseba variables from the system. */
void aseba_read_variables_from_system(AsebaVMState *vm);

/** Updates the system from the Aseba variables. */
void aseba_write_variables_to_system(AsebaVMState *vm);

void accelerometer_cb(void);
void button_cb(void);

extern struct _vmVariables vmVariables;

extern const AsebaVMDescription vmDescription;
extern const AsebaLocalEventDescription localEvents[];

extern AsebaNativeFunctionPointer nativeFunctions[];
extern const AsebaNativeFunctionDescription* nativeFunctionsDescription[];
extern const int nativeFunctions_length;

#ifdef __cplusplus
}
#endif

#endif /* SKEL_USER_H */
