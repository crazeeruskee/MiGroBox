/*
 * Author(s):         Lucas Moiseyev
 * Date Created:      1/12/21
 * Date Last Updated: 8/15/21
 *
 */
#pragma once

#include <stdint.h>
#include <time.h>
#include "esp_err.h"

#define TIME_BUFFER 64

#ifdef CONFIG_TIMEZONE
#define TIMEZONE CONFIG_TIMEZONE//Kconfig TIMEZONE
#endif

time_t* real_time_init();

struct tm* get_current_time();

