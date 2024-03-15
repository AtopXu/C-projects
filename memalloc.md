# 1.程序说明

1.使用sbrk系统函数申请内存 释放内存的方式 编写了malloc() free() relloc() calloc()
2.核心思想是程序的数据块，是一个堆，通过sbrk可以获取这个数据块的尾部地址，从尾部新增跟减少
3.使用单向链表的方式，将申请的内存连接，以便存储
4.对应内存释放，只有末尾的内存块会释放，其余的保留
5.因此设计了一个结构体header_t，成员变量有is_free 表示被释放的内存块，next指向下一个链表，size 表示内存块大小
6.同时header_t对外不存在

# 2.知识点

1.sbrk(x): x为0时，返回数据块尾部地址，其他则会返回增加、减少后的尾部地址
2.单向尾增长的链表方法
header_t *head = NULL , *tail = NULL;
block = sbrk(total_size);
header = block;
header->s.is_free = 0;
header->s.size = size;
header->s.next = NULL;
/*insert to line of header_t*/
if (!head){
/*means head=NULL,so header is first*/
head = header;
}
if (tail){
/*means tail is exsited*/
tail->s.next = header;
}
tail = header;
/*explan line status
1.head=tail=NULL
then head=tail=header1  head->s.next=NULL  tail->s.next=NULL
2.head=tail=header1
then tail->s.next = header2 that means head->s.next =header2,and tail=header2
3.head=header1 head->s.next=tail=header2 tail=header2
then tail->s.next=header3 means header2->s.next = header3,and tail =header3
*/

3.单向链表减方法
tmp = head;
while(tmp){
if(tmp->s.next == tail){
tmp->s.next = NULL;
tail = tmp;
}
tmp = tmp->s.next;
}

4、relloc()实现
直接malloc一个新的，把原来free掉

