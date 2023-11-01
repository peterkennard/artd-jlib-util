#include "artd/TypedPropertyMap.h"

ARTD_BEGIN


TypedPropertyMap::Entry::~Entry() {
    if(sp.cbPtr()) {
        sp.objPtr() = nullptr;
    }
}

class TestObject
//    : public ObjectBase
{
public:
    const char *name_;
    TestObject(const char *name) : name_(name) {}
//    void onObjectCreated() {
//        AD_LOG(info) << "created " << name_;
//    }
    
    ~TestObject() {
        AD_LOG(info) << "destroyed " << name_;
    }
};

void
TypedPropertyMap::test() {

    TypedPropertyKey<TestObject> key1(1);
    TypedPropertyKey<TestObject> key2(2);

    TypedPropertyMap map;

    ObjectPtr<TestObject> tO1 = ObjectBase::make<TestObject>("TestObject1");
    ObjectPtr<TestObject> tO2 = ObjectBase::make<TestObject>("TestObject2");

    map.setOwnedProperty(key1,tO1);
    tO1 = nullptr;
    map.setOwnedProperty(key1,tO2);  // replace object

    return;
};


ARTD_END
