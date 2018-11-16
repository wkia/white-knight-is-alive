//
// Based on http://www.modernescpp.com/index.php/memory-and-performance-overhead-of-smart-pointer
//
#include <chrono>
#include <iostream>
#include <boost/make_shared.hpp>

constexpr long long Count = 100000000;
using T = int;

template <class F>
void runTest(const std::string title, F testFunc)
{
	auto start = std::chrono::system_clock::now();

	for (long long i = 0; i < Count; ++i)
	{
		testFunc(T(i));
	}

	std::chrono::duration<double> dur = std::chrono::system_clock::now() - start;
	std::cout << title << "<" << typeid(T).name() << ">: " << dur.count() << " s" << std::endl;
}

int main()
{
	std::cout << "Number of iterations: " << Count << std::endl << std::endl;

	runTest("Raw ptr            ", [](const T &i) { T* tmp(new T(i)); delete tmp; });
	runTest("std::shared_ptr    ", [](const T &i) { std::shared_ptr<T> tmp(new T(i)); });
	runTest("std::make_shared   ", [](const T &i) { auto tmp = std::make_shared<T>(i); });
	runTest("std::unique_ptr    ", [](const T &i) { std::unique_ptr<T> tmp(new T(i)); });
	runTest("std::make_unique   ", [](const T &i) { auto tmp = std::make_unique<T>(i); });
	runTest("boost::shared_ptr  ", [](const T &i) { boost::shared_ptr<T> tmp(new T(i)); });
	runTest("boost::make_shared ", [](const T &i) { auto tmp = boost::make_shared<T>(i); });
}
