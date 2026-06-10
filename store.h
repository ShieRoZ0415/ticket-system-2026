#ifndef STORE_HPP
#define STORE_HPP

#include <cstdio>

template <class T>
class Store {
private:
    FILE *fp;
    char filename[64];
    int tot;

public:
    Store(const char *name) {
        int i = 0;
        while (name[i]) {
            filename[i] = name[i];
            ++i;
        }
        filename[i] = 0;

        fp = std::fopen(filename, "rb+");
        if (!fp) fp = std::fopen(filename, "wb+");
        std::fseek(fp, 0, SEEK_END);
        long long bytes = std::ftell(fp);
        tot = (int)(bytes / sizeof(T));
    }

    ~Store() {
        if (fp) {
            std::fflush(fp);
            std::fclose(fp);
        }
    }

    int size() {
        return tot;
    }

    int append(const T &x) {
        int p = tot;
        ++tot;
        std::fseek(fp, 1ll * p * sizeof(T), SEEK_SET);
        std::fwrite(&x, sizeof(T), 1, fp);
        return p;
    }

    void read(int p, T &x) {
        std::fseek(fp, 1ll * p * sizeof(T), SEEK_SET);
        std::fread(&x, sizeof(T), 1, fp);
    }

    void write(int p, const T &x) {
        std::fseek(fp, 1ll * p * sizeof(T), SEEK_SET);
        std::fwrite(&x, sizeof(T), 1, fp);
    }

    void clear() {
        if (fp) {
            std::fclose(fp);
        }
        fp = std::fopen(filename, "wb+");
        tot = 0;
    }
};

#endif