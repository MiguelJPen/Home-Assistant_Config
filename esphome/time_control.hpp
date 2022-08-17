#include <iostream>
#include <ctime>
#include <vector>
#include <string>
#include <clocale>
#include <cmath>
#include <algorithm>

#include "esphome.h"

using namespace std;

#define unknown "Sin programaciones"

string update_next_run(string);
string update_list(string);
string get_time_formated(string);
bool scheduled_run(string);
bool water_available();

string update_next_run(string time_list) {
    if (time_list.empty()) return unknown;

    // Split the list into a vector.
    char * token = strtok(&time_list[0], ",");
    return string(token);
}

string update_list(string time_list){
    vector<string> times;
    string ret = "", comma = "";

    if (time_list.empty()) return "";

    // Split the list into a vector.
    char * token = strtok(&time_list[0], ",");
    while (token != NULL) {
        times.push_back(token);
        token = strtok(NULL, ",");
    }

    // Retrieve the current time.
    auto time_now = id(time_sntp).now();
    int year = time_now.year, month = time_now.month, day = time_now.day_of_month;
    int hour = time_now.hour, minute = time_now.minute;

    for (string timestamp : times) {
        time_t next_run_aux = atoi(timestamp.c_str());
        struct tm next_run = *localtime(&next_run_aux);

        int next_year = next_run.tm_year + 1900, next_month = next_run.tm_mon + 1, next_day = next_run.tm_mday;
        int next_hour = next_run.tm_hour, next_minute = next_run.tm_min;

        if (year < next_year || (year == next_year && (month < next_month || (month == next_month && (day < next_day || (day == next_day && (hour < next_hour || (hour == next_hour && (minute < next_minute))))))))) {
            ret.append(comma + to_string(timestamp));
            comma = ",";
        }
    }

    return ret;
}

string get_time_formated(string time) {
    if (!time.compare(unknown))
        return time;
    
    char date[100];
    time_t t = atoi(time.c_str());

    setlocale(LC_TIME, "es_ES-UTF_8");
    strftime(date, 100, "%a %b %d %R", localtime(&t));

    return to_string(date);
}

bool scheduled_run(string time){
    if (!time.compare(unknown))
        return false;

    auto time_now = id(time_sntp).now();
    int year = time_now.year;
    int month = time_now.month;
    int day = time_now.day_of_month;
    int hour = time_now.hour;
    int minute = time_now.minute;

    time_t next_run_aux = atoi(time.c_str());
    struct tm next_run = *localtime(&next_run_aux);

    int next_year = next_run.tm_year + 1900, next_month = next_run.tm_mon + 1, next_day = next_run.tm_mday;
    int next_hour = next_run.tm_hour, next_minute = next_run.tm_min;

    //ESP_LOGD("scheduled_run", "TODAY: year:%i month:%i day:%i hour:%i minute:%i", year, month, day, hour, minute);
    //ESP_LOGD("scheduled_run", "year:%i month:%i day:%i hour:%i minute:%i", next_year, next_month, next_day, next_hour, next_minute);

    return (year == next_year && month == next_month && day == next_day && hour == next_hour && minute == next_minute);
}

bool water_available() {
    auto time_now = id(time_sntp).now();

    if (time_now.month % 2 == 0 && time_now.hour >= 16){ // Water in the afternoon (16-00)
        return true;
    }
    else if (time_now.month % 2 == 1 && time_now.hour >= 8 && time_now.hour < 16){ // Water in the morning (08-16)
        return true;
    }
    return false;
}