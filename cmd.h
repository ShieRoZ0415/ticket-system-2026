#ifndef CMD_HPP
#define CMD_HPP

#include "ticket.h"

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

    int parse();
};

class Sys {
private:
    UserSys user;
    TrainSys train;
    OrderSys order;
    TicketSys ticket;

public:
    Sys();

    void run();
    void work(Cmd &cmd);
    void clean();
};

#endif