#ifndef ORDER_HPP
#define ORDER_HPP

#include "record.hpp"
#include "store.hpp"
#include "BPT.hpp"

class OrderSys {
private:
    Store<OrderRec> orderFile;
    BPT<OrderKey> orderIdx;
    BPT<QueueKey> queueIdx;

public:
    OrderSys();

    int add_order(const OrderRec &o);
    void update_order(int pos, const OrderRec &o);
    void read_order(int pos, OrderRec &o);

    int query_order(const std::string &username);

    void add_queue(const QueueKey &k);
    void del_queue(const QueueKey &k);

    BPT<QueueKey> &queue_index();

    void clear();
};

#endif