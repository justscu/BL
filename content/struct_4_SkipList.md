## SkipList

SkipList是一种有序的链表(如从小到大排列)，跟普通链表不同的是，它有多个next指针，所以可以实现快速查找。
LevelDB是通过SkipList来实现快速查找的。
**SkipList的插入速度和删除速度慢，但查找速度快**。

![image](https://github.com/justscu/BL/blob/master/pics/struct_4_1.png)

### 1 SkipList的节点结构
```cpp
struct Node
{
    T score;  // 存放具体的数据
    Node* backward; // 后向指针
    struct SkipListLevel
    {
        Node* forward; //前向指针，有多个
    } level[]; // sizeof时，不占用空间
};
// 注意，sizeof(Node) = sizeof(T) + sizeof(Node*)，sizeof(level) = 0。
```


### 2 模板实现
```cpp
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define SKIPLIST_MAXLEVEL 8

template<class T>
class SkipList
{
private:
    struct Node
    {
        T score;
        Node* backward;
        struct SkipListLevel
        {
            Node* forward;
        } level[]; // sizeof时，不占用空间
    };
public:
    SkipList() :
        m_head(NULL), m_tail(NULL), m_length(0), m_cur_level(0)
    {
        Create();
    }
    ~SkipList() {
    }

    void Free() {
        Node* pNode = m_head->level[0].forward;
        free_node(m_head); //释放头节点
        while (pNode != NULL)
        {
            Node* pNext = pNode->level[0].forward;
            free(pNode);
            pNode = pNext;
        }
    }
    // @return: -1, failed; 0, success.
    int Delete(const T & score) {
        Node* update[SKIPLIST_MAXLEVEL];
        Node* node = m_head;

        for (int i = m_cur_level - 1; i >= 0; --i)
        {
            while (node->level[i].forward != NULL
             && node->level[i].forward->score < score)
            {
                node = node->level[i].forward;
            }

            update[i] = node;
        }

        node = node->level[0].forward;

        if (node != NULL && node->score == score)
        {
            delete_node(node, update);
            free_node(node);
            return 0;
        }

        return -1;
    }
    int Insert(const T & score) {
        Node* node = m_head;
        Node* update[SKIPLIST_MAXLEVEL]; // 需要更新的node
        for (int i = m_cur_level - 1; i >= 0; --i)
        {
            while (node->level[i].forward != NULL
             && node->level[i].forward->score < score) // 从小到大排列
            {
                node = node->level[i].forward;
            }
            update[i] = node;
        }

        int level = gen_random_level();
        if (level > m_cur_level)
        {
            for (int i = m_cur_level; i < level; ++i)
                update[i] = m_head;
            m_cur_level = level;
        }

        node = create_node(level, score);
        for (int i = 0; i < level; ++i)
        {
            node->level[i].forward = update[i]->level[i].forward;
            update[i]->level[i].forward = node;
        }

        node->backward = (update[0] == m_head) ? NULL : update[0];
        if (node->level[0].forward == NULL)
        {
            m_tail = node;
        }
        else
        {
            node->level[0].forward->backward = node;
        }
        ++m_length;

        return 0;
    }
    // @return, 0:find; else, not find.
    int Search(const T& score) {
        Node* node = m_head;
        for (int level = m_cur_level - 1; level >= 0; --level)
        {
            while (node->level[level].forward != NULL
             && node->level[level].forward->score < score)
            {
                printf("*");
                node = node->level[level].forward;
            }
        }

        node = node->level[0].forward;
        if (node != NULL && node->score == score)
        {
            return 0; // find it
        }

        return -1; // not find.
    }

    void Print() {
        for (int i = m_cur_level - 1; i >= 0; --i)
        {
            Node* node = m_head->level[i].forward;
            printf("\nlevel[%d]: ", i);
            for (; node != NULL; node = node->level[i].forward)
            {
                printf("%d-> ", (int) node->score);
            }
        }
    }

private:
    SkipList(const SkipList&);
    SkipList& operator=(const SkipList&);
    // 生成随机数
    int gen_random_level(void) {
        int level = 1;
        while ((rand() & 0xFFFF) < (0.5 * 0xFFFF))
        {
            level += 1;
        }
        return (level > SKIPLIST_MAXLEVEL) ? SKIPLIST_MAXLEVEL : level;
    }
    // 构造一个头节点, 是个SKIPLIST_MAXLEVEL级别的
    void Create(void) {
        m_head = (Node*) malloc(sizeof(Node) + SKIPLIST_MAXLEVEL
         * sizeof(struct Node::SkipListLevel));
        assert(m_head);
        m_head->backward = NULL;
        for (int i = 0; i < SKIPLIST_MAXLEVEL; ++i)
        {
            m_head->level[i].forward = NULL;
        }

        m_length = 0;
        m_cur_level = 1;
        m_tail = NULL;
    }

    // 创建一个node. @level, 有多少级别，
    Node* create_node(int level, const T & score) {
        Node* p = (Node*) malloc(sizeof(Node) + level
         * sizeof(struct Node::SkipListLevel));
        if (p == NULL)
            return NULL;

        p->score = score;
        p->backward = NULL;
        for (int i = 0; i < level; ++i)
        {
            p->level[i].forward = NULL;
        }

        return p;
    }

    // @x: 要删除的节点
    // @update: 需要跟着修改的节点
    void delete_node(Node* x, Node** update) {
        for (int i = 0; i < m_cur_level; ++i)
        {
            if (update[i]->level[i].forward == x)
            {
                update[i]->level[i].forward = x->level[i].forward;
            }
        }

        // 不是最后一个节点
        if (x->level[0].forward != NULL)
        {
            x->level[0].forward->backward = x->backward;
        }
        else //是最后一个节点
        {
            m_tail = x->backward;
        }

        while (m_cur_level > 1 && m_head->level[m_cur_level - 1].forward
         == NULL)
        {
            m_cur_level--;
        }

        m_length--;
    }
    // 释放节点
    void free_node(Node* p) {
        assert(p != NULL);
        free(p);
    }

private:
    Node* m_head;
    Node* m_tail;
    unsigned int m_length; //一共有多少个node.
    int m_cur_level; //当前最大有多少层
};
```

### 3 测试代码
```cpp
void skip_list_test()
{
    printf("### Function Test ###\n");
    SkipList<int> sl;
    for (int i = 0; i <= 300; i += 3)
    {
        sl.Insert(i);
    }

    sl.Print();

    printf("\n=== Insert Skip List ===\n");
    sl.Insert(29);
    sl.Print();
    printf("\n=== Search Skip List ===\n");
    printf("%s %d \n", 0 == sl.Search(15) ? "find" : "not find", 15);
    printf("%s %d \n", 0 == sl.Search(255) ? "find" : "not find", 255);
    printf("\n=== Delete Skip List ===\n");
    sl.Delete(15);
    printf("%s %d \n", 0 == sl.Search(15) ? "find" : "not find", 15);
    printf("%s %d \n", 0 == sl.Search(255) ? "find" : "not find", 255);
}
```
