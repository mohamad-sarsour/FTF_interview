#include <unistd.h>
#include <cstring>
#include <iostream>
//#include <stdlib.h>

struct MallocMetadata{
    size_t size;
    bool is_free;
    MallocMetadata* next;
    MallocMetadata* prev;
};

MallocMetadata* root= nullptr;
MallocMetadata* end=nullptr;


void* smalloc(size_t size){
    if(size == 0 || size > 100000000){
        return nullptr;
    }
    if (root == nullptr) {
        void *pointer;
        pointer = sbrk(size + sizeof(MallocMetadata));
        if (pointer == (void*)-1) {
            return nullptr;
        }
        MallocMetadata* metadata = (MallocMetadata*) pointer;

        metadata->size = size;
        metadata->is_free = false;
        metadata->next = nullptr;
        metadata->prev = nullptr;

        root = metadata;
        end = metadata;
        return (void *) (((unsigned char*) pointer) + sizeof(MallocMetadata));

    }
    else{
        MallocMetadata* tmp = root;
        while(tmp!= nullptr){
            if((tmp->size >= size) && (tmp->is_free)){   
                tmp->is_free = false;
                return (void *) (((unsigned char*) tmp) + sizeof(MallocMetadata));
            }
            tmp = tmp->next;
        }
		
        void *pointer = sbrk(size + sizeof(MallocMetadata));
        if (pointer ==(void*)-1) {
            return nullptr;
        }
        MallocMetadata* ptr=(MallocMetadata*)pointer;

        ptr->size = size;
        ptr->is_free= false;
        ptr->next = nullptr;
        ptr->prev = end;
        end->next = ptr;
        end = ptr;
        return (void *) (((unsigned char*) pointer) + sizeof(MallocMetadata));
    }

}

void* scalloc(size_t num,size_t size){
 
    void* pointer = smalloc(size*num);
    if(pointer==nullptr){
	return nullptr;
    }
    memset(pointer, 0, size * num);
    return (void*)pointer;
}


void sfree(void* p ) {
    if (p == nullptr) return;

    auto *p_metadata = (MallocMetadata*)((unsigned char*)p - sizeof(MallocMetadata));
    if(p_metadata->is_free){
        return;
    }
    p_metadata->is_free = true;
  //  return;
}


void* srealloc(void* oldp,size_t size){
    if(size == 0 || size > 100000000){
        return nullptr;
    }
    if(oldp == nullptr){
        return smalloc(size);
    }
    auto* pointer=(MallocMetadata*)(((unsigned char*)oldp)-sizeof(MallocMetadata));
    if(size <= pointer->size){
        //pointer->size=size;
        return oldp;
    }
    void* tmp=smalloc(size);
    if(tmp == (void*)-1){
        return nullptr;
    }
    std::memmove(tmp,oldp,size);
    sfree(oldp);
    return tmp;
}


size_t _num_free_blocks(){
    size_t blocks=0;//,allBlocks=0;
    MallocMetadata* iterator=root;
    while (iterator != nullptr){
        if(iterator->is_free){
            blocks++;
        }
         //  allBlocks++;
        iterator=iterator->next;
    }
  //  std::cout<<blocks<<" instead of "<<allBlocks<<std::endl;
    return blocks;
}


size_t _num_free_bytes(){
    size_t bytes=0;
    MallocMetadata* iterator = root;
    while (iterator != nullptr){
        if(iterator->is_free){
            bytes+=iterator->size;
        }
        iterator = iterator->next;
    }
    return bytes;
}


size_t _num_allocated_blocks(){
    size_t counter=0;
    MallocMetadata* iterator=root;
    while (iterator != nullptr){
        counter++;
        iterator=iterator->next;
    }
    return counter;
}


size_t _num_allocated_bytes(){
    size_t bytes=0;
    MallocMetadata* iterator=root;
    while (iterator != nullptr){
        bytes+=iterator->size;
        iterator=iterator->next;
    }
    return bytes;
}


size_t _num_meta_data_bytes(){
    size_t res = _num_allocated_blocks();
    return sizeof(MallocMetadata)* res;
}


size_t _size_meta_data(){
    return sizeof(MallocMetadata);
}