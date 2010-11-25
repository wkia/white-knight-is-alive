#include <cstdlib>
#include <iostream>

using std::cout;
using std::endl;

const size_t MaxNum = 10000000;
float Percent = 0;
float Divisor = 0;

bool func ()
#ifdef _USE_EXCEPTIONS
throw (int)
#else
throw ()
#endif
{
  const int x = rand();
  if (x > (RAND_MAX - RAND_MAX/Divisor))
  {
#ifdef _USE_EXCEPTIONS
    throw 1;
#else
    return false;
#endif
  }

  return true;
}

int main (int argc, char **argv)
{
  Percent = atof(argv[1]);
  Divisor = 100 / Percent;

  srand(1);

  size_t trueCounter = 0;
  size_t falseCounter = 0;

  size_t n = MaxNum;
  while (n--)
  {
    try
    {
      const bool r = func();
#ifndef _USE_EXCEPTIONS
      if (r)
#endif
      {
        trueCounter += 1;
      }
#ifndef _USE_EXCEPTIONS
      else
      {
        falseCounter += 1;
      }
#endif
    }
    catch (int)
    {
      falseCounter += 1;
    }
  }

  cout << "false: " << falseCounter << endl;
  cout << "true:  " << trueCounter << endl;
  cout << "percentage ~" << float(falseCounter) / MaxNum * 100 << endl;
  return 0;
}

