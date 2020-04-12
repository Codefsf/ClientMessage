#include "ThreadLocal.h"
#include <Windows.h>
#include <assert.h>

void ThreadLocal::AllocateSlot(SlotType &slot)
{
	slot = ::TlsAlloc();
	assert(slot != TLS_OUT_OF_INDEXES);
}

void ThreadLocal::FreeSlot(SlotType &slot)
{
	if (!::TlsFree(slot))
	{
		assert(false);
	}
}

void* ThreadLocal::GetValueFromSlot(SlotType &slot)
{
	return ::TlsGetValue(slot);
}

void ThreadLocal::SetValueInSlot(SlotType &slot, void *value)
{
	if (!::TlsSetValue(slot, value))
	{
		assert(false);
	}
}
