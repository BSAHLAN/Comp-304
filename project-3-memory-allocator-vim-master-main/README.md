# MEMORY ALLOCATION
In our Implementation malloc, free, calloc and fragmantation for 16 bytes work. Locality works but the logic behind it is different than the autograder expecting. Also, the blocks are multiples of 8.

## KUMALLOC

- Firstly we are aligning the size asked summed by the meta data size.

- After that we are performing the necessary checks. Firstly, we are controlling whether the size is 0. After that, we are controlling if we have a pointer pointing the start of the heap. If a pointer does not exist we assign a pointer to the start of the heap. Thirdly, we are searching if there is a free block existing by simply iteraating over all blocks. If there is no free block we are requesting new heap space from the kernel.

-If an existing free block found, we are checking whether the size of the block is greater than the requested size. If the requested size is smaller, we are splitting the heap space.

## KUCALLOC

- In kucalloc function we are simply call kumalloc with the input "number_of_elements * size_of_element".

- After allocating the heap space, we assing zeros by memset.

## KUFREE

- To free the allocated space, we are getting the meta data of the heap space called block and change its free attribute to "false".

- After that we are controlling whether the space we freed is at the end of the heap. If it is, we are giving memory back to kernel.

- Finally, we are merging the consecutive free blocks by iterating from the start of the heap.

## KUREALLOC

- If there is no previously allocated space inputted, the function returns kumalloc(requested_size).

- If the size is zero, the function calls kufree.

- If the current heap block has enough space, the function returns the same block.

- If we need new space, we simply call kumalloc. After that, copy the existing data to the new allocated heap space and delete the previous space.

