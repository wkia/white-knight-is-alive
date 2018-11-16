//
// Based on http://www.modernescpp.com/index.php/memory-and-performance-overhead-of-smart-pointer
//
#include <chrono>
#include <iostream>
#include <boost/make_shared.hpp>

constexpr long long Count = 100000000;

struct S
{
	S(size_t i) : a(i), b(i + .1) { s.resize(i % 100, ' '); }
	int a;
	double b;
	std::string s;
};

using T = int;

template <class F>
void runTest(const std::string &title, F testFunc)
{
	auto start = std::chrono::system_clock::now();

	for (size_t i = 0; i < Count; ++i)
	{
		testFunc(i);
	}

	std::chrono::duration<double> dur = std::chrono::system_clock::now() - start;
	std::cout << title << "<" << typeid(T).name() << "> : " << dur.count() << " secs" << std::endl;
}

int main()
{
	std::cout << "Number of iterations: " << Count << std::endl << std::endl;

	runTest("Raw ptr new/delete ", [](const size_t &i) { T* tmp = new T(i); delete tmp; });
	runTest("std::shared_ptr    ", [](const size_t &i) { std::shared_ptr<T> tmp(new T(i)); });
	runTest("std::make_shared   ", [](const size_t &i) { auto tmp = std::make_shared<T>(i); });
	runTest("std::unique_ptr    ", [](const size_t &i) { std::unique_ptr<T> tmp(new T(i)); });
	runTest("std::make_unique   ", [](const size_t &i) { auto tmp = std::make_unique<T>(i); });
	runTest("boost::shared_ptr  ", [](const size_t &i) { boost::shared_ptr<T> tmp(new T(i)); });
	runTest("boost::make_shared ", [](const size_t &i) { auto tmp = boost::make_shared<T>(i); });
}
