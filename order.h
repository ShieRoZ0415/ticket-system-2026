#ifndef ORDER_H
#define ORDER_H

#include "record.h"
#include "store.h"
#include "BPT.h"

class OrderSys {
private:
    Store<OrderRec> orderFile;
    BPT<OrderKey> orderIdx;
    BPT<QueueKey> queueIdx;

public:
    OrderSys()
        : orderFile("order.dat"),
          orderIdx("order.bpt"),
          queueIdx("queue.bpt") {
    }

    int add_order(const OrderRec &o) {
        int pos = orderFile.append(o);

        OrderKey k = {};
        std::strcpy(k.username, o.username);
        k.revTime = 2000000000 - o.time;
        k.pos = pos;

        orderIdx.insert(k);

        return pos;
    }

    void update_order(int pos, const OrderRec &o) {
        orderFile.write(pos, o);
    }

    void read_order(int pos, OrderRec &o) {
        orderFile.read(pos, o);
    }

    bool get_nth_order(const std::string &username, int nth, OrderRec &o, int &pos) {
        OrderKey k = {};
        std::strcpy(k.username, username.c_str());
        k.revTime = -1;
        k.pos = -1;

        int p;
        int id;
        OrderKey res;

        if (!orderIdx.cursor_lower_bound(k, p, id, res)) {
            return false;
        }

        int cnt = 0;

        while (std::strcmp(res.username, username.c_str()) == 0) {
            ++cnt;

            if (cnt == nth) {
                pos = res.pos;
                orderFile.read(pos, o);
                return true;
            }

            if (!orderIdx.cursor_next(p, id, res)) {
                break;
            }
        }

        return false;
    }

    int query_order(const std::string &username) {
        OrderKey k = {};
        std::strcpy(k.username, username.c_str());
        k.revTime = -1;
        k.pos = -1;

        int p;
        int id;
        OrderKey res;
        int cnt = 0;

        if (orderIdx.cursor_lower_bound(k, p, id, res)) {
            while (std::strcmp(res.username, username.c_str()) == 0) {
                ++cnt;

                if (!orderIdx.cursor_next(p, id, res)) {
                    break;
                }
            }
        }
        std::cout << cnt << '\n';

        if (orderIdx.cursor_lower_bound(k, p, id, res)) {
            while (std::strcmp(res.username, username.c_str()) == 0) {
                OrderRec o;
                orderFile.read(res.pos, o);

                if (o.status == success) {
                    std::cout << "[success] ";
                } else if (o.status == pending) {
                    std::cout << "[pending] ";
                } else {
                    std::cout << "[refunded] ";
                }
                std::cout << o.trainID << ' '
                          << o.from << ' ';
                print_time(o.leaveTime);
                std::cout << " -> "
                          << o.to << ' ';
                print_time(o.arriveTime);
                std::cout << ' '
                          << o.price << ' '
                          << o.num << '\n';

                if (!orderIdx.cursor_next(p, id, res)) {
                    break;
                }
            }
        }

        return 0;
    }

    void add_queue(const QueueKey &k) {
        queueIdx.insert(k);
    }

    void del_queue(const QueueKey &k) {
        queueIdx.erase(k);
    }

    BPT<QueueKey> &queue_index() {
        return queueIdx;
    }

    void clear() {
        orderFile.clear();
        orderIdx.clear();
        queueIdx.clear();
    }
};

#endif