#include <unistd.h>
#include <stdio.h>
#define HEAP_SIZE 512



struct memory_manager_t
{
    void *memory_start;
    size_t memory_size;
    struct memory_chunk_t *first_memory_chunk;
}memory_manager;

struct memory_chunk_t
{
    struct memory_chunk_t* prev;
    struct memory_chunk_t* next;
    size_t size;
    int free;
};


void memory_init(void *address, size_t size){
    if(!address||size<1)return;
    memory_manager.first_memory_chunk=NULL;
    memory_manager.memory_size=size;
    memory_manager.memory_start=address;
}


void *memory_malloc(size_t size){
    if(size<1)return NULL;
    if(memory_manager.memory_size==0)return NULL;

    int pointerAdd=0;
    void *memPointer=NULL;

    if(memory_manager.first_memory_chunk==NULL){
        if(size>memory_manager.memory_size)return NULL;
        memory_manager.first_memory_chunk;
    }
    else{
        while(1){

        }
    }

    return memPointer;
}
void memory_free(void *address){

}

char heapMemory[HEAP_SIZE];


int main(){
    printf("%lld",sizeof(struct memory_chunk_t));
}