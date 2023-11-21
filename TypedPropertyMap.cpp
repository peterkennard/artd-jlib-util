#include "artd/TypedPropertyMap.h"
#include <map>
#include <vector>
#include <unordered_map>
#include "artd/Logger.h"
#include "artd/RcString.h"

ARTD_BEGIN

#define INL ARTD_ALWAYS_INLINE

namespace {

class KeyRegistrarImpl {
public:
    
    uint32_t lastId_ = ~0;
    
    typedef DefaultKeyRegistrar::PodDeleter PodDeleter;

    class RegEntry;
    
    typedef std::unordered_map<int32_t, RegEntry> InvMap;
    typedef std::map<RcString, int32_t> MapT;
    
    class RegEntry {
    public:
        RegEntry(int32_t key, const char *name, PodDeleter pd)
            : keyInt_(key)
            , name_(name)
            , pd_(pd)
        {}
        
        const MapT::iterator *MapIt_ = nullptr;

        
        const char *getName() {
            return(name_);
            return("it == null");
        }
        int32_t keyInt_;
        const char *name_;
        PodDeleter pd_;
    };

    
    MapT   keyByString_;
    InvMap byKey_;
    
    static KeyRegistrarImpl &instance() {
        static KeyRegistrarImpl i;
        return(i);
    }
    uint32_t registerKey(StringArg key, PodDeleter pd) {
        
        RcString keyStr(key);

        auto found = keyByString_.find(keyStr);
        if(found != keyByString_.end()) {
            AD_LOG(error) << "Key for \"" << key << "\" already registerd";
            return;
        }
        ++lastId_;
        keyByString_.emplace(keyStr, lastId_);

        auto ret = byKey_.emplace(lastId_,RegEntry(lastId_, keyStr.c_str(), pd));

        RegEntry &entry  = ret.first->second;
        
        AD_LOG(print) << " registered key: " <<  entry.getName() << " at: " << entry.keyInt_ << " pd: " << (void *)pd;

        return(entry.keyInt_);
    }
    INL const char *nameForKey(int key) {
        auto found = byKey_.find(key);
        if(found != byKey_.end()) {
            return(found->second.getName());
        }
        return("Key not found!");
    }
    
    INL const RegEntry *getEntryForKey(uint32_t key) {
        auto found = byKey_.find(key);
        if(found != byKey_.end()) {
            return(&(found->second));
        }
        return(nullptr);
    }
};

} // end namespace

int
DefaultKeyRegistrar::registerTypedKey(StringArg name, PodDeleter pd) {
    KeyRegistrarImpl::instance().registerKey(name, pd);
}

const char *
DefaultKeyRegistrar::nameForKey(int key) {
    return(KeyRegistrarImpl::instance().nameForKey(key));
}

void
TypedPropertyMap::_setSharedProperty_(uint32_t key, ObjectPtr<ObjectBase> &value) {
   auto found = valuesByInt_.find(key);
    void *oPtr = value.get();
    if(found == valuesByInt_.end()) {
        // adding new item in the map.
        if(oPtr) {
            valuesByInt_.emplace(std::make_pair(key, Entry(key,value)));
            ::new(&value) HackStdShared<ObjectBase>(nullptr,nullptr);  // clear so we leave input reference in map
        }
    } else {
        // replace existing item in the map
        if(oPtr) {
            valuesByInt_[key] = Entry(key,value);
            ::new(&value) HackStdShared<ObjectBase>(nullptr,nullptr);  // clear so we leave input reference in map
        } else {
            valuesByInt_.erase(key);
        }
    }
}

TypedPropertyMap::Entry::~Entry() {
    switch(getType()) {
        case TypeUninitialized:
            break;
        case TypeTrivialPod:  // do nothing
            break;
        case TypePod: {
            uint32_t iKey = getIntKey();
            auto pEntry = KeyRegistrarImpl::instance().getEntryForKey(iKey);
            if(pEntry) {
                // AD_LOG(print) << "###### pd: " << (void*)(pEntry->pd_) << " deleting for: " << pEntry->name_;
                pEntry->pd_(&this->sp);
            }
            break;
        }
        case TypeShared:
            sp.objPtr() = nullptr;
            break;
        case TypeWeak:
            sp.weakPtr() = nullptr;
            break;
        default:
            break;
    }
    typeKey_ = TypeUninitialized;
}

class Foo {
public:
    int xx = 10;
};

class TestObject
{
public:
    
    static const TypedPropertyKey<Foo> fooKey;
    
    const char *name_;
    TestObject(const char *name) : name_(name) {
    }
    TestObject(TestObject &&from)
        : name_(from.name_)
    {
        from.name_ = "Moved Out";
    }
    TestObject(const TestObject &other)
        : name_(other.name_)
    {
    }
    
    ~TestObject() {
        AD_LOG(info) << "destroyed " << name_;
    }
};

struct simple {
    float a = 1.1;
};


const TypedPropertyKey<Foo> TestObject::fooKey("foo");

//int valtest = 4;
//int valtest2 = 5;

void callFn(std::function<void()> f) {
    f();
    
    AD_LOG(print) << sizeof(f);
}


void
TypedPropertyMap::test() {

//    AD_LOG(print) << "vec size " << sizeof(std::vector<void *>);
    
//    int valtest5 = 99;
//
//    int *val = &valtest;
//    int *val2 = &valtest2;
//    int *val3 = &valtest;
//    int *val4 = &valtest2;
//    int *val5 = &valtest5;
//
//    auto fn = [val,val2,val3,val4,val5]() {
//        AD_LOG(print) << *val;
//        AD_LOG(print) << *val2;
//        AD_LOG(print) << *val3;
//        AD_LOG(print) << *val4;
//        AD_LOG(print) << *val5;
//        return;
//    };
//
//    callFn(fn);

    TypedPropertyKey<TestObject> key1("key1");
    TypedPropertyKey<TestObject> key2("key2");
    TypedPropertyKey<simple> key3("Simple");

    TypedPropertyMap map;

    ObjectPtr<TestObject> tO1 = ObjectPtr<TestObject>::make("TestObject1");
    ObjectPtr<TestObject> tO2 = ObjectPtr<TestObject>::make("TestObject2");
    
    map.setSharedProperty(key1,tO1);
    tO1 = nullptr;
    map.setSharedProperty(key1,tO2);  // replace object

    map.setPodProperty(key1, TestObject("Pod Test Object"));
    
    simple sim;
    sim.a = 2.2;
    
    map.setPodProperty(key3,sim);
    
    AD_LOG(print) << sim.a;
    return;
};


ARTD_END
