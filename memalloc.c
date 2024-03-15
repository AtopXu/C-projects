#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
/* defind struct */
typedef char ALIGN[16];
union header {
        struct {
                size_t size;
                unsigned is_free;
                union header *next;
        } s;
        ALIGN stud;
};
typedef union header header_t;

header_t *head = NULL , *tail = NULL;
pthread_mutex_t global_malloc_loc;

header_t *get_free_block(size_t size)
{
    header_t *curr = head;
    // if (!curr){
    //     return NULL;
    // }
    while (curr){
       if (curr->s.is_free == 1 && curr->s.size >= size)
           return curr;
       curr = curr->s.next;
    }
    return NULL;
}

void *malloc(size_t size)
{
    size_t total_size;
    void *block; //due to unkown the type of user define,so use void
    header_t *header;
    if (!size)
        return NULL;
    pthread_mutex_lock(&global_malloc_loc);
    header = get_free_block(size);
    if (header){
        header->s.is_free = 0;
        pthread_mutex_unlock(&global_malloc_loc);
        return (void *)(header + 1); //malloc(total_size) = malloc(header_size)+malloc(block_size),so header+1 = block start address
    }
    /*there are no free block,use sbkr() to malloc */
    total_size = size + sizeof(header_t);
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
    pthread_mutex_unlock(&global_malloc_loc);
    return (void *)(header + 1);

    /*explan line status
    1.head=tail=NULL
    then head=tail=header1  head->s.next=NULL  tail->s.next=NULL
    2.head=tail=header1
    then tail->s.next = header2 that means head->s.next =header2,and tail=header2
    3.head=header1 head->s.next=tail=header2 tail=header2
    then tail->s.next=header3 means header2->s.next = header3,and tail =header3
    */

}

void free(void *block)
{
    header_t *tmp, *header;
    void *programbreak;
    if (!block)
        return;
    pthread_mutex_lock(&global_malloc_loc);
    programbreak = sbrk(0);
    header = (header_t *)block - 1;
    /*start update line
    if block is end of data program,release memory to OS
    else keep it ,just free it
    */
   if ((char *)block + header->s.size == programbreak){
        if (head == tail) {
            head = tail = NULL;

        }else{
            tmp = head;
            while(tmp){
               if(tmp->s.next == tail){
                   tmp->s.next = NULL;
                   tail = tmp;
               }
               tmp = tmp->s.next;
            }
        }
        sbrk(0 - sizeof(header_t) - header->s.size);
        pthread_mutex_unlock(&global_malloc_loc);
        return;
   }
   header->s.is_free = 1;
   pthread_mutex_unlock(&global_malloc_loc);
}

/*when return type is not int ,we use void to define function*/
void *calloc(size_t num, size_t nsize)
{
    void *block;
    size_t size;
    if (!num || !nsize)
        return NULL;
    size = num * nsize;
    if (nsize != size/num)
        return NULL;
    block = malloc(size);
    if (block == NULL)
        return NULL;
    memset(block, 0, size);
    printf("block_address:%p\n", block);
    return block;
}

void *relloc(void *block, size_t rsize)
{
    header_t *header;
    if (!block || !rsize)
        return malloc(rsize);
    header = (header_t *)block - 1;
    /*questions:
    1.how to change a assined memery? 
    2.increase it use which function,and it may be over anyother header
    3.decresae it,if just return a smaller memery , it will generate small memory blocks
    */
   /*just malloc a new block and free old block*/
    if (header->s.size == rsize)
        return malloc;
    void *tmp;
    tmp = malloc(rsize);
    if(tmp){
        memcpy(tmp, block, header->s.size);
        free(block);
    }
    printf("block_address:%p\n", tmp);
    return tmp;
}

