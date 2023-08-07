#include <iostream>
#include <ctime>
#include <string>
#include <clocale>
#include <cmath>
#include <algorithm>

#include "esphome.h"

#define DAY_SECS 86400
#define CADUCA "Hoja caduca"
#define PERENNE "Hoja perenne"
#define FRUTALES "Frutales"

using namespace std;

pair<string, string> get_irrigation_time(time_t, time_t, float, float, string, int, int, int, int);
void set_time_at(time_t &, int, int, int, int); // Internal
time_t get_next_irrigation_day(time_t, time_t, float, float, string);
time_t get_next_time_at(int, int, int, int);
string add_two_minutes_from_now();
string manual_set(int, string);
string delete_first_timestamp(string);
time_t add_n_days(time_t, int); // Internal

pair<string, string> get_irrigation_time(time_t next_irrigation, time_t last_auto_irrigation, float mean_tmp, float mean_hum, float min_hum, string irrigation_mode, 
int h_morn, int m_morn, int h_afte, int m_afte) {
    time_t today_timestamp = id(time_sntp).now().timestamp;
    int irrigation_frequency = 1, mins = 20;
    int days_to_next_irrigation = max(0, (int) floor(difftime(next_irrigation, today_timestamp) / DAY_SECS));
    int days_since_last_irrigation = max(0, (int) floor(difftime(today_timestamp, last_auto_irrigation) / DAY_SECS) - 1);
    string turn_on = "", turn_off = "", comma = ",";

    if (irrigation_mode.compare(CADUCA) == 0 || irrigation_mode.compare(PERENNE) == 0) {
        mins = max(mins, (int) (5.5 * mean_tmp - 15));

        if (mean_tmp <= 10) irrigation_frequency = 5;
        else if (mean_tmp <= 13) irrigation_frequency = 4;
        else if (mean_tmp <= 16) irrigation_frequency = 3;
        else if (mean_tmp <= 19) irrigation_frequency = 2;
    }
    else if (irrigation_mode.compare(FRUTALES) == 0) {
        mins = max(mins, (int) (7 * mean_tmp - 10));

        if (mean_tmp <= 10) irrigation_frequency = 3;
        else if(mean_tmp <= 14) irrigation_frequency = 2;
    }

    if (min_hum < 25 || mean_hum < 45) mins += 25;
    else if (min_hum < 45) mins += 15;

    time_t irrigation_timestamp = max(today_timestamp, max(today_timestamp + (days_to_next_irrigation * DAY_SECS), 
                                        today_timestamp + ((irrigation_frequency - days_since_last_irrigation) * DAY_SECS)));
    set_time_at(irrigation_timestamp, h_morn, m_morn, h_afte, m_afte);
    turn_on.append(to_string(irrigation_timestamp));
    turn_off.append(to_string(irrigation_timestamp + (mins * 60)));

    irrigation_timestamp += irrigation_frequency * DAY_SECS;
    set_time_at(irrigation_timestamp, h_morn, m_morn, h_afte, m_afte);
    turn_on.append(comma + to_string(irrigation_timestamp));
    turn_off.append(comma + to_string(irrigation_timestamp + (mins * 60)));

    turn_on = update_list(turn_on);
    turn_off = update_list(turn_off);

    return make_pair(turn_on, turn_off);
}

void set_time_at(time_t &day, int h_morn, int m_morn, int h_afte, int m_afte) {
    struct tm new_time = *localtime(&day);

    if (new_time.tm_mon % 2 == 0) {
        new_time.tm_hour = h_morn;
        new_time.tm_min = m_morn;
    }
    else {
        new_time.tm_hour = h_afte;
        new_time.tm_min = m_afte;
    }
    new_time.tm_sec = 0;

    day = mktime(&new_time);
}

time_t get_next_irrigation_day(time_t old_next_irrigation, time_t last_auto_irrigation, float precipitation, float mean_tmp, string irrigation_mode) {
    /* This function is executed every day to check if there has been a massive rainfall to 
    adjust the irrigation days and prevent water waste and flooding the roots. */
    time_t today_timestamp = id(time_sntp).now().timestamp;
    struct tm now_time = *localtime(&today_timestamp);
    int no_water_days = max(0, (int) floor(difftime(old_next_irrigation, today_timestamp) / DAY_SECS));

    // Set a fixed hour, we want to set the day
    now_time.tm_hour = 12;
    now_time.tm_min = 0;
    now_time.tm_sec = 0;
    today_timestamp = mktime(&now_time);

    //ESP_LOGD("get_next_irrigation_day", "Option of my select: %s", irrigation_mode.c_str());

    if(irrigation_mode.compare(CADUCA) == 0) {
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
    else if(irrigation_mode.compare(PERENNE) == 0) {
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
    else if(irrigation_mode.compare(FRUTALES) == 0) {
        if (precipitation >= 10) {
            if (mean_tmp < 18) no_water_days = max(no_water_days, max(3, int(((precipitation + 10) / 10))));
            else if (mean_tmp < 25) no_water_days = max(no_water_days, max(2, int((precipitation / 20))));
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

string add_two_minutes_from_now() {
    time_t now_timestamp = id(time_sntp).now().timestamp;
    struct tm new_time = *localtime(&now_timestamp);

    new_time.tm_sec = 0;
    time_t aux = mktime(&new_time);
    return to_string(aux + 2 * 60);
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
