#pragma once

#include "artd/jlib_util.h"
#include "artd/ObjectBase.h"
#include <unordered_map>
#include "artd/StringArg.h"
#include "artd/Logger.h"

ARTD_BEGIN

#define INL ARTD_ALWAYS_INLINE

class TypedPropertyMap;

class ARTD_API_JLIB_UTIL DefaultKeyRegistrar {
public:
    static int registerTypedKey(StringArg name);
};

int registerTypedKeyId(const char *name);


template<typename T>
class TypedPropertyKey {
    friend class TypedPropertyMap;
    const int key_;
public:
    TypedPropertyKey(StringArg name) : key_(DefaultKeyRegistrar::registerTypedKey(name))
    {}
};

class ARTD_API_JLIB_UTIL TypedPropertyMap
{
private:
    
    static const int TypePod = 0;
    static const int TypeShared = 1;
    static const int TypeWeak = 2;
    
    class Entry {
    public:
        uint32_t type_ = 0;
        HackStdShared<ObjectBase> sp;
        
        INL Entry()
            : sp(nullptr,nullptr)
        {}
        INL Entry(Entry &&e)
            : sp(std::move(e.sp))
        {
            type_ = e.type_;
            e.type_ = TypePod;
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
        INL Entry(ObjectPtr<T> &op)
            : type_(TypeShared)
            , sp(reinterpret_cast<ObjectBase *>(op.get()), HackStdShared<ObjectBase>::cbPtr(op))
        {}
        template<class T>
        INL Entry(WeakPtr<T> &op)
            : type_(TypeWeak)
            , sp(reinterpret_cast<ObjectBase *>(op.get()), HackStdShared<ObjectBase>::cbPtr(op))
        {}
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
                valuesByInt_.emplace(std::make_pair(key.key_, Entry(value))); // insert({ key.key_, Entry(value) } );
                ::new(&value) HackStdShared<ObjT>(nullptr,nullptr);  // clear so we leave input reference in map
            }
        } else {
            // replace existing item in the map
            if(oPtr) {
                valuesByInt_[key.key_] = Entry(value);
                ::new(&value) HackStdShared<ObjT>(nullptr,nullptr);  // clear so we leave input reference in map
            } else {
                valuesByInt_.erase(key.key_);
            }
        }
    }
    static void test();
};

#undef INL

ARTD_END
