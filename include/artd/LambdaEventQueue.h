#ifndef __artd_LambdaEventQueue_h
#define __artd_LambdaEventQueue_h

#include "artd/jlib_util.h"
#include <functional>
#include <new>
#include "artd/IntrusiveList.h"
#include "artd/Mutex.h"
#include "artd/synchronized.h"
#include "artd/Logger.h"

ARTD_BEGIN

#define INL ARTD_ALWAYS_INLINE

class ARTD_API_JLIB_UTIL LambdaEventQueue
    : public ObjectBase
{
public:
    typedef std::function<bool(void* arg)> Lamb;
private:

    class LambdaEvent
        : public ObjectBase
        , public DlNode
    {
        friend class LambdaEventQueue;
        friend class LambdaEventCache;
        
        Lamb f_;
        void* arg_;
        
    public:
        LambdaEvent(void *&cBlockAt, const Lamb &f, void *arg)
            : f_(f) 
            , arg_(arg) 
        {}
        ~LambdaEvent() override {
        }
        INL bool execute() {
            return(f_(arg_));
        }
    };


    class LambdaEventList
        : public IntrusiveList<LambdaEvent>
    {
    public:
        ~LambdaEventList() {
            if (!empty()) {
                LambdaEvent* e;
                while ((e = getHead()) != nullptr) {
                    e->release();
                }
            }
        }
    };

public:

    class ARTD_API_JLIB_UTIL LambdaEventCache
        : public ObjectBase
    {
        IntrusiveList<DlNode> freeMemory_;
        uint32_t freeCount_;
        uint32_t blockSize_;
    public:
        LambdaEventCache()
            : freeCount_(0), blockSize_(0)
        {}
        ~LambdaEventCache();

        void* allocateBlock(size_t size);
        void releaseBlock(void* block);
    };

private:

    Mutex listLock_;
    ObjectPtr<LambdaEventCache> eventCache_;
    LambdaEventList queue_;

    LambdaEvent* getNewEvent(void* arg, const Lamb& f);
    void releaseEvent(LambdaEvent* toAdd);

public:

    LambdaEventQueue() : eventCache_(ObjectBase::make<LambdaEventCache>()) {
    }

    LambdaEventQueue(ObjectPtr<LambdaEventCache> cache) : eventCache_(std::move(cache)) {
    }
    ~LambdaEventQueue() {
    }
    
    void executeEvents() {

        LambdaEventList toRun;
        {
        synchronized(listLock_);
            toRun.appendFrom(queue_);
            if (toRun.empty()) {
                return;
            }
        }
        try {
            LambdaEventList doAgain;
            LambdaEventList doneWith;

            while (!toRun.empty()) {

                // TODO: this should handle exceptions 
                // and deallocate and survive.
                LambdaEvent* he = toRun.getHead();
                if (!he) {
                    break;
                }
                if (he->execute()) {
                    doAgain.addTail(he);
                } else {
                    doneWith.addTail(he);
                }
            }
            // release all the executed events inside a list lock 
            // schedule all the do-again events to be executed again.
            {
            synchronized(listLock_)
                for (;;) {
                    LambdaEvent* he = toRun.getHead();
                    if(!he) break;
                    releaseEvent(he);
                }
                queue_.appendFrom(doAgain);
            }
        }
        catch (...) {
        }
    }
    void postEvent(void *arg, const Lamb lambda) {
        synchronized(listLock_);
        LambdaEvent *he = getNewEvent(arg, lambda);
        queue_.addTail(he);
    }

};

#undef INL

ARTD_END

#endif // __artd_LambdaEventQueue_h

