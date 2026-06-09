#ifndef USER_HPP
#define USER_HPP

#include "record.h"
#include "store.h"
#include "BPT.h"

class UserSys {
private:
    Store<UserRec> userFile;
    BPT<UserKey> userIdx;

    char online[MAX_ONLINE][USER_LEN];
    int onlineCnt;

    bool is_online(const char *username) {
        for (int i = 0; i < onlineCnt; ++i) {
            if (std::strcmp(online[i], username) == 0) {
                return true;
            }
        }

        return false;
    }


    int find_user_pos(const char *username) {
        UserKey k = {};
        std::strcpy(k.username, username);
        k.pos = -1;

        UserKey res;

        if (!userIdx.lower_bound(k, res)) {
            return -1;
        }

        if (std::strcmp(res.username, username) != 0) {
            return -1;
        }

        return res.pos;
    }

    bool get_user(const char *username, UserRec &u, int &pos) {
        pos = find_user_pos(username);

        if (pos == -1) {
            return false;
        }

        userFile.read(pos, u);
        return true;
    }

public:
    UserSys()
        : userFile("user.dat"),
          userIdx("user.bpt") {
        onlineCnt = 0;
    }

    int add_user(const std::string &cur,
                 const std::string &username,
                 const std::string &password,
                 const std::string &name,
                 const std::string &mail,
                 int privilege) {
        if (find_user_pos(username.c_str()) != -1) {
            return -1;
        }

        UserRec u = {};

        std::strcpy(u.username, username.c_str());
        std::strcpy(u.password, password.c_str());
        std::strcpy(u.name, name.c_str());
        std::strcpy(u.mail, mail.c_str());

        if (userFile.size() == 0) {
            u.privilege = 10;
        } else {
            if (!is_online(cur.c_str())) {
                return -1;
            }

            UserRec curUser;
            int curPos;

            if (!get_user(cur.c_str(), curUser, curPos)) {
                return -1;
            }

            if (curUser.privilege <= privilege) {
                return -1;
            }

            u.privilege = privilege;
        }

        int pos = userFile.append(u);

        UserKey k = {};
        std::strcpy(k.username, u.username);
        k.pos = pos;

        userIdx.insert(k);

        return 0;
    }

    int login(const std::string &username,
              const std::string &password) {
        if (is_online(username.c_str())) {
            return -1;
        }

        UserRec u;
        int pos;

        if (!get_user(username.c_str(), u, pos)) {
            return -1;
        }

        if (std::strcmp(u.password, password.c_str()) != 0) {
            return -1;
        }

        std::strcpy(online[onlineCnt], u.username);
        onlineCnt++;

        return 0;
    }

    int logout(const std::string &username) {
        for (int i = 0; i < onlineCnt; ++i) {
            if (std::strcmp(online[i], username.c_str()) == 0) {
                for (int j = i; j + 1 < onlineCnt; ++j) {
                    std::strcpy(online[j], online[j + 1]);
                }

                onlineCnt--;
                return 0;
            }
        }

        return -1;
    }

    int query_profile(const std::string &cur,
                      const std::string &username,
                      UserRec &ans) {
        if (!is_online(cur.c_str())) {
            return -1;
        }

        UserRec curUser;
        int curPos;

        if (!get_user(cur.c_str(), curUser, curPos)) {
            return -1;
        }

        UserRec target;
        int targetPos;

        if (!get_user(username.c_str(), target, targetPos)) {
            return -1;
        }

        if (std::strcmp(cur.c_str(), username.c_str()) != 0 &&
            curUser.privilege <= target.privilege) {
            return -1;
        }

        ans = target;
        return 0;
    }

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
                       UserRec &ans) {
        if (!is_online(cur.c_str())) {
            return -1;
        }

        UserRec curUser;
        int curPos;

        if (!get_user(cur.c_str(), curUser, curPos)) {
            return -1;
        }

        UserRec target;
        int targetPos;

        if (!get_user(username.c_str(), target, targetPos)) {
            return -1;
        }

        if (std::strcmp(cur.c_str(), username.c_str()) != 0 &&
            curUser.privilege <= target.privilege) {
            return -1;
        }

        if (hasG && curUser.privilege <= privilege) {
            return -1;
        }

        if (hasP) {
            std::strcpy(target.password, password.c_str());
        }

        if (hasN) {
            strcpy(target.name, name.c_str());
        }

        if (hasM) {
            strcpy(target.mail, mail.c_str());
        }

        if (hasG) {
            target.privilege = privilege;
        }

        userFile.write(targetPos, target);

        ans = target;
        return 0;
    }

    bool check_login(const char *username) {
        return is_online(username);
    }

    void clear() {
        userFile.clear();
        userIdx.clear();
        onlineCnt = 0;
    }
};

#endif