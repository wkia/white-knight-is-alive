#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

#include <Windows.h>

const size_t T = 10;
const size_t N = 3000000;
volatile uint64_t var = 0;

namespace
{
	class CriticalSection 
	{
		CRITICAL_SECTION cs;
	public:
		CriticalSection() { InitializeCriticalSection(&cs); }
		~CriticalSection() { DeleteCriticalSection(&cs); }
		void lock() { EnterCriticalSection(&cs); }
		void unlock() { LeaveCriticalSection(&cs); }
	};

	class SRWLock
	{
		SRWLOCK cs;
	public:
		SRWLock() { InitializeSRWLock(&cs); }
		void lock() { AcquireSRWLockExclusive(&cs); }
		void unlock() { ReleaseSRWLockExclusive(&cs); }
	};
}

template <class M>
void doLock(void *param)
{
	M &m = *static_cast<M*>(param);
	for (size_t n = 0; n < N; ++n)
	{
		m.lock();
		var += 1;
		m.unlock();
	}
}

template <class M>
void runTest(size_t threadCount)
{
	M m;
	std::vector<std::thread> thrs(threadCount);

	auto start = std::chrono::system_clock::now();
	
	for (auto &t : thrs) t = std::thread(doLock<M>, &m);
	for (auto &t : thrs) t.join();
	
	auto end = std::chrono::system_clock::now();

	const std::chrono::duration<double> diff = end - start;
	std::cout << ";" << diff.count();
}

int main()
{
	for (size_t n = 0; n < T; ++n)
	{
		std::cout << n;
		runTest<std::mutex>(n);
		runTest<CriticalSection>(n);
		runTest<SRWLock>(n);
		std::cout << std::endl;
	}
    return 0;
}

