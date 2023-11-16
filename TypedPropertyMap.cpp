#include "artd/TypedPropertyMap.h"
#include <map>
#include <vector>

ARTD_BEGIN


namespace {

class KeyRegistrarImpl {
public:
    
    uint32_t lastId_ = ~0;

    typedef std::map<std::string, uint32_t> MapT;
    MapT registered_;
    
    static KeyRegistrarImpl &instance() {
        static KeyRegistrarImpl i;
        return(i);
    }
    uint32_t registerKey(StringArg key) {
        std::string ss = key.c_str();
        
        auto found = registered_.find(ss);
        if(found != registered_.end()) {
            AD_LOG(error) << "Key for \"" << key << "\" already registerd";
        }
        registered_.emplace(ss,++lastId_);
        return(lastId_);
    }
};

} // end namespace

int
DefaultKeyRegistrar::registerTypedKey(StringArg name) {
    KeyRegistrarImpl::instance().registerKey(name);
}


TypedPropertyMap::Entry::~Entry() {
    switch(type_) {
        case TypeShared:
            sp.objPtr() = nullptr;
            break;
        case TypeWeak:
            sp.weakPtr() = nullptr;
            break;
        default:
            break;
    }
    type_ = 0;
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

    return;
};


ARTD_END
