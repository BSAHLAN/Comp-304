#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define BLOCK_SIZE sizeof(Block)
#define ALIGNMENT sizeof(long) // Alignment to sizeof(long)
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))
#define min(a, b) ((a) < (b) ? (a) : (b))
#define CHUNK_SIZE 4096
/*
block for metadata
*/
typedef struct Block {
    size_t size;         // Size of the memory block 56 byte calismiyor nedense
    bool free;            // Flag to indicate if the block is free
}Block;

Block *heap_start = NULL; // to keep track of the start of the heap 
Block *heap_end = NULL; 
Block *last_freed_block = NULL; // locality 

void merge_free_blocks() {
    Block *current = heap_start;

    while (current != NULL && (char*)current < (char*)heap_end) {
        // Check if the current block and the next block are free
        Block *next = (Block *)((char*)(current + 1) + current->size);
        while ((char*)next < (char*)heap_end && current->free && next->free) {

            if((char *)last_freed_block == (char *)next){
                last_freed_block = current;
            }
            // Merge the current block and the next block
            current->size += next->size + BLOCK_SIZE;
            next = (Block*)((char*)(current + 1) + current->size);

            
        }

        // Move to the next block
        current = (Block*)((char*)(current + 1) + current->size);
    }

}


Block *find_free_block(size_t size) {

    Block *current = heap_start;
    while (current ) {

        if(current >= heap_end){
            return NULL;
        }
        if (current->free && current->size >= size) {           
            return current;
        }
        
        current = (Block *)((char *)current + BLOCK_SIZE + current->size); // Move to the next block (reason for char* is poiter aritmatik )
        
    }
    return NULL;

}

Block *request_space(size_t size) {

    Block *block = sbrk(0);
    void *request = sbrk(size);

    if (request == (void*) -1) {
        return NULL;
    }
  
    block->size = size - BLOCK_SIZE;
    block->free = false;

    heap_end = sbrk(0);// setting the end of the heap

    return block;
}

void *kumalloc(size_t size)
{
    size_t alignedSize = ALIGN(size + BLOCK_SIZE); // align the size of the allocation

    Block *block;

    if (alignedSize <= 0 || size <= 0) { // if empty allocation 
        return NULL;
    }

    if (!heap_start) { // Initialize heap_start on the first allocation
        block = request_space(alignedSize);
        if (!block) {
            return NULL;
        }
        heap_start = block;       
    } else {
        if (last_freed_block && last_freed_block->size >= size) {
            block = last_freed_block;
            last_freed_block = NULL; // Reset last_freed_block 
        } else {
            block = find_free_block(size);
            if (!block) {
                block = request_space(alignedSize);
                if (!block) {
                    return NULL;
                }
            }
        }


        // Split the block if it's bigger than the requested size by at least the size of a Block
         if (block->size >= alignedSize + BLOCK_SIZE) {
             //printf("\nproblem here 1 \n");
             Block *new_block = (Block *)((char*)block + alignedSize); // Get the address of the new block
             new_block->size = block->size - alignedSize; // Set the size of the new block
             new_block->free = true; // Set the new block as free
             block->size = alignedSize - BLOCK_SIZE; // Set the size of the original block

             // Update heap_end if the new block is at the end of the heap
             if ((char*)(new_block + 1) + new_block->size == (char *)heap_end) {
   
                 heap_end = (Block *)((char*)new_block + ALIGN(new_block->size + BLOCK_SIZE));
            }
        }

        block->free = false;

    }
    
    return (void*)(block + 1); // Return a pointer to the usable memory (after the Block metadata)
}




void *kucalloc(size_t nmemb, size_t size)
{

 size_t total_size =  nmemb * size;
 void *arr = kumalloc(total_size);

 if(arr != NULL){
   memset(arr, 0, total_size);
 }

return arr;

}

void kufree(void *ptr)
{
    
    if (!ptr) {
        return; // Do nothing if NULL pointer
    }
    Block *block = (Block*)ptr - 1; // Get the block metadata
    block->free = true; // Mark the block as free


    Block *current = block;
    
    // Return memory to the kernel if it's at the end of the program break
    if ((char*)(current + 1) + current->size == (char *)heap_end) {

        int status = brk(current);
        heap_end = current;
        if(current == heap_start){
            heap_start = NULL;
        }
        if (status == -1) {
            
        }
    }else{
        last_freed_block = current;
    }

    //merging the free blocks
    merge_free_blocks();
}


void *kurealloc(void *ptr, size_t size)
{
   
    if (!ptr) {
        
        return kumalloc(size);
    }

    if (size == 0) {
        
        kufree(ptr);
        return NULL;
    }

    Block *block = (Block*)ptr - 1; 
    if (block->size >= size) { 
        return ptr;
    }

    void *new_ptr = kumalloc(size); 
    if (!new_ptr) {
        return NULL; 
    }

   
    memcpy(new_ptr, ptr, min(size,block->size));

    kufree(ptr); 

    return new_ptr;

}

/*
 * Enable the code below to enable system allocator support for your allocator.
 * Doing so will make debugging much harder (e.g., using printf may result in
 * infinite loops).
 */
#if 0
void *malloc(size_t size) { return kumalloc(size); }
void *calloc(size_t nmemb, size_t size) { return kucalloc(nmemb, size); }
void *realloc(void *ptr, size_t size) { return kurealloc(ptr, size); }
void free(void *ptr) { kufree(ptr); }
#endif

