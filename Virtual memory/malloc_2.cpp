#include <unistd.h>
#include <cstring>
#include <iostream>
#include <sys/mman.h>

#define MAX_ALLOCATION 10e7
#define MMAP_ALLOCATION_LIMIT 128*1024

struct MallocMetadata{
    size_t size;
    bool is_free;
    MallocMetadata* next;
    MallocMetadata* prev;
};

MallocMetadata* root= nullptr;
MallocMetadata* end=nullptr;

MallocMetadata* mmap_head= nullptr;
MallocMetadata* mmap_end= nullptr;


void split(MallocMetadata* place,size_t size) {
    MallocMetadata *new_metadata = (MallocMetadata *) ((size_t) place + size + sizeof(MallocMetadata));
    new_metadata->size = place->size - size - sizeof(MallocMetadata);
    new_metadata->is_free = true;
    new_metadata->prev = place;
    new_metadata->next = place->next;
    if (place->next != nullptr) {
        place->next->prev = new_metadata;
    }
    place->next = new_metadata;
    place->size = size;
    if(place==end){
        end=new_metadata;
    }
}

void combine(MallocMetadata* first,MallocMetadata* second){
    first->size += second->size + sizeof(MallocMetadata);
    first->next = second->next;
    if(second->next != nullptr){
        second->next->prev = first;
    }
    if(second == end){
        end = first;
    }
    first->is_free= true;
}

void* smallocSbrk(size_t size){
    if(size == 0 || size > MAX_ALLOCATION){
        return nullptr;
    }
    if (root == nullptr) {
        void *pointer;
        pointer = sbrk(size + sizeof(MallocMetadata));
        if (pointer == (void*)-1) {
            return nullptr;
        }
        auto* metadata = (MallocMetadata*) pointer;

        metadata->size = size;
        metadata->is_free = false;
        metadata->next = nullptr;
        metadata->prev = nullptr;

        root = metadata;
        end = metadata;


        return (void *) (((size_t) pointer) + sizeof(MallocMetadata));

    }
    else{
        MallocMetadata* tmp = root;
        while(tmp!= nullptr){
            if((tmp->size >= size) && (tmp->is_free)){
                if(tmp->size >= 128 + sizeof(MallocMetadata) + size){
                    split(tmp,size);
                }
                tmp->is_free = false;
                return (void *) (((size_t) tmp) + sizeof(MallocMetadata));
            }
            tmp = tmp->next;
        }
        if(end->is_free){
            void* pointer=sbrk(size-end->size);
            if (pointer ==(void*)-1) {
                return nullptr;
            }
            end->size=size;
            end->is_free = false;
            return (void*)((size_t)end + sizeof(MallocMetadata));
        }
        void *pointer = sbrk(size + sizeof(MallocMetadata));
        if (pointer ==(void*)-1) {
            return nullptr;
        }
        auto* tmp1=(MallocMetadata*)pointer;

        tmp1->size = size;
        tmp1->is_free= false;
        tmp1->next = nullptr;
        tmp1->prev = end;
        end->next = tmp1;
        end = tmp1;
        return (void *) (((size_t) pointer) + sizeof(MallocMetadata));
    }
}


void* smallocMmap(size_t size){
    if (mmap_head == nullptr) {
        void* pointer=  mmap(nullptr,size+sizeof(MallocMetadata),PROT_READ | PROT_WRITE
                ,MAP_ANONYMOUS | MAP_PRIVATE,-1,0);
        if(pointer == (void*)-1){
            return nullptr;
        }
        auto* metadata = (MallocMetadata*) pointer;

        metadata->size = size;
        metadata->is_free = false;
        metadata->next = nullptr;
        metadata->prev = nullptr;

       mmap_head = metadata;
       mmap_end = metadata;

        return (void *) (((size_t) pointer) + sizeof(MallocMetadata));

    }
    else{
        MallocMetadata* tmp = mmap_head;
        while(tmp!= nullptr){
            if((tmp->size >= size) && (tmp->is_free)){
                if(tmp->size >= 128 + sizeof(MallocMetadata) + size){
                    split(tmp,size);
                }
                tmp->is_free = false;
                return (void *) (((size_t) tmp) + sizeof(MallocMetadata));
            }
            tmp = tmp->next;
        }
        if(mmap_end->is_free){
            void* pointer = mmap(nullptr,size - end->size,PROT_READ | PROT_WRITE
                    ,MAP_ANONYMOUS | MAP_PRIVATE,-1,0);
            if(pointer == (void*)-1){
                return nullptr;
            }
            mmap_end->size=size;
            mmap_end->is_free = false;
            return (void*)((size_t)mmap_end + sizeof(MallocMetadata));

        }
        void* pointer=  mmap(nullptr,size+sizeof(MallocMetadata),PROT_READ | PROT_WRITE
                ,MAP_ANONYMOUS | MAP_PRIVATE,-1,0);
        if(pointer == (void*)-1){
            return nullptr;
        }

        auto* tmp1=(MallocMetadata*)pointer;

        tmp1->size = size;
        tmp1->is_free= false;
        tmp1->next = nullptr;
        tmp1->prev = end;
        end->next = tmp1;
        end = tmp1;
        return (void *) (((size_t) pointer) + sizeof(MallocMetadata));
    }

}

void* smalloc(size_t size){
    if(size == 0 || size > MAX_ALLOCATION){
        return nullptr;
    }
    if(size < MMAP_ALLOCATION_LIMIT) {
        return smallocSbrk(size);
    }
    return smallocMmap(size);
}


void* scalloc(size_t num,size_t size){
    void * ptr = smalloc(num*size);
    if (!ptr) return nullptr;
    std::memset(ptr,0,num*size);
    return ptr;
}

void sfree(void* p ) {
  if(p == nullptr){
      return;
  }
  auto *p_metadata = (MallocMetadata*)((size_t)p - sizeof(MallocMetadata));
  if(p_metadata->is_free){
      return;
  }
  bool mmaped= p_metadata->size >= MMAP_ALLOCATION_LIMIT;
  if(!mmaped) {
      if(p_metadata==root){
          if(p_metadata->next== nullptr){
              end=root;
              p_metadata->is_free=true;
              return;
          }
          else{
              if(p_metadata->next->is_free) {
                  combine(p_metadata, p_metadata->next);
                  root = p_metadata;
                  p_metadata->is_free=true;
                  return;
              }
          }
      }
      else if(p_metadata==end){
          if(p_metadata->prev->is_free){
              combine(p_metadata->prev,p_metadata);
              end=p_metadata->prev;
          }
          p_metadata->is_free=true;
          return;
      }
      if (p_metadata->next != nullptr && p_metadata->next->is_free) {
            combine(p_metadata,p_metadata->next);
      }
       if (p_metadata->prev != nullptr && p_metadata->prev->is_free) {
            combine(p_metadata->prev,p_metadata);
      }
      p_metadata->is_free=true;
  }

  else{
      ///mmap list
      if(p_metadata->prev == nullptr){
          mmap_head = p_metadata->next;
          if(p_metadata->next != nullptr) mmap_head->prev= nullptr;


      }
      else if(p_metadata->next == nullptr){
          mmap_end = mmap_end->prev;
          if(p_metadata->prev != nullptr ){
              mmap_end->next = nullptr;
          }
      }
      else {
          MallocMetadata *prev = p_metadata->prev;
          MallocMetadata *next = p_metadata->next;
          prev->next = next;
          next->prev = prev;
      }
      munmap(p_metadata,p_metadata->size + sizeof (MallocMetadata));
  }
}


void* srealloc(void* oldp,size_t size) {
    if(size == 0 || size > MAX_ALLOCATION){
        return nullptr;
    }
    if(oldp == nullptr){
        return smalloc(size);
    }
    void* dest;
    auto* pointer=(MallocMetadata*)(((size_t)oldp)-sizeof(MallocMetadata));
    size_t tocopy=pointer->size;
    MallocMetadata* previous=pointer->prev;
    MallocMetadata* next=pointer->next;
    bool merged=false;

    ///try to reuse 1.a
    if(size <= pointer->size){
        if(pointer->size - size >= 128 + sizeof(MallocMetadata)){
            split(pointer,size);
        }
        if(pointer->is_free){
            pointer->size=size;
        }
        pointer->is_free=false;
        return oldp;
    }
    ///merge with lower address 1.b
    else if(pointer->prev!= nullptr && pointer->prev->size + pointer->size +sizeof(MallocMetadata) >= size &&
    pointer->prev->is_free){

        combine(previous,pointer);
        previous->is_free=false;
        merged=true;
        dest=previous;
    }
    ///if pointer on wildreness chunck
    else if(pointer->next != nullptr) {

        ///merge with higher address 1.c
        if (pointer->next->size + pointer->size + sizeof(MallocMetadata) >= size && pointer->next->is_free) {
            combine(pointer,next);
            pointer->is_free= false;
            merged=true;
            dest=pointer;
        }
        ///merge with both low and high adress 1.d
        else if  (pointer->prev != nullptr) {
            if(pointer->prev->size + pointer->next->size + pointer->size + 2* sizeof(MallocMetadata)>=size
            && pointer->prev->is_free && pointer->next->is_free){
                combine(previous,pointer);
                combine(previous,next);
                previous->is_free=false;
                merged=true;
                dest=previous;
            }
        }
    }
    ///1e
    if(merged){
        if(((MallocMetadata*)dest)->size - size >= 128 + sizeof(MallocMetadata)){
            split(((MallocMetadata*)dest),size);
        }
        std::memmove((void*)(((size_t) dest) + sizeof(MallocMetadata)),oldp,tocopy);
    }
    else{
        dest=smalloc(size);
        if(dest == (void*)-1){
            return nullptr;
        }
        std::memmove(dest,oldp,tocopy);
        if(pointer!=(MallocMetadata*)(((size_t) dest) - sizeof(MallocMetadata))) {
            sfree(oldp);
        }
        return (void*)((size_t)dest);
    }
    if(pointer!=((MallocMetadata*)dest)) {
        sfree(oldp);
    }
    return  (void *)(((size_t) dest) + sizeof(MallocMetadata));
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
    MallocMetadata* it2 = mmap_head;
    while (iterator != nullptr || it2 != nullptr) {
        counter += (it2 != nullptr ? 1 : 0) + (iterator != nullptr ? 1 : 0);
        iterator = (iterator != nullptr ? iterator->next : nullptr);
        it2 = (it2 != nullptr ? it2->next : nullptr);
    }
    return counter;
}


size_t _num_allocated_bytes(){
    size_t bytes=0;
    MallocMetadata* iterator=root;
    MallocMetadata* it2=mmap_head;
    int counter=0;
    while (iterator != nullptr || it2 != nullptr){
        bytes+=(it2!= nullptr ? it2->size : 0)+(iterator!= nullptr ? iterator->size : 0);
        iterator=(iterator!= nullptr ? iterator->next : nullptr);
        it2=(it2!= nullptr ? it2->next : nullptr);
        counter++;
    }
    return bytes;
}


size_t _num_meta_data_bytes(){
    size_t res = _num_allocated_blocks();
    return sizeof(MallocMetadata)* res;
}

size_t _size_meta_data(){
    return sizeof (MallocMetadata);
}