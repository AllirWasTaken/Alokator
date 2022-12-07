#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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

size_t CheckSpaceBetweenChunks(struct memory_chunk_t *chunk){
    if(chunk->next==NULL)return 0;
    if(chunk+sizeof(struct memory_chunk_t)+chunk->size==chunk->next)return 0;
    else{
        size_t pointerAdd=0;
        struct memory_chunk_t *temp=chunk->next;
        char* pointer=(void*)chunk;
        pointer+=sizeof(struct memory_chunk_t)+chunk->size;
        while((void*)(pointer+pointerAdd)!=(void*)(temp))pointerAdd++;
        return pointerAdd;
    }
}

void *memory_malloc(size_t size) {
    //printf("trying to allocate %d bytes\n",(int)size);
    size_t sizeOfChunk=sizeof(struct memory_chunk_t);
    if(size<1||size>memory_manager.memory_size-sizeOfChunk)return NULL;

    struct memory_chunk_t *newChunk=NULL;
    struct memory_chunk_t *temp;
    size_t pointerAdd=0;
    size_t between;

    if(memory_manager.first_memory_chunk==NULL){
        newChunk=memory_manager.memory_start;
        newChunk->next=NULL;
        newChunk->prev=NULL;
        newChunk->size=size;
        newChunk->free=0;
        memory_manager.first_memory_chunk=newChunk;
    }
    else {
        temp=memory_manager.first_memory_chunk;
        while(1){
            pointerAdd+=sizeOfChunk;
            pointerAdd+=temp->size;
            if(temp->next==NULL)break;
            if(temp->free){
                if(temp->size>=size){
                    temp->size=size;
                    temp->free=0;
                    newChunk=temp;
                    break;
                }
            }
            between=CheckSpaceBetweenChunks(temp);
            if(between!=0){
                if(between>=sizeOfChunk+size){
                    newChunk=(void*)((char*)memory_manager.memory_start+pointerAdd);
                    newChunk->next=temp->next;
                    newChunk->prev=temp;
                    newChunk->prev->next=newChunk;
                    newChunk->next->prev=newChunk;
                    newChunk->size=size;
                    newChunk->free=0;
                    break;
                }
            }
            else{
                temp=temp->next;
            }
        }
    }

    if(pointerAdd+size+sizeOfChunk<=memory_manager.memory_size&&newChunk==NULL){
        newChunk=(void*)((char*)memory_manager.memory_start+pointerAdd);
        newChunk->next=NULL;
        newChunk->prev=temp;
        newChunk->prev->next=newChunk;
        newChunk->free=0;
        newChunk->size=size;
    }

    if(newChunk==NULL)return NULL;
    else return (void*)newChunk+sizeOfChunk;

}

void memory_free(void *address) {
    if(address==NULL)return;
    if(memory_manager.first_memory_chunk==NULL)return;
    size_t sizeOfChunk=sizeof(struct memory_chunk_t);

    struct memory_chunk_t *tempChunk=memory_manager.first_memory_chunk;
    struct memory_chunk_t *freeChunk=NULL;

    while(tempChunk){
        if((void*)((char*)tempChunk+sizeOfChunk)==address){
            freeChunk=tempChunk;
            break;
        }
        tempChunk=tempChunk->next;
    }

    if(!freeChunk)return;

    freeChunk->free=1;
    if(freeChunk->prev){
        if(freeChunk->prev->free){
            freeChunk->prev->size+=CheckSpaceBetweenChunks(freeChunk->prev);
            freeChunk->prev->size+=freeChunk->size+sizeOfChunk;
            freeChunk=freeChunk->prev;
            freeChunk->next=freeChunk->next->next;
            if(freeChunk->next)freeChunk->next->prev=freeChunk;
        }
    }
    if(freeChunk->next){
        freeChunk->size+= CheckSpaceBetweenChunks(freeChunk);
        if(freeChunk->next->free){
            freeChunk->size+=sizeOfChunk+freeChunk->next->size;
            freeChunk->next=freeChunk->next->next;
            if(freeChunk->next)freeChunk->next->prev=freeChunk;
        }
    }

    if(freeChunk->next==NULL){
        if(freeChunk->prev==NULL)memory_manager.first_memory_chunk=NULL;
        else freeChunk->prev->next=NULL;
    }
}


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
            printf("Block of address %p <\n"
                   "%p "
                   "%p\n"
                   "Size %d "
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




    struct memory_chunk_t *p1= (void*)((char*)memory_malloc(125)-32);
    memory_malloc(51);
    memory_malloc(54);
    memory_malloc(138);
    memory_free(p1);

    /*

    char mem[256];

    memory_init(&mem, 256);

    struct memory_chunk_t *chunk= (void *)((char*)memory_malloc(9)-32);
    char *a=(void*)((char*)chunk+32);
    char *b=memory_malloc(11);
    char *c=memory_malloc(7);
    memory_free(b);
    b=memory_malloc(5);
    memory_free(b);
    b=memory_malloc(11);
    memory_free(a);
    StackStatus();
    memory_free(c);
    StackStatus();
    memory_free(b);
    StackStatus();
    */
    return 0;
}