#include "clang/Tooling/Tooling.h"

int main ()
{
  auto astUnit = clang::tooling::buildASTFromCode("struct C {};");
  return 0;
}

