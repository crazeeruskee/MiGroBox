/*
 * Author(s):         Lucas Moiseyev
 * Date Created:      1/12/21
 * Date Last Updated: 1/13/21
 *
 */
#pragma once

#include <stdint.h>
#include <time.h>
#include "esp_err.h"

time_t* real_time_init();

struct tm* get_current_time();

