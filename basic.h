#ifndef BASIC_HPP
#define BASIC_HPP

#include <iostream>
#include <string>
#include <cstdio>
#include <cmath>
#include <fstream>
#include <filesystem>

const int USER_LEN = 21;
const int PASS_LEN = 31;
const int NAME_LEN = 31;
const int MAIL_LEN = 31;
const int TRAIN_LEN = 21;
const int STA_LEN = 31;

const int MAX_STA = 100;
const int MAX_DAY = 92;
const int MAX_ONLINE = 20000;

int str_cmp(const char *a, const char *b);
void str_copy(char *to, const std::string &from, int limit);
bool str_equal(const char *a, const char *b);

int date_to_int(const char *s);
void int_to_date(int d, char *s);

int time_to_int(const char *s);
void print_time(int minute);

#endif