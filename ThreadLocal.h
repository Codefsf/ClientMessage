#pragma once

struct ThreadLocal
{
	typedef unsigned long SlotType;

	static void AllocateSlot(SlotType &slot);
	static void FreeSlot(SlotType &slot);
	static void* GetValueFromSlot(SlotType &slot);
	static void SetValueInSlot(SlotType &slot, void *value);
};

template<typename Type>
class ThreadLocalPointer
{
public:

	ThreadLocalPointer() : slot_()
	{
		ThreadLocal::AllocateSlot(slot_);
	}

	~ThreadLocalPointer()
	{
		ThreadLocal::FreeSlot(slot_);
	}

	Type* Get()
	{
		return static_cast<Type*>(ThreadLocal::GetValueFromSlot(slot_));
	}

	void Set(Type *ptr)
	{
		ThreadLocal::SetValueInSlot(slot_, ptr);
	}

private:
	typedef ThreadLocal::SlotType SlotType;
	SlotType slot_;
};

class ThreadLocalBoolean
{
public:

	ThreadLocalBoolean() {}
	~ThreadLocalBoolean() {}

	bool Get()
	{
		return !!tlp_.Get();
	}

	void Set(bool val)
	{
		tlp_.Set(reinterpret_cast<void*>(val ? 1 : 0));
	}

private:
	ThreadLocalPointer<void> tlp_;
};

