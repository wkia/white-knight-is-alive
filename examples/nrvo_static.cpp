#include <iostream>
#include <vector>
#include <string>
#include <sstream>

std::vector<int> testNRVO(int value, size_t size, const std::vector<int> **localVec)
{
        std::vector<int> vec;
        if (0 == value)
        {
                vec = std::vector<int>(size, value);

                *localVec = &vec;
                return vec;
        }
        else
        {
                vec = std::vector<int>(size, value);

                *localVec = &vec;
                return vec;
        }
}

std::string testNRVOString(const char *str, const std::string **localVec)
{
        std::string vec;
        vec = std::string(str);
        *localVec = &vec;
        return vec;
}

std::string testNRVOStringStream(const char *str, const std::string **localVec)
{
        std::ostringstream os;
        os << str;
        std::string vec;
        vec = std::string(os.str());
        *localVec = &vec;
        return vec;
}

int main()
{
        {
                const std::vector<int> *localVec = 0;
                std::vector<int> vec = testNRVO(0, 10, &localVec);
                if (&vec == localVec)
                        std::cout << "NRVO was applied" << std::endl;
                else
                        std::cout << "NRVO was not applied" << std::endl;
        }
        {
                const std::vector<int> *localVec = 0;
                std::vector<int> vec = testNRVO(1, 10, &localVec);
                if (&vec == localVec)
                        std::cout << "NRVO was applied" << std::endl;
                else
                        std::cout << "NRVO was not applied" << std::endl;
        }
        {
                const std::string *localVec = 0;
                std::string str = testNRVOString("test nrvo", &localVec);
                if (&str == localVec)
                        std::cout << "NRVO was applied" << std::endl;
                else
                        std::cout << "NRVO was not applied" << std::endl;
        }
        {
                const std::string *localVec = 0;
                std::string str = testNRVOStringStream("test nrvo", &localVec);
                if (&str == localVec)
                        std::cout << "NRVO was applied" << std::endl;
                else
                        std::cout << "NRVO was not applied" << std::endl;
        }
}
