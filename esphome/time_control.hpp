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
time_t timestamp_from_string(string);
pair<int, int> parse_hh_mm(string);
int minutes_until(string, time_t);
bool timestamp_after(string, string);

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
        time_t next_run_timestamp = timestamp_from_string(timestamp);
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
    time_t t = timestamp_from_string(time);
    auto local_time = esphome::ESPTime::from_epoch_local(t);

    setlocale(LC_TIME, "es_ES-UTF_8");
    local_time.strftime(date, 100, "%a %b %d %R");

    return string(date);
}

bool scheduled_run(string time){
    if (!time.compare(unknown))
        return false;

    time_t now_timestamp = id(time_sntp).now().timestamp;
    time_t next_run_timestamp = timestamp_from_string(time);

    return now_timestamp >= next_run_timestamp && now_timestamp < next_run_timestamp + 60;
}

time_t timestamp_from_string(string timestamp) {
    if (timestamp.empty() || !timestamp.compare(unknown))
        return 0;

    return (time_t) atol(timestamp.c_str());
}

pair<int, int> parse_hh_mm(string time) {
    size_t separator = time.find(":");
    if (separator == string::npos)
        return make_pair(0, 0);

    int hour = atoi(time.substr(0, separator).c_str());
    int minute = atoi(time.substr(separator + 1).c_str());
    return make_pair(hour, minute);
}

int minutes_until(string timestamp, time_t now_timestamp) {
    return ((timestamp_from_string(timestamp) - now_timestamp) / 60) + 1;
}

bool timestamp_after(string lhs, string rhs) {
    return timestamp_from_string(lhs) > timestamp_from_string(rhs);
}
