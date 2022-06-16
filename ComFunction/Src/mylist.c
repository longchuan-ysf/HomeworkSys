/*************************
*** File myList.c
**************************/
#include "stdlib.h"
#include "string.h"
#include "mylist.h"
#include "malloc.h"

#define list_malloc(n) mymalloc(SRAMIN,n)
#define list_frre(n) myfree(SRAMIN,n)
//创建链表
MyList * createMyList()
{
    MyList * re = (MyList *) list_malloc(sizeof(MyList));
    re->count = 0;
    re->first = NULL;
    re->last = NULL;
    re->equal = NULL;
    return re;
}
 
//释放链表
void freeMyList(MyList * list)
{
    MyNode * p;
    if(list == NULL)
    {
        return;
    }
    while (list->first)
    {
        p = list->first->next;
        list_frre(list->first->data);
        list_frre(list->first);
        list->first = p;
    }
    list_frre(list);
}
 
//插入在尾部_链表会自动申请内存存放数据，所以外面的数据暂存区可以重复利用而不影响以及插入数据的内容
//如果不申请内存，万一外面的内存是重复利用的，然后链表每个节点都指向这个内存，但是每次数据都不一样
//最后导致所有节点的数据一样
void myListInsertDataAtLast(MyList * const list, void* const data,int datalen)
{
    MyNode * node = (MyNode *) list_malloc(sizeof(MyNode));
    char * datatemp = (char *)list_malloc(datalen);
	memset(datatemp,0,datalen);
    memcpy(datatemp,data,datalen);
    node->data = datatemp;
    node->next = NULL;
    if (list->count)
    {
        list->last->next = node;//上一个节点的next指针指向新节点
        list->last = node;//将尾指针指向node
    }
    else
    {
        list->first = node;
        list->last = node;
    }
    (list->count)++;
}
 
//插入在首部
void myListInsertDataAtFirst(MyList * const list, void* const data,int datalen)
{
    MyNode * node = (MyNode *) list_malloc(sizeof(MyNode));
    char * datatemp = (char *)list_malloc(datalen);
    memcpy(datatemp,data,datalen);
    node->data = datatemp;
    node->next = NULL;
 
    if (list->count)
    {
        node->next = list->first;//将当前指针的next指向头指针
        list->first = node;//然后头指针指向node
    }
    else
    {
        list->first = node;
        list->last = node;
    }
    (list->count)++;
}
 
//长度
int myListGetSize(const MyList * const list)
{
    return list->count;
}
 
//打印
void myListOutput(const MyList * const list, void(*pt)(const void * const))
{
    MyNode * p = list->first;
    while (p)
    {
        (*pt)(p->data);
        p = p->next;
    }
}
 
//删除在尾部
void myListRemoveDataAtLast(MyList* const list)
{
	 MyNode * p;
    if (list->count == 1)
    {
		myListRemoveDataAtFirst(list);
        return ;
    }
    //找到尾节点的上一个节点
    p = list->first;
    while (p->next != list->last)
    {
        p = p->next;
    }
    //删除尾节点
    list_frre(list->last->data);
    list_frre(list->last);
    //尾节点的上一个节点的next指向NULL
    p->next = NULL;
    list->last = p;
    (list->count)--;
    return ;
}
 
//删除在首部
void myListRemoveDataAtFirst(MyList * const list)
{
    MyNode *p = list->first;
    list->first = p->next;
    list_frre(p->data);
    list_frre(p);
    (list->count)--;
    if (list->count == 0)
    {
        list->last = NULL;
    } 
    return ;
}
 
//插入
void myListInsertDataAt(MyList * const list, void* const data,int datalen, int index)
{
    int i;
	MyNode * node;
	MyNode * p;
	char * datatemp;
    if (index == 0)
    {
        myListInsertDataAtFirst(list, data,datalen);
        return;
    }
    if (index == list->count)
    {
        myListInsertDataAtLast(list, data,datalen);
        return;
    }
    //只能在中间插入
    if(index>list->count)
        return;
    node = (MyNode *) list_malloc(sizeof(MyNode));
    datatemp = (char *)list_malloc(datalen);
    memcpy(datatemp,data,datalen);
    node->data = datatemp;
    node->next = NULL;
 
    p = list->first;
    for (i = 0; i < index - 1; i++)
    {
        p = p->next;
    }
    node->next = p->next;
    p->next = node;
 
    (list->count)++;
}
 
//删除
void myListRemoveDataAt(MyList* const list, int index)
{
    int i;
	MyNode * p;
	MyNode *tp;
    if (index == 0)
    {
         myListRemoveDataAtFirst(list);
    }
    if (index == list->count - 1)
    {
         myListRemoveDataAtLast(list);
    }
    if(index > list->count-1)
        return;
 
    p = list->first;
    for (i = 0; i < index - 1; i++)
    {
        p = p->next;
    }
    tp = p->next;
    p->next = p->next->next;
    list_frre(tp->data);
    list_frre(tp);
    (list->count)--;
    return ;
}
 
//取得数据
void* myListGetDataAt(const MyList * const list, int index)
{
    int i;
	MyNode * p;
    if (index == list->count - 1)
    {
        return myListGetDataAtLast(list);
    }
    if(index>list->count - 1)
        return NULL;
    p = list->first;
    for (i = 0; i < index; i++)
    {
        p = p->next;
    }
    return p->data;
}
 
//取得第一个数据
void* myListGetDataAtFirst(const MyList * const list)
{
    return list->first->data;
}
 
//取得最后一个数据
void* myListGetDataAtLast(const MyList * const list)
{
    return list->last->data;
}
 
//查找某个数据的位置,如果equal方法为空，比较地址，否则调用equal方法
//如果不存在返回-1，如果存在，返回出现的第一个位置
int myListFindDataIndex(const MyList * const list, void * data)
{
    MyNode * p = list->first;
    int re = 0;
    if (list->equal)
    {
        while (p)
        {
            if (p->data == data || (*(list->equal))(p->data, data))
            {
                return re;
            }
            re++;
            p = p->next;
        }
 
    }
    else
    {
        while (p)
        {
            if (p->data == data)
            {
                return re;
            }
            re++;
            p = p->next;
        }
    }
    return -1;
}
 
//创建链表，带有相等参数，用于查找
MyList * createMySearchList(int(*equal)(void * a, void * b))
{
    MyList * re = createMyList();
    re->equal = equal;
    return re;
}
 
//创建遍历器
MyListIterator* createMyListIterator(const MyList * const list)
{
    MyListIterator * re = (MyListIterator *) list_malloc(sizeof(MyListIterator));
    re->p = list->first;
    re->allSize = list->count;
    re->count = 0;
    return re;
}
 
//释放遍历器
void freeMyListIterator(MyListIterator* iterator)
{
    list_frre(iterator);
}
 
//遍历器是否有下一个元素
int myListIteratorHasNext(const MyListIterator* const iterator)
{
    return iterator->count < iterator->allSize;
}
 
//返回遍历器的下一个元素
void * myListIteratorNext(MyListIterator* const iterator)
{
    void * re = iterator->p->data;
    iterator->p = iterator->p->next;
    (iterator->count)++;
    return re;
}
 
//删除对象,返回是否删除成功
int myListRemoveDataObject(MyList* const list, void * data)
{
    MyListIterator * it = createMyListIterator(list);
    int a = 0;
    while (myListIteratorHasNext(it))
    {
        void * ld = myListIteratorNext(it);
        if (data == ld || (list->equal != NULL && (*(list->equal))(ld, data)))
        {
            a = 1;
            break;
        }
    }
    if (a)
    {
        myListRemoveDataAt(list, it->count - 1);
    }
    return a;
}
