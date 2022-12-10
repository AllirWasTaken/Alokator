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
    char *a=(void*)chunk;
    char *b=(void*)((char*)chunk->next-32-chunk->size);
    size_t diff=b-a;
    return diff;
}

void *memory_malloc(size_t size) {
   // printf("trying to allocate %d bytes\n",(int)size);
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
                pointerAdd+=between;
            }
            temp=temp->next;
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
    else return (char*)newChunk+sizeOfChunk;

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
    //printf("\n>freeing %d bytes\n",(int)((struct memory_chunk_t*)(void*)((char*)address-32))->size);
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
    printf("Memory %d/%d\n %.2f",
           (int)usedMemory,(int)memory_manager.memory_size, (float)usedMemory/(float)memory_manager.memory_size*(float)100);
    putchar('%');
    printf("\nEND OF STATUS\n");
}


int main() {

    return 0;
}