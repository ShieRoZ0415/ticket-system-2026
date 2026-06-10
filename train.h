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

    struct MemStaNode {
        int trainPos;
        int pos;
        int next;
    };

    struct MemStaHead {
        char station[STA_LEN];
        int head;
        int cnt;
    };

    static const int MEM_HEAD_MAX = 20000;
    static const int MEM_NODE_MAX = 400000;

    MemStaHead *memHead;
    MemStaNode *memNode;

    int memHeadCnt;
    int memNodeCnt;

    int find_train_pos(const char *trainID) {
        TrainKey k = {};
        std::strcpy(k.trainID, trainID);
        k.pos = -1;

        TrainKey res;

        if (!trainIdx.lower_bound(k, res)) {
            return -1;
        }

        if (std::strcmp(res.trainID, trainID) != 0) {
            return -1;
        }

        return res.pos;
    }

    bool get_train(const char *trainID, TrainRec &t, int &pos) {
        pos = find_train_pos(trainID);

        if (pos == -1) {
            return false;
        }

        trainFile.read(pos, t);
        return true;
    }

    void build_time_price(TrainRec &t) {
        for (int i = 0; i < MAX_STA; ++i) {
            t.arrOffset[i] = 0;
            t.depOffset[i] = 0;
            t.prePrice[i] = 0;
        }

        t.arrOffset[0] = 0;
        t.depOffset[0] = 0;
        t.prePrice[0] = 0;

        for (int i = 1; i < t.stationNum; ++i) {
            t.arrOffset[i] = t.depOffset[i - 1] + t.travel[i - 1];

            if (i == t.stationNum - 1) {
                t.depOffset[i] = t.arrOffset[i];
            } else {
                t.depOffset[i] = t.arrOffset[i] + t.stopover[i];
            }
        }

        for (int i = 1; i < t.stationNum; ++i) {
            t.prePrice[i] = t.prePrice[i - 1] + t.price[i - 1];
        }
    }

    int find_station(const TrainRec &t, const char *station) {
        for (int i = 0; i < t.stationNum; ++i) {
            if (std::strcmp(t.stations[i], station) == 0) {
                return i;
            }
        }

        return -1;
    }

public:
    TrainSys()
        : trainFile("train.dat"),
          seatFile("seat.dat"),
          trainIdx("train.bpt"),
          staIdx("station.bpt") {
        memHead = new MemStaHead[MEM_HEAD_MAX];
        memNode = new MemStaNode[MEM_NODE_MAX];
        rebuild_mem_station();
    }


    void init_mem_station() {
        memHeadCnt = 0;
        memNodeCnt = 0;
    }

    int find_mem_head(const char *station) {
        int l = 0;
        int r = memHeadCnt - 1;

        while (l <= r) {
            int m = (l + r) >> 1;
            int c = std::strcmp(memHead[m].station, station);

            if (c == 0) {
                return m;
            }

            if (c < 0) {
                l = m + 1;
            } else {
                r = m - 1;
            }
        }

        return -1;
    }

    int lower_mem_head(const char *station) {
        int l = 0;
        int r = memHeadCnt;

        while (l < r) {
            int m = (l + r) >> 1;

            if (std::strcmp(memHead[m].station, station) < 0) {
                l = m + 1;
            } else {
                r = m;
            }
        }

        return l;
    }

    int get_mem_head(const char *station) {
        int p = lower_mem_head(station);

        if (p < memHeadCnt &&
            std::strcmp(memHead[p].station, station) == 0) {
            return p;
            }

        if (memHeadCnt >= MEM_HEAD_MAX) {
            return -1;
        }

        for (int i = memHeadCnt; i > p; --i) {
            memHead[i] = memHead[i - 1];
        }

        std::strcpy(memHead[p].station, station);
        memHead[p].head = -1;
        memHead[p].cnt = 0;

        ++memHeadCnt;

        return p;
    }

    void add_mem_station(const char *station, int trainPos, int pos) {
        int h = get_mem_head(station);

        if (h == -1) {
            return;
        }

        if (memNodeCnt >= MEM_NODE_MAX) {
            return;
        }

        int id = memNodeCnt++;

        memNode[id].trainPos = trainPos;
        memNode[id].pos = pos;
        memNode[id].next = memHead[h].head;
        memHead[h].head = id;

        ++memHead[h].cnt;
    }

    void rebuild_mem_station() {
        init_mem_station();

        StaKey k = {};
        k.station[0] = '\0';
        k.trainID[0] = '\0';
        k.pos = -1;

        int p;
        int id;
        StaKey res;

        if (!staIdx.cursor_lower_bound(k, p, id, res)) {
            return;
        }

        while (true) {
            add_mem_station(res.station, res.trainPos, res.pos);

            if (!staIdx.cursor_next(p, id, res)) {
                break;
            }
        }
    }

    int station_head(const char *station) {
        int h = find_mem_head(station);

        if (h == -1) {
            return -1;
        }

        return memHead[h].head;
    }

    int station_next(int id) {
        return memNode[id].next;
    }

    int station_train_pos(int id) {
        return memNode[id].trainPos;
    }

    int station_pos(int id) {
        return memNode[id].pos;
    }

    bool station_info(const char *station, int &head, int &cnt) {
        int h = find_mem_head(station);

        if (h == -1) {
            head = -1;
            cnt = 0;
            return false;
        }

        head = memHead[h].head;
        cnt = memHead[h].cnt;
        return true;
    }

    ~TrainSys() {
        delete[] memHead;
        delete[] memNode;
    }

    bool get_train_by_pos(int pos, TrainRec &t) {
        if (pos < 0) {
            return false;
        }

        trainFile.read(pos, t);
        return true;
    }


    int add_train(const TrainRec &src) {
        if (find_train_pos(src.trainID) != -1) {
            return -1;
        }

        TrainRec t = src;

        t.released = 0;
        t.seatPos = -1;

        build_time_price(t);

        int pos = trainFile.append(t);

        TrainKey k = {};
        std::strcpy(k.trainID, t.trainID);
        k.pos = pos;

        trainIdx.insert(k);

        return 0;
    }

    int delete_train(const std::string &trainID) {
        TrainRec t;
        int pos;

        if (!get_train(trainID.c_str(), t, pos)) {
            return -1;
        }

        if (t.released) {
            return -1;
        }

        TrainKey k = {};
        std::strcpy(k.trainID, t.trainID);
        k.pos = pos;

        trainIdx.erase(k);

        return 0;
    }

    int release_train(const std::string &trainID) {
        TrainRec t;
        int pos;

        if (!get_train(trainID.c_str(), t, pos)) {
            return -1;
        }

        if (t.released) {
            return -1;
        }

        int firstSeatPos = -1;

        for (int d = 0; d < MAX_DAY; ++d) {
            SeatRec seat = {};
            for (int i = 0; i + 1 < t.stationNum; ++i) {
                seat.seat[i] = t.seatNum;
            }
            int curPos = seatFile.append(seat);
            if (d == 0) {
                firstSeatPos = curPos;
            }
        }

        t.released = 1;
        t.seatPos = firstSeatPos;

        trainFile.write(pos, t);

        for (int i = 0; i < t.stationNum; ++i) {
            StaKey sk = {};
            std::strcpy(sk.station, t.stations[i]);
            std::strcpy(sk.trainID, t.trainID);

            sk.pos = i;
            sk.trainPos = pos;

            staIdx.insert(sk);
            add_mem_station(t.stations[i], pos, i);
        }

        return 0;
    }

    int query_train(const std::string &trainID,
                    const std::string &date) {
        TrainRec t;
        int pos;

        if (!get_train_by_id(trainID.c_str(), t, pos)) {
            return -1;
        }

        int month = (date[0] - '0') * 10 + (date[1] - '0');
        int dayInMonth = (date[3] - '0') * 10 + (date[4] - '0');

        int monthDay[13] = {
            0,
            31, 28, 31, 30, 31, 30,
            31, 31, 30, 31, 30, 31
        };

        int day = 0;

        if (month < 6) {
            day = -1;
        } else {
            for (int i = 6; i < month; ++i) {
                day += monthDay[i];
            }
            day += dayInMonth - 1;
        }

        if (day < t.saleL || day > t.saleR) {
            return -1;
        }

        if (day < 0 || day >= MAX_DAY) {
            return -1;
        }

        SeatRec seat;
        bool hasSeat = false;

        if (t.released) {
            if (!get_seat(t, day, seat)) {
                return -1;
            }
            hasSeat = true;
        }

        std::cout << t.trainID << ' ' << t.type << '\n';

        for (int i = 0; i < t.stationNum; ++i) {
            std::cout << t.stations[i] << ' ';

            if (i == 0) {
                std::cout << "xx-xx xx:xx";
            } else {
                print_time(day * 1440 + t.startTime + t.arrOffset[i]);
            }

            std::cout << " -> ";

            if (i == t.stationNum - 1) {
                std::cout << "xx-xx xx:xx";
            } else {
                print_time(day * 1440 + t.startTime + t.depOffset[i]);
            }

            std::cout << ' ' << t.prePrice[i] << ' ';

            if (i == t.stationNum - 1) {
                std::cout << 'x';
            } else {
                if (hasSeat) {
                    std::cout << seat.seat[i];
                } else {
                    std::cout << t.seatNum;
                }
            }

            std::cout << '\n';
        }

        return 0;
    }

    bool get_train_by_id(const char *trainID, TrainRec &t, int &pos) {
        return get_train(trainID, t, pos);
    }

    bool get_seat(const TrainRec &t, int day, SeatRec &s) {
        if (!t.released) {
            return false;
        }

        if (day < t.saleL || day > t.saleR) {
            return false;
        }

        seatFile.read(t.seatPos + day, s);
        return true;
    }

    void write_seat(const TrainRec &t, int day, const SeatRec &s) {
        if (!t.released) {
            return;
        }

        if (day < t.saleL || day > t.saleR) {
            return;
        }

        seatFile.write(t.seatPos + day, s);
    }

    int station_id(const TrainRec &t, const char *station) {
        return find_station(t, station);
    }

    BPT<StaKey> &station_index() {
        return staIdx;
    }

    void clear() {
        trainFile.clear();
        seatFile.clear();
        trainIdx.clear();
        staIdx.clear();
        init_mem_station();
    }
};

#endif