#ifndef TRAIN_HPP
#define TRAIN_HPP

#include "record.h"
#include "store.h"
#include "BPT.h"

class TrainSys {
private:
    Store<TrainRec> trainFile;
    Store<SeatRec> seatFile;

    BPT<TrainKey> trainIdx;
    BPT<StaKey> staIdx;

    int find_train_pos(const char *trainID);
    bool get_train(const char *trainID, TrainRec &t, int &pos);

    void build_time_price(TrainRec &t);
    int find_station(const TrainRec &t, const char *station);

public:
    TrainSys();

    int add_train(const TrainRec &t);
    int delete_train(const std::string &trainID);
    int release_train(const std::string &trainID);
    int query_train(const std::string &trainID,
                    const std::string &date);

    bool get_train_by_id(const char *trainID, TrainRec &t, int &pos);
    bool get_seat(const TrainRec &t, int day, SeatRec &s);
    void write_seat(const TrainRec &t, int day, const SeatRec &s);

    int station_id(const TrainRec &t, const char *station);

    BPT<StaKey> &station_index();

    void clear();
};

#endif