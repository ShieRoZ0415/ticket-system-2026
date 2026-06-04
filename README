# Ticket System

## 1. 项目简介

本项目实现一个简化版火车票订票系统，支持用户管理、车次管理、车票查询、换乘查询、购票、订单查询、退票和候补队列等功能。

程序通过标准输入读取命令，并通过标准输出返回结果。系统数据保存在本地文件中，程序多次启动后仍可继续使用已有数据。

## 2. 文件结构

```text
TicketSystem
├── main.cpp
├── BPT.h
├── Store.h
├── Struct.h
├── UserSystem.h
├── TrainSystem.h
├── OrderSystem.h
├── TicketSystem.h
├── Command.h
├── README.md
└── CMakeLists.txt
```

## 3. 文件说明

### main.cpp

程序入口。负责创建系统对象，进入命令读取和执行循环。

### BPT.h

通用 B+ 树模板。由之前实现的文件版 B+ 树改造而来，用于保存索引。

本项目中主要使用以下索引：

```text
user.bpt
train.bpt
station.bpt
order.bpt
queue.bpt
```

B+ 树中的 Key 只能是固定长度结构体，不能使用 `std::string`、指针或动态数组。

### Store.h

固定长度记录文件管理类。负责对数据文件进行追加、读取、修改和清空。

主要接口：

```cpp
template <class T>
class Store {
public:
    Store(const char *name);
    ~Store();

    int size();
    int append(const T &x);
    void read(int pos, T &x);
    void write(int pos, const T &x);
    void clear();
};
```

### Struct.h

保存所有基础结构体和索引 Key。

主要包括：

```cpp
struct UserRec;
struct TrainRec;
struct SeatRec;
struct OrderRec;

struct UserKey;
struct TrainKey;
struct StaKey;
struct OrderKey;
struct QueueKey;
```

### UserSystem.h

用户模块。负责用户注册、登录、登出、查询和修改。

主要类：

```cpp
class UserSystem;
```

主要接口：

```cpp
int add_user(...);
int login(...);
int logout(...);
int query_profile(...);
int modify_profile(...);
bool check_login(...);
void clear();
```

### TrainSystem.h

车次模块。负责添加车次、删除车次、发布车次和查询车次。

主要类：

```cpp
class TrainSystem;
```

主要接口：

```cpp
int add_train(...);
int delete_train(...);
int release_train(...);
int query_train(...);

bool get_train_by_id(...);
int station_id(...);

int get_seat(...);
void add_seat(...);
void sub_seat(...);
void clear();
```

### OrderSystem.h

订单模块。负责保存订单、查询订单和维护候补队列。

主要类：

```cpp
class OrderSystem;
```

主要接口：

```cpp
int add_order(...);
void read_order(...);
void write_order(...);

int query_order(...);

void add_queue(...);
void del_queue(...);
void clear();
```

### TicketSystem.h

票务模块。负责车票查询、换乘查询、购票和退票。

主要类：

```cpp
class TicketSystem;
```

主要接口：

```cpp
int query_ticket(...);
int query_transfer(...);
int buy_ticket(...);
int refund_ticket(...);
void clear();
```

### Command.h

命令解析和命令分发模块。负责读取输入命令、解析参数，并调用对应业务函数。

主要类：

```cpp
class Command;
class System;
```

## 4. 数据文件设计

系统使用固定长度记录文件保存完整数据。

```text
user.dat
train.dat
seat.dat
order.dat
```

其中：

`user.dat` 保存用户记录。

`train.dat` 保存车次记录。

`seat.dat` 保存每日每段余票。

`order.dat` 保存订单记录。

每条记录在文件中的编号记为 `pos`。索引中保存 `pos`。

## 5. 索引设计

系统使用 B+ 树保存索引。

```text
user.bpt
train.bpt
station.bpt
order.bpt
queue.bpt
```

### user.bpt

按用户名建立索引，用于快速查找用户。

Key 设计：

```cpp
struct UserKey {
    char username[21];
    int pos;
};
```

### train.bpt

按车次编号建立索引，用于快速查找车次。

Key 设计：

```cpp
struct TrainKey {
    char trainID[21];
    int pos;
};
```

### station.bpt

按车站名和车次编号建立索引，用于查询经过某个车站的所有车次。

Key 设计：

```cpp
struct StaKey {
    char station[31];
    char trainID[21];
    int pos;
};
```

`query_ticket` 和 `query_transfer` 主要复用这个索引。

### order.bpt

按用户名和反向时间建立索引，用于查询某个用户的所有订单。

Key 设计：

```cpp
struct OrderKey {
    char username[21];
    int revTime;
    int pos;
};
```

其中 `revTime` 用来让订单按新到旧排列。

### queue.bpt

按车次、日期和下单时间建立索引，用于维护候补订单。

Key 设计：

```cpp
struct QueueKey {
    char trainID[21];
    int trainDate;
    int time;
    int pos;
};
```

## 6. 存储方法

完整数据保存在 `.dat` 文件中，B+ 树只保存索引 Key。

例如用户查询流程为：

```text
1. 根据 username 构造 UserKey
2. 在 user.bpt 中查找对应 key
3. 得到用户记录在 user.dat 中的位置 pos
4. 通过 Store<UserRec> 读取完整用户信息
```

车次、订单和候补队列也采用类似方式。

## 7. query_ticket 设计

`query_ticket` 根据出发站、到达站和日期查询直达车票。

基本流程：

```text
1. 在 station.bpt 中找到所有经过出发站的车次
2. 逐个读取车次信息
3. 判断该车是否也经过到达站
4. 判断出发站是否在到达站之前
5. 根据用户给出的出发日期计算该车始发日期
6. 判断日期是否在售卖区间内
7. 查询对应区间余票
8. 保存符合条件的答案
9. 按 time 或 cost 进行排序输出
```


## 8. query_transfer 设计

`query_transfer` 查询恰好换乘一次的最优方案。

本项目采用较直接的枚举方法，主要复用 `station.bpt`。

基本流程：

```text
1. 在 station.bpt 中扫描所有经过出发站的第一程车次
2. 对每个第一程车次，枚举出发站之后的每个车站作为换乘站
3. 在 station.bpt 中扫描所有经过换乘站的第二程车次
4. 排除第一程和第二程为同一车次的情况
5. 判断第二程车次是否能从换乘站到达终点站
6. 判断第二程出发时间是否不早于第一程到达时间
7. 查询两段车票余票
8. 若方案合法，则计算总时间和总价格
9. 按题目要求更新最优答案
```

当 `-p time` 时，比较顺序为：

```text
总时间
总价格
第一程 trainID
第二程 trainID
```

当 `-p cost` 时，比较顺序为：

```text
总价格
总时间
第一程 trainID
第二程 trainID
```


## 9. clean 和 exit 设计

`clean` 清空所有数据文件和索引文件，并清空在线用户数组。

`exit` 输出 `bye`，程序结束。在线用户只保存在内存中，程序退出后自动全部下线。

## 10. 当前进度

目前完成初期设计，包括：

```text
1. 文件结构设计
2. B+ 树索引复用设计
3. 固定长度记录文件设计
4. 用户、车次、订单、票务模块划分
5. query_transfer 初步算法设计
```

下一步将按模块依次实现：

```text
1. Store 文件读写
2. BPT 索引测试
3. 用户模块
4. 车次模块
5. 车票查询
6. 购票和订单
7. 候补队列和退票
```