// Lock implementation and Auto lock implementation

#ifndef BASE_SYNCHRONIZATION_LOCK_H
#define BASE_SYNCHRONIZATION_LOCK_H

#include <windows.h>
#include <assert.h>

class NLock
{
public:
      typedef CRITICAL_SECTION OSLockType;

      NLock();
      ~NLock();

      // If the lock is not held, take it and return true.  If the lock is already
      // held by something else, immediately return false.
      bool Try();

      // Take the lock, blocking until it is available if necessary.
      void Lock();

      // Release the lock.  This must only be called by the lock's holder: after
      // a successful call to Try, or a call to Lock.
      void Unlock();

      // Return the native underlying lock.  Not supported for Windows builds.
#if !defined(OS_WIN)
      OSLockType* os_lock() { return &os_lock_; }
#endif

private:
      OSLockType os_lock_;
};

class NAutoLock
{
public:
	NAutoLock(NLock * lock)
	{
		assert(lock);
		lock_ = lock;
		lock_->Lock();
	}

	~NAutoLock()
	{
		if (lock_)
			lock_->Unlock();
	}

private:
	NLock *lock_;
};

class NAutoUnlock
{
public:
	NAutoUnlock(NLock * lock)
	{
		assert(lock);
		lock_ = lock;
		lock_->Unlock();
	}

	~NAutoUnlock()
	{
		if (lock_)
			lock_->Lock();
	}

private:
	NLock *lock_;
};


#endif  // BASE_SYNCHRONIZATION_LOCK_H
