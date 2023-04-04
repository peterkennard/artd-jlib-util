#include "artd/ObjLinkedList.h"
#include "artd/ObjectBase.h"
#include "artd/IntrusiveList.h"

ARTD_BEGIN

class ObjListBase::Node
	: public RawDlNode
{
public:
	Node(ObjectPtr<ObjectBase> &&p) : o_(std::move(p)) {}
	ObjectPtr<ObjectBase> o_;
};

class ObjListBase::List
	: public SimpleDlist
{
public:
	RawDlNode *pEnd() { return((RawDlNode *)(void *)&(list_.next)); }
	RawDlNode *pHead() { return(list_.next); }
	RawDlNode *pTail() { return(list_.prev); }
};

inline ObjListBase::List &ObjListBase::l() 
{ 
	return(*(List *)(void *)&l_[0]); 
}
void 
ObjListBase::add_(ObjectPtr<ObjectBase> && o)
{
	if(o) {
		l().virginAddTail(new Node(std::move(o)));
		++size_;
	}
}
bool 
ObjListBase::contains_(void *o)
{
	List::iterator it = l().begin();
	while(it != l().end()) {
		if(((Node *)&it)->o_.ptr() == o) {
			return(true);
		}
		++it;
	}
	return(false);
}

ObjectPtr<ObjectBase>
ObjListBase::retrieve_(const void *o)
{
	List::iterator it = l().begin();
	while(it != l().end()) {
		Node *n = ((Node *)&it); 
		if(n->o_.ptr() == o) {
			--size_;
			ObjectPtr<ObjectBase> ret(std::move(n->o_));
			n->detach();
			delete(n);
			return(ret);
		}
		++it;
	}
	return(nullptr);
}

ObjectPtr<ObjectBase>
ObjListBase::getFirst_() {
	Node *n = (Node*)(l().peekHead());
	if (n != nullptr) {
		ObjectPtr<ObjectBase> ret(std::move(n->o_));
		n->detach();
		delete(n);
		return(ret);
	}
	return(ObjectPtr<ObjectBase>());
}
ObjectPtr<ObjectBase>
ObjListBase::getLast_() {
	Node *n = (Node*)(l().peekTail());
	if (n != nullptr) {
		ObjectPtr<ObjectBase> ret(std::move(n->o_));
		n->detach();
		delete(n);
		return(ret);
	}
	return(ObjectPtr<ObjectBase>());
}

bool 
ObjListBase::remove_(void *o)
{
	ObjectPtr<ObjectBase> ret = retrieve_(o);
	if(ret) {
		return(true);
	}
	return(false);
}
void
ObjListBase::clear()
{
	Node *n;
	while((n = (Node *)(l().getHead()))) {
		delete(n);
	}
	size_ = 0;
}

ARTD_ALWAYS_INLINE static ObjectPtr<ObjectBase>& nullObjectPointer() {
	static ObjectPtr<ObjectBase> nullOne;
	return(nullOne);
}


ObjectPtr<ObjectBase>&
ObjListBase::peekLast_() {
	Node *n = (Node*)(l().peekHead());
	if (n != nullptr) {
		return(n->o_);
	}
	return(nullObjectPointer());
}
ObjectPtr<ObjectBase>&
ObjListBase::peekFirst_() {
	Node *n = (Node*)(l().peekTail());
	if (n != nullptr) {
		return(n->o_);
	}
	return(nullObjectPointer());
}

#if 0
class ObjListBase::It
	: public VIteratorBase
{
public:

	It(ObjListBase::List &l, RawDlNode *n)
	{
		new(p) Data(l,n);
		ARTD_STATIC_ASSERT(sizeof(p) == sizeof(Data));
	}
	class Data {
	public:
		Data(ObjListBase::List &l, RawDlNode *n) : next_(n), list_(l) {}
	
		RawDlNode *			next_; // test for hasNext
		ObjListBase::List &	list_;
		RawDlNode *			last_;
	};
	inline Data &d() { return(*((Data *)(void *)&p)); }
	void *	_next() 
	{ 
		d().last_ = d().next_; 
		d().next_ = d().next_->next; 
		
		if(d().next_ == d().list_.pEnd()) {
			d().next_ = 0;
		}
		return(((ObjListBase::Node *)d().last_)->o_.ptr()); 
	}
	void	_remove() { } // TODO: unimplemented
};

class ObjListBase::RIt
	: public ObjListBase::It
{
public:
	RIt(ObjListBase::List &l, RawDlNode *n)
		: It(l,n)	
	{
	}
	void *_next() 
	{ 
		d().last_ = d().next_; // reversed for this :) 
		d().next_ = d().next_->prev; // get PREVOUS node
		
		if(d().next_ == d().list_.pEnd()) {
			d().next_ = 0;
		}
		return(((ObjListBase::Node *)d().last_)->o_.ptr()); 
	}
};


void
ObjListBase::getIterator_(VIteratorBase &it)
{
	new(&it) It(l(),l().pHead());
}
void
ObjListBase::getReverseIterator_(VIteratorBase &it)
{
	new(&it) RIt(l(),l().pTail());
}

#endif

ObjListBase::ObjListBase()
{
	size_ = 0;
	ARTD_STATIC_ASSERT(sizeof(l_) <= sizeof(List));
	new(&l()) List();
}
ObjListBase::~ObjListBase()
{
	clear();
}

#ifdef DEBUGx
	static void testFunc()
	{
		ObjLinkedList<ObjListBase> l;
		ObjLinkedList<ObjListBase>::Iterator it = l.iterator();
	
		while(it.hasNext()) {
			ObjListBase *l = it.next();
		}
	}
#endif


ARTD_END
