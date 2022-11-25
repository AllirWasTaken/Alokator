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


int main() {

    srand (time(NULL));

    char memory[63130];

    char *ptr[370];
    int ptr_state[370] = {0};

    int is_allocated = 0;

    memory_init(memory, 63130);

    for (int i = 0; i < 370; ++i)
    {
        if (rand() % 100 < 66)
        {
            for (int j = 0; j < 370; ++j)
                if (ptr_state[j] == 0)
                {
                    ptr_state[j] = 1;
                    ptr[j] = memory_malloc(rand() % 100 + 50);
                    is_allocated++;
                    break;
                }
        }
        else
        {
            if (is_allocated)
            {
                int to_free = rand() % is_allocated;
                for (int j = 0; j < 370; ++j)
                    if (ptr_state[j] == 1 && !(to_free--))
                    {
                        ptr_state[j] = 0;
                        is_allocated--;
                        memory_free(ptr[j]);
                        break;
                    }
            }
        }
    }


    for (int j = 0; j < 370; ++j)
        if (ptr_state[j] == 1)
            memory_free(ptr[j]);

    return 0;
}