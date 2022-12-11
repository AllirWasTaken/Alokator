#include "heap.h"
#include "custom_unistd.h"

#define chunkSize sizeof(struct chunkHeap_t)



struct chunkHeap_t{
    struct chunkHeap_t *next;
    struct chunkHeap_t *prev;
    size_t size;
};

struct heapManager_t{
    void *memory;
    unsigned numberOfChunks;
    size_t size;
    size_t used;
}heapManager;

enum allocationType_t{
    NONE,
    NEW_HEAP,
    END_OF_HEAP,
    BETWEEN_CHUNKS
};

int Validate(void){
    if(heapManager.size%4096)return 1;
    if(heapManager.memory==NULL)return 2;
    if(heapManager.size==0)return 3;

    return 0;
}

int heap_setup(void){
    void *mem= custom_sbrk(4096);
    if(!mem)return -1;
    heapManager.memory=mem;
    heapManager.size=4096;
    heapManager.used=0;
    heapManager.numberOfChunks=0;
    return 0;
}

void heap_clean(void){
    heapManager.memory=NULL;
    custom_sbrk((intptr_t)heapManager.size*-1);
    heapManager.size=0;
    heapManager.used=0;
    heapManager.numberOfChunks=0;
}

size_t SpaceBetweenChunks(struct chunkHeap_t *chunk){
    if(chunk->next==NULL)return 0;
    return ((char*)chunk->next-(char*)chunk)-chunkSize-chunk->size;
}

void* heap_malloc(size_t size){
    if(!size||Validate())return NULL;
    if(size+heapManager.used+chunkSize>heapManager.size)return NULL;

    size_t pointerAdd;
    enum allocationType_t allocationType=NONE;
    if(heapManager.numberOfChunks==0)allocationType=NEW_HEAP;

    if(allocationType==NONE){

    }
    if(allocationType==NONE){

    }

}

void* heap_calloc(size_t number, size_t size){
    return NULL;
}

void* heap_realloc(void* memblock, size_t count){
    return NULL;
}

void  heap_free(void* memblock){

}

size_t   heap_get_largest_used_block_size(void){
    return 0;
}

enum pointer_type_t get_pointer_type(const void* const pointer){
    return 0;
}