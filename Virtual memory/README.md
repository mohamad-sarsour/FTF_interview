## In this project i will implement a simple memory allocation library, which will include my implementation of malloc(), free(), calloc() and realloc() functions
## part 1
###### 1. How do we know the size of the allocated space that was sent to free?
● On each allocation, do allocate the required memory, but before it – append a
meta-data structure to the block. This means your total allocation would be the
requested size + the meta-data structure size. The meta-data will contain an unsigned
integer that will save the size of the effective allocation (i.e. the requested size).
###### 2. How could we mark a space that was just allocated as free?
● Add a Boolean to the meta-data structure – “is_free”.
###### 3. How can we easily look-up and reuse previously freed memory sectors?
● We can save a global pointer to a list that will contain all the data sectors described
before. We can use this list to search for freed spaces upon allocation requests, instead
of increasing the program break again and enlarging the heap unnecessarily.
The global pointer to the list will point to the first metadata structure.
## part 2
###### the previous implementation has a few fragmentation issues, in this part i will work on solutions for some of those issues.
###### **● Challenge 1 (Memory utilization):**
If we reuse freed memory sectors with bigger sizes than required, we’ll be wasting memory (internal fragmentation).
Solution: Implement a function that smalloc(size_t size) (it searches for a free block with up to ‘size’ bytes or allocates sbrk() one if none are
found) will use, such that if a pre-allocated block is reused and is large enough, the function will cut the block into two smaller blocks
with two separate meta-data structs. One will serve the current allocation, and another will
remain unused for later (marked free and added to the list).
Definition of “large enough”: After splitting, the remaining block (the one that is not used)
has at least 128 bytes of free memory, excluding the size of your meta-data structure.
###### **● Challenge 2 (Memory utilization):**
Many allocations and de-allocations might cause two adjacent blocks to be free, but separate.
Solution: Implement a function that sfree() will use, such that if one adjacent block
(next or previous) was free, the function will automatically combine both free blocks (the
current one and the adjacent one) into one large free block.
###### **● Challenge 3 (Memory utilization):**
Define the “Wilderness” chunk as the topmost allocated chunk. Let’s presume this chunk
is free, and all others are full. It is possible that the new allocation requested is bigger than
the wilderness block, thus requiring us to call sbrk() once more – but now, it is easier to
simply enlarge the wilderness block, saving us an addition of a meta-data structure.
Solution: Change the previous implementation, such that if:
1. A new request has arrived, and no free memory chunk was found big enough.
2. And the wilderness chunk is free.
Then enlarge the wilderness chunk enough to store the new request.
###### **● Challenge 4 (Large allocations):**
modern dynamic memory managers not only use sbrk() but also mmap(). This process helps reduce the negative effects of memory
fragmentation when large blocks of memory are freed but locked by smaller, more recently
allocated blocks lying between them and the end of the allocated space. In this case, had
the block been allocated with sbrk(), it would have probably remained unused by the
system for some time (or at least most of it).
Solution: Change the previous implementation, by looking up how we can use mmap()
and munmap() instead of sbrk() for our memory allocation unit. Use this only for allocations that require 128kb space or more.
