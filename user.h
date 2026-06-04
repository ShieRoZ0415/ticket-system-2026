#ifndef USER_HPP
#define USER_HPP

#include "record.hpp"
#include "store.hpp"
#include "BPT.hpp"

class UserSys {
private:
    Store<UserRec> userFile;
    BPT<UserKey> userIdx;

    char online[MAX_ONLINE][USER_LEN];
    int onlineCnt;

    bool is_online(const char *username);
    int find_user_pos(const char *username);
    bool get_user(const char *username, UserRec &u, int &pos);

public:
    UserSys();

    int add_user(const std::string &cur,
                 const std::string &username,
                 const std::string &password,
                 const std::string &name,
                 const std::string &mail,
                 int privilege);

    int login(const std::string &username,
              const std::string &password);

    int logout(const std::string &username);

    int query_profile(const std::string &cur,
                      const std::string &username,
                      UserRec &ans);

    int modify_profile(const std::string &cur,
                       const std::string &username,
                       const std::string &password,
                       const std::string &name,
                       const std::string &mail,
                       int privilege,
                       bool hasP,
                       bool hasN,
                       bool hasM,
                       bool hasG,
                       UserRec &ans);

    bool check_login(const char *username);
    void clear();
};

#endif