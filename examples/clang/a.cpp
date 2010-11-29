struct MyAnotherClass
{
  unsigned long _a;
  mutable unsigned long _b;

  MyAnotherClass () : _a(4), _b(44) {}
  unsigned long func ()
  {
    unsigned long r = _a + _b;
    return r;
  }
};

