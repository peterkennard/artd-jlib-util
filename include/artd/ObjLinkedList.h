#ifndef __artd_ObjList_h
#define __artd_ObjList_h

#include "artd/jlib_util.h"
#include "artd/ObjectBase.h"
// #include "Viterator.h"

ARTD_BEGIN

#define INL ARTD_ALWAYS_INLINE

class ARTD_API_JLIB_UTIL ObjListBase
	: public ObjectBase
{
public:

	void clear();
	size_t size() { return(size_); }
	bool isEmpty() { return(size_ == 0); }

	// void getIterator_(VIteratorBase &it);
	// void getReverseIterator_(VIteratorBase &it);

protected:
	ObjListBase();
	~ObjListBase();

	void add_(ObjectPtr<ObjectBase> && o);
	bool contains_(void *o);
	bool remove_(void *o);
	ObjectPtr<ObjectBase> retrieve_(const void *o);
	ObjectPtr<ObjectBase>& peekFirst_();
	ObjectPtr<ObjectBase>& peekLast_();
	ObjectPtr<ObjectBase> getFirst_();
	ObjectPtr<ObjectBase> getLast_();

	class List;
	class Node;
	class It;
	class RIt;

	List &l();

	size_t	size_;
	void *	l_[2];
};


template<class ObjT=ObjectBase>
class ObjLinkedList
	: public ObjListBase
{
	typedef ObjLinkedList<ObjT> ThisT;

	// Note - one MUST not apply offsets an htis will handle
	// items where there are multiply inherited ObjectBased item
	// in the parent class.

	static INL ObjectPtr<ObjectBase>& castToBase(ObjectPtr<ObjT>& inp) {
		return(reinterpret_cast<ObjectPtr<ObjectBase>&>(inp));
	}
	static INL ObjectPtr<ObjT>& castFromBase(ObjectPtr<ObjectBase>& inp) {
		return(*reinterpret_cast<ObjectPtr<ObjT>*>(&inp));
	}
public:

	INL void add(ObjectPtr<ObjT> o) {
		add_(std::move(castToBase(o))); 
	}

	INL bool contains(ObjT* o) { return(contains_(o)); }

	template<class CheckT>
	INL bool contains(ObjectPtr<CheckT> &o) { return(contains_(o.get())); }

	template<class T>
	INL bool remove(ObjectPtr<T> &o) { return(remove_(o.get())); }
	INL bool remove(void *o) { return(remove_(o)); }
	
	INL ObjectPtr<ObjT> &peekFirst() {
		return(castFromBase(peekFirst_()));
	}
	INL ObjectPtr<ObjT> &peekLast()	{
		return(castFromBase(peekLast_()));
	}
	/** remove an object and if found and removed return the object with a reference +1 otherwise empty pointer
	 * destructor in returned pointer will de-reference 
	 */   
	template<class T>
	INL ObjectPtr<ObjT> retrieve(ObjectPtr<T> &o) {
		ObjectPtr<ObjectBase> ret = retrieve_(o.get());
		return(castFromBase(ret));
	}
	INL ObjectPtr<ObjT> retrieve(void *o) {
		ObjectPtr<ObjectBase> ret = retrieve_(o);
		return(castFromBase(ret));
	}

    //	typedef VIterator<ObjT>     Iterator;
	// INL Iterator iterator() { return Iterator(this,&VIteratorBase::constructIterator<Iterator,ThisT>); }
	// INL Iterator reverseIterator() { return Iterator(this,&VIteratorBase::constructReverseIterator<Iterator,ThisT>); }
};

#undef INL 

ARTD_END

#endif // __artd_ObjList_h
