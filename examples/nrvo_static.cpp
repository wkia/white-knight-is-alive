#include <iostream>
#include <vector>
#include <string>
#include <sstream>

typedef std::vector<int> int_vector;
typedef int_vector Base;

class RVO : protected Base
{
    public:
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
    
    RVO(const RVO &r)
    {
        std::cout << this << " copy ctor\n";
        *((Base *)this) = (Base)r;
    }
    RVO & operator=(const RVO &r)
    {
        std::cout << this << " operator=\n";
        if (&r != this)
        {
            *((Base *)this) = (Base)r;
        }
        return *this;
    }
    
};


RVO testRVO0(int value, size_t size, const void **localVec)
{
        std::cout << __FUNCTION__ << std::endl;
        return RVO(size, value);
}


RVO testNRVO0(int value, size_t size, const void **localVec)
{
        std::cout << __FUNCTION__ << std::endl;
        RVO vec(size, value);
        *localVec = &vec;
        return vec;
}


RVO testNRVO1(int value, size_t size, const void **localVec)
{
        std::cout << __FUNCTION__ << std::endl;
        RVO vec;
        if (0 == value)
        {
                vec = RVO(size * 2, value);

                *localVec = &vec;
                return vec;
        }
        else
        {
                vec = RVO(size, value);

                *localVec = &vec;
                return vec;
        }
}


RVO testNRVO2(int value, size_t size, const void **localVec)
{
        std::cout << __FUNCTION__ << std::endl;
        RVO vec;
        if (0 == value)
        {
                vec = RVO(size * 2, value);

                *localVec = &vec;
        }
        else
        {
                vec = RVO(size, value);

                *localVec = &vec;
        }
        return vec;
}


int main()
{
        {
                std::cout << "=----" << std::endl;
                const void *localVec = 0;
                RVO vec = testRVO0(0, 10, &localVec);
                std::cout << "-----" << std::endl;
        }
        {
                std::cout << "=----" << std::endl;
                const void *localVec = 0;
                RVO vec = testNRVO0(0, 10, &localVec);
                if (&vec == localVec)
                        std::cout << "NRVO was applied" << std::endl;
                else
                        std::cout << "NRVO was not applied" << std::endl;
                std::cout << "-----" << std::endl;
        }
        {
                std::cout << "=----" << std::endl;
                const void *localVec = 0;
                RVO vec = testNRVO1(0, 10, &localVec);
                if (&vec == localVec)
                        std::cout << "NRVO was applied" << std::endl;
                else
                        std::cout << "NRVO was not applied" << std::endl;
                std::cout << "-----" << std::endl;
        }
        {
                std::cout << "=----" << std::endl;
                const void *localVec = 0;
                RVO vec = testNRVO1(1, 10, &localVec);
                if (&vec == localVec)
                        std::cout << "NRVO was applied" << std::endl;
                else
                        std::cout << "NRVO was not applied" << std::endl;
                std::cout << "-----" << std::endl;
        }
        {
                std::cout << "=----" << std::endl;
                const void *localVec = 0;
                RVO vec = testNRVO2(0, 10, &localVec);
                if (&vec == localVec)
                        std::cout << "NRVO was applied" << std::endl;
                else
                        std::cout << "NRVO was not applied" << std::endl;
                std::cout << "-----" << std::endl;
        }
        {
                std::cout << "=----" << std::endl;
                const void *localVec = 0;
                RVO vec = testNRVO2(1, 10, &localVec);
                if (&vec == localVec)
                        std::cout << "NRVO was applied" << std::endl;
                else
                        std::cout << "NRVO was not applied" << std::endl;
                std::cout << "-----" << std::endl;
        }
}
