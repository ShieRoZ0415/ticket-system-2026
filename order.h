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

        OrderKey k;
        std::memset(&k, 0, sizeof(k));
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

    int query_order(const std::string &username) {
        return -1;
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