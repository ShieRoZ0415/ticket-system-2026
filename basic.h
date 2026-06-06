#ifndef BASIC_HPP
#define BASIC_HPP

#include <iostream>
#include <string>
#include <cstdio>
#include <cmath>
#include <fstream>
#include <filesystem>
#include <cstring>

const int USER_LEN = 21;
const int PASS_LEN = 31;
const int NAME_LEN = 31;
const int MAIL_LEN = 31;
const int TRAIN_LEN = 21;
const int STA_LEN = 31;

const int MAX_STA = 100;
const int MAX_DAY = 92;
const int MAX_ONLINE = 20000;

inline int date_to_int(const char *s){
    int month = (s[0] - '0') * 10 + (s[1] - '0');
    int day = (s[3] - '0') * 10 + (s[4] - '0');

    int before = 0;

    if (month == 6) {
        before = 0;
    } else if (month == 7) {
        before = 30;
    } else {
        before = 61;
    }

    return before + day - 1;
}

inline void int_to_date(int d, char *s) {
    int month;
    int day;

    if (d < 30) {
        month = 6;
        day = d + 1;
    } else if (d < 61) {
        month = 7;
        day = d - 30 + 1;
    } else {
        month = 8;
        day = d - 61 + 1;
    }

    s[0] = char('0' + month / 10);
    s[1] = char('0' + month % 10);
    s[2] = '-';
    s[3] = char('0' + day / 10);
    s[4] = char('0' + day % 10);
    s[5] = '\0';
}


inline int time_to_int(const char *s) {
    int hour = (s[0] - '0') * 10 + (s[1] - '0');
    int minute = (s[3] - '0') * 10 + (s[4] - '0');

    return hour * 60 + minute;
}

inline void print_time(int minute) {
    int day = minute / 1440;
    int t = minute % 1440;

    int hour = t / 60;
    int min = t % 60;

    char date[8];
    int_to_date(day, date);

    std::cout << date << ' ';

    if (hour < 10) std::cout << '0';
    std::cout << hour << ':';

    if (min < 10) std::cout << '0';
    std::cout << min;
}

#endif