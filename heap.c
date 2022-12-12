#include "heap.h"
#include "custom_unistd.h"
#include <stdio.h>

//Don't Adjust
#define chunkSize sizeof(struct chunkHeap_t)
#define fenceSize 4
#define pageSize 4096

//Adjustable
#define fencesCheckSum 4294967295


struct chunkHeap_t {
    size_t checkSum;
    struct chunkHeap_t *prev;
    struct chunkHeap_t *next;
    size_t size;
};

struct heapManager_t {
    void *memory;
    unsigned numberOfChunks;
    struct chunkHeap_t *firstChunk;
    size_t size;
    size_t used;
} heapManager;

enum allocationType_t {
    NONE,
    NEW_HEAP,
    END_OF_HEAP,
    BETWEEN_CHUNKS,
    START_OF_HEAP
};

int Validate(void) {
    if (heapManager.size % pageSize)return 1;
    if (heapManager.memory == NULL)return 2;
    if( heapManager.size==0)return 3;

    return 0;
}

int heap_setup(void) {
    void *mem = custom_sbrk(pageSize);
    if (mem == (void *) -1)return -1;
    heapManager.memory = mem;
    heapManager.size = pageSize;
    heapManager.used = 0;
    heapManager.numberOfChunks = 0;
    return 0;
}

void heap_clean(void) {
    custom_sbrk((intptr_t) heapManager.size * -1);
    heapManager.size = 0;
    heapManager.used = 0;
    heapManager.numberOfChunks = 0;
}


size_t CalculateCheckSum(struct chunkHeap_t *chunk){
    size_t sum=0;
    sum+=(size_t)chunk->next;
    sum+=(size_t)chunk->prev;
    sum+=chunk->size;
    return sum;
}

void CalculateCheckSumForChunk(struct chunkHeap_t *chunk){
    if(chunk==NULL)return;
    chunk->checkSum= CalculateCheckSum(chunk);
}


void DoFences(void *memory, size_t size) {
    char *workingMem = memory;
    unsigned *fence;
    fence = (void *) workingMem;
    *fence = fencesCheckSum;
    workingMem += size+fenceSize;
    fence = (void *) workingMem;
    *fence = fencesCheckSum;
}

int IncreaseHeapSize(size_t lackingBytes) {
    size_t newPages = lackingBytes / pageSize;
    newPages++;
    void *ret;
    ret = custom_sbrk((intptr_t) newPages * pageSize);
    if (ret == (void *) -1)return 1;
    heapManager.size += newPages*pageSize;
    return 0;
}

size_t SpaceBetweenChunks(struct chunkHeap_t *chunk) {
    if(chunk==NULL)return 0;
    if (chunk->next == NULL)return 0;
    return ((char *) chunk->next - (char *) chunk) - chunkSize - chunk->size - fenceSize * 2;
}

void *heap_malloc(size_t size) {
    if (!size ||heap_validate())return NULL;
    size_t requiredSpace = chunkSize + fenceSize * 2 + size;
    if (size + heapManager.used + requiredSpace > heapManager.size) {
        if (IncreaseHeapSize(requiredSpace))return NULL;
    }

    size_t pointerAdd = 0;


    struct chunkHeap_t *temp = heapManager.firstChunk;
    enum allocationType_t allocationType = NONE;
    if (heapManager.numberOfChunks == 0)allocationType = NEW_HEAP;
    size_t pointerDiff=(char *)heapManager.firstChunk-(char*)heapManager.memory;
    if(pointerDiff>=requiredSpace&&allocationType==NONE){
        allocationType=START_OF_HEAP;
    }

    if(allocationType == NONE) {
        pointerAdd = pointerDiff;
        size_t between;
        for (unsigned i = 0; i < heapManager.numberOfChunks; i++) {
            if (i)temp = temp->next;
            between = SpaceBetweenChunks(temp);
            pointerAdd += chunkSize + fenceSize * 2 + between + temp->size;
            if (between >= requiredSpace) {
                pointerAdd-=between;
                allocationType = BETWEEN_CHUNKS;
                break;
            }
        }
        if (pointerAdd + requiredSpace > heapManager.size) {
            if (IncreaseHeapSize(requiredSpace))return NULL;
        } else if(allocationType==NONE)allocationType = END_OF_HEAP;
    }

    if (allocationType == NONE)return NULL;
    struct chunkHeap_t *newChunk;
    newChunk = (void *) ((char *) heapManager.memory + pointerAdd);
    newChunk->size = size;
    if (allocationType == NEW_HEAP) {
        heapManager.firstChunk=newChunk;
        newChunk->next = NULL;
        newChunk->prev = NULL;
    }
    if(allocationType== START_OF_HEAP){
        newChunk->prev=NULL;
        newChunk->next=heapManager.firstChunk;
        newChunk->next->prev=newChunk;
        heapManager.firstChunk=newChunk;
    }
    if (allocationType == END_OF_HEAP) {
        newChunk->next = NULL;
        newChunk->prev = temp;
        newChunk->prev->next = newChunk;
    }
    if (allocationType == BETWEEN_CHUNKS) {
        newChunk->prev = temp;
        newChunk->next = newChunk->prev->next;
        newChunk->prev->next = newChunk;
        newChunk->next->prev = newChunk;
    }


    pointerAdd += chunkSize;
    DoFences((void *) ((char *) heapManager.memory + pointerAdd), size);
    pointerAdd += fenceSize;
    heapManager.numberOfChunks++;
    heapManager.used += requiredSpace;
    CalculateCheckSumForChunk(newChunk);
    CalculateCheckSumForChunk(newChunk->next);
    CalculateCheckSumForChunk(newChunk->prev);
    return (void *) ((char *) heapManager.memory + pointerAdd);
}

void *heap_calloc(size_t number, size_t size) {
    if(number<1||size<1)return NULL;
    void *memory = heap_malloc(size*number);
    if(memory==NULL)return NULL;
    char *workingMem=memory;
    for(size_t i=0;i<size*number;i++){
        workingMem[i]=0;
    }
    return memory;
}

void *heap_realloc(void *memblock, size_t count) {
    if(memblock!=NULL&&count==0){
        heap_free(memblock);
        return NULL;
    }
    if(memblock==NULL)return heap_malloc(count);
    if(count<1||Validate()||heap_validate())return NULL;

    int found=0;
    struct chunkHeap_t *chunk=heapManager.firstChunk;
    for(unsigned i=0;i<heapManager.numberOfChunks;i++){
        if((void *)((char*)chunk+chunkSize+fenceSize)==memblock){
            found=1;
            break;
        }
        chunk=chunk->next;
    }


    if(!found)return NULL;

    if(chunk->size>=count){
        chunk->size=count;
        CalculateCheckSumForChunk(chunk);
        DoFences((char*)chunk+chunkSize,count);
        return memblock;
    }

    if(chunk->next==NULL){
        if(IncreaseHeapSize(count-chunk->size))return NULL;
        chunk->size=count;
        CalculateCheckSumForChunk(chunk);
        DoFences((char*)chunk+chunkSize,count);
        return memblock;
    }

    if(SpaceBetweenChunks(chunk)>=count-chunk->size){
        chunk->size=count;
        CalculateCheckSumForChunk(chunk);
        DoFences((char*)chunk+chunkSize,count);
        return memblock;
    }

    char *newMemory= heap_malloc(count);
    if(newMemory==NULL)return NULL;

    for(size_t i=0;i<chunk->size;i++){
        newMemory[i]=((char*)memblock)[i];
    }
    heap_free(memblock);

    CalculateCheckSumForChunk(chunk);
    return newMemory;
}

void heap_free(void *memblock) {
    if(memblock==NULL||heap_validate())return;
    if(Validate())return;
    int free=0;
    struct chunkHeap_t *chunk=heapManager.firstChunk;
    for(unsigned i=0;i<heapManager.numberOfChunks;i++){
        if((void *)((char*)chunk+chunkSize+fenceSize)==memblock){
            free=1;
            break;
        }
        chunk=chunk->next;
    }

    if(free){
        if(chunk->next){
            chunk->next->prev=chunk->prev;
        }
        if(chunk->prev){
            chunk->prev->next=chunk->next;
        }
        else{
            heapManager.firstChunk=chunk->next;
        }
        CalculateCheckSumForChunk(chunk->next);
        CalculateCheckSumForChunk(chunk->prev);
        heapManager.used-=chunk->size+fenceSize*2+chunkSize;
        heapManager.numberOfChunks--;
    }
}

size_t heap_get_largest_used_block_size(void) {
    if(heap_validate())return 0;
    size_t max=0;
    struct chunkHeap_t *chunk=heapManager.firstChunk;
    for(size_t i=0;i<heapManager.numberOfChunks;i++){
        if(i)chunk=chunk->next;
        if(max<chunk->size)max=chunk->size;
    }
    return max;
}

enum pointer_type_t get_pointer_type(const void *const pointer) {
    if (pointer==NULL)return pointer_null;
    if(heap_validate())return pointer_heap_corrupted;
    if(pointer<heapManager.memory)return pointer_unallocated;

    struct chunkHeap_t *chunk=heapManager.firstChunk;

    char *ptr=heapManager.memory;
    size_t ptrAdd=(char *)heapManager.firstChunk-(char*)heapManager.memory;
    int found=0;
    size_t diff;

    for(unsigned i=0;i<heapManager.numberOfChunks;i++){
        if(i)chunk=chunk->next;
        diff=chunk->size+chunkSize+fenceSize*2+SpaceBetweenChunks(chunk);
        ptrAdd+=diff;
        if(pointer<(void*)(ptr+ptrAdd)){
            found=1;
            break;
        }
    }

    if(found){
        ptrAdd-=diff;
        for(size_t i=0;i<chunkSize;i++,ptrAdd++){
            if(pointer==(void*)(ptr+ptrAdd))return pointer_control_block;
        }
        for(size_t i=0;i<fenceSize;i++,ptrAdd++){
            if(pointer==(void*)(ptr+ptrAdd))return pointer_inside_fences;
        }
        for(size_t i=0;i<chunk->size;i++,ptrAdd++){
            if(pointer==(void*)(ptr+ptrAdd)){
                if(i==0)return pointer_valid;
                return pointer_inside_data_block;
            }
        }
        for(size_t i=0;i<fenceSize;i++,ptrAdd++){
            if(pointer==(void*)(ptr+ptrAdd))return pointer_inside_fences;
        }
    }

    return pointer_unallocated;
}


int heap_validate(void){
    if(Validate())return 2;

    unsigned *fence;

    struct chunkHeap_t *chunk=heapManager.firstChunk;
    for(unsigned i=0;i<heapManager.numberOfChunks;i++){
        if(i)chunk=chunk->next;
        if(CalculateCheckSum(chunk)!=chunk->checkSum)return 3;
        if(chunk==NULL)return 3;

        fence=(void*)((char*)chunk+chunkSize);
        if(*fence!=fencesCheckSum)return 1;
        fence=(void*)((char*)chunk+chunkSize+chunk->size+fenceSize);
        if(*fence!=fencesCheckSum)return 1;
    }

    return 0;
}
