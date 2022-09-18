#include <iostream>
#include <ctime>
#include <string>
#include <clocale>
#include <cmath>
#include <algorithm>
#include <utility>

#include "esphome.h"

using namespace std;

/* 
Need to have in packages:
- common/time.yaml
- common/sun.yaml
*/

pair<string,string> get_automatic_pumping_time(float);
void set_automatic_day_pumping(string&, string&, time_t, struct tm, struct tm, int); // Internal
pair<string,string> get_automatic_full_pumping_time();
void set_automatic_full_day_pumping(string&, string&, time_t, struct tm, struct tm); // Internal
pair<string,string> get_winter_pumping_time(int);
void set_winter_day_pumping(string&, string&, time_t, int); // Internal
time_t get_sunrise(); // Internal
time_t get_sunset(); // Internal

pair<string,string> get_automatic_pumping_time(float mean_tmp) {
    int mins = 30 + (mean_tmp * 1.5); // Cycles are every 2 and a half hours
    time_t now_timestamp = id(time_sntp).now().timestamp;
    time_t tomorrow_timestamp = now_timestamp + (3600 * 24);
    time_t sunrise = get_sunrise(), sunset = get_sunset();
    struct tm sunrise_st = *localtime(&sunrise), sunset_st = *localtime(&sunset);
    string pump_on = "", pump_off = "";

    set_automatic_day_pumping(pump_on, pump_off, now_timestamp, sunrise_st, sunset_st, mins);
    set_automatic_day_pumping(pump_on, pump_off, tomorrow_timestamp, sunrise_st, sunset_st, mins);

    pump_on = update_list(pump_on);
    pump_off = update_list(pump_off);

    return make_pair(pump_on, pump_off);
}

void set_automatic_day_pumping(string &pump_on, string &pump_off, time_t timestamp, struct tm sunrise_st, struct tm sunset_st, int mins) {
    struct tm new_time = *localtime(&timestamp);
    string comma = (pump_on.empty()? "" : ",");

    //ESP_LOGD("set_automatic_day_pumping", "pump_on: %s, comma: %s", pump_on, comma);

    new_time.tm_hour = sunrise_st.tm_hour;
    new_time.tm_min = sunrise_st.tm_min;
    new_time.tm_sec = 0;
    time_t aux = mktime(&new_time);
    new_time.tm_hour = sunset_st.tm_hour;
    new_time.tm_min = sunset_st.tm_min;
    time_t sunset_local = mktime(&new_time);

    // First and last 1.5 hours of the day it must be on, due to ineficient pumping
    pump_on.append(comma + to_string(aux));
    pump_off.append(comma + to_string(aux + 5400));
    comma = ",";
    aux += 8400;

    while (aux + mins * 60 < sunset_local - 5400) { // To prevent overlapping (max time stopped: 50 mins)
        pump_on.append(comma + to_string(aux));
        pump_off.append(comma + to_string(aux + mins * 60));
        aux += mins * 60 + 3000; // Turn ON every 50 mins to clean the surface
    }
    pump_on.append(comma + to_string(sunset_local - 5400));
    pump_off.append(comma + to_string(sunset_local));
}

pair<string,string> get_automatic_full_pumping_time() {
    time_t now_timestamp = id(time_sntp).now().timestamp;
    time_t tomorrow_timestamp = now_timestamp + (3600 * 24);
    time_t sunrise = get_sunrise(), sunset = get_sunset();
    struct tm sunrise_st = *localtime(&sunrise), sunset_st = *localtime(&sunset);
    string pump_on = "", pump_off = "";

    set_automatic_full_day_pumping(pump_on, pump_off, now_timestamp, sunrise_st, sunset_st);
    set_automatic_full_day_pumping(pump_on, pump_off, tomorrow_timestamp, sunrise_st, sunset_st);

    pump_on = update_list(pump_on);
    pump_off = update_list(pump_off);

    return make_pair(pump_on, pump_off);
}

void set_automatic_full_day_pumping(string &pump_on, string &pump_off, time_t timestamp, struct tm sunrise_st, struct tm sunset_st) {
    struct tm new_time = *localtime(&timestamp);
    string comma = (pump_on.empty()? "" : ",");

    new_time.tm_hour = sunrise_st.tm_hour;
    new_time.tm_min = sunrise_st.tm_min;
    new_time.tm_sec = 0;
    time_t sunrise_local = mktime(&new_time);
    new_time.tm_hour = sunset_st.tm_hour;
    new_time.tm_min = sunset_st.tm_min;
    time_t sunset_local = mktime(&new_time);

    pump_on.append(comma + to_string(sunrise_local));
    pump_off.append(comma + to_string(sunset_local));
}

pair<string,string> get_winter_pumping_time(int mins) {
    time_t now_timestamp = id(time_sntp).now().timestamp;
    time_t tomorrow_timestamp = now_timestamp + (3600 * 24);
    string pump_on = "", pump_off = "";

    set_winter_day_pumping(pump_on, pump_off, now_timestamp, mins);
    set_winter_day_pumping(pump_on, pump_off, tomorrow_timestamp, mins);

    pump_on = update_list(pump_on);
    pump_off = update_list(pump_off);

    return make_pair(pump_on, pump_off);
}

void set_winter_day_pumping(string &pump_on, string &pump_off, time_t timestamp, int mins) {
    struct tm new_time = *localtime(&timestamp);
    string comma = (pump_on.empty()? "" : ",");

    new_time.tm_hour = 13;
    new_time.tm_min = 0;
    new_time.tm_sec = 0;
    time_t aux = mktime(&new_time);

    pump_on.append(comma + to_string(aux));
    pump_off.append(comma + to_string(aux + mins * 60));
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