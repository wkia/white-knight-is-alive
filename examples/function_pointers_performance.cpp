#include <chrono>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <string>

constexpr long long Count = 300000000;
volatile int g_v;

template <class F>
void runTest(std::string title, F testFunc)
{
	auto start = std::chrono::system_clock::now();

	for (size_t i = 0; i < Count; ++i)
	{
		testFunc(rand());
	}

	std::chrono::duration<double> dur = std::chrono::system_clock::now() - start;
	std::cout << title << ": <" << typeid(F).name() << "> : " << dur.count() << " secs" << std::endl;
}

void test(const size_t &i)
{
	g_v = i + rand();
}

int main()
{
	srand(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count());
	std::cout << "Number of iterations: " << Count << std::endl << std::endl;

	using TestFunc = std::function<void(const size_t &)>;
	auto &v = g_v;

	runTest("Function pointer", test);
	{
		runTest("Lambda inlined impl", [&v](const size_t &i) { v = i + rand(); });
		auto f2 = [&v](const size_t &i) { v = i + rand(); };
		runTest("Lambda inlined impl (2)", f2);
		TestFunc f3 = [&v](const size_t &i) { v = i + rand(); };
		runTest("Lambda inlined impl (3)", f3);
	}
	{
		runTest("Lambda function call", [&v](const size_t &i) { test(i); });
		auto f2 = [&v](const size_t &i) { test(i); };
		runTest("Lambda function call (2)", f2);
		TestFunc f3 = [&v](const size_t &i) { test(i); };
		runTest("Lambda function impl (3)", f3);
	}
	{
		runTest("std::function", TestFunc(test));
		auto f = TestFunc(test);
		runTest("std::function (2)", f);
	}
}
