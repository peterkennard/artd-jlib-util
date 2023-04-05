#include "artd/LambdaEventQueue.h"
#include "artd/Logger.h"
#include <iostream>

ARTD_BEGIN


template<typename T>
class PooledMessageAllocator
{
public:
    LambdaEventQueue::LambdaEventCache* owner_;
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;
    typedef T* pointer;
    typedef const T* const_pointer;
    typedef T& reference;
    typedef const T& const_reference;
    typedef T value_type;


    template<typename U>
    struct rebind { typedef PooledMessageAllocator<U> other; };

    void setOwner(LambdaEventQueue::LambdaEventCache* owner) {
        owner_ = owner;
    }
    PooledMessageAllocator(LambdaEventQueue::LambdaEventCache* owner) noexcept 
        : owner_(owner) 
    {}
    PooledMessageAllocator() noexcept {}

    PooledMessageAllocator(const PooledMessageAllocator& other) noexcept 
        : owner_(other.owner_) {
    }
    template<typename U>
    PooledMessageAllocator(const PooledMessageAllocator<U>& other) noexcept : owner_(other.owner_) {
    }
    template<typename U>
    PooledMessageAllocator& operator = (const PooledMessageAllocator<U>& other) {
        owner_ = other.owner_;
        return *this;
    }
    PooledMessageAllocator<T>& operator = (const PooledMessageAllocator& other) { 
        owner_ = other.owner_;
        return *this;
    }
    ~PooledMessageAllocator() {}

    pointer allocate(size_type n) {
        return((pointer)(owner_->allocateBlock(n * sizeof(T))));
    }
    void deallocate(T* ptr, size_type n) {
        owner_->releaseBlock(ptr);
    }
};

// TODO: not very sophisticated in if we have lots and lots of messages flying about
// probably want to alloate large blocks and always allocated from the earliest block
// first so eventually all end up compacted i the first block and later blocks freed
// anyway some better algorithm.

void* LambdaEventQueue::LambdaEventCache::allocateBlock(size_t size) {
    ObjAllocatorArg* a = ObjAllocatorArg::getArg();
    if (a) {
        void* block = freeMemory_.getTail();
        a->allocatedSize = size;
        if (!block) {
            if (blockSize_ == 0) {
                blockSize_ = (int32_t)size;
            }
            ARTD_ASSERT(size == blockSize_);
            block = ::operator new(size);
        }
        else {
            --freeCount_;
        }
        a->allocatedAt = block;
        return(block);
    }
    return(nullptr);  // yes kaboom
}

void LambdaEventQueue::LambdaEventCache::releaseBlock(void* block) {
    //    ::operator delete(block);
    ++freeCount_;
    freeMemory_.addHead(new(block) DlNode);
}

LambdaEventQueue::LambdaEventCache::~LambdaEventCache() {
    if (!freeMemory_.empty()) {
        DlNode *e;
        while ((e = freeMemory_.getHead()) != nullptr) {
            ::operator delete(e);
        }
    }
}


LambdaEventQueue::LambdaEvent* LambdaEventQueue::getNewEvent(void* arg, const Lamb& f) {

    ObjAllocatorArg allocArg(0);
    PooledMessageAllocator<LambdaEvent> allocEvent(eventCache_.get());
    std::shared_ptr<LambdaEvent> sptr = std::allocate_shared<LambdaEvent>(allocEvent, allocArg.allocatedAt, f, arg);
    LambdaEvent* me = sptr.get();
    new(&sptr) std::shared_ptr<int>(); // will clear cb and pre leaving it with one reference
    return(me);
}

void LambdaEventQueue::releaseEvent(LambdaEvent* toAdd) {
    if(toAdd) toAdd->release();
}

ARTD_END