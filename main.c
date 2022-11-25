#include <unistd.h>
#define HEAP_SIZE 128



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

}
void memory_free(void *address){

}

char heapMemory[HEAP_SIZE];


int main(){

}