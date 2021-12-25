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
The global pointer to the list will point to the first metadata structure (see metadata
figure).
