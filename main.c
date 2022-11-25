#include <unistd.h>
#include <stdio.h>

#define HEAP_SIZE 512


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
                return (void *) ((char *) memPointer + pointerAdd);
            }
        }

        pointerAdd += temp->size;
        if (temp->next == NULL)break;
        temp = temp->next;
    }

    if (memory_manager.memory_size - pointerAdd < size)return NULL;
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
    if (address == NULL)return;
    size_t sizeOfBlock = sizeof(struct memory_chunk_t);
    struct memory_chunk_t *temp = memory_manager.first_memory_chunk;
    void *memPointer = memory_manager.memory_start;
    size_t pointerAdd = 0;

    while (1) {
        pointerAdd += sizeOfBlock;
        if (address == (void *) ((char *) memPointer + pointerAdd))break;
        pointerAdd += temp->size;
        if (temp->next == NULL)return;
        temp = temp->next;
    }

    struct memory_chunk_t *memoryChunk = (void*)((char*)memPointer - sizeOfBlock);
    memoryChunk->free = 1;
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
        memoryChunk->prev->next = NULL;
    }
}

char heapMemory[HEAP_SIZE];


int main() {
    memory_init(&heapMemory, HEAP_SIZE);

    int *a = memory_malloc(4);
    int *b = memory_malloc(4);
    int *c = memory_malloc(4);
    *a = 4;
    *b = 5;
    *c = *b + *a;
    printf("%d", *c);


}