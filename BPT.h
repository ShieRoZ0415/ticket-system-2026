#ifndef BPT_H
#define BPT_H

#include <cstdio>

template <class Key>
struct BPTDefaultLess {
    bool operator()(const Key &a, const Key &b) const {
        return a < b;
    }
};

template <class Key, int MAXN = 100, class Less = BPTDefaultLess<Key> >
class BPT {
private:
    struct Header {
        int root;
        int tot;
    };

    struct Node {
        int leaf;
        int cnt;
        int parent;
        int next;
        int prev;
        Key key[MAXN + 2];
        int son[MAXN + 3];
    };

    static const int MIN_LEAF = (MAXN + 1) / 2;
    static const int MIN_INTERNAL = MAXN / 2;

    FILE *fp;
    Header h;
    Less less;

    static const int CACHE_SIZE = 64;

    Node cacheNode[CACHE_SIZE];
    int cachePos[CACHE_SIZE];
    int cacheTime[CACHE_SIZE];
    int cacheNow;

    void init_cache() {
        cacheNow = 0;

        for (int i = 0; i < CACHE_SIZE; ++i) {
            cachePos[i] = -1;
            cacheTime[i] = 0;
        }
    }

    int find_cache(int p) {
        for (int i = 0; i < CACHE_SIZE; ++i) {
            if (cachePos[i] == p) {
                return i;
            }
        }

        return -1;
    }

    void put_cache(int p, const Node &x) {
        int id = find_cache(p);

        if (id == -1) {
            id = 0;

            for (int i = 1; i < CACHE_SIZE; ++i) {
                if (cachePos[i] == -1) {
                    id = i;
                    break;
                }

                if (cachePos[id] != -1 && cacheTime[i] < cacheTime[id]) {
                    id = i;
                }
            }
        }

        cachePos[id] = p;
        cacheNode[id] = x;
        cacheTime[id] = ++cacheNow;
    }

    long long off(int p) {
        return sizeof(Header) + 1ll * p * sizeof(Node);
    }

    bool lesskey(const Key &a, const Key &b) const {
        return less(a, b);
    }

    bool equalkey(const Key &a, const Key &b) const {
        return !less(a, b) && !less(b, a);
    }

    void initnode(Node &x, int leaf, int parent) {
        x.leaf = leaf;
        x.cnt = 0;
        x.parent = parent;
        x.next = -1;
        x.prev = -1;

        for (int i = 0; i < MAXN + 3; ++i) {
            x.son[i] = -1;
        }
    }

    bool readhead() {
        std::fseek(fp, 0, SEEK_SET);
        return std::fread(&h, sizeof(h), 1, fp) == 1;
    }

    void writehead() {
        std::fseek(fp, 0, SEEK_SET);
        std::fwrite(&h, sizeof(h), 1, fp);
    }

    bool readnode(int p, Node &x) {
        int id = find_cache(p);

        if (id != -1) {
            x = cacheNode[id];
            cacheTime[id] = ++cacheNow;
            return true;
        }

        std::fseek(fp, off(p), SEEK_SET);

        if (std::fread(&x, sizeof(x), 1, fp) != 1) {
            return false;
        }

        put_cache(p, x);

        return true;
    }

    void writenode(int p, const Node &x) {
        std::fseek(fp, off(p), SEEK_SET);
        std::fwrite(&x, sizeof(x), 1, fp);

        put_cache(p, x);
    }

    int newnode(int leaf, int parent) {
        Node x;
        initnode(x, leaf, parent);

        int p = h.tot++;
        writenode(p, x);
        return p;
    }

    int lower_pos(const Node &x, const Key &k) {
        int l = 0;
        int r = x.cnt;

        while (l < r) {
            int m = (l + r) >> 1;

            if (lesskey(x.key[m], k)) {
                l = m + 1;
            } else {
                r = m;
            }
        }

        return l;
    }

    int go_pos(const Node &x, const Key &k) {
        int l = 0;
        int r = x.cnt;

        while (l < r) {
            int m = (l + r) >> 1;

            if (lesskey(k, x.key[m])) {
                r = m;
            } else {
                l = m + 1;
            }
        }

        return l;
    }

    int child_index(const Node &x, int c) {
        for (int i = 0; i <= x.cnt; ++i) {
            if (x.son[i] == c) return i;
        }

        return -1;
    }

    void set_parent(int p, int fa) {
        Node x;
        readnode(p, x);
        x.parent = fa;
        writenode(p, x);
    }

    int find_leaf(const Key &k) {
        int p = h.root;
        Node x;

        while (true) {
            readnode(p, x);

            if (x.leaf) return p;

            int id = go_pos(x, k);
            p = x.son[id];
        }
    }

    Key first_key(int p) {
        Node x;
        readnode(p, x);

        while (!x.leaf) {
            p = x.son[0];
            readnode(p, x);
        }

        return x.key[0];
    }

    void rebuild_internal_keys(int p) {
        Node x;
        readnode(p, x);

        if (x.leaf) return;

        for (int i = 0; i < x.cnt; ++i) {
            x.key[i] = first_key(x.son[i + 1]);
        }

        writenode(p, x);
    }

    void change_parent_key(int p) {
        if (p == h.root) return;

        Node x;
        Node fa;

        readnode(p, x);

        int fp = x.parent;

        readnode(fp, fa);

        int id = child_index(fa, p);

        if (id == -1) return;

        if (id > 0) {
            fa.key[id - 1] = first_key(p);
            writenode(fp, fa);
        } else {
            change_parent_key(fp);
        }
    }

    void shrink_internal(int p) {
        Node x;
        readnode(p, x);

        if (p != h.root) return;
        if (x.leaf) return;
        if (x.cnt != 0) return;

        int c = x.son[0];

        h.root = c;
        set_parent(c, -1);
    }

    void insert_in_parent(int left, const Key &k, int right) {
        Node l;
        readnode(left, l);

        if (l.parent == -1) {
            int rp = newnode(0, -1);

            Node r;
            readnode(rp, r);

            r.cnt = 1;
            r.key[0] = k;
            r.son[0] = left;
            r.son[1] = right;

            writenode(rp, r);

            l.parent = rp;
            writenode(left, l);

            set_parent(right, rp);

            h.root = rp;

            return;
        }

        int fp = l.parent;

        Node fa;
        readnode(fp, fa);

        int id = child_index(fa, left);

        if (id == -1) return;

        for (int i = fa.cnt; i > id; --i) {
            fa.key[i] = fa.key[i - 1];
        }

        for (int i = fa.cnt + 1; i > id + 1; --i) {
            fa.son[i] = fa.son[i - 1];
        }

        fa.key[id] = k;
        fa.son[id + 1] = right;
        fa.cnt++;

        writenode(fp, fa);

        set_parent(right, fp);

        if (fa.cnt > MAXN) {
            split_internal(fp);
        }
    }

    void split_leaf(int p) {
        Node x;
        readnode(p, x);

        int q = newnode(1, x.parent);

        Node y;
        readnode(q, y);

        int mid = x.cnt / 2;
        int old = x.cnt;

        y.cnt = old - mid;

        for (int i = 0; i < y.cnt; ++i) {
            y.key[i] = x.key[mid + i];
        }

        x.cnt = mid;

        y.next = x.next;
        y.prev = p;

        if (x.next != -1) {
            Node z;
            readnode(x.next, z);
            z.prev = q;
            writenode(x.next, z);
        }

        x.next = q;

        writenode(p, x);
        writenode(q, y);

        insert_in_parent(p, y.key[0], q);
    }

    void split_internal(int p) {
        Node x;
        readnode(p, x);

        int q = newnode(0, x.parent);

        Node y;
        readnode(q, y);

        int old = x.cnt;
        int mid = old / 2;

        Key up = x.key[mid];

        y.cnt = old - mid - 1;

        for (int i = 0; i < y.cnt; ++i) {
            y.key[i] = x.key[mid + 1 + i];
        }

        for (int i = 0; i <= y.cnt; ++i) {
            y.son[i] = x.son[mid + 1 + i];
            set_parent(y.son[i], q);
        }

        x.cnt = mid;

        for (int i = x.cnt + 1; i < MAXN + 3; ++i) {
            x.son[i] = -1;
        }

        writenode(p, x);
        writenode(q, y);

        insert_in_parent(p, up, q);
    }

    void remove_child_from_parent(int fp, int child_pos) {
        Node fa;
        readnode(fp, fa);

        for (int i = child_pos; i < fa.cnt; ++i) {
            fa.son[i] = fa.son[i + 1];
        }

        fa.son[fa.cnt] = -1;
        fa.cnt--;

        writenode(fp, fa);

        if (fp == h.root) {
            if (fa.cnt == 0) {
                shrink_internal(fp);
            } else {
                rebuild_internal_keys(fp);
            }

            return;
        }

        if (fa.cnt < MIN_INTERNAL) {
            rebalance_internal(fp);
        } else {
            rebuild_internal_keys(fp);
            change_parent_key(fp);
        }
    }

    void rebalance_leaf(int p) {
        Node x;
        readnode(p, x);

        if (p == h.root) return;
        if (x.cnt >= MIN_LEAF) return;

        int fp = x.parent;

        Node fa;
        readnode(fp, fa);

        int id = child_index(fa, p);

        if (id == -1) return;

        int lp = -1;
        int rp = -1;

        if (id > 0) lp = fa.son[id - 1];
        if (id < fa.cnt) rp = fa.son[id + 1];

        if (lp != -1) {
            Node l;
            readnode(lp, l);

            if (l.cnt > MIN_LEAF) {
                for (int i = x.cnt; i > 0; --i) {
                    x.key[i] = x.key[i - 1];
                }

                x.key[0] = l.key[l.cnt - 1];

                x.cnt++;
                l.cnt--;

                writenode(lp, l);
                writenode(p, x);

                rebuild_internal_keys(fp);
                change_parent_key(fp);

                return;
            }
        }

        if (rp != -1) {
            Node r;
            readnode(rp, r);

            if (r.cnt > MIN_LEAF) {
                x.key[x.cnt] = r.key[0];
                x.cnt++;

                for (int i = 0; i + 1 < r.cnt; ++i) {
                    r.key[i] = r.key[i + 1];
                }

                r.cnt--;

                writenode(p, x);
                writenode(rp, r);

                rebuild_internal_keys(fp);
                change_parent_key(fp);

                return;
            }
        }

        if (lp != -1) {
            Node l;
            readnode(lp, l);

            for (int i = 0; i < x.cnt; ++i) {
                l.key[l.cnt + i] = x.key[i];
            }

            l.cnt += x.cnt;

            l.next = x.next;

            if (x.next != -1) {
                Node nxt;
                readnode(x.next, nxt);
                nxt.prev = lp;
                writenode(x.next, nxt);
            }

            x.cnt = 0;
            x.prev = -1;
            x.next = -1;

            writenode(lp, l);
            writenode(p, x);

            remove_child_from_parent(fp, id);

            return;
        }

        if (rp != -1) {
            Node r;
            readnode(rp, r);

            for (int i = 0; i < r.cnt; ++i) {
                x.key[x.cnt + i] = r.key[i];
            }

            x.cnt += r.cnt;

            x.next = r.next;

            if (r.next != -1) {
                Node nxt;
                readnode(r.next, nxt);
                nxt.prev = p;
                writenode(r.next, nxt);
            }

            r.cnt = 0;
            r.prev = -1;
            r.next = -1;

            writenode(p, x);
            writenode(rp, r);

            remove_child_from_parent(fp, id + 1);

            return;
        }
    }

    void rebalance_internal(int p) {
        Node x;
        readnode(p, x);

        if (p == h.root) {
            shrink_internal(p);
            return;
        }

        if (x.cnt >= MIN_INTERNAL) return;

        int fp = x.parent;

        Node fa;
        readnode(fp, fa);

        int id = child_index(fa, p);

        if (id == -1) return;

        int lp = -1;
        int rp = -1;

        if (id > 0) lp = fa.son[id - 1];
        if (id < fa.cnt) rp = fa.son[id + 1];

        if (lp != -1) {
            Node l;
            readnode(lp, l);

            if (l.cnt > MIN_INTERNAL) {
                for (int i = x.cnt + 1; i > 0; --i) {
                    x.son[i] = x.son[i - 1];
                }

                x.son[0] = l.son[l.cnt];

                set_parent(x.son[0], p);

                l.son[l.cnt] = -1;
                l.cnt--;
                x.cnt++;

                writenode(lp, l);
                writenode(p, x);

                rebuild_internal_keys(lp);
                rebuild_internal_keys(p);
                rebuild_internal_keys(fp);
                change_parent_key(fp);

                return;
            }
        }

        if (rp != -1) {
            Node r;
            readnode(rp, r);

            if (r.cnt > MIN_INTERNAL) {
                x.son[x.cnt + 1] = r.son[0];

                set_parent(x.son[x.cnt + 1], p);

                x.cnt++;

                for (int i = 0; i < r.cnt; ++i) {
                    r.son[i] = r.son[i + 1];
                }

                r.son[r.cnt] = -1;
                r.cnt--;

                writenode(p, x);
                writenode(rp, r);

                rebuild_internal_keys(p);
                rebuild_internal_keys(rp);
                rebuild_internal_keys(fp);
                change_parent_key(fp);

                return;
            }
        }

        if (lp != -1) {
            Node l;
            readnode(lp, l);

            int base = l.cnt + 1;

            for (int i = 0; i <= x.cnt; ++i) {
                l.son[base + i] = x.son[i];
                set_parent(x.son[i], lp);
            }

            l.cnt += x.cnt + 1;

            for (int i = 0; i <= x.cnt; ++i) {
                x.son[i] = -1;
            }

            x.cnt = 0;

            writenode(lp, l);
            writenode(p, x);

            rebuild_internal_keys(lp);

            remove_child_from_parent(fp, id);

            return;
        }

        if (rp != -1) {
            Node r;
            readnode(rp, r);

            int base = x.cnt + 1;

            for (int i = 0; i <= r.cnt; ++i) {
                x.son[base + i] = r.son[i];
                set_parent(r.son[i], p);
            }

            x.cnt += r.cnt + 1;

            for (int i = 0; i <= r.cnt; ++i) {
                r.son[i] = -1;
            }

            r.cnt = 0;

            writenode(p, x);
            writenode(rp, r);

            rebuild_internal_keys(p);

            remove_child_from_parent(fp, id + 1);

            return;
        }
    }

public:
    explicit BPT(const char *name) {
        init_cache();

        fp = std::fopen(name, "rb+");

        if (!fp) {
            fp = std::fopen(name, "wb+");
        }

        std::fseek(fp, 0, SEEK_END);

        long long sz = std::ftell(fp);

        if (sz < (long long)sizeof(Header)) {
            h.root = 0;
            h.tot = 1;

            writehead();

            Node r;
            initnode(r, 1, -1);

            writenode(0, r);
        } else {
            bool ok = readhead();

            if (!ok) {
                h.root = 0;
                h.tot = 1;

                writehead();

                Node r;
                initnode(r, 1, -1);

                writenode(0, r);
            }
        }
    }

    ~BPT() {
        writehead();
        std::fflush(fp);
        std::fclose(fp);
    }

    void flush() {
        writehead();
        std::fflush(fp);
    }

    void clear() {
        init_cache();

        h.root = 0;
        h.tot = 1;

        writehead();

        Node r;
        initnode(r, 1, -1);

        writenode(0, r);

        std::fflush(fp);
    }

    bool insert(const Key &k) {
        int p = find_leaf(k);

        Node x;
        readnode(p, x);

        int id = lower_pos(x, k);

        if (id < x.cnt && equalkey(x.key[id], k)) {
            return false;
        }

        for (int i = x.cnt; i > id; --i) {
            x.key[i] = x.key[i - 1];
        }

        x.key[id] = k;
        x.cnt++;

        writenode(p, x);

        if (id == 0 && p != h.root) {
            change_parent_key(p);
        }

        if (x.cnt > MAXN) {
            split_leaf(p);
        }

        return true;
    }

    bool erase(const Key &k) {
        int p = find_leaf(k);

        Node x;
        readnode(p, x);

        int id = lower_pos(x, k);

        if (id >= x.cnt || !equalkey(x.key[id], k)) {
            return false;
        }

        for (int i = id; i + 1 < x.cnt; ++i) {
            x.key[i] = x.key[i + 1];
        }

        x.cnt--;

        if (p == h.root) {
            x.prev = -1;
            x.next = -1;
            writenode(p, x);
            return true;
        }

        writenode(p, x);

        if (x.cnt < MIN_LEAF) {
            rebalance_leaf(p);
        } else if (id == 0) {
            change_parent_key(p);
        }

        return true;
    }

    bool find(const Key &k) {
        int p = find_leaf(k);

        Node x;
        readnode(p, x);

        int id = lower_pos(x, k);

        return id < x.cnt && equalkey(x.key[id], k);
    }

    bool lower_bound(const Key &k, Key &res) {
        int p = find_leaf(k);

        Node x;
        readnode(p, x);

        int id = lower_pos(x, k);

        while (true) {
            if (id < x.cnt) {
                res = x.key[id];
                return true;
            }

            p = x.next;

            if (p == -1) return false;

            readnode(p, x);
            id = 0;
        }
    }

    bool cursor_lower_bound(const Key &k, int &p, int &id, Key &res) {
        p = find_leaf(k);

        Node x;
        readnode(p, x);

        id = lower_pos(x, k);

        while (true) {
            if (id < x.cnt) {
                res = x.key[id];
                return true;
            }

            p = x.next;

            if (p == -1) return false;

            readnode(p, x);
            id = 0;
        }
    }

    bool cursor_next(int &p, int &id, Key &res) {
        if (p == -1) return false;

        Node x;
        readnode(p, x);

        id++;

        while (true) {
            if (id < x.cnt) {
                res = x.key[id];
                return true;
            }

            p = x.next;

            if (p == -1) return false;

            readnode(p, x);
            id = 0;
        }
    }
};

#endif