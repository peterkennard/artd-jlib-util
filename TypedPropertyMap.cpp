#include "artd/TypedPropertyMap.h"
#include <map>
#include <vector>
#include <unordered_map>
#include "artd/Logger.h"

ARTD_BEGIN

#define INL ARTD_ALWAYS_INLINE

namespace {

class KeyRegistrarImpl {
public:
    
    uint32_t lastId_ = ~0;
    
    typedef DefaultKeyRegistrar::PodDeleter PodDeleter;

    class Entry;
    
    typedef std::unordered_map<int32_t, Entry> InvMap;
    typedef std::map<std::string, const InvMap::iterator &> MapT;
    
    class Entry {
    public:
        Entry(int32_t key, PodDeleter pd)
            : keyInt_(key)
            , pd_(pd)
        {}
        
        const MapT::iterator *MapIt_;

        void setMapTIterator(const MapT::iterator &it) {
            MapIt_ = &it;
        }
        const char *getName() const {
            return("name");
        }
        int32_t keyInt_;
        
        PodDeleter pd_;
    };

    
    
    MapT registered_;
    InvMap byKey_;
    
    static KeyRegistrarImpl &instance() {
        static KeyRegistrarImpl i;
        return(i);
    }
    uint32_t registerKey(StringArg key, PodDeleter pd) {
        std::string ss = key.c_str();

        // TODO: maybe inverse map shoudl be actual entry, and restered_ only the rteference.
        
        auto found = registered_.find(ss);
        if(found != registered_.end()) {
            AD_LOG(error) << "Key for \"" << key << "\" already registerd";
            return;
        }
        ++lastId_;
        auto ret = byKey_.emplace(lastId_,Entry(lastId_, pd));
        const InvMap::iterator &newEntry = ret.first;
        // AD_LOG(print) << "new key: " << newEntry->first << " entry: " << newEntry->second.keyInt_;

        auto ret2 = registered_.emplace(ss,newEntry);
        const MapT::iterator &newEntry2 = ret2.first;

    //    newEntry->second.setRegisteredEntry(newEntry2);
        
        auto found2 = byKey_.find(lastId_);

        return(found2->second.keyInt_);
    }
    INL const char *nameForKey(int key) {
        auto found = byKey_.find(key);
        if(found != byKey_.end()) {
            return(found->second.getName());
        }
        return("Key not found!");
    }
    
    INL const Entry *getEntryForKey(uint32_t key) {
        auto found = byKey_.find(key);
        if(found != byKey_.end()) {
            return(&found->second);
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

TypedPropertyMap::Entry::~Entry() {
    switch(getType()) {
        case TypeUninitialized:
            break;
        case TypePod: {
            uint32_t iKey = getIntKey();
            auto *entry = KeyRegistrarImpl::instance().getEntryForKey(iKey);
            if(entry) {
                entry->pd_(&this->sp);
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
//    : public ObjectBase
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
    
    //    void onObjectCreated() {
//        AD_LOG(info) << "created " << name_;
//    }
    
    ~TestObject() {
        AD_LOG(info) << "destroyed " << name_;
    }
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

    AD_LOG(print) << "vec size " << sizeof(std::vector<void *>);
    
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

    TypedPropertyMap map;

    ObjectPtr<TestObject> tO1 = ObjectBase::make<TestObject>("TestObject1");
    ObjectPtr<TestObject> tO2 = ObjectBase::make<TestObject>("TestObject2");
    
    map.setSharedProperty(key1,tO1);
    tO1 = nullptr;
    map.setSharedProperty(key1,tO2);  // replace object

    map.setPodProperty(key1, TestObject("Pod Test Object"));

    
    return;
};


ARTD_END
