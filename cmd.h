#ifndef CMD_HPP
#define CMD_HPP

#include "ticket.h"

inline int split_str(const std::string& s, std::string a[]) {
    if (s == "_") {
        return 0;
    }

    int cnt = 0;
    std::string cur;

    for (int i = 0; i < (int)s.size(); ++i) {
        if (s[i] == '|') {
            a[cnt++] = cur;
            cur.clear();
        }
        else {
            cur += s[i];
        }
    }

    a[cnt++] = cur;
    return cnt;
}

class Cmd {
public:
    int timestamp;
    std::string name;

    std::string c;
    std::string u;
    std::string p;
    std::string n;
    std::string m;
    std::string g;

    std::string i;
    std::string s;
    std::string t;
    std::string d;
    std::string f;
    std::string q;
    std::string y;
    std::string x;
    std::string o;

    bool hasP;
    bool hasN;
    bool hasM;
    bool hasG;
    bool hasQ;

    void init() {
        timestamp = 0;
        name.clear();

        c.clear();
        u.clear();
        p.clear();
        n.clear();
        m.clear();
        g.clear();

        i.clear();
        s.clear();
        t.clear();
        d.clear();
        f.clear();
        q.clear();
        y.clear();
        x.clear();
        o.clear();

        hasP = false;
        hasN = false;
        hasM = false;
        hasG = false;
        hasQ = false;
    }

    int parse_line(const std::string& line) {
        init();

        int len = (int)line.size();
        int pos = 0;

        if (len == 0) {
            return 0;
        }

        if (line[pos] != '[') {
            return 0;
        }

        ++pos;

        timestamp = 0;

        while (pos < len && line[pos] != ']') {
            timestamp = timestamp * 10 + line[pos] - '0';
            ++pos;
        }

        if (pos < len && line[pos] == ']') {
            ++pos;
        }

        while (pos < len && line[pos] == ' ') {
            ++pos;
        }

        int l = pos;

        while (pos < len && line[pos] != ' ') {
            ++pos;
        }

        name.assign(line, l, pos - l);

        while (pos < len) {
            while (pos < len && line[pos] == ' ') {
                ++pos;
            }

            if (pos >= len) {
                break;
            }

            if (line[pos] != '-') {
                break;
            }

            char key = line[pos + 1];
            pos += 2;

            while (pos < len && line[pos] == ' ') {
                ++pos;
            }

            l = pos;

            while (pos < len && line[pos] != ' ') {
                ++pos;
            }

            if (key == 'c') {
                c.assign(line, l, pos - l);
            }
            else if (key == 'u') {
                u.assign(line, l, pos - l);
            }
            else if (key == 'p') {
                p.assign(line, l, pos - l);
                hasP = true;
            }
            else if (key == 'n') {
                n.assign(line, l, pos - l);
                hasN = true;
            }
            else if (key == 'm') {
                m.assign(line, l, pos - l);
                hasM = true;
            }
            else if (key == 'g') {
                g.assign(line, l, pos - l);
                hasG = true;
            }
            else if (key == 'i') {
                i.assign(line, l, pos - l);
            }
            else if (key == 's') {
                s.assign(line, l, pos - l);
            }
            else if (key == 't') {
                t.assign(line, l, pos - l);
            }
            else if (key == 'd') {
                d.assign(line, l, pos - l);
            }
            else if (key == 'f') {
                f.assign(line, l, pos - l);
            }
            else if (key == 'q') {
                q.assign(line, l, pos - l);
                hasQ = true;
            }
            else if (key == 'y') {
                y.assign(line, l, pos - l);
            }
            else if (key == 'x') {
                x.assign(line, l, pos - l);
            }
            else if (key == 'o') {
                o.assign(line, l, pos - l);
            }
        }

        return 1;
    }
};

inline void build_train_from_cmd(const Cmd& cmd, TrainRec& t) {
    std::memset(&t, 0, sizeof(t));

    std::strcpy(t.trainID, cmd.i.c_str());

    t.stationNum = to_int(cmd.n);
    t.seatNum = to_int(cmd.m);

    std::string tmp[MAX_STA];

    int cnt = split_str(cmd.s, tmp);
    for (int i = 0; i < cnt; ++i) {
        std::strcpy(t.stations[i], tmp[i].c_str());
    }

    cnt = split_str(cmd.p, tmp);
    for (int i = 0; i < cnt; ++i) {
        t.price[i] = to_int(tmp[i]);
    }

    cnt = split_str(cmd.t, tmp);
    for (int i = 0; i < cnt; ++i) {
        t.travel[i] = to_int(tmp[i]);
    }

    cnt = split_str(cmd.o, tmp);
    for (int i = 0; i < cnt; ++i) {
        t.stopover[i + 1] = to_int(tmp[i]); // 第 1 个停站时间应对应第 1 站
    }

    cnt = split_str(cmd.d, tmp);
    t.saleL = date_to_int(tmp[0].c_str());
    t.saleR = date_to_int(tmp[1].c_str());

    t.startTime = time_to_int(cmd.x.c_str());
    t.type = cmd.y[0];

    t.released = 0;
    t.seatPos = -1;
}


class Sys {
private:
    UserSys user;
    TrainSys train;
    OrderSys order;
    TicketSys ticket;

public:
    Sys() : ticket(&user, &train, &order) {}

    void run() {
        std::string line;

        while (std::getline(std::cin, line)) {
            if (line.empty()) continue;

            Cmd cmd;
            if (!cmd.parse_line(line)) continue;

            if (cmd.name == "exit") {
                std::cout << '[' << cmd.timestamp << "] bye\n";
                break;
            }

            work(cmd);
        }
    }

    void work(Cmd& cmd) {
        std::cout << '[' << cmd.timestamp << "] ";

        if (cmd.name == "add_user") {
            int pri = 0;
            if (!cmd.g.empty()) pri = to_int(cmd.g);

            int res = user.add_user(cmd.c, cmd.u, cmd.p, cmd.n, cmd.m, pri);
            std::cout << res << '\n';
        }
        else if (cmd.name == "login") {
            std::cout << user.login(cmd.u, cmd.p) << '\n';
        }
        else if (cmd.name == "logout") {
            std::cout << user.logout(cmd.u) << '\n';
        }
        else if (cmd.name == "query_profile") {
            UserRec ans;
            int res = user.query_profile(cmd.c, cmd.u, ans);

            if (res == -1) {
                std::cout << -1 << '\n';
            }
            else {
                std::cout << ans.username << ' '
                    << ans.name << ' '
                    << ans.mail << ' '
                    << ans.privilege << '\n';
            }
        }
        else if (cmd.name == "modify_profile") {
            int pri = 0;
            if (!cmd.g.empty()) pri = to_int(cmd.g);

            UserRec ans;
            int res = user.modify_profile(cmd.c,
                                          cmd.u,
                                          cmd.p,
                                          cmd.n,
                                          cmd.m,
                                          pri,
                                          cmd.hasP,
                                          cmd.hasN,
                                          cmd.hasM,
                                          cmd.hasG,
                                          ans);

            if (res == -1) {
                std::cout << -1 << '\n';
            }
            else {
                std::cout << ans.username << ' '
                    << ans.name << ' '
                    << ans.mail << ' '
                    << ans.privilege << '\n';
            }
        }
        else if (cmd.name == "add_train") {
            TrainRec t;
            build_train_from_cmd(cmd, t);

            std::cout << train.add_train(t) << '\n';
        }
        else if (cmd.name == "delete_train") {
            std::cout << train.delete_train(cmd.i) << '\n';
        }
        else if (cmd.name == "release_train") {
            std::cout << train.release_train(cmd.i) << '\n';
        }
        else if (cmd.name == "query_train") {
            int res = train.query_train(cmd.i, cmd.d);

            if (res == -1) {
                std::cout << -1 << '\n';
            }
        }
        else if (cmd.name == "clean") {
            clean();
            std::cout << 0 << '\n';
        }
        else if (cmd.name == "query_ticket") {
            std::string sortType = "time";
            if (cmd.hasP) {
                sortType = cmd.p;
            }
            ticket.query_ticket(cmd.s, cmd.t, cmd.d, sortType);
        }
        else if (cmd.name == "buy_ticket") {
            int num = to_int(cmd.n);
            bool queue = false;
            if (cmd.hasQ && cmd.q == "true") {
                queue = true;
            }
            int res = ticket.buy_ticket(cmd.u, cmd.i, cmd.d, num, cmd.f, cmd.t, queue, cmd.timestamp);
            if (res == -2) {
                std::cout << "queue\n";
            }
            else {
                std::cout << res << '\n';
            }
        }
        else if (cmd.name == "query_order") {
            if (!user.check_login(cmd.u.c_str())) {
                std::cout << -1 << '\n';
            }
            else {
                order.query_order(cmd.u);
            }
        }
        else if (cmd.name == "query_transfer") {
            std::string sortType = "time";

            if (cmd.hasP) {
                sortType = cmd.p;
            }

            ticket.query_transfer(cmd.s, cmd.t, cmd.d, sortType);
        }
        else if (cmd.name == "refund_ticket") {
            int nth = 1;

            if (!cmd.n.empty()) {
                nth = to_int(cmd.n);
            }

            std::cout << ticket.refund_ticket(cmd.u, nth) << '\n';
        }
        else {
            std::cout << -1 << '\n';
        }
    }

    void clean() {
        user.clear();
        train.clear();
        order.clear();
        ticket.clear();
    }
};

#endif
