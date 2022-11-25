#pragma once
#include <unistd.h>

struct memory_manager_t
{
    void *memory_start;
    size_t memory_size;
    struct memory_chunk_t *first_memory_chunk;
};

struct memory_chunk_t
{
    struct memory_chunk_t* prev;
    struct memory_chunk_t* next;
    size_t size;
    int free;
};

void memory_init(void *address, size_t size);
void *memory_malloc(size_t size);
void memory_free(void *address);