#include "interfaces.h"
#include "objects.h"

namespace Tempe {

	GCReg::GCReg() :prev(0), next(0)
	{

	}

	GCReg::~GCReg()
	{
		unregisterFromGC();
	}

	void GCReg::registerToGC(GCReg &root)
	{
		next = root.next;
		prev = &root;
		if (next) next->prev = this;
		root.next = this;
		//	LS_LOG.debug("Registered object: %1") << (natural)this;
	}

	void GCReg::unregisterFromGC()
	{
		if (next) next->prev = prev;
		if (prev) prev->next = next;
		next = prev = 0;
		//		LS_LOG.debug("Unregistered object: %1") << (natural)this;
	}

	bool GCReg::isRegistered() const
	{
		return next != 0 || prev != 0;
	}

	void Object::clear()
	{
		JSON::PNode hold = this;
		fields.clear();
	}

	void Array::clear()
	{
		JSON::PNode hold = this;
		list.clear();
	}

	void GCRegRoot::clear()
	{

	}

	void GCRegRoot::clearAll()
	{
		GCReg *x = next;
		AutoArray<std::pair<Value, GCReg *>, SmallAlloc<256> > regs;
		while (x) {
			regs.add(std::make_pair(Value(dynamic_cast<JSON::INode *>(x)), x));
			x = x->next;
		}
		for (natural i = 0; i < regs.length(); i++) {
			regs[i].second->clear();
		}

	}

}