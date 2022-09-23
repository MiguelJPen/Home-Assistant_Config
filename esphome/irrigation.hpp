#include <iostream>
#include <ctime>
#include <string>
#include <clocale>
#include <cmath>
#include <algorithm>

#include "esphome.h"

using namespace std;

void set_new_irrigation_time();
void set_irrigation(time_t, int, int);

pair<string, string> get_irrigation_time(time_t, time_t, string);
time_t get_next_irrigation_day(time_t, time_t, float, float, string);
time_t get_next_time_at(int, int, int, int);
string add_three_minutes_from_now();
string manual_set(int, string);
string delete_first_timestamp(string);
time_t add_n_days(time_t, int); // Internal

void set_new_irrigation_time() {
    int time_per_day = 15, no_water_days = 0, freq_day = 1, next_from_today = 0, last_day = 0;
    float mean_tmp = id(mean_temp);
    float mean_hum = id(mean_humid);
    float min_hum = id(min_humid);
    float precip = id(precip_yesterday);
    //time_t old_next_irrigation = atoi((id(north_next_irrigation_day).state).c_str());
    time_t last_irrigation = id(north_last_automatic_execution);

    time_t now_timestamp = id(time_sntp).now().timestamp;
    //double diff_secs = difftime(old_next_irrigation, now_timestamp);
    //if (diff_secs > 0) no_water_days = (int) floor(diff_secs / (3600 * 24));

    //ESP_LOGD("set_new_irrigation_time", "DIFF SECS: %f, NO WATER DAYS: %i", diff_secs, no_water_days);

    /*id(north_timestamps_on).publish_state("");
    id(north_timestamps_off).publish_state("");
    id(south_timestamps_on).publish_state("");
    id(south_timestamps_off).publish_state("");*/


    // Set how many irrigation days
    /*if (precip >= 20) {
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
    }*/
    //id(north_next_irrigation_day).publish_state(to_string(add_n_days(now_timestamp, no_water_days)));

    // Set the time per day
    time_per_day += (int) (pow(2, 3.2 + max((float)0, (mean_tmp - 10)/5)));

    if (mean_tmp <= 14) {
        freq_day = 3;
    }
    else if (mean_tmp > 14 && mean_tmp < 19) {
        freq_day = 2;
    }
    else if (mean_tmp >= 19) {
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
    string north_on = id(north_timestamps_on).state;
    string north_off = id(north_timestamps_off).state;
    string south_on = id(south_timestamps_on).state;
    string south_off = id(south_timestamps_off).state;
    string comma_north_on = (north_on.empty() ? "" : ",");
    string comma_north_off = (north_off.empty() ? "" : ",");
    string comma_south_on = (south_on.empty() ? "" : ",");
    string comma_south_off = (south_off.empty() ? "" : ",");

    if (new_time.tm_mon % 2 == 0) new_time.tm_hour = 9;
    else new_time.tm_hour = 17;
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
    id(north_timestamps_on).publish_state(north_on);
    id(north_timestamps_off).publish_state(north_off);
    id(south_timestamps_on).publish_state(south_on);
    id(south_timestamps_off).publish_state(south_off);
}

pair<string, string> get_irrigation_time(time_t next_irrigation, time_t last_auto_irrigation, float mean_hum, float mean_tmp, string irrigation_mode, 
int h_morn, int m_morn, int h_afte, int m_afte) {
    time_t today_timestamp = id(time_sntp).now().timestamp;
    int irrigation_frequency = 1;
    int days_to_next_irrigation = max(0, (int) floor(difftime(today_timestamp, next_irrigation) / (60 * 60 * 24)));
    int days_since_last_irrigation = max(0, (int) floor(difftime(today_timestamp, last_auto_irrigation) / (60 * 60 * 24)));
    string turn_on = "", turn_off = "";

    if (irrigation_mode.compare("Hoja caduca")) {
        
    }
    else if (irrigation_mode.compare("Hoja perenne")) {

    }
    else if (irrigation_mode.compare("Frutales")) {

    }

    return {"", ""};
}

time_t get_next_irrigation_day(time_t old_next_irrigation, time_t last_auto_irrigation, float precipitation, float mean_tmp, string irrigation_mode) {
    /* This function is executed every day to check if there has been a massive rainfall to 
    adjust the irrigation days and prevent water waste and flooding the roots. */
    time_t today_timestamp = id(time_sntp).now().timestamp;
    struct tm now_time = *localtime(&today_timestamp);
    int no_water_days = max(0, (int) floor(difftime(old_next_irrigation, today_timestamp) / (60 * 60 * 24)));

    // Set a fixed hour, we want to set the day
    now_time.tm_hour = 12;
    now_time.tm_min = 0;
    now_time.tm_sec = 0;
    today_timestamp = mktime(&now_time);

    if(irrigation_mode.compare("Hoja caduca")) {
        // No irrigation during winter
        if (now_time.tm_mon > 10) {
            now_time.tm_year++;
            now_time.tm_mon = 2;
            now_time.tm_mday = 1;

            time_t aux = mktime(&now_time);
            return aux;
        }
        else if (now_time.tm_mon < 2) {
            now_time.tm_mon = 2;
            now_time.tm_mday = 1;

            time_t aux = mktime(&now_time);
            return aux;
        }
        // Rest of the year
        else {
            if (precipitation > 5) {
                if (mean_tmp < 15) no_water_days = max(no_water_days, max(5, int(precipitation / 5)));
                else if (mean_tmp < 20) no_water_days = max(no_water_days, max(4, int(precipitation / 7)));
                else if (mean_tmp < 25) no_water_days = max(no_water_days, max(3, int(precipitation / 10)));
                else no_water_days = max(no_water_days, max(2, int(precipitation / 15)));
            }
        }
    }
    else if(irrigation_mode.compare("Hoja perenne")) {
        if (now_time.tm_mon < 2 || now_time.tm_mon > 10) {
            if (precipitation > 5) no_water_days = max(no_water_days, max(5, int(precipitation / 5)));
        }
        else {
            if (precipitation > 5) {
                if (mean_tmp < 15) no_water_days = max(no_water_days, max(5, int(precipitation / 5)));
                else if (mean_tmp < 20) no_water_days = max(no_water_days, max(4, int(precipitation / 7)));
                else if (mean_tmp < 25) no_water_days = max(no_water_days, max(3, int(precipitation / 10)));
                else no_water_days = max(no_water_days, max(2, int(precipitation / 15)));
            }
        }
    }
    else if(irrigation_mode.compare("Frutales")) {
        if (precipitation >= 10) {
            if (mean_tmp < 18) no_water_days = max(no_water_days, max(3, int(((precipitation + 10) / 10))));
            else if (mean_tmp < 25) no_water_days = max(no_water_days, max(2, int((precipitation / 15))));
            else no_water_days = max(no_water_days, 1);
        }
    }
    
    return add_n_days(today_timestamp, no_water_days);
}

time_t get_next_time_at(int h_morn, int m_morn, int h_afte, int m_afte) {
    time_t now_timestamp = id(time_sntp).now().timestamp;
    struct tm new_time = *localtime(&now_timestamp);

    //ESP_LOGD("get_next_time_at", "HM: %i, MM: %i, HA %i, MA %i", h_morn, m_morn, h_afte, m_afte);

    if (new_time.tm_mon % 2 == 0 && new_time.tm_hour < h_morn) {
        new_time.tm_hour = h_morn;
        new_time.tm_min = m_morn;
    }
    else if (new_time.tm_mon % 2 == 0) {
        new_time.tm_mday++;
        time_t aux = mktime(&new_time);
        new_time = *localtime(&aux);

        if (new_time.tm_mon % 2 == 0) {
            new_time.tm_hour = h_morn;
            new_time.tm_min = m_morn;
        }
        else {
            new_time.tm_hour = h_afte;
            new_time.tm_min = m_afte;
        }
    }
    else if (new_time.tm_hour < h_afte) {
        new_time.tm_hour = h_afte;
        new_time.tm_min = m_afte;
    }
    else {
        new_time.tm_mday++;
        time_t aux = mktime(&new_time);
        new_time = *localtime(&aux);

        if (new_time.tm_mon % 2 != 0) {
            new_time.tm_hour = h_afte;
            new_time.tm_min = m_afte;
        }
        else {
            new_time.tm_hour = h_morn;
            new_time.tm_min = m_morn;
        }
    }
    new_time.tm_sec = 0;

    time_t aux = mktime(&new_time);

    //ESP_LOGD("get_next_time_at", "Time aux: %ld", aux);

    return aux;
}

string add_three_minutes_from_now() {
    time_t now_timestamp = id(time_sntp).now().timestamp;
    struct tm new_time = *localtime(&now_timestamp);

    new_time.tm_sec = 0;
    time_t aux = mktime(&new_time);
    return to_string(aux + 3 * 60);
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

time_t add_n_days(time_t time, int days) {
    struct tm strtime = *localtime(&time);
    strtime.tm_mday += days;

    return mktime(&strtime);
}
