## 一致性hash

### 1 概念
一个分布式缓存系统，需要解决下面几个问题：
- 缓存数据(kv)应该比较均匀的分布在多台数据库(redis)中。
- 当某台redis数据库挂掉了。对kv数据进行查询操作时，只有存储在该redis中的kv数据受到影响（查不到），其他数据不受影响。
- 当新加入一台redis数据库时，影响的kv数据范围应该尽可能的小。

在分布式缓存和分布式数据库中，常使用一致性hash，根据hash的值，来决定将数据发往后端哪台真实的redis。

以分布式缓存为例（kv数据），rNode为真实的数据库（如redis），一个rNode对应多个vNode。vNode是虚拟节点，有很多个。
如果hash的结果是32bit的，则hash结果的分布区间为[-2^31, 2^31]。
如果hash(data)的结果落在v1所在的区间，则最终会路由到n1上；若hash(data)的结果落在v3所在的区间，则最终会路由到n3上。

### 2 代码
RNode代码
```cpp
// real node
class RNode
{
public:
    RNode(const char* ident, int vNodeNum, void* data): m_vNodeNum(vNodeNum), m_data(data) {
        memset(m_ident, 0x00, sizeof(m_ident));
        strncpy(m_ident, ident, strlen(ident));
        printf("%s RNode() \n", m_ident);
    }
    ~RNode() {
        printf("%s ~RNode() \n", m_ident);
    }

    const char* get_ident() const {
        return m_ident;
    }
    const int get_nodenum() const {
        return m_vNodeNum;
    }
    void* get_data() {
        return m_data;
    }
    void set_data(void* data) {
        m_data = data;
    }

private:
    RNode(const RNode&);
    RNode operator= (const RNode&);

private:
    char m_ident[128]; // 标识串，可以为机器的真实IP地址，如 "192.168.1.5"
    int m_vNodeNum; // 可以对应的vNode数目
    void* m_data; // 节点数据
};
```
VNode代码
```cpp
// virtual node
class VNode
{
public:
    VNode(const int hash, const RNode* node): m_hash(hash), m_rNode(node){
//        printf("VNode() \n");
    }
    ~VNode() {
//        printf("~VNode() \n");
    }

    void set_rNode(const RNode* node) {
        m_rNode = node;
    }

    const RNode* get_rNode() const {
        return m_rNode;
    }

    long int get_hash() const {
        return m_hash;
    }
public:
    // 重载运算符，以便支持 AVL树的比较运算，AVL算法
    bool operator < (const VNode& v) {
        return m_hash < v.m_hash;
    }
    bool operator > (const VNode& v) {
            return m_hash > v.m_hash;
        }
    bool operator == (const VNode& v) {
        return m_hash == v.m_hash;
    }
    bool operator >= (const VNode& v) {
        return m_hash >= v.m_hash;
    }

private:
    const int m_hash;     // hash的计算结果
    const RNode* m_rNode; // a pointer to Real node
};
```
hash函数
```cpp
class HashFunc  // 虚函数接口
{
public:
    virtual ~HashFunc() {}
    virtual int hash_value(const char* str) = 0; //计算hash值，32bit
};

class MD5HashFunc: public HashFunc
{
public:
    virtual int hash_value(const char* str)
    {
        long hash = 0;
        char digest[16];
        md5_string((const unsigned char*)str, digest, strlen(str));
        for (int i = 0; i < 4; ++i) {
            hash += (digest[i*4+3]&0xFF << 24)
                    | (digest[i*4+2]&0xFF << 16)
                    | (digest[i*4+1]&0xFF << 8)
                    | (digest[i*4]&0xFF);
        }

        return hash;
    }
};
```

提供给AVL树的打印函数
```cpp
void pfunc(const void* v) {
    VNode & pv = *(VNode*)v;
    printf("[%9d]-vNode[%p]-rNode[%p]: identR[%s]\n", (int)pv.get_hash(), (void*)&pv, pv.get_rNode(), pv.get_rNode()->get_ident());
}
```

一致性hash算法类
```cpp
class CContHash
{
public:
    CContHash(HashFunc* func): m_func(func), m_avltree(pfunc) {
    }
    ~CContHash() {
        Exit();
    }
    // 往系统中加入一个real节点
    bool add_rNode(const RNode* pRNode) {
        int pLen = strlen(pRNode->get_ident());
        char buf[256] = {0};
        strncpy(buf, pRNode->get_ident(), pLen);
        const int num = pRNode->get_nodenum();
        for (int i = 0; i < num; ++i) {
            sprintf(buf+pLen, "%d", i);
            int hash = m_func->hash_value(buf);
            VNode* pVNode = new VNode(hash, pRNode);
            printf("pVNode new[%p] \n", pVNode);
            m_avltree.Insert(*pVNode);
            m_set.insert(pVNode);
        }
        return true;
    }
    // 删除一个real节点
    bool del_rNode(const RNode* pRNode) {
        int pLen = strlen(pRNode->get_ident());
        char buf[256] = {0};
        strncpy(buf, pRNode->get_ident(), pLen);
        const int num = pRNode->get_nodenum();
        for(int i = 0; i < num; ++i) {
            sprintf(buf+pLen, "%d", i);
            int hash = m_func->hash_value(buf);
            VNode node(hash, NULL);
            AVL_tree<VNode&>::AVLNode* p = m_avltree.Find(node);
            if (p != NULL) {
                m_avltree.Delete(node);
            }

        }
        return true;
    }
    // 路由算法
    const RNode* router (const char* str) {
        int hash = m_func->hash_value(str); // 计算 hash值
        VNode node(hash, NULL);
        const AVL_tree<VNode&>::AVLNode* p = m_avltree.Find_conhash(node);
        if (p != NULL) {
            return p->data.get_rNode();
        }

        return NULL;
    }
    // 打印所有的虚拟节点
    void Print() {
        m_avltree.Inorder_traversal();
    }
    // 释放资源
    void Exit() {
        std::set<VNode*>::iterator it = m_set.begin();
        for (; it != m_set.end(); ++it) {
            delete *it;
        }
    }
private:
    HashFunc* m_func; // hash函数
    AVL_tree<VNode&> m_avltree; // 使用avl树进行查找
    std::set<VNode*> m_set;
};
```

测试代码
```cpp
void test() {
    MD5HashFunc hashfun;
    RNode* pRNode1 = new RNode("192.168.1.1", 5, NULL); // new real node，每个real node对应5个 virtual node
    RNode* pRNode2 = new RNode("192.168.1.2", 5, NULL);
    RNode* pRNode3 = new RNode("192.168.1.3", 5, NULL);
    RNode* pRNode4 = new RNode("192.168.1.4", 6, NULL);
    RNode* pRNode5 = new RNode("192.168.1.5", 5, NULL);

    CContHash co(&hashfun);  
    co.add_rNode(pRNode1);  // 添加 real node
    co.add_rNode(pRNode2);
    co.add_rNode(pRNode3);
    co.add_rNode(pRNode4);
    co.add_rNode(pRNode5);

    co.Print();
    printf("------------\n");
    co.del_rNode(pRNode4); // 删除 real node
    co.Print();
    delete pRNode4;

    for (int i = 0; i < 30; ++i) {
        char buf[8] = {0};
        sprintf(buf, "#%.2d: ", i);
        const RNode* p = co.router(buf); // 路由
        int hash = hashfun.hash_value(buf);
        printf("[%s] [%9d] [%s] \n", buf, hash, p->get_ident());
    }

    delete pRNode1;
    delete pRNode2;
    delete pRNode3;

    delete pRNode5;
}
```
