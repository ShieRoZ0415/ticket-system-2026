#ifndef TICKET_HPP
#define TICKET_HPP

#include "user.hpp"
#include "train.hpp"
#include "order.hpp"

struct Ticket {
    char trainID[TRAIN_LEN];
    char from[STA_LEN];
    char to[STA_LEN];

    int leaveTime;
    int arriveTime;
    int price;
    int seat;

    int trainDate;
    int fromID;
    int toID;
};

class TicketSys {
private:
    UserSys *user;
    TrainSys *train;
    OrderSys *order;

    int get_origin_day(const TrainRec &t, int stationID, int queryDay);
    int calc_seat(const TrainRec &t, int trainDay, int l, int r);
    void add_seat(const TrainRec &t, int trainDay, int l, int r, int num);
    void sub_seat(const TrainRec &t, int trainDay, int l, int r, int num);

    bool make_ticket(const TrainRec &t,
                     int fromID,
                     int toID,
                     int queryDay,
                     Ticket &ans);

    bool better_transfer(const Ticket &a1,
                         const Ticket &a2,
                         const Ticket &b1,
                         const Ticket &b2,
                         const std::string &sortType);

    void check_queue(const char *trainID, int trainDay);

public:
    TicketSys(UserSys *u, TrainSys *t, OrderSys *o);

    int query_ticket(const std::string &from,
                     const std::string &to,
                     const std::string &date,
                     const std::string &sortType);

    int query_transfer(const std::string &from,
                       const std::string &to,
                       const std::string &date,
                       const std::string &sortType);

    int buy_ticket(const std::string &username,
                   const std::string &trainID,
                   const std::string &date,
                   int num,
                   const std::string &from,
                   const std::string &to,
                   bool queue);

    int refund_ticket(const std::string &username,
                      int nth);

    void clear();
};

#endif