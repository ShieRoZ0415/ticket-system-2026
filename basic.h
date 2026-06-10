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

inline int to_int(const std::string &s) {
    int x = 0;
    for (int i = 0; i < (int)s.size(); ++i) {
        if (s[i] >= '0' && s[i] <= '9') {
            x = x * 10 + s[i] - '0';
        }
    }
    return x;
}

inline int date_to_int(const char *s) {
    int month = (s[0] - '0') * 10 + (s[1] - '0');
    int day = (s[3] - '0') * 10 + (s[4] - '0');

    static int pre[13] = {
        0, 0, 0, 0, 0, 0,
        0, 30, 61, 92, 122, 153, 183
    };

    if (month < 6) {
        return -100000;
    }

    if (month > 12) {
        return 100000;
    }

    return pre[month] + day - 1;
}

inline void int_to_date(int d, char *s) {   // 列车运行可能跨天到 9 月
    int month = 6;
    int days[7] = {30, 31, 31, 30, 31, 30, 31};

    int idx = 0;
    while (idx < 7 && d >= days[idx]) {
        d -= days[idx];
        ++idx;
        ++month;
    }

    int day = d + 1;

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

    char s[12];

    int month = 6;
    int days[7] = {30, 31, 31, 30, 31, 30, 31};
    int idx = 0;

    while (idx < 7 && day >= days[idx]) {
        day -= days[idx];
        ++idx;
        ++month;
    }

    int d = day + 1;

    s[0] = char('0' + month / 10);
    s[1] = char('0' + month % 10);
    s[2] = '-';
    s[3] = char('0' + d / 10);
    s[4] = char('0' + d % 10);
    s[5] = ' ';
    s[6] = char('0' + hour / 10);
    s[7] = char('0' + hour % 10);
    s[8] = ':';
    s[9] = char('0' + min / 10);
    s[10] = char('0' + min % 10);

    std::cout.write(s, 11);
}

#endif