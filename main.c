#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define HEAP_SIZE 512

void StackStatus();

struct memory_manager_t {
    void *memory_start;
    size_t memory_size;
    struct memory_chunk_t *first_memory_chunk;
} memory_manager;

struct memory_chunk_t {
    struct memory_chunk_t *prev;
    struct memory_chunk_t *next;
    size_t size;
    int free;
};


void memory_init(void *address, size_t size) {
    if (!address || size < 1)return;
    memory_manager.first_memory_chunk = NULL;
    memory_manager.memory_size = size;
    memory_manager.memory_start = address;
}


void *memory_malloc(size_t size) {
    printf("Trying to allocate %d bytes\n",(int)size);
    if (size < 1)return NULL;
    if (memory_manager.memory_size == 0)return NULL;

    size_t pointerAdd = 0;
    size_t sizeOfBlock = sizeof(struct memory_chunk_t);
    void *memPointer = memory_manager.memory_start;
    struct memory_chunk_t *memoryChunk;

    if (memory_manager.first_memory_chunk == NULL) {
        if (size > memory_manager.memory_size - sizeOfBlock)return NULL;
        memoryChunk = memory_manager.memory_start;
        memoryChunk->size = size;
        memoryChunk->next = NULL;
        memoryChunk->prev = NULL;
        memoryChunk->free = 0;
        memory_manager.first_memory_chunk = memoryChunk;
        return (void *) ((char *) memPointer + sizeOfBlock);
    }
    struct memory_chunk_t *temp;
    temp = memory_manager.first_memory_chunk;
    while (1) {
        pointerAdd += sizeOfBlock;
        if (temp->free) {
            if (temp->size >= size) {
                temp->size = size;
                temp->free=0;
                return (void *) ((char *) memPointer + pointerAdd);
            }
        }

        pointerAdd += temp->size;
        if (temp->next == NULL)break;
        temp = temp->next;
    }

    if (memory_manager.memory_size - pointerAdd -sizeOfBlock < size)return NULL;
    memoryChunk = (void *) ((char *) memPointer + pointerAdd);
    temp->next = memoryChunk;
    memoryChunk->next = NULL;
    memoryChunk->prev = temp;
    memoryChunk->size = size;
    memoryChunk->free = 0;

    pointerAdd += sizeOfBlock;

    return (void *) ((char *) memPointer + pointerAdd);;
}

void memory_free(void *address) {
    if (address == NULL||memory_manager.first_memory_chunk==NULL)return;
    size_t sizeOfBlock = sizeof(struct memory_chunk_t);
    struct memory_chunk_t *memoryChunk  = memory_manager.first_memory_chunk;


    while (1) {
        if((void*)((char*)memoryChunk+sizeOfBlock)==address)break;
        if(memoryChunk->next==NULL)return;
        memoryChunk=memoryChunk->next;
    }

    memoryChunk->free = 1;
    printf("Freed %d bytes\n",(int)memoryChunk->size);
    if (memoryChunk->next != NULL && memoryChunk->next->free) {
        memoryChunk->size += memoryChunk->next->size + sizeOfBlock;
        memoryChunk->next = memoryChunk->next->next;
        if (memoryChunk->next != NULL) {
            memoryChunk->next->prev = memoryChunk;
        }
    }
    if (memoryChunk->prev != NULL && memoryChunk->prev->free) {
        memoryChunk = memoryChunk->prev;
        memoryChunk->size += memoryChunk->next->size + sizeOfBlock;
        memoryChunk->next = memoryChunk->next->next;
        if (memoryChunk->next != NULL) {
            memoryChunk->next->prev = memoryChunk;
        }
    }

    if (memoryChunk->next == NULL) {
        if(memoryChunk->prev==NULL){
            memory_manager.first_memory_chunk=NULL;
            return;
        }
        memoryChunk->prev->next = NULL;
    }
}

char heapMemory[HEAP_SIZE];


void StackStatus(){
    printf("STACK STATUS\n");
    size_t usedMemory=0;
    struct memory_chunk_t *chunk;
    if(memory_manager.first_memory_chunk==NULL)printf("Empty stack\n");
    else{
        chunk=memory_manager.first_memory_chunk;
        while(chunk!=NULL){
            usedMemory+=32;
            if(chunk->free==0)usedMemory+=chunk->size;
            printf("Block of address %p\n"
                   "Prev %p\n"
                   "Next %p\n"
                   "Size %d\n"
                   "IsFree %d\n",(void*)chunk,(void*)chunk->prev,(void*)chunk->next,(int)chunk->size,chunk->free);
            chunk=chunk->next;
        }
    }
    printf("Memory %d/%d\n %f%\n",
           (int)usedMemory,(int)memory_manager.memory_size, (float)usedMemory/(float)memory_manager.memory_size*(float)100);
    printf("END OF STATUS\n");
}


int main() {

    srand (time(NULL));

    char memory[63130];
    char *ptr;


    memory_init(memory, 63130);


    memory_malloc(76);
    memory_malloc(74);
    memory_malloc(82);
    memory_malloc(135);
    memory_malloc(68);
    memory_malloc(108);
    memory_malloc(58);
    memory_malloc(105);
    memory_malloc(119);
    memory_malloc(69);
    memory_malloc(91);
    ptr=memory_malloc(64);
    memory_malloc(95);
    memory_malloc(146);
    memory_free(ptr);
    StackStatus();
    memory_malloc(73);
    StackStatus();


    return 0;
}