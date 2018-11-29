#include <algorithm>
#include <chrono>
#include <deque>
#include <iomanip>
#include <iostream>
#include <list>
#include <set>
#include <vector>
#include <boost/container/flat_set.hpp>
#include <memory>

constexpr long long Count = 10000000;
constexpr long long Step = 2;
constexpr int ColumnWidth = 14;

template <class T>
class std_list : public std::list<T>
{
public:
	using std::list<T>::iterator;
	iterator find(const T &v) { return std::find(this->begin(), this->end(), v); }
};

struct Struct
{
	explicit Struct(size_t i) : a(i), b(i + .1) { s.resize(i % 1000, ' '); }
	int a;
	double b;
	std::string s;

	bool operator<(const Struct &r) const { return a < r.a; }
	bool operator==(const Struct &r) const { return a == r.a && b == r.b && s == r.s; }
};

template <typename T> T make_value() { return T(rand()); }
template <> std::unique_ptr<Struct> make_value() { return std::make_unique<Struct>(rand()); }
template <> std::shared_ptr<Struct> make_value() { return std::make_shared<Struct>(rand()); }

template <typename T> T clone(const T &v) { return v; }
template <> std::unique_ptr<Struct> clone(const std::unique_ptr<Struct> &v) { return std::make_unique<Struct>(v->a); }

template <class C>
double searchLowerBound(C &c)
{
	std::vector<C::value_type> cc;
	std::transform(std::begin(c), std::end(c), std::back_inserter(cc), [](auto &e) { return clone(e); });

	double t = 0;
	const auto N = c.size();
	const auto start = std::chrono::system_clock::now();
	for (const auto &e : cc)
	{
		auto lower = std::lower_bound(c.begin(), c.end(), e);
	}
	t += std::chrono::duration<double>(std::chrono::system_clock::now() - start).count();
	return t / N;
}

template <class C>
double searchFind(C &c)
{
	std::vector<C::value_type> cc;
	std::transform(std::begin(c), std::end(c), std::back_inserter(cc), [](auto &e) { return clone(e); });

	double t = 0;
	const auto N = c.size();
	const auto start = std::chrono::system_clock::now();
	for (const auto &e : cc)
	{
		auto lower = c.find(e);
	}
	t += std::chrono::duration<double>(std::chrono::system_clock::now() - start).count();
	return t / N;
}

template <class C>
double eraseMiddle(C &c)
{
	double t = 0;
	const auto N = c.size();
	while (!c.empty())
	{
		auto it = c.begin();
		std::advance(it, c.size() / 2);
		auto it2 = it;
		std::advance(it2, 1);
		const auto start = std::chrono::system_clock::now();
		c.erase(it, it2);
		t += std::chrono::duration<double>(std::chrono::system_clock::now() - start).count();
	}
	return t / N;
}

void printResults(const std::vector<double> &times, const std::string &typeName)
{
	auto str = typeName;

	const std::vector<std::string> ext = { "class ", "struct "};
	for (const auto &e : ext)
	{
		auto pos = str.find(e);
		while (std::string::npos != pos)
		{
			str.erase(pos, e.size());
			pos = str.find(e);
		}
	}

	std::cout
		<< std::fixed << std::setprecision(6) << std::setw(ColumnWidth) << std::right
		<< times[0]
		<< std::fixed << std::setprecision(9) << std::setw(ColumnWidth) << std::right
		<< times[1]
		<< std::fixed << std::setprecision(9) << std::setw(ColumnWidth) << std::right
		<< times[2]
		<< " : "
		<< str
		<< std::endl;
}

template <class C>
void runTestVector(C &c, long long n)
{
	const bool isPrepared = c.capacity() >= n;

	std::vector<double> times;
	{
		const auto start = std::chrono::system_clock::now();
		while (n--)
		{
			c.push_back(make_value<C::value_type>());
		}
		std::sort(c.begin(), c.end());
		times.push_back(std::chrono::duration<double>(std::chrono::system_clock::now() - start).count());
	}
	{
		const auto t = searchLowerBound(c);
		times.push_back(t);
	}
	{
		const auto t = eraseMiddle(c);
		times.push_back(t);
	}

	printResults(times, std::string(isPrepared ? "RESERVED " : "") + typeid(C).name());
}

template <class C>
void runTestDeque(C &c, long long n)
{
	std::vector<double> times;
	{
		const auto start = std::chrono::system_clock::now();
		while (n--)
		{
			c.push_back(make_value<C::value_type>());
		}
		std::sort(c.begin(), c.end());
		times.push_back(std::chrono::duration<double>(std::chrono::system_clock::now() - start).count());
	}
	{
		const auto t = searchLowerBound(c);
		times.push_back(t);
	}
	{
		const auto t = eraseMiddle(c);
		times.push_back(t);
	}

	printResults(times, typeid(C).name());
}

template <class C>
void runTestList(C &c, long long n)
{
	std::vector<double> times;
	{
		const auto start = std::chrono::system_clock::now();
		while (n--)
		{
			c.push_back(make_value<C::value_type>());
		}
		c.sort();
		times.push_back(std::chrono::duration<double>(std::chrono::system_clock::now() - start).count());
	}
	{
		const auto t = searchFind(c);
		times.push_back(t);
	}
	{
		const auto t = eraseMiddle(c);
		times.push_back(t);
	}

	printResults(times, typeid(C).name());
}

template <class C>
void runTestSet(C &c, long long n)
{
	std::vector<double> times;
	{
		const auto start = std::chrono::system_clock::now();
		while (n--)
		{
			c.insert(make_value<C::value_type>());
		}
		times.push_back(std::chrono::duration<double>(std::chrono::system_clock::now() - start).count());
	}
	{
		const auto t = searchFind(c);
		times.push_back(t);
	}
	{
		const auto t = eraseMiddle(c);
		times.push_back(t);
	}

	printResults(times, typeid(C).name());
}

template <class T>
void runTest(long long n)
{
	const auto t = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();

	{
		srand(int(t));
		std::vector<T> v;
		runTestVector(v, n);
	}
	{
		srand(int(t));
		std::vector<T> v;
		v.reserve(decltype(v)::size_type(n));
		runTestVector(v, n);
	}
	{
		srand(int(t));
		std::deque<T> d;
		runTestDeque(d, n);
	}
	{
		srand(int(t));
		std::set<T> s;
		runTestSet(s, n);
	}
	{
		srand(int(t));
		boost::container::flat_set<T> s;
		runTestSet(s, n);
	}
	//{
	//	srand(int(t));
	//	std_list<T> s;
	//	runTestList(s, n);
	//}
	std::cout << std::endl;
}

int main()
{
	try
	{
		for (long long n = 128; n < Count; n *= Step)
		{
			std::cout << "--- Number of iterations: " << n << std::endl;
			std::cout
				<< std::setw(ColumnWidth) << std::right << "fill & sort"
				<< std::setw(ColumnWidth) << std::right << "search (avg)"
				<< std::setw(ColumnWidth) << std::right << "erase (avg)"
				<< std::endl;
			runTest<int>(n);
			runTest<Struct>(n);
			runTest<std::shared_ptr<Struct>>(n);
			runTest<std::unique_ptr<Struct>>(n);
		}
	}
	catch (const std::exception &e)
	{
		std::cout << "ERROR: " << e.what() << std::endl;
	}
	catch (...)
	{
		std::cout << "ERROR: unknown exception\n";
	}
}
