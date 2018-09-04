#include <chrono>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include <Windows.h>

const size_t T = 10;
const size_t N = 3000000;
volatile uint64_t var = 0;

const std::string sep = ";";

namespace WinApi
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
		SRWLOCK srw;
	public:
		SRWLock() { InitializeSRWLock(&srw); }
		void lock() { AcquireSRWLockExclusive(&srw); }
		void unlock() { ReleaseSRWLockExclusive(&srw); }
	};

	class Mutex
	{
		HANDLE h;
	public:
		Mutex():h(CreateMutex(NULL, FALSE, NULL)) { }
		~Mutex() { CloseHandle(h); }
		void lock() { WaitForSingleObject(h, INFINITE); }
		void unlock() { ReleaseMutex(h); }
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

	const auto start = std::chrono::system_clock::now();

	for (auto &t : thrs) t = std::thread(doLock<M>, &m);
	for (auto &t : thrs) t.join();

	const auto end = std::chrono::system_clock::now();

	const std::chrono::duration<double> diff = end - start;
	std::cout << sep << diff.count();
}

template <class ...Args>
void runTests(size_t threadMax)
{
	//const std::size_t value = sizeof...(Args);
	//std::cout << value << std::endl;
	std::cout << "Threads Count" << sep;
	{
		int dummy[] = { (std::cout << typeid(Args).name() << sep, 0)... };
		(void)dummy;
	}
	std::cout << std::endl;

	for (size_t n = 1; n <= threadMax; ++n)
	{
		std::cout << n;
		{
			int dummy[] = { (runTest<Args>(n), 0)... };
			(void)dummy;
		}
		std::cout << std::endl;
	}
}

int main()
{
	runTests<std::mutex, WinApi::CriticalSection, WinApi::SRWLock>(T);
	return 0;
}
