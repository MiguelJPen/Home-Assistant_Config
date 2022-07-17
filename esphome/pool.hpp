#include <iostream>
#include <ctime>
#include <string>
#include <clocale>
#include <cmath>
#include <algorithm>

#include "esphome.h"

using namespace std;

void set_new_pumping_time();
void set_winter_timestamps(time_t, int, int);
void set_pumping(time_t, time_t, time_t, int, int);
time_t get_sunrise();
time_t get_sunset();

void set_new_pumping_time() {
    float mean_tmp = id(mean_temp);
    int mins_per_cycle = 30 + (mean_tmp * 1.5); // Cycles are every 2 and a half hours
    time_t now_timestamp = id(time_sntp).now().timestamp;
    time_t sunrise = get_sunrise(), sunset = get_sunset();

    //ESP_LOGD("set_new_pumping_time", "SUNRISE: %ld, SUNSET: %ld", sunrise, sunset);

    id(pump_timestamps_on).publish_state("");
    id(pump_timestamps_off).publish_state("");

    // Set how many time per days
    if (id(pump_winter_mode_bool)) {
        set_winter_timestamps(now_timestamp, 0, 30);
        set_winter_timestamps(now_timestamp, 1, 30);
        return;
    }

    // If winter mode is not activated
    set_pumping(now_timestamp, sunrise, sunset, 0, mins_per_cycle);
    set_pumping(now_timestamp, sunrise, sunset, 1, mins_per_cycle);

}

void set_winter_timestamps(time_t today, int days_ahead, int mins) {
    time_t new_pump_day = today + (days_ahead * 3600 * 24);
    struct tm new_time = *localtime(&new_pump_day);
    string pump_on = id(pump_timestamps_on).state;
    string pump_off = id(pump_timestamps_off).state;
    string comma_on = (pump_on.empty() ? "" : ", ");
    string comma_off = (pump_off.empty() ? "" : ", ");

    new_time.tm_hour = 13;
    new_time.tm_min = 0;
    new_time.tm_sec = 0;
    time_t aux = mktime(&new_time);

    pump_on = update_list(pump_on);
    pump_off = update_list(pump_off);
    pump_on.append(comma_on + to_string(aux));
    pump_off.append(comma_off + to_string(aux + mins * 60));

    //ESP_LOGD("set_winter_times", "TIME PER DAY: %i", mins);

    id(pump_timestamps_on).publish_state(pump_on);
    id(pump_timestamps_off).publish_state(pump_off);
}

void set_pumping(time_t today, time_t sunrise, time_t sunset, int days_ahead, int mins) {
    time_t new_pump_day = today + (days_ahead * 3600 * 24);
    struct tm new_time = *localtime(&new_pump_day);
    string pump_on = id(pump_timestamps_on).state;
    string pump_off = id(pump_timestamps_off).state;
    string comma_on = (pump_on.empty() ? "" : ", ");
    string comma_off = (pump_off.empty() ? "" : ", ");
    struct tm sunrise_st = *localtime(&sunrise), sunset_st = *localtime(&sunset);

    new_time.tm_hour = sunrise_st.tm_hour;
    new_time.tm_min = sunrise_st.tm_min;
    new_time.tm_sec = 0;
    time_t aux = mktime(&new_time);
    new_time.tm_hour = sunset_st.tm_hour;
    new_time.tm_min = sunset_st.tm_min;
    time_t sunset_local = mktime(&new_time);

    //ESP_LOGD("set_pumping", "SUNRISE: %ld, SUNSET: %ld", sunrise, sunset);
    //ESP_LOGD("set_pumping", "SUNRISE: %ld, SUNSET: %ld", aux, sunset_local);

    // First and last one and half hours of the day it must be on, due to ineficient pumping
    pump_on.append(comma_on + to_string(aux));
    pump_off.append(comma_off + to_string(aux + 5400));
    aux += 9000;
    comma_on = ", ";
    comma_off = ", ";

    while (aux < sunset_local - 10800) { // Three hours before sunset, to prevent overlapping
        pump_on.append(comma_on + to_string(aux));
        pump_off.append(comma_off + to_string(aux + mins * 60));
        aux += 9000;
    }
    pump_on.append(comma_on + to_string(sunset_local - 5400));
    pump_off.append(comma_off + to_string(sunset_local));

    pump_on = update_list(pump_on);
    pump_off = update_list(pump_off);
    id(pump_timestamps_on).publish_state(pump_on);
    id(pump_timestamps_off).publish_state(pump_off);
}

time_t get_sunrise() {
    auto sunrise = id(sun_sun).sunrise(0);
    if (!sunrise.has_value())
        return 0;
    return sunrise.value().timestamp;
}

time_t get_sunset() {
    auto sunset = id(sun_sun).sunset(0);
    if (!sunset.has_value())
        return 0;
    return sunset.value().timestamp;
}