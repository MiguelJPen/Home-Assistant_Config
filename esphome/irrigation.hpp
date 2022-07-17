#include <iostream>
#include <ctime>
#include <string>
#include <clocale>
#include <cmath>
#include <algorithm>

#include "esphome.h"

using namespace std;

time_t add_day(time_t, int);
void set_new_irrigation_time();
void set_irrigation(time_t, int, int);
void set_pool_filling();
void set_more_pool_filling();
string manual_set(int, string);
string delete_first_timestamp(string);

void set_new_irrigation_time() {
    int time_per_day = 15, no_water_days = 0, freq_day = 1, next_from_today = 0, last_day = 0;
    float mean_tmp = id(mean_temp);
    float mean_hum = id(mean_humid);
    float min_hum = id(min_humid);
    float precip = id(precip_yesterday);
    time_t old_next_irrigation = atoi((id(next_day_irrigation).state).c_str());
    time_t last_irrigation = id(north_zone_last_automatic_irrigation);

    time_t now_timestamp = id(time_sntp).now().timestamp;
    double diff_secs = difftime(old_next_irrigation, now_timestamp);
    if (diff_secs > 0) no_water_days = (int) floor(diff_secs / (3600 * 24));

    //ESP_LOGD("set_new_irrigation_time", "DIFF SECS: %f, NO WATER DAYS: %i", diff_secs, no_water_days);

    id(north_zone_timestamps_on).publish_state("");
    id(north_zone_timestamps_off).publish_state("");
    id(south_zone_timestamps_on).publish_state("");
    id(south_zone_timestamps_off).publish_state("");


    // Set how many irrigation days
    if (precip >= 20) {
        if (mean_tmp <= 20)
            no_water_days = 4;
        else
            no_water_days = max(2, no_water_days);
    }
    else if (precip >= 7){
        if (mean_tmp <= 13)
            no_water_days = 4;
        else if (mean_tmp > 13 && mean_tmp <= 20)
            no_water_days = max(2, no_water_days);
        else no_water_days = max(1, no_water_days);
    }
    id(next_day_irrigation).publish_state(to_string(add_day(now_timestamp, no_water_days)));

    // Set the time per day
    if (mean_tmp <= 14) {
        freq_day = 3;
    }
    else if (mean_tmp > 14 && mean_tmp < 19) {
        time_per_day += 15;
        freq_day = 2;
    }
    else if (mean_tmp >= 19 && mean_tmp < 24) {
        time_per_day += 30;
        freq_day = 1;
    }
    else if (mean_tmp >= 24) {
        time_per_day += 45;
        freq_day = 1;
    }
    if (min_hum < 25 || mean_hum < 50)
        time_per_day += 20;
    else if (min_hum < 45)
        time_per_day += 15;

    // Set the timestamps for the irrigation
    if (no_water_days == 0) { // If it hasn't rained
        double diff_secs = difftime(now_timestamp, last_irrigation);
        if (diff_secs > 0) last_day = (int) floor(diff_secs / (3600 * 24));
        int new_day = max(freq_day - last_day - 1, 0);

        //ESP_LOGD("set_new_irrigation_time", "LAST DAY: %i, NEW DAY: %i, FREQ DAY: %i", last_day, new_day, freq_day);

        set_irrigation(now_timestamp, new_day, time_per_day);
        set_irrigation(now_timestamp, new_day + freq_day, time_per_day);
    }
    else {
        set_irrigation(now_timestamp, no_water_days, time_per_day);
        set_irrigation(now_timestamp, no_water_days + freq_day, time_per_day);
    }
}

void set_irrigation(time_t today, int days_ahead, int mins) {
    time_t new_irrigation_day = today + (days_ahead * 3600 * 24);
    struct tm new_time = *localtime(&new_irrigation_day);
    string north_on = id(north_zone_timestamps_on).state;
    string north_off = id(north_zone_timestamps_off).state;
    string south_on = id(south_zone_timestamps_on).state;
    string south_off = id(south_zone_timestamps_off).state;
    string comma_north_on = (north_on.empty() ? "" : ",");
    string comma_north_off = (north_off.empty() ? "" : ",");
    string comma_south_on = (south_on.empty() ? "" : ",");
    string comma_south_off = (south_off.empty() ? "" : ",");

    if (new_time.tm_mon % 2 == 0) new_time.tm_hour = 11;
    else new_time.tm_hour = 18;
    new_time.tm_min = 0;
    new_time.tm_sec = 0;

    time_t aux = mktime(&new_time);
    north_on.append(comma_north_on + to_string(aux));
    south_on.append(comma_south_on + to_string(aux + 2 * 60));
    north_off.append(comma_north_off + to_string(aux + mins * 60));
    south_off.append(comma_south_off + to_string(aux + (mins + 2) * 60));

    //ESP_LOGD("set_irrigation", "TIME PER DAY: %i", mins);

    north_on = update_list(north_on);
    north_off = update_list(north_off);
    south_on = update_list(south_on);
    south_off = update_list(south_off);
    id(north_zone_timestamps_on).publish_state(north_on);
    id(north_zone_timestamps_off).publish_state(north_off);
    id(south_zone_timestamps_on).publish_state(south_on);
    id(south_zone_timestamps_off).publish_state(south_off);
}

time_t add_day(time_t time, int days) {
    struct tm strtime = *localtime(&time);
    strtime.tm_hour = 12;
    strtime.tm_min = 0;
    strtime.tm_sec = 0;
    strtime.tm_mday += days;

    return mktime(&strtime);
}

void set_pool_filling() {
    time_t now_timestamp = id(time_sntp).now().timestamp;
    struct tm new_time = *localtime(&now_timestamp);

    if (new_time.tm_mon % 2 == 0 && new_time.tm_hour < 15) new_time.tm_hour = 15;
    else if (new_time.tm_mon % 2 == 0) {
        new_time.tm_mday++;
        time_t aux = mktime(&new_time);
        new_time = *localtime(&aux);

        if (new_time.tm_mon % 2 == 0) new_time.tm_hour = 15;
        else new_time.tm_hour = 23;
    }
    else if (new_time.tm_hour < 23) new_time.tm_hour = 23;
    else {
        new_time.tm_mday++;
        time_t aux = mktime(&new_time);
        new_time = *localtime(&aux);

        if (new_time.tm_mon % 2 != 0) new_time.tm_hour = 23;
        else new_time.tm_hour = 15;
    }
    new_time.tm_min = 0;
    new_time.tm_sec = 0;

    time_t aux = mktime(&new_time);
    id(pool_filling_timestamps_on).publish_state(to_string(aux));
    id(pool_filling_timestamps_off).publish_state(to_string(aux + 3 * 60));
}

void set_more_pool_filling() {
    time_t now_timestamp = id(time_sntp).now().timestamp;
    struct tm new_time = *localtime(&now_timestamp);

    new_time.tm_sec = 0;
    time_t aux = mktime(&new_time);
    id(pool_filling_timestamps_off).publish_state(to_string(aux + 3 * 60));
}

string manual_set(int duration, string time_list) {
    time_t now_timestamp = id(time_sntp).now().timestamp;
    struct tm now_time = *localtime(&now_timestamp);
    string comma = (time_list.empty() ? "" : ",");

    now_time.tm_sec = 0;
    time_t aux = mktime(&now_time);

    time_list.insert(0, to_string((long int)(aux + ((duration - 1) * 60))) + comma);

    return time_list;
}

string delete_first_timestamp(string time_list) {
    size_t pos = time_list.find(",");

    //ESP_LOGD("delete_first_timestamp", "POS: %zu, NPOS: %zu", pos, string::npos);

    if (pos == string::npos) time_list = "";
    else time_list.erase(0, pos + 1); 
        
    //ESP_LOGD("delete_first_timestamp", "TIMELIST: %s", time_list.c_str());

    return time_list;
}