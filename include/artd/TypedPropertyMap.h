#pragma once

#include "artd/jlib_util.h"
#include "artd/ObjectBase.h"
#include <type_traits>
#include <unordered_map>
#include "artd/StringArg.h"
#include "artd/Logger.h"

ARTD_BEGIN

#define INL ARTD_ALWAYS_INLINE

class TypedPropertyMap;

class ARTD_API_JLIB_UTIL DefaultKeyRegistrar {
public:
    typedef void (*PodDeleter)(void *);

    static int registerTypedKey(StringArg name, PodDeleter pd);
    static const char *nameForKey(int key);
};

int registerTypedKeyId(const char *name);


template<typename T>
class TypedPropertyKey {
    friend class TypedPropertyMap;
    const int key_;

    static void deleteT(void *pt) {
        reinterpret_cast<T *>(pt)->~T();
    }
    
public:
    typedef DefaultKeyRegistrar::PodDeleter PodDeleter;
    
    TypedPropertyKey(StringArg name) : key_(DefaultKeyRegistrar::registerTypedKey(name,((sizeof(T) <= sizeof(HackStdShared<ObjectBase>)) ? &deleteT : nullptr)))
    {}
};

class ARTD_API_JLIB_UTIL TypedPropertyMap
{
private:
    
    static const int TypeUninitialized = 0;
    static const int TypeTrivialPod = 1;
    static const int TypePod = 2;  // has empty destructor
    static const int TypeShared = 3;
    static const int TypeWeak = 4;
 

    class Entry {
    public:
        uint32_t typeKey_ = 0;
        HackStdShared<ObjectBase> sp;
        
        INL Entry()
        {}
        INL Entry(Entry &&e)
            : sp(std::move(e.sp))
        {
            typeKey_ = e.typeKey_;
            e.typeKey_ = TypeUninitialized;
        }
        INL int getType() {
            return(typeKey_ & 0x07);
        }
        INL uint32_t getIntKey() {
            return(typeKey_ >> 3);
        }
        INL Entry &operator=(Entry &&e) {
            void *oldCb = sp.cbPtr();
            if(oldCb != e.sp.cbPtr()) {
                this->~Entry();
            }
            ::new(this) Entry(std::move(e));
            return(*this);
        }
        template<class T>
        INL Entry(uint32_t intKey, ObjectPtr<T> &op)
            : typeKey_((intKey << 3) | TypeShared)
            , sp(reinterpret_cast<ObjectBase *>(op.get()), HackStdShared<ObjectBase>::cbPtr(op))
        {}
        template<class T>
        INL Entry(uint32_t intKey, WeakPtr<T> &op)
            : typeKey_((intKey << 3) | TypeWeak)
            , sp(reinterpret_cast<ObjectBase *>(op.get()), HackStdShared<ObjectBase>::cbPtr(op))
        {}

        // TODO: this could be cleaner
        template<class T>
        INL Entry(uint32_t intKey, T &&op)
            : typeKey_((intKey << 3) | (std::is_trivially_destructible<T>::value ? TypeTrivialPod : TypePod))
        {
            if(sizeof(T) <= sizeof(sp)) {
                if(getType() == TypePod) { // std::is_trivially_destructible<T>::value) {
                    ::new((void *)&sp) T(std::move(op));
                } else {
                    ::new((void *)&sp) T(op); // )(*reinterpret_cast<T*>(&sp) = op;
                }
            } else {
                new((void *)this) Entry();
            }
        }
        template<class T>
        INL Entry(uint32_t intKey, const T &op)
            : typeKey_((intKey << 3) | (std::is_trivially_destructible<T>::value ? TypeTrivialPod : TypePod))
        {
            if(sizeof(T) <= sizeof(sp)) {
                ::new((void *)&sp) T(op); // )(*reinterpret_cast<T*>(&sp) = op;
            } else {
                new((void *)this) Entry();
            }
        }
        ~Entry();
    };

    typedef std::unordered_map<int,Entry> MapT;
    MapT valuesByInt_;
public:

    template<class ObjT>
    void setSharedProperty(const TypedPropertyKey<ObjT> key, ObjectPtr<ObjT> value) {
        auto found = valuesByInt_.find(key.key_);
        void *oPtr = value.get();
        if(found == valuesByInt_.end()) {
            // adding new item in the map.
            if(oPtr) {
                valuesByInt_.emplace(std::make_pair(key.key_, Entry(key.key_,value))); // insert({ key.key_, Entry(value) } );
                ::new(&value) HackStdShared<ObjT>(nullptr,nullptr);  // clear so we leave input reference in map
            }
        } else {
            // replace existing item in the map
            if(oPtr) {
                valuesByInt_[key.key_] = Entry(key.key_,value);
                ::new(&value) HackStdShared<ObjT>(nullptr,nullptr);  // clear so we leave input reference in map
            } else {
                valuesByInt_.erase(key.key_);
            }
        }
    }

    template<class PodT>
    void setPodProperty(const TypedPropertyKey<PodT> key, PodT &&pod) {
        auto found = valuesByInt_.find(key.key_);
        if(found == valuesByInt_.end()) {
            valuesByInt_.emplace(std::make_pair(key.key_, Entry(key.key_,std::move(pod))));
        } else {
            valuesByInt_[key.key_] = Entry(key.key_, std::move(pod));
        }
    }

    template<class PodT>
    void setPodProperty(const TypedPropertyKey<PodT> key, const PodT &pod) {
        auto found = valuesByInt_.find(key.key_);
        if(found == valuesByInt_.end()) {
            valuesByInt_.emplace(std::make_pair(key.key_, Entry(key.key_,pod)));
        } else {
            valuesByInt_[key.key_] = Entry(key.key_, pod);
        }
    }


    static void test();
};

#undef INL

ARTD_END
