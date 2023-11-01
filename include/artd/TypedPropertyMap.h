#pragma once

#include "artd/jlib_util.h"
#include "artd/ObjectBase.h"
#include <unordered_map>
#include "artd/Logger.h"

ARTD_BEGIN

#define INL ARTD_ALWAYS_INLINE

class TypedPropertyMap;

template<typename T>
class TypedPropertyKey {
    friend class TypedPropertyMap;
    const int key_;
    TypedPropertyKey(int key) : key_(key) {}
//    INL T *cast(void *p) {
//        return(renterpret_cast<T *>(p));
//    }
};

class ARTD_API_JLIB_UTIL TypedPropertyMap
{
private:
    class Entry {
    public:
        HackStdShared<ObjectBase> sp;
        INL Entry()
            : sp(nullptr,nullptr)
        {}
        INL Entry(Entry &&e)
            : sp(std::move(e.sp)) {
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
            : sp(reinterpret_cast<ObjectBase *>(op.get()), HackStdShared<ObjectBase>::cbPtr(op))
        {}
        ~Entry();
    };

    typedef std::unordered_map<int,Entry> MapT;
    MapT valuesByInt_;
public:

    template<class ObjT>
    void setOwnedProperty(const TypedPropertyKey<ObjT> key, ObjectPtr<ObjT> value) {
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
