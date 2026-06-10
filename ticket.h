#ifndef TICKET_HPP
#define TICKET_HPP

#include "user.h"
#include "train.h"
#include "order.h"

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
    UserSys* user;
    TrainSys* train;
    OrderSys* order;

    int get_origin_day(const TrainRec& t, int stationID, int queryDay) {
        int leaveOffset = t.startTime + t.depOffset[stationID];
        return queryDay - leaveOffset / 1440;
    }

    int calc_seat(const TrainRec& t, int trainDay, int l, int r) {
        SeatRec s;

        if (!train->get_seat(t, trainDay, s)) {
            return -1;
        }

        int ans = t.seatNum;

        for (int i = l; i < r; ++i) {
            // 余票区间为 (l, r)
            if (s.seat[i] < ans) {
                ans = s.seat[i];
            }
        }

        return ans;
    }

    void add_seat(const TrainRec& t, int trainDay, int l, int r, int num) {
        SeatRec s;

        if (!train->get_seat(t, trainDay, s)) {
            return;
        }

        for (int i = l; i < r; ++i) {
            // 余票区间为 (l, r)
            s.seat[i] += num;
        }

        train->write_seat(t, trainDay, s);
    }

    void sub_seat(const TrainRec& t, int trainDay, int l, int r, int num) {
        SeatRec s;

        if (!train->get_seat(t, trainDay, s)) {
            return;
        }

        for (int i = l; i < r; ++i) {
            // 余票区间为 (l, r)
            s.seat[i] -= num;
        }

        train->write_seat(t, trainDay, s);
    }

    bool make_ticket(const TrainRec& t,
                     int fromID,
                     int toID,
                     int queryDay,
                     Ticket& ans) {
        if (!t.released) {
            return false;
        }

        if (fromID < 0 || toID < 0 || fromID >= toID) {
            return false;
        }

        int trainDay = get_origin_day(t, fromID, queryDay);

        if (trainDay < t.saleL || trainDay > t.saleR) {
            return false;
        }

        int seat = calc_seat(t, trainDay, fromID, toID);

        if (seat < 0) {
            return false;
        }

        std::strcpy(ans.trainID, t.trainID);
        std::strcpy(ans.from, t.stations[fromID]);
        std::strcpy(ans.to, t.stations[toID]);

        ans.trainDate = trainDay;
        ans.fromID = fromID;
        ans.toID = toID;

        ans.leaveTime = trainDay * 1440 + t.startTime + t.depOffset[fromID];
        ans.arriveTime = trainDay * 1440 + t.startTime + t.arrOffset[toID];
        ans.price = t.prePrice[toID] - t.prePrice[fromID];
        ans.seat = seat;

        return true;
    }

    bool better_ticket(const Ticket& a,
                       const Ticket& b,
                       const std::string& sortType) {
        if (sortType == "time") {
            int ta = a.arriveTime - a.leaveTime;
            int tb = b.arriveTime - b.leaveTime;

            if (ta != tb) {
                return ta < tb;
            }

            return std::strcmp(a.trainID, b.trainID) < 0;
        }
        else {
            if (a.price != b.price) {
                return a.price < b.price;
            }

            return std::strcmp(a.trainID, b.trainID) < 0;
        }
    }

    void swap_ticket(Ticket& a, Ticket& b) {
        Ticket c = a;
        a = b;
        b = c;
    }

    void sort_ticket(Ticket a[],
                     int l,
                     int r,
                     const std::string& sortType) {
        int i = l;
        int j = r;
        Ticket mid = a[(l + r) >> 1];

        while (i <= j) {
            while (better_ticket(a[i], mid, sortType)) {
                ++i;
            }

            while (better_ticket(mid, a[j], sortType)) {
                --j;
            }

            if (i <= j) {
                swap_ticket(a[i], a[j]);
                ++i;
                --j;
            }
        }

        if (l < j) {
            sort_ticket(a, l, j, sortType);
        }

        if (i < r) {
            sort_ticket(a, i, r, sortType);
        }
    }

    void print_ticket(const Ticket& x) {
        std::cout << x.trainID << ' '
            << x.from << ' ';

        print_time(x.leaveTime);

        std::cout << " -> "
            << x.to << ' ';

        print_time(x.arriveTime);

        std::cout << ' '
            << x.price << ' '
            << x.seat << '\n';
    }


    bool better_transfer(const Ticket& a1,
                         const Ticket& a2,
                         const Ticket& b1,
                         const Ticket& b2,
                         const std::string& sortType) {
        if (sortType == "cost") {
            int ca = a1.price + a2.price;
            int cb = b1.price + b2.price;

            if (ca != cb) {
                return ca < cb;
            }

            int ta = a2.arriveTime - a1.leaveTime;
            int tb = b2.arriveTime - b1.leaveTime;

            if (ta != tb) {
                return ta < tb;
            }

            int c = std::strcmp(a1.trainID, b1.trainID);
            if (c != 0) {
                return c > 0;
            }

            return std::strcmp(a2.trainID, b2.trainID) > 0;
        }

        int ta = a2.arriveTime - a1.leaveTime;
        int tb = b2.arriveTime - b1.leaveTime;

        if (ta != tb) {
            return ta < tb;
        }

        int ca = a1.price + a2.price;
        int cb = b1.price + b2.price;

        if (ca != cb) {
            return ca < cb;
        }

        int c = std::strcmp(a1.trainID, b1.trainID);
        if (c != 0) {
            return c > 0;
        }

        return std::strcmp(a2.trainID, b2.trainID) > 0;
    }

    void check_queue(const char* trainID, int trainDay) {
        while (true) {
            QueueKey k = {};
            std::strcpy(k.trainID, trainID);
            k.trainDate = trainDay;
            k.time = -1;
            k.pos = -1;

            int p;
            int id;
            QueueKey res;

            bool changed = false;

            if (!order->queue_index().cursor_lower_bound(k, p, id, res)) {
                return;
            }

            while (std::strcmp(res.trainID, trainID) == 0 &&
                res.trainDate == trainDay) {
                OrderRec o;
                order->read_order(res.pos, o);

                if (o.status == pending) {
                    TrainRec t;
                    int trainPos;

                    if (!train->get_train_by_id(o.trainID, t, trainPos)) {
                        return;
                    }

                    int seat = calc_seat(t, o.trainDate, o.fromID, o.toID);

                    if (seat >= o.num) {
                        sub_seat(t, o.trainDate, o.fromID, o.toID, o.num);

                        o.status = success;
                        order->update_order(res.pos, o);

                        order->del_queue(res);

                        changed = true;
                        break;
                    }
                }

                if (!order->queue_index().cursor_next(p, id, res)) {
                    break;
                }
            }

            if (!changed) {
                return;
            }
        }
    } // 每删掉一个候补，从队列开头重新扫描，避免删除当前节点后继续使用旧位置。

public:
    TicketSys(UserSys* u, TrainSys* t, OrderSys* o)
        : user(u), train(t), order(o) {}

    int query_ticket(const std::string& from,
                 const std::string& to,
                 const std::string& date,
                 const std::string& sortType) {
        static Ticket ans[20005];
        int cnt = 0;

        int queryDay = date_to_int(date.c_str());

        int fromHead;
        int fromCnt;
        int toHead;
        int toCnt;

        bool hasFrom = train->station_info(from.c_str(), fromHead, fromCnt);
        bool hasTo = train->station_info(to.c_str(), toHead, toCnt);

        if (!hasFrom || !hasTo) {
            std::cout << 0 << '\n';
            return 0;
        }

        if (fromCnt <= toCnt) {
            int it = fromHead;

            while (it != -1) {
                TrainRec t;

                if (train->get_train_by_pos(train->station_train_pos(it), t)) {
                    int fromID = train->station_pos(it);
                    int toID = train->station_id(t, to.c_str());

                    if (toID != -1 && fromID < toID) {
                        Ticket cur;

                        if (make_ticket(t, fromID, toID, queryDay, cur)) {
                            ans[cnt++] = cur;
                        }
                    }
                }

                it = train->station_next(it);
            }
        } else {
            int it = toHead;

            while (it != -1) {
                TrainRec t;

                if (train->get_train_by_pos(train->station_train_pos(it), t)) {
                    int toID = train->station_pos(it);
                    int fromID = train->station_id(t, from.c_str());

                    if (fromID != -1 && fromID < toID) {
                        Ticket cur;

                        if (make_ticket(t, fromID, toID, queryDay, cur)) {
                            ans[cnt++] = cur;
                        }
                    }
                }

                it = train->station_next(it);
            }
        }

        if (cnt > 1) {
            sort_ticket(ans, 0, cnt - 1, sortType);
        }

        std::cout << cnt << '\n';

        for (int i = 0; i < cnt; ++i) {
            print_ticket(ans[i]);
        }

        return 0;
    }

    int query_transfer(const std::string &from,
                   const std::string &to,
                   const std::string &date,
                   const std::string &sortType) {
    Ticket best1;
    Ticket best2;
    bool found = false;

    int queryDay = date_to_int(date.c_str());

    StaKey k = {};
    std::strcpy(k.station, from.c_str());
    k.trainID[0] = '\0';
    k.pos = -1;

    int p;
    int id;
    StaKey res;

    if (train->station_index().cursor_lower_bound(k, p, id, res)) {
        while (std::strcmp(res.station, from.c_str()) == 0) {
            TrainRec t1;

            if (train->get_train_by_pos(res.trainPos, t1)) {
                int fromID = res.pos;

                if (fromID != -1) {
                    for (int midID1 = fromID + 1; midID1 < t1.stationNum; ++midID1) {
                        Ticket first;

                        if (!make_ticket(t1, fromID, midID1, queryDay, first)) {
                            continue;
                        }

                        if (found) {
                            int bestCost = best1.price + best2.price;
                            int bestTime = best2.arriveTime - best1.leaveTime;

                            if (sortType == "cost") {
                                if (first.price > bestCost) {
                                    continue;
                                }
                            } else {
                                if (first.arriveTime - first.leaveTime > bestTime) {
                                    continue;
                                }
                            }
                        }

                        StaKey mk = {};
                        std::strcpy(mk.station, t1.stations[midID1]);
                        mk.trainID[0] = '\0';
                        mk.pos = -1;

                        int p2;
                        int id2;
                        StaKey res2;

                        if (!train->station_index().cursor_lower_bound(mk, p2, id2, res2)) {
                            continue;
                        }

                        while (std::strcmp(res2.station, t1.stations[midID1]) == 0) {
                            if (std::strcmp(res2.trainID, t1.trainID) != 0) {
                                TrainRec t2;

                                if (train->get_train_by_pos(res2.trainPos, t2)) {
                                    int midID2 = res2.pos;
                                    int toID2 = train->station_id(t2, to.c_str());

                                    if (toID2 != -1 && midID2 < toID2) {
                                        int leaveOffset = t2.startTime + t2.depOffset[midID2];

                                        int need = first.arriveTime - leaveOffset;
                                        int originDay;

                                        if (need <= 0) {
                                            originDay = 0;
                                        } else {
                                            originDay = (need + 1439) / 1440;
                                        }

                                        if (originDay < t2.saleL) {
                                            originDay = t2.saleL;
                                        }

                                        if (originDay <= t2.saleR) {
                                            int secondQueryDay = originDay + leaveOffset / 1440;

                                            Ticket second;

                                            if (make_ticket(t2, midID2, toID2, secondQueryDay, second)) {
                                                if (second.leaveTime < first.arriveTime) {
                                                    ++originDay;

                                                    if (originDay <= t2.saleR) {
                                                        secondQueryDay = originDay + leaveOffset / 1440;

                                                        if (!make_ticket(t2, midID2, toID2, secondQueryDay, second)) {
                                                            if (!train->station_index().cursor_next(p2, id2, res2)) {
                                                                break;
                                                            }

                                                            continue;
                                                        }
                                                    }
                                                }

                                                if (second.leaveTime >= first.arriveTime) {
                                                    if (!found ||
                                                        better_transfer(first, second, best1, best2, sortType)) {
                                                        best1 = first;
                                                        best2 = second;
                                                        found = true;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }

                            if (!train->station_index().cursor_next(p2, id2, res2)) {
                                break;
                            }
                        }
                    }
                }
            }

            if (!train->station_index().cursor_next(p, id, res)) {
                break;
            }
        }
    }

    if (!found) {
        std::cout << 0 << '\n';
        return 0;
    }

    std::cout << best1.trainID << ' '
              << best1.from << ' ';

    print_time(best1.leaveTime);

    std::cout << " -> "
              << best1.to << ' ';

    print_time(best1.arriveTime);

    std::cout << ' '
              << best1.price << ' '
              << best1.seat << '\n';

    std::cout << best2.trainID << ' '
              << best2.from << ' ';

    print_time(best2.leaveTime);

    std::cout << " -> "
              << best2.to << ' ';

    print_time(best2.arriveTime);

    std::cout << ' '
              << best2.price << ' '
              << best2.seat << '\n';

    return 0;
}


    int buy_ticket(const std::string& username,
                   const std::string& trainID,
                   const std::string& date,
                   int num,
                   const std::string& from,
                   const std::string& to,
                   bool queue,
                   int timestamp) {
        if (!user->check_login(username.c_str())) {
            return -1;
        }

        TrainRec t;
        int trainPos;

        if (!train->get_train_by_id(trainID.c_str(), t, trainPos)) {
            return -1;
        }

        if (!t.released) {
            return -1;
        }

        if (num > t.seatNum) {
            return -1;
        }

        int fromID = train->station_id(t, from.c_str());
        int toID = train->station_id(t, to.c_str());

        if (fromID < 0 || toID < 0 || fromID >= toID) {
            return -1;
        }

        int queryDay = date_to_int(date.c_str());

        Ticket tk;

        if (!make_ticket(t, fromID, toID, queryDay, tk)) {
            return -1;
        }

        if (tk.seat >= num) {
            // 相等时购票成功
            sub_seat(t, tk.trainDate, fromID, toID, num);

            OrderRec o = {};

            o.status = success;
            o.time = timestamp;

            std::strcpy(o.username, username.c_str());
            std::strcpy(o.trainID, trainID.c_str());
            std::strcpy(o.from, from.c_str());
            std::strcpy(o.to, to.c_str());

            o.trainDate = tk.trainDate;
            o.leaveTime = tk.leaveTime;
            o.arriveTime = tk.arriveTime;
            o.fromID = fromID;
            o.toID = toID;
            o.price = tk.price;
            o.num = num;

            order->add_order(o);

            return tk.price * num;
        }

        if (queue) {
            OrderRec o = {};

            o.status = pending;
            o.time = timestamp;

            std::strcpy(o.username, username.c_str());
            std::strcpy(o.trainID, trainID.c_str());
            std::strcpy(o.from, from.c_str());
            std::strcpy(o.to, to.c_str());

            o.trainDate = tk.trainDate;
            o.leaveTime = tk.leaveTime;
            o.arriveTime = tk.arriveTime;
            o.fromID = fromID;
            o.toID = toID;
            o.price = tk.price;
            o.num = num;

            int pos = order->add_order(o);

            QueueKey qk = {};
            std::strcpy(qk.trainID, trainID.c_str());
            qk.trainDate = tk.trainDate;
            qk.time = timestamp;
            qk.pos = pos;

            order->add_queue(qk);

            return -2;
        }

        return -1;
    }

    int refund_ticket(const std::string& username,
                      int nth) {
        if (!user->check_login(username.c_str())) {
            return -1;
        }

        OrderRec o;
        int pos;

        if (!order->get_nth_order(username, nth, o, pos)) {
            return -1;
        }

        if (o.status == refunded) {
            return -1;
        }

        if (o.status == pending) {
            o.status = refunded;
            order->update_order(pos, o);
            QueueKey qk = {};
            std::strcpy(qk.trainID, o.trainID);
            qk.trainDate = o.trainDate;
            qk.time = o.time;
            qk.pos = pos;

            order->del_queue(qk); // 将已退款的后补订单从等待队列中删除
            return 0;
        }

        TrainRec t;
        int trainPos;

        if (!train->get_train_by_id(o.trainID, t, trainPos)) {
            return -1;
        }

        o.status = refunded;
        order->update_order(pos, o);

        add_seat(t, o.trainDate, o.fromID, o.toID, o.num);

        check_queue(o.trainID, o.trainDate);

        return 0;
    }

    void clear() {}
};

#endif
