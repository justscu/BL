### 1 命名空间
使用方法
```cpp
#include <iostream>
#include <string>
using  std::string;
using  std::cin;
using  std::cout;

// 或者 using namespace std;

int a = 5;
cout << a << endl;  
// 若没有使用using namespace std::cout;，则需要用std::cout << a << std::endl;
```

### 2 类型
顺序容器，根据位置来存储和访问这些元素
* vector - 支持快速随机访问
* list - 支持快速插入/删除
* deque - 双端队列

顺序容器适配器
* stack - 后进先出（LIFO）栈
* queue - 先进先出（FIFO）队列
* priority_queue - 有优先级管理的队列

关联容器，其元素按键（key）排序
* map - 关联数组；元素可以通过键来存储和读取
* set - 大小可变的集合，支持通过键快速读取
* multimap - 支持一个键出现多次的map
* multiset - 支持一个键出现多次的set

map是STL的一个关联容器，它提供`KV对`的数据处理能力(其中K称为关键字，每个关键字只能在map中出现一次，V称为该关键字的值)，由于这个特性，map内部的实现自建一颗红黑树(一种非严格意义上的平衡二叉树)，这颗树具有对数据自动排序的功能。

set是集合，set中不会包含重复的元素，这是和vector的区别。set的实现采用了平衡二叉树，因此，set中的元素必须是可排序的。如果是自定义的类型，那在定义
类型的同时必须给出运算符`<`的定义。

对于我们自己定义的类或结构，系统一般不能替我做比较运算，需要我们自己定义相应的运算符`<`。
```cpp
bool operator<(const MyType &x, const MyType &y) {
        // Return true if x<y, false if x>=y
}
```

### 3 标准库string

```cpp
#include <string>
using namespace std::string;

// 读取一整行的内容
string lineStr;
while(getline(cin, lineStr)) {
    cout << lineStr << endl;
}

string s;
// 常用函数
s.empty(),
s.size(), 
s.length(),// 返回string::size_type类型，是一个unsinged 类型，长度根据编译器来决定是int,long ...
s.c_str(), // 返回常量字符串
s[n] // 返回第n个字符的值

for(string::size_type i = 0; i < s.zize(); ++i) {
	s[i] = i;
}

// 赋值函数
string s1("test_s1");
string s2("test_s2--------");
s2 = s1; // 程序的做法，需要先释放掉s2的内存，然后重新申请一块内存，来存放新的数据

// 字符处理，在cctype头文件中。用于判断这些字符是否为数字、字母、控制字符、大写、小写等
isalnum(); 
isalpha();
iscntrl();
```

### 4 标准库vector
一个vector类似于一个动态的一维数组。vector对象（以及其他标准库容器对象）可以在运行的时候，高效的添加元素。
可以给vector预先分配好预定个数的内存，但更有效的方法，是先初始化一个空的vector对象，然后再动态的增加元素。
一个二维数组的方法：`vector< vector<int> > v;`

```cpp
// 头文件
#include <vector>
using namespace std::vector;

// 常用函数
v.size();
v.empty();
v.push_back(t) // 在末尾添加元素 
v[n]  // 要注意下标操作是否会越界
v.begin()
v.end() // vector的“末端元素的下一个”，指向一个不存在的元素。
```

添加元素的正确做法是`v.push_back(t);`，
不能对不存在的元素进行下标操作。对不存在的元素进行下标操作，会导致缓冲区溢出。

#### 4.1 迭代器 iterator & const_iterator
```cpp
vector<int> v(50);
if (vector<int>::iterator it = v.begin(); it != v.end(); ++it) {
	*it = 0; //赋值为0
}
```
迭代器iterator也可以用来比较。当it1 和 it2指向同一个元素时，才相等。
```cpp
if(vector<string>:: const_iterator it = v.begin(); it != v.end(); ++it){
	cout << *it << end; //不能通过该迭代器，改变元素的内容
}

// 该迭代器，只能够指向v.begin()，不能够指向其他地方，但可以通过该迭代器修改所指向的内容。
const vector<string>:: iterator it = v.begin(); 
```
#### 4.2 迭代器的计算
迭代器可以支持it++, it--这类操作，也可以支持iter+n ,iter-n。

也支持`iter1 - iter2`，得到的是`vector<T>::difference_type`类型，或者`vector<T>::size_type`类型，这2种类型，定义的是signed，

`vector<T>::iterator mid = v.begin+ v.size()/2;` 但不能够使用 `(v.begin() + v.end())/2;`

### 5 迭代器
#### 5.1 迭代器提供的运算
- *iter
- iter->mem
- ++iter, --iter, iter++, iter--
- iter1 == iter2 ，当2个迭代器指向同一个容器中的同一个元素时，才相等
- vector & deque提供的额外运算: `iter+n ,  iter-n,  iter1 += iter2, > , < >=, <=`，和指针的操作一样，不同于链表的操作

#### 5.2 **迭代器失效**
修改容器的内在状态 或 移动容器内的元素时，要特别注意。这些操作会使指向被移动的元素的迭代器失效，也可能会使其他迭代器失效(如erase操作)。


### 6 顺序容器
顺序容器根据位置来存储和访问元素，如vector/list/deque。
- list   - 快速插入、删除。但随机访问的速度慢
- vector - 快速的随机访问。push_back的速度也快。但在前面和中间插入、删除的速度慢
- deque  - 快速的随机访问。push_front, push_back速度快。但在中间插入、删除的速度慢 

#### 6.1 常用类型
```cpp
size_type       // unsigned类型
iterator        // 迭代器
const_iterator 
difference_type  // 两个迭代器的差值，signed类型，可能为负值
value_type  　　　　　// 元素类型  
reference, const_reference // 元素的左值类型，同value_type&，如 list<int>::reference val = *ilist.begin();
```

#### 6.2 常用操作
任何push/insert/delete/resize/erase都可能导致**迭代器失效**。
当编写循环将元素插入到vector或deque中时，程序要确保每次循环后，迭代器都得到更新。
```cpp
// 错误的示范
vector<int>::iterator first = v.begin(), last = v.end();
while(first != last){
	first = v.insert(fisrt, 2); //插入数据后，last失效。返回一个迭代器，指向新插入的元素
	++first;
}

// 正确的示范
vector<int>::iterator first = v.begin();
while(first != v.end()){ //每次程序重新计算v.end值
	first = v.insert(first, 2);
	++first;
}
```

```cpp
c.begin(), c.end()  // 返回迭代器
c.front(), c.back() // 返回元素

c.push_back(t)     // 在容器c的尾部添加元素t，返回void
c.push_front(t)    // 在容器c的前段，添加元素t，返回void
c.insert(p, t)     // 在 p所指向 的元素的前面，插入t，返回指向新元素的迭代器
c.insert(p, n, t)    // 在p所指向的元素的前面，插入n个t，返回void
c.insert(p, b, e)    // 在p所指向 的元素的前面，插入b到e之间的元素，返回void

c.erase(p)     // 删除p指向的元素，返回指向p的下一个元素的迭代器 。当p指向c.end()时，行为未定义
c.erase(b, e)  // 删除b, e间的所有元素，返回指向e的下一个元素的迭代器
c.clear()      // 等价于 c.erase(c.begin(), c.end()); 返回void
c.pop_back()   // 只支持 list& deque，返回void
c.pop_front()  // 只支持 list& deque，返回void

while(c.empty() == false) {
	process(c.front());  // 返回第一个元素
	c.pop_front();       // 删除第一个元素，返回void
}

c.size()      // 返回c中有的元素的个数， c::size_type
c.max_size()  // 返回c中可以容纳的元素个数， c::size_type
c.empty()     // 为空时，返回true.
c.resize(n), c.resize(n, t)     

c[n], c.at(n); // 使用下标操作时，一定要注意下标是否会越界；越界会使程序崩溃，只支持vector & deque
```

#### 6.3 赋值操作
assign、=、swap

```cpp
c1 = c2         //先删除c1的所有元素，再将c2的元素赋值到c1.  c1 & c2，必须要有相同的类型
c1.assign(v.begin(), v.end())  // 先删除c1的所有元素。c1和v的类型可以不同。
c1.swap(c2)  // 交换c1 & c2中的元素，c1 & c2的类型要相同
```

### 7 关联容器
关联容器包括map, set , multimap, multiset等，通过键（key）来存储和读取元素。

如pair类型
```cpp
pair<string, int> p1;
string f;
int s;
while(cin >> f >> s;) {
	p1 = make_pair(f, s);
}
```

### 8 关联容器map

在使用关联容器时，K必须有类型，而且K必须有个相关的比较函数（键必须定义"<"函数，其他的比较函数不做要求）
#### 8.1 定义的类型
```cpp
// 键的类型
map<K, V>::key_type
// pair的类型。它的first具有const map<K, V>::key_type类型，second具有map<K, V>::mapped_type类型
map<K, V>::mapped_type
// 键所关联的值的类型
map<K, V>::value_type
```

#### 8.2 查找
常用的方法有find、count，[]，
下标操作，有个副作用：当下标的元素不存在时，会创建一个元素。
```cpp
map<string, int> comp_count;
comp_count["dzh"] = 1; // 若找到，替换该值；若没找到，为map创建一个新K-V值，并插入该值到map中
int i = comp_count["zdwx"]; // 若找到，返回该值；若没有找到，创建一个新的K-V值，插入到map中，并返回V。

// 无副作用的做法
if(0 != comp_count.count("foobar")) { //计算个数，对map，返回0或1
	int occurs = comp_count["foobar"];
}

//更高效的做法
map<string, int>::iterator it = comp_count.find("foobar");
if(it != comp_count.end()){
	int occurs = it->second;
}
```

#### 8.3 insert
- 若插入的K值存在，则原map中K-V值不变，返回pair对象。其类型为pair<map<string, int>::iterator, bool>。bool为false.
- 若插入的K值不存在，则插入新的K-V值到map中，返回pair对象。其类型为pair<map<string, int>::iterator, bool>。bool为true.
- comp_count.insert(map<string, int>::value_type("Anna", 1));
- comp_count.insert(make_pair<string, int>("Anna", 1));

#### 8.4 erase
- m.erase(K);  删除键为K的元素，返回size_type类型，表示删除的个数 【顺序容器返回被删除的元素的下一个iterator】
- m.erase(p); 删除迭代器it指向的元素。p指向的元素必须存在，且不能为m.end(), 返回void。
- m.erase(b, e); 删除 [b, e)间的元素。b指向的元素必须存在或m.end()

```cpp
map<string, reg_info>::iterator it = g_map_reg_info.begin();
//错误的删除方法
for(; it != g_map_reg_info.end();++it){
    if(XXX){
        g_map_reg_info.erase(it); //把原来的it删掉了，后面再++it，会崩溃
    }
}
//正确的删除方法
for(; it != g_map_reg_info.end(); ) {
    if(XXX) {
        g_map_reg_info.erase(it++); //正确的删除方法
       // 操作过程为:（1）先把it备份一份（2）再把原来的it进行++（3）再把备份的it返回
    }else{
        ++it;
    }
}
```

### 9 关联容器set
set是单纯的K的集合，K的值必须唯一。set中的V唯一，顺序容器（vector、list）中的V可以重复。
set,　list不支持下标访问，vector,deque,map支持下标访问。
map和set中的K，是不能够修改的。

支持insert、count、find、erase操作；不支持下标操作

### 10 关联容器 multimap & multiset

multimap/multiset，允许一个K，对应多个实例。
在multimap/multiset中，相同的K及其实例，都是相邻地存放的。
multimap，不支持下标操作。

#### 10.1 删除
- erase(K)；带K时，删除所有拥有该K的元素，并返回被删除的个数。
- erase(p)；带迭代器时，只删除迭代器指定的元素，并返回void。

#### 10.2 查找方法
```cpp
multimap<string, string> test;
multimap<string, string>::size_type c = test.count("aaaa"); //返回K=“aaa”的值的个数
multimap<string, string>::iterator iter = test.find("aaaa");
for(multimap<string, string>::size_type ct = 0; ct != c; ++ct, ++iter){
	cout << iter->second << endl;
}

// 或

typedef  multimap<string, string>::iterator authors_it;
authors_it beg = authors.lower_bound("aaaa"); //若查找的元素存在，则返回一个迭代器，指向第一个实例
authors_it end = authors.upper_bound("aaaa"); //返回的位置为最后一个实例的下一个位置
while(beg != end){
	cout << beg->second << endl;
	++beg;
}
// 若没有找到相关的元素，lower_bound, upper_bound返回相同的迭代器：都指向同一个元素或指向multimap的超出末端位置。

// 或

typedef  multimap<string, string>::iterator authors_it;
pair<authors_it, authors_it> pos = authors.equal_range("aaaa"); 
//equal_range返回pair类型。pair.first指向第一个位置，pair.second指向最后一个元素的下一个位置
while(pos.first != pos.second){
	cout << pos.first->second << endl;
	++pos.first;
}
```

### 11 erase的正确方法
```cpp
// map
std::map<char> m;
// return an iterator to the element that follows the last element removed
// (or map::end, if the last element was removed).
for (auto it = m.begin(); it != m.end();) {
    if (it->second == 'b' || it->second == 'c' || it->second == 'd') {
        it = m.erase(it);
        // m.erase(it++); // 用这个也正确
    } else { ++it; }
}

// vector
std::vector<char> v;
// An iterator pointing to the new location of the element that followed 
// the last element erased by the function call.
// This is the container end if the operation erased the last element 
// in the sequence.
for (auto it = v.begin(); it != v.end();) {
    if (*it == 'b' || *it == 'c' || *it == 'd') {
        it = v.erase(it);
        // v.erase(it++); // 不正确
    } else { ++it; }
}

// list
std::list<char> l;
// An iterator pointing to the element that followed the last element erased by the function call.
// This is the container end if the operation erased the last element in the sequence.
for (auto it = l.begin(); it != l.end();) {
    if (*it == 'b' || *it == 'c' || *it == 'd') {
        it = l.erase(it);
        // l.erase(it++); // 用这个也正确
    } else { ++it; }
}

// set
std::set<char> s;
// return an iterator to the element that follows the last element removed 
// (or set::end, if the last element was removed).
for (auto it = s.begin(); it != s.end();) {
    if (*it == 'a' || *it == 'b' || *it == 'c') {
        it = s.erase(it);
        // s.erase(it++); // 用这个也正确
    } else { ++it; }
}
``` 
