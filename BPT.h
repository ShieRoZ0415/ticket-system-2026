#include <iostream>
#include <string>
#include <cstdio>

using namespace std;

const int MAXN = 100;
const int INT_NEG = (-2147483647 - 1);

struct Key {
    char s[65];
    int v;
};

int cmpstr(const char *a, const char *b) {
    for (int i = 0; i < 65; i++) {
        unsigned char x = (unsigned char)a[i];
        unsigned char y = (unsigned char)b[i];

        if (x < y) return -1;
        if (x > y) return 1;
        if (x == 0 && y == 0) return 0;
    }

    return 0;
}

int cmpkey(const Key &a, const Key &b) {
    int c = cmpstr(a.s, b.s);

    if (c != 0) return c;
    if (a.v < b.v) return -1;
    if (a.v > b.v) return 1;

    return 0;
}

bool lesskey(const Key &a, const Key &b) {
    return cmpkey(a, b) < 0;
}

bool equalkey(const Key &a, const Key &b) {
    return cmpkey(a, b) == 0;
}

Key makekey(const string &s, int v) {
    Key k;

    for (int i = 0; i < 65; i++) k.s[i] = 0;

    int n = (int)s.size();

    if (n > 64) n = 64;

    for (int i = 0; i < n; i++) k.s[i] = s[i];

    k.v = v;

    return k;
}

struct Header {
    int root;
    int tot;
};

struct Node {
    int leaf;   // 1--leaf ; 0 -- internal
    int cnt;    // key 数量
    int parent;     // 根节点 parent = -1
    int next;
    int prev;
    Key key[MAXN + 2];  // 防溢出
    int son[MAXN + 3];
};

const int CSIZE = 2048;

Node cache_node[CSIZE];
int cache_id[CSIZE];
bool cache_valid[CSIZE];
bool cache_dirty[CSIZE];

class BPT {
private:
    FILE *fp;
    Header h;

    long long off(int p) {
        return sizeof(Header) + 1ll * p * sizeof(Node);
    }

    void initnode(Node &x, int leaf, int parent) {
        x.leaf = leaf;
        x.cnt = 0;
        x.parent = parent;
        x.next = -1;
        x.prev = -1;

        for (int i = 0; i < MAXN + 2; i++) {
            x.key[i] = makekey("", 0);
        }

        for (int i = 0; i < MAXN + 3; i++) {
            x.son[i] = -1;
        }
    }

    bool readhead() {
        fseek(fp, 0, SEEK_SET);
        return fread(&h, sizeof(h), 1, fp) == 1;
    }

    void writehead() {
        fseek(fp, 0, SEEK_SET);
        fwrite(&h, sizeof(h), 1, fp);
    }

    bool raw_read(int p, Node &x) {
        fseek(fp, off(p), SEEK_SET);
        return fread(&x, sizeof(x), 1, fp) == 1;
    }

    void raw_write(int p, const Node &x) {
        fseek(fp, off(p), SEEK_SET);
        fwrite(&x, sizeof(x), 1, fp);
    }

    void readnode(int p, Node &x) {
        int c = p % CSIZE;

        if (cache_valid[c] && cache_id[c] == p) {
            x = cache_node[c];
            return;
        }

        if (cache_valid[c] && cache_dirty[c]) {
            raw_write(cache_id[c], cache_node[c]);
        }

        raw_read(p, cache_node[c]);

        cache_id[c] = p;
        cache_valid[c] = true;
        cache_dirty[c] = false;

        x = cache_node[c];
    }

    void writenode(int p, const Node &x) {
        int c = p % CSIZE;

        if (cache_valid[c] && cache_id[c] != p && cache_dirty[c]) {
            raw_write(cache_id[c], cache_node[c]);
        }

        cache_node[c] = x;
        cache_id[c] = p;
        cache_valid[c] = true;
        cache_dirty[c] = true;
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

            if (lesskey(x.key[m], k)) l = m + 1;
            else r = m;
        }

        return l;
    }

    int go_pos(const Node &x, const Key &k) {
        int l = 0;
        int r = x.cnt;

        while (l < r) {
            int m = (l + r) >> 1;

            if (lesskey(k, x.key[m])) r = m;
            else l = m + 1;
        }

        return l;
    }

    int child_index(const Node &x, int c) {
        for (int i = 0; i <= x.cnt; i++) {
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

    void change_parent_key(int p) {
        if (p == h.root) return;

        Node x;
        Node fa;

        readnode(p, x);

        int fp = x.parent;

        readnode(fp, fa);

        int id = child_index(fa, p);

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

        if (x.cnt != 0) return;

        int c = x.son[0];

        if (p == h.root) {
            h.root = c;
            set_parent(c, -1);
            return;
        }

        int fp = x.parent;

        Node fa;

        readnode(fp, fa);

        int id = child_index(fa, p);

        if (id == -1) return;

        fa.son[id] = c;

        writenode(fp, fa);

        set_parent(c, fp);

        if (id > 0) {
            fa.key[id - 1] = first_key(c);
            writenode(fp, fa);
        } else {
            change_parent_key(fp);
        }
    }

    void remove_empty_leaf(int p, Node &x) {
        int lp = x.prev;
        int rp = x.next;

        if (lp != -1) {
            Node l;

            readnode(lp, l);

            l.next = rp;

            writenode(lp, l);
        }

        if (rp != -1) {
            Node r;

            readnode(rp, r);

            r.prev = lp;

            writenode(rp, r);
        }

        if (p == h.root) {
            x.prev = -1;
            x.next = -1;

            writenode(p, x);

            return;
        }

        int fp = x.parent;

        Node fa;

        readnode(fp, fa);

        int id = child_index(fa, p);

        if (id == -1) {
            x.prev = -1;
            x.next = -1;

            writenode(p, x);

            return;
        }

        int kp;

        if (id == 0) kp = 0;
        else kp = id - 1;

        for (int i = kp; i + 1 < fa.cnt; i++) {
            fa.key[i] = fa.key[i + 1];
        }

        for (int i = id; i < fa.cnt; i++) {
            fa.son[i] = fa.son[i + 1];
        }

        fa.son[fa.cnt] = -1;
        fa.cnt--;

        x.prev = -1;
        x.next = -1;

        writenode(p, x);
        writenode(fp, fa);

        if (fa.cnt == 0) {
            shrink_internal(fp);
        } else if (id == 0) {
            change_parent_key(fp);
        }
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

        for (int i = fa.cnt; i > id; i--) {
            fa.key[i] = fa.key[i - 1];
        }

        for (int i = fa.cnt + 1; i > id + 1; i--) {
            fa.son[i] = fa.son[i - 1];
        }

        fa.key[id] = k;
        fa.son[id + 1] = right;
        fa.cnt++;

        writenode(fp, fa);

        set_parent(right, fp);

        if (fa.cnt > MAXN) split_internal(fp);
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

        for (int i = 0; i < y.cnt; i++) {
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

        for (int i = 0; i < y.cnt; i++) {
            y.key[i] = x.key[mid + 1 + i];
        }

        for (int i = 0; i <= y.cnt; i++) {
            y.son[i] = x.son[mid + 1 + i];
            set_parent(y.son[i], q);
        }

        x.cnt = mid;

        writenode(p, x);
        writenode(q, y);

        insert_in_parent(p, up, q);
    }

public:
    BPT(const char *name) {
        fp = fopen(name, "rb+");

        if (!fp) fp = fopen(name, "wb+");

        setvbuf(fp, 0, _IOFBF, 1 << 20);

        fseek(fp, 0, SEEK_END);

        long long sz = ftell(fp);

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
        flush_cache();
        writehead();
        fflush(fp);
        fclose(fp);
    }

    void flush_cache() {
        for (int i = 0; i < CSIZE; i++) {
            if (cache_valid[i] && cache_dirty[i]) {
                raw_write(cache_id[i], cache_node[i]);
                cache_dirty[i] = false;
            }
        }
    }

    void insert(const string &s, int v) {
        Key k = makekey(s, v);

        int p = find_leaf(k);

        Node x;

        readnode(p, x);

        int id = lower_pos(x, k);

        if (id < x.cnt && equalkey(x.key[id], k)) return;

        for (int i = x.cnt; i > id; i--) {
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
    }

    void erase(const string &s, int v) {
        Key k = makekey(s, v);

        int p = find_leaf(k);

        Node x;

        readnode(p, x);

        int id = lower_pos(x, k);

        if (id >= x.cnt || !equalkey(x.key[id], k)) return;

        for (int i = id; i + 1 < x.cnt; i++) {
            x.key[i] = x.key[i + 1];
        }

        x.cnt--;

        if (x.cnt == 0) {
            remove_empty_leaf(p, x);
            return;
        }

        writenode(p, x);

        if (id == 0 && p != h.root) {
            change_parent_key(p);
        }
    }

    void find(const string &s) {
        Key k = makekey(s, INT_NEG);

        int p = find_leaf(k);

        Node x;

        readnode(p, x);

        int id = lower_pos(x, k);
        bool ok = false;

        while (p != -1) {
            while (id < x.cnt) {
                int c = cmpstr(x.key[id].s, k.s);

                if (c > 0) {
                    if (!ok) cout << "null";
                    cout << '\n';
                    return;
                }

                if (c == 0) {
                    if (ok) cout << ' ';
                    cout << x.key[id].v;
                    ok = true;
                }

                id++;
            }

            p = x.next;

            if (p != -1) {
                readnode(p, x);
                id = 0;
            }
        }

        if (!ok) cout << "null";

        cout << '\n';
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(0);

    BPT t("bpt.dat");

    int n;

    cin >> n;

    string op;
    string idx;
    int val;

    for (int i = 0; i < n; i++) {
        cin >> op >> idx;

        if (op[0] == 'i') {
            cin >> val;
            t.insert(idx, val);
        } else if (op[0] == 'd') {
            cin >> val;
            t.erase(idx, val);
        } else {
            t.find(idx);
        }
    }

    return 0;
}