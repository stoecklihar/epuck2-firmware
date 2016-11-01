#include "motor_controller.h"
#include "motor_pwm.h"
#include "pid/pid.h"
#include "sensors/battery_level.h"
#include "msgbus/messagebus.h"
#include "main.h"
#include "parameter/parameter.h"
#include <math.h>
#include <string.h>

void pid_param_update(struct pid_param_s *p, pid_ctrl_t *ctrl)
{
    if (parameter_changed(&p->kp) ||
        parameter_changed(&p->ki) ||
        parameter_changed(&p->kd)) {
        pid_set_gains(ctrl, parameter_scalar_get(&p->kp),
                            parameter_scalar_get(&p->ki),
                            parameter_scalar_get(&p->kd));
        pid_reset_integral(ctrl);
    }
    if (parameter_changed(&p->i_limit)) {
        pid_set_integral_limit(ctrl, parameter_scalar_get(&p->i_limit));
    }
}

static void pid_param_declare(struct pid_param_s *p)
{
    parameter_scalar_declare_with_default(&p->kp, &p->ns, "kp", 0);
    parameter_scalar_declare_with_default(&p->ki, &p->ns, "ki", 0);
    parameter_scalar_declare_with_default(&p->kd, &p->ns, "kd", 0);
    parameter_scalar_declare_with_default(&p->i_limit, &p->ns, "i_limit", INFINITY);
}

static void declare_parameters(motor_controller_t *controller, parameter_namespace_t *root)
{
    parameter_namespace_declare(&controller->param_ns_control, root, "control");
    parameter_scalar_declare(&controller->param_vel_limit, &controller->param_ns_control, "velocity_limit");
    parameter_scalar_declare(&controller->param_torque_limit, &controller->param_ns_control, "torque_limit");
    parameter_scalar_declare(&controller->param_acc_limit, &controller->param_ns_control, "acceleration_limit");

    parameter_namespace_declare(&controller->params_pos_pid.ns, &controller->param_ns_control, "position");
    parameter_namespace_declare(&controller->params_vel_pid.ns, &controller->param_ns_control, "velocity");
    parameter_namespace_declare(&controller->params_cur_pid.ns, &controller->param_ns_control, "current");

    pid_param_declare(&controller->params_pos_pid);
    pid_param_declare(&controller->params_vel_pid);
    pid_param_declare(&controller->params_cur_pid);
}

void motor_controller_init(motor_controller_t *controller, parameter_namespace_t *parent)
{
    memset(controller, 0, sizeof(motor_controller_t));
    declare_parameters(controller, parent);
    pid_init(&controller->cur_pid);
    pid_init(&controller->vel_pid);
    pid_init(&controller->pos_pid);
}

float motor_controller_process(motor_controller_t *controller)
{
    /* Update controller gains. */
    pid_param_update(&controller->params_pos_pid, &controller->pos_pid);
    pid_param_update(&controller->params_vel_pid, &controller->vel_pid);
    pid_param_update(&controller->params_cur_pid, &controller->cur_pid);

    float current = 0.;

    /* Get current. */
    if (controller->callbacks.get_current.fn) {
        current = controller->callbacks.get_current.fn(controller->callbacks.get_current.arg);
    }

    float current_error = current - controller->cur_setpoint;

    return pid_process(&controller->cur_pid, current_error);
}
