set(component_srcs "src/fan_pwm.c" "src/endstop.c" "src/stepper.c" "utils/src/real_time.c")

idf_component_register(SRCS "${component_srcs}"
                       INCLUDE_DIRS "include" "utils/include"
                       PRIV_INCLUDE_DIRS ""
                       PRIV_REQUIRES "driver"
                       REQUIRES "")
