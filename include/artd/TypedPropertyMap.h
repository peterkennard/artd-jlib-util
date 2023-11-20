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
    static const int TypeTrivialPod = 1; // no destructor
    static const int TypePod = 2;  // has a destructor
    static const int TypeShared = 3;  // dereference destructor
    static const int TypeWeak = 4;    // weak dereference destructor

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
            switch(getType()) {
                case TypeTrivialPod:
                    goto moveIt;
                case TypePod:
                case TypeShared:
                case TypeWeak:
                    this->~Entry();
                moveIt:
                    new(this) Entry(std::move(e));
                default:
                    break;
            }
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
                ::new((void *)&sp) T(std::move(op));
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

    // NOTE: this will std::move the value into the map !!!
    // SO Do not use this except when you want that !!
    void _setSharedProperty_(uint32_t key, ObjectPtr<ObjectBase> &value);
public:

    template<class ObjT>
    INL void setSharedProperty(const TypedPropertyKey<ObjT> key, ObjectPtr<ObjT> value) {
        _setSharedProperty_(key.key_, *reinterpret_cast<ObjectPtr<ObjectBase> *>(&value));
    }

    template<class PodT>
    void setPodProperty(const TypedPropertyKey<PodT> &key, PodT &&pod) {
        auto found = valuesByInt_.find(key.key_);
        if(found == valuesByInt_.end()) {
            valuesByInt_.emplace(std::make_pair(key.key_, Entry(key.key_,std::move(pod))));
        } else {
            valuesByInt_[key.key_] = std::move(Entry(key.key_, std::move(pod)));
        }
    }

    template<class PropT>
    bool hasProperty(const TypedPropertyKey<PropT> &key) {
        return(valuesByInt_.find(key.key_) != valuesByInt_.end());
    }

    template<class PodT>
    void setPodProperty(const TypedPropertyKey<PodT> &key, const PodT &pod) {
        auto found = valuesByInt_.find(key.key_);
        if(found == valuesByInt_.end()) {
            valuesByInt_.emplace(std::make_pair(key.key_, Entry(key.key_,pod)));
        } else {
            valuesByInt_[key.key_] = Entry(key.key_, pod);
        }
    }

    template<class PodT>
    INL void setProperty(const TypedPropertyKey<PodT> &key, const PodT &pod) {
        setPodProperty(key,pod);
    }

    template<class PodT>
    INL void setProperty(const TypedPropertyKey<PodT> &key, PodT &&pod) {
        setPodProperty(key,std::move(pod));
    }

    template<class PropT>
    INL void setProperty(const TypedPropertyKey<PropT> &key, ObjectPtr<PropT> &prop) {
        setSharedProperty(key,prop);
    }

    template<class PropT>
    PropT *getProperty(const TypedPropertyKey<PropT> &key) {
        auto found = valuesByInt_.find(key.key_);
        if(found != valuesByInt_.end()) {
            switch(found->second.getType()) {
                case TypeUninitialized:
                    break;
                case TypeTrivialPod:
                case TypePod:
                   return(reinterpret_cast<PropT*>(&(found->second.sp)));
                case TypeShared:
                case TypeWeak:
                   return(reinterpret_cast<PropT*>(found->second.sp.objPtr().get()));
                default:
                    break;
            }
        }
        return(nullptr);
    }
    static void test();
};

#undef INL

ARTD_END
