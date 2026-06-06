#ifndef RECORD_HPP
#define RECORD_HPP

#include "basic.h"
#include <cstring>

struct UserRec {
    char username[USER_LEN];
    char password[PASS_LEN];
    char name[NAME_LEN];
    char mail[MAIL_LEN];
    int privilege;
};

struct TrainRec {
    char trainID[TRAIN_LEN];

    int stationNum;
    int seatNum;

    char stations[MAX_STA][STA_LEN];

    int price[MAX_STA];
    int travel[MAX_STA];
    int stopover[MAX_STA];

    int startTime;
    int saleL;
    int saleR;

    char type;
    int released;

    int arrOffset[MAX_STA];
    int depOffset[MAX_STA];
    int prePrice[MAX_STA];

    int seatPos;
};

struct SeatRec {
    int seat[MAX_DAY][MAX_STA];
};

enum OrderStatus {
    success = 1,
    pending = 2,
    refunded = 3
};

struct OrderRec {
    int status;
    int time;

    char username[USER_LEN];
    char trainID[TRAIN_LEN];

    char from[STA_LEN];
    char to[STA_LEN];

    int trainDate;
    int leaveTime;
    int arriveTime;

    int fromID;
    int toID;

    int price;
    int num;
};

struct UserKey {
    char username[USER_LEN];
    int pos;

    bool operator<(const UserKey &o) const {
        int c = strcmp(username, o.username);
        if (c != 0) return c < 0;
        return pos < o.pos;
    }
};

struct TrainKey {
    char trainID[TRAIN_LEN];
    int pos;

    bool operator<(const TrainKey &o) const {
        int c = strcmp(trainID, o.trainID);
        if (c != 0) return c < 0;
        return pos < o.pos;
    }
};

struct StaKey {
    char station[STA_LEN];
    char trainID[TRAIN_LEN];
    int pos;

    bool operator<(const StaKey &o) const {
        int c = strcmp(station, o.station);
        if (c != 0) return c < 0;

        c = strcmp(trainID, o.trainID);
        if (c != 0) return c < 0;

        return pos < o.pos;
    }
};

struct OrderKey {
    char username[USER_LEN];
    int revTime;
    int pos;

    bool operator<(const OrderKey &o) const {
        int c = strcmp(username, o.username);
        if (c != 0) return c < 0;

        if (revTime != o.revTime) return revTime < o.revTime;
        return pos < o.pos;
    }
};

struct QueueKey {
    char trainID[TRAIN_LEN];
    int trainDate;
    int time;
    int pos;

    bool operator<(const QueueKey &o) const {
        int c = strcmp(trainID, o.trainID);
        if (c != 0) return c < 0;

        if (trainDate != o.trainDate) return trainDate < o.trainDate;
        if (time != o.time) return time < o.time;
        return pos < o.pos;
    }
};

#endif