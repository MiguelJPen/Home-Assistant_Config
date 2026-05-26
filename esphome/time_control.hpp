#include <iostream>
#include <ctime>
#include <vector>
#include <string>
#include <clocale>
#include <cmath>
#include <algorithm>
#include <cstdlib>

#include "esphome.h"

using namespace std;

#define unknown "Sin programaciones"

string update_next_run(string);
string update_list(string);
string get_time_formated(string);
bool scheduled_run(string);

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
    time_t now_timestamp = id(time_sntp).now().timestamp;

    for (string timestamp : times) {
        time_t next_run_timestamp = (time_t) atol(timestamp.c_str());
        if (next_run_timestamp > now_timestamp) {
            ret.append(comma + timestamp);
            comma = ",";
        }
    }

    return ret;
}

string get_time_formated(string time) {
    if (!time.compare(unknown))
        return time;
    
    char date[100];
    time_t t = (time_t) atol(time.c_str());
    auto local_time = esphome::ESPTime::from_epoch_local(t);

    setlocale(LC_TIME, "es_ES-UTF_8");
    local_time.strftime(date, 100, "%a %b %d %R");

    return string(date);
}

bool scheduled_run(string time){
    if (!time.compare(unknown))
        return false;

    time_t now_timestamp = id(time_sntp).now().timestamp;
    time_t next_run_timestamp = (time_t) atol(time.c_str());

    return now_timestamp >= next_run_timestamp && now_timestamp < next_run_timestamp + 60;
}
