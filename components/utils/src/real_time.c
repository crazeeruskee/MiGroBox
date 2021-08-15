/*
 * Author(s):         Lucas Moiseyev
 * Date Created:      1/12/21
 * Date Last Updated: 8/15/21
 *
 */
#include "esp_log.h"
#include "real_time.h"
#include "esp_sntp.h"

//RTC_DATA_ATTR static int boot_count = 0;

time_t* real_time_init(){
    time_t* now_p;

    if((now_p = malloc(sizeof(time_t))) == NULL){
        printf("REAL TIME INIT: MALLOC FAILED!");
    }

    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_init();

#ifdef TIMEZONE
    setenv("TZ", TIMEZONE, 1);
#endif

    tzset();
    *now_p = time(now_p);

    return now_p;
}

struct tm* get_current_time(time_t* now_p){
    struct tm* timeinfo_p;
    char strftime_buf[(size_t)TIME_BUFFER];

    if ((timeinfo_p = malloc(sizeof(struct tm))) == NULL){
        printf("GET CURRENT TIME: MALLOC FAILED!");
    }

    time(now_p);
    localtime_r(now_p, timeinfo_p);

    // Is time set? If not, tm_year will be (1970 - 1900).
    if (timeinfo_p->tm_year < (2021 - 1900)) {
        ESP_LOGI("REAL TIME CLOCK", "Time is not set yet."
                "Connecting to WiFi and getting time over NTP.");
/*
        sntp_setoperatingmode(SNTP_OPMODE_POLL);
        sntp_setservername(0, "pool.ntp.org");
        sntp_init();
*/
        // wait for time to be set
        int retry = 0;
        const int retry_count = 15;
        while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET
                && ++retry < retry_count) {
            ESP_LOGI("REAL TIME CLOCK", "Waiting for system time to be set..."
                    " (%d/%d)", retry, retry_count);
            vTaskDelay(2000 / portTICK_PERIOD_MS);
        }
        time(now_p);
        localtime_r(now_p, timeinfo_p);

        // update 'now' variable with current time
        time(now_p);
    }

    strftime(strftime_buf, TIME_BUFFER, "%c", timeinfo_p);
    ESP_LOGI("REAL TIME CLOCK", "The current date/time (EST) is: %s",
        strftime_buf);

    return timeinfo_p;
}





