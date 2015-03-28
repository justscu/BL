## 二叉树

### 1 基本概念
二叉树是指每个节点最多有2个子节点的有序树。二叉树常用于实现二叉查找和二叉堆。二叉树的遍历顺序，是对根节点来说的。
二叉树的子树(左子树、右子树)有顺序之分，不能够颠倒。 

|名称 | 深度 | 插入/删除 | 查找次数（跟深度有关） |
| --- | ---- | --------- | ---------------------- |
|最小堆 | logn | 需自顶向下或自下向上调整 | 获取最小值O(1) |
| AVL-tree | logn | logn，需多次旋转 | 当查找>>插入/删除，推荐使用AVL |
| RB-tree | >=logn | logn，插入最多2次旋转；删除最多3次旋转 |   |

### 2 最小(大)堆
**堆是**一种经过排序的[完全二叉树](http://baike.baidu.com/link?url=l3ZLrDoDn-YyMSyXwAhFirqBffUv6KAgP6FeU-2refVUbJSoE9OMq2-SnLcycVOS6rf6kfnj5rGtoT-3dLV90q)，其中任一非终端节点的数据值均不大于（或不小于）其左孩子和右孩子节点的值。
最小堆是一种经过排序的二叉树，其中任一父节点的值均不大于其左孩子和右孩子节点的值。
顶点的值最小，所以获取最小值的时间为O(1)。

有些网络库的定时器，是用最小堆来实现的。

```cpp
    #include <stddef.h>
    #include <stdlib.h>
    #include <assert.h>
    #include <iostream>

    template <class T >
    class MinHeap
    {
    public:
        MinHeap(int n = 10000): m_max_size(n), m_cur_size(0) {
            m_heap = new T[m_max_size];
            assert(m_heap != NULL);
        }
        ~MinHeap() {
            if(m_heap != NULL)
                delete [] m_heap;
        }
        bool Insert(const T & x) {
            if(m_cur_size == m_max_size)
                return false; // full
            m_heap[m_cur_size] = x;
            FilterUp(m_cur_size++);
            return true;
        }
        T RemoveMin() {
            T x = m_heap[0];
            m_heap[0] = m_heap[m_cur_size-1];
            --m_cur_size;
            FilterDown(0, m_cur_size-1);
            return x;
        }
        T& GetMin() {
            return m_heap[0];
        }
        bool IsEmpty() const {
            return 0 == m_cur_size;
        }
        bool IsFull() const {
            return m_cur_size == m_max_size;
        }
        void Clear() {
            m_cur_size = 0;
        }

        void print() const{
            int num = 0;
            int line = 1;
            for(int i = 0; i < m_cur_size; ++i) {
                std::cout << m_heap[i] << " ";
            }
            std::cout << std::endl;
        }
    private:
        MinHeap(const MinHeap &);
        MinHeap & operator = (MinHeap&);

        // 从下往上调整: @start，开始调整的位置
        void FilterUp(const int start) {
            int child = start;
            int parent = (child-1)/2;
            T temp = m_heap[child];
            while(child > 0)
            {
                if(temp >= m_heap[parent]) //T 必须支持 operator >=
                    break;
                else{
                    m_heap[child] = m_heap[parent];
                    child = parent;
                    parent = (child-1)/2;

                }
            }

            m_heap[child] = temp;
        }
        // 从上到下调整: @start，开始调整的位置；@end，最后调整的位置
        void FilterDown(const int start, const int end) {
            int parent = start;
            int child = start*2 +1;
            T temp = m_heap[parent];
            while(child <= end)
            {
                if(child+1 <= end && m_heap[child] > m_heap[child+1]) //T 必须支持 operator >
                {
                    child++; // 取子节点中，较小的一个
                }
                if(temp <= m_heap[child])
                    break;
                else {
                    m_heap[parent] = m_heap[child];
                    parent = child;
                    child = parent*2+1;
                }
            }

            m_heap[parent] = temp;
        }
    private:
        int m_max_size;
        int m_cur_size; // 当前已用的个数，实际为 m_heap[0, m_cur_size-1]
        T* m_heap;
    };
```

### 3 平衡二叉树(AVL树)
为了解决二叉树退化为线性（链表，时间复杂度为O(n)），提出了AVL树。
它是一棵空树或它的左右两个子树的高度差的绝对值不超过1，并且左右两个子树都是一棵平衡二叉树。

平衡二叉树的插入/删除出现不平衡时，需要旋转。
查找、插入和删除在平均和最坏情况下都是O(log n)。增加和删除可能需要通过一次或多次树旋转来重新平衡这个树。

二叉树将信息存储在节点上。节点的声明如下
```cpp
    struct AVLNode {
        const TYPE& data;
        int hight; //高度
        unsigned int times; // 出现次数
        AVLNode* LChild;
        AVLNode* RChild;
    public:
        AVLNode(const TYPE& d):data(d), hight(0), times(1), LChild(NULL), RChild(NULL) {
        }
        ~AVLNode() {}
    private:
        AVLNode(const AVLNode&);
        AVLNode& operator= (const AVLNode&);
    };
```

#### 3.1 AVL树的遍历
```cpp
    // 先序遍历: 先根节点 -> 左节点 -> 右节点
    void preorder_traversal(const AVLNode* pTop) {
        if (pTop == NULL)
            return;

        if (m_printFunc) {
            m_printFunc(&pTop->data);
        }
        if (pTop->LChild != NULL) {
            preorder_traversal(pTop->LChild);
        }

        if (pTop->RChild != NULL) {
            preorder_traversal(pTop->RChild);
        }
    }

    // 中序遍历: 先左节点 -> 根节点 -> 右节点
    void inorder_traversal(const AVLNode* pTop) {
        if (pTop == NULL)
            return;

        if (pTop->LChild != NULL)
            inorder_traversal(pTop->LChild);
        if (m_printFunc) {
            m_printFunc(&pTop->data);
        }
        if (pTop->RChild != NULL)
            inorder_traversal(pTop->RChild);
    }

    // 后序遍历: 先左节点 -> 右节点 -> 根节点
    void lastorder_traversal(const AVLNode* pTop) {
        if (pTop == NULL)
            return;

        if (pTop->LChild != NULL)
            lastorder_traversal(pTop->LChild);
        if (pTop->RChild != NULL)
            lastorder_traversal(pTop->RChild);
        if (m_printFunc) {
            m_printFunc(&pTop->data);
        }
    }
```

#### 3.2 一些辅助函数
```cpp
    inline int max(int a, int b) {
        return a>=b?a:b;
    }
    // 计算节点的高度
    inline int hight(const AVLNode* pNode) {
        if (pNode == NULL)
            return -1;
        else
            return pNode->hight;
    }

    // 获取data的父节点(递归)
    inline AVLNode* getParent2(AVLNode* pTop, const TYPE& data) {
        if (pTop == NULL || pTop->data == data)
            return NULL;
        AVLNode* node = pTop;
        if ((node->LChild && node->LChild->data == data) || (node->RChild && node->RChild->data == data))
            return node;
        else if (node->LChild && node->data > data)
            return getParent2(node->LChild, data);
        else // if (node->RChild && node->data < data)
            return getParent2(node->RChild, data);
    }
    // 获取data的父节点(非递归)
    inline AVLNode* getParent(AVLNode* pTop, const TYPE& data) {
        if (pTop == NULL || pTop->data == data)
            return NULL;

        AVLNode* node = pTop;
        while(node) {
            if ((node->LChild && node->LChild->data == data) || (node->RChild && node->RChild->data == data)) {
                return node;
            } else if (node->data > data && node->LChild != NULL)
                node = node->LChild;
            else if (node->data < data && node->RChild != NULL)
                node = node->RChild;
            else
                return NULL;
        }

        return node;
    }
    // 获取最小节点（即最左边的节点）
    inline AVLNode* getMinNode(AVLNode* pTop) {
        if (pTop != NULL) {
            while ( pTop->LChild != NULL ) {
                pTop = pTop->LChild;
            }
        }

        return pTop;
    }
    // 获取最大节点（即最右边的节点）
    inline AVLNode* getMaxNode(AVLNode* pTop) {
        if (pTop != NULL) {
            while ( pTop->RChild != NULL ) {
                pTop = pTop->RChild;
            }
        }

        return pTop;
    }
```

#### 3.3 增删节点
AVL树在增删节点时，可能会失去平衡。为了保持再平衡，需要对AVL树进行旋转。
```cpp
    // 右旋 -- 左-左插入情况下的旋转：即在根节点的左子树的左子树上插入节点，需右旋
    AVLNode* r_Rotate(AVLNode* pTop) {
        if (pTop == NULL)
            return NULL;
        AVLNode* pRet = pTop->LChild;
        pTop->LChild = pRet->RChild;
        pRet->RChild = pTop;

        pTop->hight = max(hight(pTop->LChild), hight(pTop->RChild))+1;
        pRet->hight = max(hight(pRet->LChild), pTop->hight)+1;
        return pRet;
    }
    // 左旋 -- 右-右插入情况下的旋转：即在根节点的右子树的右子树上插入节点，需左旋
    AVLNode* l_Rotate(AVLNode* pTop) {
        if (pTop == NULL)
            return NULL;
        AVLNode* pRet = pTop->RChild;
        pTop->RChild = pRet->LChild;
        pRet->LChild = pTop;

        pTop->hight = max(hight(pTop->LChild), hight(pTop->RChild))+1;
        pRet->hight = max(hight(pRet->LChild), pTop->hight)+1;
        return pRet;
    }
    // 先左旋后右旋 -- 左-右插入插入情况下的旋转：即在根节点的左子树的右子树上插入节点，需先左旋后右旋
    AVLNode* lr_Rotate(AVLNode* pTop) {
        if (pTop == NULL)
            return NULL;
        pTop->LChild = l_Rotate(pTop->LChild);
        return r_Rotate(pTop);
    }
    // 先右旋后左旋 -- 右-左插入情况下的旋转：即在根节点的右子树的左子树上插入节点，需先右旋后左旋
    AVLNode* rl_Rotate(AVLNode* pTop) {
        if (pTop == NULL)
            return NULL;
        pTop->RChild = r_Rotate(pTop->RChild);
        return l_Rotate(pTop);
    }
```

#### 3.4 查找
```cpp
    // 查找(递归)，返回值为找到的节点
    AVLNode* find2(AVLNode* pTop, const TYPE& data) {
        if (pTop == NULL || pTop->data == data) // TYPE 需要重载 '=='
            return pTop;
        if (pTop->data > data) {
            return find2(pTop->LChild, data);
        } else {
            return find2(pTop->RChild, data);
        }
    }
    // 查找(非递归)
    AVLNode* find(AVLNode* pTop, const TYPE& data) {
        AVLNode* p = pTop;
        while(p != NULL) {
            if(p->data == data) {
                return p;
            } else if (p->data > data){ // TYPE 需要重载 '>'
                p = p->LChild;
            } else {
                p = p->RChild;
            }
        }

        return NULL;
    }
    // 一致性hash需要的查找算法
    AVLNode* find_conhash(AVLNode* pTop, const TYPE& data) {
        AVLNode* p = pTop;
        while(p != NULL) {
            if(p->data == data) {
                return p;
            } else if (p->data > data){
                if (p->LChild != NULL)
                    p = p->LChild;
                else
                    return p;
            } else {
                if (p->RChild != NULL)
                    p = p->RChild;
                else
                    return p;
            }
        }

        return NULL;
    }
```

#### 3.5 插入
```cpp
    // 插入，返回值为新的top节点
    AVLNode* insert(AVLNode* pTop, const TYPE& data) {
        if (pTop == NULL) {
            pTop = new AVLNode(data);
            if (pTop == NULL) {
                return NULL;
            }

            return pTop;
        }

        // 插入左子树
        if (pTop->data > data) {
            pTop->LChild = insert(pTop->LChild, data);
            // 失衡
            if (hight(pTop->LChild)-hight(pTop->RChild) == 2) {
                if (pTop->LChild->data > data) {
                    pTop = r_Rotate(pTop); // (1)根节点的左子树的左子树插入，需右旋
                } else {
                    pTop = lr_Rotate(pTop); // (2)根节点的左子树的右子树插入，需先左旋后右旋
                }
            }
        // 插入右子树
        } else if (pTop->data < data) {
            pTop->RChild = insert(pTop->RChild, data);
            // 失衡
            if (hight(pTop->LChild)-hight(pTop->RChild) == -2) {
                if (pTop->RChild->data < data) {
                    pTop = l_Rotate(pTop); // (3)根节点的右子树的右子树，需左旋
                } else {
                    pTop = rl_Rotate(pTop);    // (4)根节点的右子树的左子树，需先右旋后左旋
                }
            }
        } else {
            pTop->times++;
        }

        max(hight(pTop->LChild), hight(pTop->RChild));
        pTop->hight = max(hight(pTop->LChild), hight(pTop->RChild)) + 1;
        return pTop;
    }
```

#### 3.6 删除
```cpp
    // 删除，返回值为新的top节点
    // @data为要删除的节点
    // out，将保存在AVL树上的数据传回
    AVLNode* _delete(AVLNode* pTop, const TYPE& data) {
        if (pTop == NULL)
            return NULL;
        // 找到了要删除的节点，引用次数>1，修改引用次数
        if (pTop->data == data && pTop->times > 1) { // -1 就可以了，
            --pTop->times;
        // 引用次数为1, 删除该节点
        } else if (pTop->data == data) {
            AVLNode* t = pTop;
            AVLNode* minR = getMinNode(pTop->RChild); // 右孩子最小节点
            if (minR == NULL) { // 右子树不存在，左子树肯定只有一个节点或没有节点
                pTop = pTop->LChild;
            } else { // 右子树存在
                if (pTop->RChild->LChild == NULL) { // 没有左子树
                    pTop = pTop->RChild;
                    pTop->LChild = t->LChild;
                } else {
                    AVLNode* par = getParent(pTop->RChild, minR->data);
                    par->LChild = NULL;
                    minR->LChild = pTop->LChild;
                    minR->RChild = pTop->RChild;
                }
                pTop = minR;
            }

            printf("delete: ");
            m_printFunc(&t->data);
            printf("\n");
            delete t;

        } else if (pTop->data > data) { // 在左子树上删
            pTop->LChild = _delete(pTop->LChild, data);
            if (hight(pTop->LChild) - hight(pTop->RChild) == -2) {
                if (hight(pTop->RChild->LChild) < hight(pTop->RChild->RChild)) {
                    pTop = l_Rotate(pTop);
                } else {
                    pTop = rl_Rotate(pTop);
                }
            }
        } else { // 在右子树上删
            pTop->RChild = _delete(pTop->RChild, data);
            if (hight(pTop->LChild) - hight(pTop->RChild) == 2) {
                if (hight(pTop->LChild->LChild) > hight(pTop->LChild->RChild)) {
                    pTop = r_Rotate(pTop);
                } else {
                    pTop = lr_Rotate(pTop);
                }
            }
        }

        if ( pTop != NULL ) {
            pTop->hight = max(hight(pTop->LChild), hight(pTop->RChild)) + 1;
        }
        return pTop;
    }
```

#### 3.7 统一接口
对上述函数进行包装，提供统一的接口
```cpp
    // 声明打印函数
    typedef void (*FUNPRINT) (const void* data);

    template<class TYPE>
    class AVL_tree
    {
    public:
        AVL_tree(FUNPRINT func):m_printFunc(func), m_pTop(NULL) {
            printf("AVL_tree() \n");
        }
        ~AVL_tree() {
            printf("~AVL_tree() \n ");
            destroy(m_pTop);
        }
        bool Insert(const TYPE& data) {
            m_pTop = insert(m_pTop, data);
            return m_pTop == NULL;
        }
        bool Delete(const TYPE& data) {
            m_pTop = _delete(m_pTop, data);
            return true;
        }
        AVLNode* Find(const TYPE& data) {
            return find(m_pTop, data);
        }
        // 一致性hash的查找函数
        AVLNode* Find_conhash(const TYPE& data) {
            return find_conhash(m_pTop, data);
        }

        void Preorder_traversal() {
            preorder_traversal(m_pTop);
        }
        void Inorder_traversal() {
            inorder_traversal(m_pTop);
        }
        void Lastorder_traversal() {
            lastorder_traversal(m_pTop);
        }

    private:
        FUNPRINT m_printFunc;
        AVLNode* m_pTop; // avl树的顶节点

    private:
        // 删除分配的资源
        void destroy(AVLNode* pTop) {
            if (pTop != NULL) {
                if (pTop->LChild != NULL) {
                    destroy(pTop->LChild);
                }
                if (pTop->RChild != NULL) {
                    destroy(pTop->RChild);
                }
                delete pTop;
            }
        }
    };
```

#### 3.8 测试用例
```cpp
    void pf(const void* a) {
        printf("%2d ", *(int*)a);
    }

    void Test_AVL() {
        AVL_tree<int> avltree(&pf);

        //
        int data[] = {1,4,9,10,14,15,16,17,18,2,3,19,20,11,12,5,6,7,8,13, 21, 22, 24, 23, 6, 9};
        for (unsigned int i = 0; i < sizeof(data)/sizeof(int); ++i) {
            avltree.Insert(data[i]);
        }

        avltree.Preorder_traversal();printf("\n");
        printf("Inorder_traversal:");
        avltree.Inorder_traversal();
        printf("\n");

        avltree.Delete(6);
        avltree.Inorder_traversal();
        printf("\n");
        avltree.Delete(9);
        avltree.Inorder_traversal();
        printf("\n");

        avltree.Delete(6);
        avltree.Inorder_traversal();
        printf("\n");

        int b[] = {4, 5, 6, 7};
        for (unsigned int i = 0; i < sizeof(b)/sizeof(int); ++i) {
            if (avltree.Find(b[i]) != NULL) {
                printf("find [%d] \n", b[i]);
            } else {
                printf("not find [%d] \n", b[i]);
            }
        }
    }
```

### 4 红黑树（RB-tree）

-  红黑树是一种自平衡二叉查找树，也叫做二叉B树。它可以在O(logn)时间内做查找，插入和删除，和AVL树相同。但由于RB-tree不是严格的平衡二叉树，所以在插入/删除时，旋转的次数少于AVL树。插入最多旋转2次，删除最多旋转3次。RB-tree树的深度可能大于AVL树。
- **若查找远大于插入/删除时，推荐使用AVL树；否则使用RB-tree。**
- nginx的定时器，就用的是RB-tree。 
