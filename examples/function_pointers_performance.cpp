#include <chrono>
#include <iostream>
#include <cstdlib>
#include <functional>

constexpr long long Count = 300000000;
volatile int g_v;

template <class F>
void runTest(F testFunc)
{
	auto start = std::chrono::system_clock::now();

	for (size_t i = 0; i < Count; ++i)
	{
		testFunc(rand());
	}

	std::chrono::duration<double> dur = std::chrono::system_clock::now() - start;
	std::cout << "<" << typeid(F).name() << "> : " << dur.count() << " secs" << std::endl;
}

void test(const size_t &i)
{
	g_v = i + rand();
}

int main()
{
	srand(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count());
	std::cout << "Number of iterations: " << Count << std::endl << std::endl;

	auto &v = g_v;
	runTest(test);

	runTest([&v](const size_t &i) { v = i + rand(); });
	auto lm = [&v](const size_t &i) { v = i + rand(); };
	runTest(lm);

	runTest([&v](const size_t &i) { test(i); });
	auto lm2 = [&v](const size_t &i) { test(i); };
	runTest(lm2);

	runTest(std::function<void(const size_t &)>(test));
	auto fn = std::function<void(const size_t &)>(test);
	runTest(fn);
}
