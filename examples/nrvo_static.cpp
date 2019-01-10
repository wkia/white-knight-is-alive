#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <tuple>

typedef std::vector<int> int_vector;
typedef int_vector Base;

class RVO : protected Base
{
public:
	using Base::push_back;
	RVO()
	{
		std::cout << this << " ctor\n";
	}

	~RVO()
	{
		std::cout << this << " dtor\n";
	}

	RVO(size_t size, Base::value_type value) : Base(size, value)
	{
		std::cout << this << " param ctor\n";
	}

	RVO(const RVO &r) : Base(r)
	{
		std::cout << this << " copy ctor\n";
	}

	RVO(RVO &&r) : Base(r)
	{
		std::cout << this << " move ctor\n";
	}

	RVO & operator=(const RVO &r)
	{
		std::cout << this << " operator=\n";
		if (&r != this)
		{
			*((Base *)this) = (const Base &)r;
		}
		return *this;
	}

	RVO & operator=(RVO &&r)
	{
		std::cout << this << " move operator=\n";
		if (&r != this)
		{
			*((Base *)this) = (Base&&)r;
		}
		return *this;
	}
};

RVO &ReturnSelfRef(RVO &r)
{
	return r;
}

RVO testRVO0(int value, size_t size)
{
	std::cout << __FUNCTION__ << std::endl;
	return RVO(size, value);
}

RVO testRVO1(int value, size_t size)
{
	std::cout << __FUNCTION__ << std::endl;
	if (0 == (value & 1))
	{
		return RVO(size * 2, value);
	}
	else
	{
		return RVO(size, value);
	}
}

RVO testNRVO0(int value, size_t size)
{
	std::cout << __FUNCTION__ << std::endl;
	RVO vec(size, value);
	//*localVec = &vec;
	return vec;
}

RVO testNRVO0_ref(int value, size_t size)
{
	std::cout << __FUNCTION__ << std::endl;
	RVO vec(size, value);
	//*localVec = &vec;
	return ReturnSelfRef(vec);
}

RVO testNRVO1(int value, size_t size)
{
	std::cout << __FUNCTION__ << std::endl;
	RVO vec;
	if (0 == value)
	{
		vec.push_back(value);
		//*localVec = &vec;
		return vec;
	}

	vec.push_back(value + 2);
	//*localVec = &vec;
	return vec;
}

RVO testNRVO2(int value, size_t size)
{
	std::cout << __FUNCTION__ << std::endl;
	RVO vec;
	//*localVec = &vec;
	if (0 == value)
	{
		vec.push_back(value);
	}
	else
	{
		vec.push_back(value + 2);
	}
	return vec;
}

RVO testNRVO3(int value, size_t size)
{
	std::cout << __FUNCTION__ << std::endl;
	if (0 == value)
	{
		RVO vec(size * 2, value);

		//*localVec = &vec;
		return vec;
	}
	else
	{
		RVO vec(size, value);

		//*localVec = &vec;
		return vec;
	}
}

std::tuple<RVO, int> testTuple(int value, size_t size)
{
	std::cout << __FUNCTION__ << std::endl;
	RVO vec(size, value);
	return std::make_tuple(vec, value);
}

std::tuple<RVO, int> testTupleWithMove(int value, size_t size)
{
	std::cout << __FUNCTION__ << std::endl;
	RVO vec(size, value);
	return std::make_tuple(std::move(vec), value);
}

int main()
{
	srand((unsigned int)time(NULL));
	const size_t size = (size_t)rand() % 100;
	const int value = rand();

	{
		std::cout << "\n=----\n";
		const void *localVec = 0;
		const RVO &vec = testRVO0(value, size);
		std::cout << "-----\n";
	}
	{
		std::cout << "\n=----\n";
		const void *localVec = 0;
		const RVO &vec = testRVO1(0, size);
		std::cout << "-----\n";
	}
	{
		std::cout << "\n=----\n";
		const void *localVec = 0;
		const RVO &vec = testRVO1(1, size);
		std::cout << "-----\n";
	}
	{
		std::cout << "\n=----\n";
		const void *localVec = 0;
		const RVO &vec = testNRVO0(value, size);
		std::cout << "-----\n";
	}
	{
		std::cout << "\n=----\n";
		const void *localVec = 0;
		const RVO &vec = testNRVO0_ref(0, size);
		std::cout << "-----\n";
	}
	{
		std::cout << "\n=----\n";
		const void *localVec = 0;
		const RVO &vec = testNRVO1(0, size);
		std::cout << "-----\n";
	}
	{
		std::cout << "\n=----\n";
		const void *localVec = 0;
		const RVO &vec = testNRVO1(1, size);
		std::cout << "-----\n";
	}
	{
		std::cout << "\n=----\n";
		const void *localVec = 0;
		const RVO &vec = testNRVO2(0, size);
		std::cout << "-----\n";
	}
	{
		std::cout << "\n=----\n";
		const void *localVec = 0;
		const RVO &vec = testNRVO2(1, size);
		std::cout << "-----\n";
	}
	{
		std::cout << "\n=----\n";
		const void *localVec = 0;
		const RVO &vec = testNRVO3(0, size);
		std::cout << "-----\n";
	}
	{
		std::cout << "\n=----\n";
		const void *localVec = 0;
		const RVO &vec = testNRVO3(1, size);
		std::cout << "-----\n";
	}
	{
		std::cout << "\n=----\n";
		const void *localVec = 0;
		auto res = testTuple(1, size);
		const RVO &vec = std::get<RVO>(res);
		std::cout << "-----\n";
	}
	{
		std::cout << "\n=----\n";
		const void *localVec = 0;
		auto res = testTupleWithMove(1, size);
		const RVO &vec = std::get<0>(res);
		std::cout << "-----\n";
	}
	{
		std::cout << "\n=---- using C++14 std::tie\n";
		const void *localVec = 0;
		RVO vec;
		std::tie(vec, std::ignore) = testTupleWithMove(1, size);
		std::cout << "-----\n";
	}
	{
		std::cout << "\n=---- using C++17 structured binding\n";
		const void *localVec = 0;
		auto [vec, val] = testTuple(1, size);
		std::cout << "-----\n";
	}
}
