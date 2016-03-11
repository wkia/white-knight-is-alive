#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/GraphWriter.h"

#include <fstream>
#include <iostream>
#include <sstream>

#include <getopt.h>

using namespace clang;

static bool g_dump = false;

///////////////////////////////////////////////////////////////////////////////

namespace view
{
  typedef std::map<std::string, std::string> Classes; // Id - Name
  typedef std::multimap<std::string, std::string> Inheritance; // Id - Base Id

  Classes classes;
  Inheritance inheritance;

  std::string nameToId (const std::string &name)
  {
    std::string id;
    for (auto i : name)
    {
      id.push_back((':' == i) ? '_' : i);
    }
    return id;
  }

  void generateDot ()
  {
    int FD;
    SmallString<128> Filename;
    if (std::error_code EC = llvm::sys::fs::createTemporaryFile("my", "dot", FD, Filename))
    {
      llvm::errs() << "Error: " << EC.message() << "\n";
      return;
    }

    llvm::errs() << "Writing '" << Filename << "'... ";
    llvm::raw_fd_ostream O(FD, true);

    raw_ostream& Out = O;
    Out << "digraph \"" << llvm::DOT::EscapeString("MyDiagram") << "\" {\n";
    for (auto i : classes)
    {
      Out << "  ";
      Out << nameToId(i.first);
      Out << " [ shape=\"box\", label=\"" << llvm::DOT::EscapeString(i.second);
      Out << "\"];\n";
    }
    for (auto i : inheritance)
    {
      Out << "  ";
      Out << llvm::DOT::EscapeString(nameToId(i.first));
      Out << " -> ";
      Out << llvm::DOT::EscapeString(nameToId(i.second));
      Out << ";\n";
    }
    Out << "}\n";

    llvm::errs() << " done. \n";
    O.close();
  }
}

///////////////////////////////////////////////////////////////////////////////

class MyVisitor : public RecursiveASTVisitor<MyVisitor>
{
public:

  bool VisitTranslationUnitDecl (clang::TranslationUnitDecl *D)
  {
    if (g_dump)
    {
      D->dump();
    }
    return true;
  }

  bool VisitCXXRecordDecl (CXXRecordDecl *D)
  {
    auto loc = D->getLocation();
    if (!loc.isValid() || !D->getASTContext().getSourceManager().isInMainFile(loc))
    {
      return true;
    }

    const std::string qName = D->getQualifiedNameAsString();

    llvm::outs() << "CXXRecord: " << D->getKindName();
    if (D->getIdentifier())
    {
      llvm::outs() << ", id: " << D->getIdentifier()->getName();
      llvm::outs() << " " << D;
    }
    llvm::outs() << " | " << qName;
    llvm::outs() << "\n";

    view::classes[qName] = qName;

    if (!D->getDefinition())
    {
      llvm::outs() << "  ...no definition...\n";
      return true;
    }

    // List base classes
    {
      if (D->getNumBases())
      {
        llvm::outs() << "- Bases (" << D->getNumBases() << ")\n";
        for (const auto &base : D->bases())
        {
          llvm::outs() << "- - ";
          const QualType type = base.getType();
          const RecordType *recType = type->getAs<RecordType>();
          if (!recType)
          {
            continue;
          }
          const CXXRecordDecl *cxxDecl = cast_or_null<CXXRecordDecl>(recType->getDecl()->getDefinition());
          assert(cxxDecl);

          const std::string qNameBase = cxxDecl->getQualifiedNameAsString();
          llvm::outs() << type.getAsString() << " | " << qNameBase;
          if (base.isVirtual())
          {
            llvm::outs() << " (virtual)";
          }
          llvm::outs() << "\n";

          view::inheritance.insert(view::Inheritance::value_type(qName, qNameBase));
          view::classes[qNameBase] = qNameBase; // update classes map
        }

        llvm::outs() << "- All bases\n";
        auto cb = [](const CXXRecordDecl *base, void *param) {
          const CXXRecordDecl *D = reinterpret_cast<CXXRecordDecl*>(param);
          llvm::outs() << "- - ";
          if (base->getIdentifier())
          {
            llvm::outs() << "id: " << base->getIdentifier()->getName();
            llvm::outs() << " " << base;
          }
          llvm::outs() << " | " << base->getQualifiedNameAsString();
          if (D->isVirtuallyDerivedFrom(base))
          {
            llvm::outs() << " (virtual)";
          }
          llvm::outs() << "\n";
          return true;
        };

        D->forallBases(cb, D);
      }

      llvm::outs() << "- Methods\n";
      for (auto i: D->methods())
      {
        llvm::outs() << "- - " << (*i).getQualifiedNameAsString() << "\n";
      }
    }
    llvm::outs() << "------------------------------------------\n";

    return true;
  }
};

///////////////////////////////////////////////////////////////////////////////

class MyConsumer : public clang::ASTConsumer
{
public:

  virtual void HandleTranslationUnit (clang::ASTContext &Context) override
  {
    _visitor.TraverseDecl(Context.getTranslationUnitDecl());
  }
private:
  MyVisitor _visitor;
};

///////////////////////////////////////////////////////////////////////////////

class MyAction : public clang::ASTFrontendAction
{
protected:
  typedef std::unique_ptr<clang::ASTConsumer> ASTConsumerPtr;

  virtual ASTConsumerPtr CreateASTConsumer (clang::CompilerInstance &Compiler, llvm::StringRef InFile) override
  {
    return ASTConsumerPtr(new MyConsumer);
  }
};

///////////////////////////////////////////////////////////////////////////////

std::string trim (const std::string& str)
{
  const size_t first = str.find_first_not_of(' ');
  const size_t last = str.find_last_not_of(' ');
  return str.substr(first, (last - first + 1));
}

std::vector<std::string> splitOpts (const std::string &s, const std::string &delim, std::vector<std::string> &elems)
{
  size_t start = 0;
  size_t pos;
  while (std::string::npos != start)
  {
    pos = s.find(delim, start + 1);
    elems.push_back(trim(s.substr(start, pos - start)));
    start = pos;
  }
  return elems;
}

///////////////////////////////////////////////////////////////////////////////

int main (int argc, char *argv[])
{
  using namespace std;

  string filename = "myfile.cpp";
  vector<string> arrOpts;
  string strCode =
      "struct MyStruct {};"
      "class MyClass {};"
      "namespace Nspace {"
      "class MyDerived : public MyStruct {};"
      "class MyDerived2 : public MyDerived, public virtual MyClass {};"
      "}";

  if (1 < argc)
  {
    const option long_options[] =
    {
      {"dump",    no_argument,       0, 'd'},
      {"options", required_argument, 0, 'o'},
      {0, 0, 0, 0}
    };

    int option_index = 0;
    int opt;
    while (-1 != (opt = getopt_long(argc, argv, "do:", long_options, &option_index)))
    {
      switch (opt)
      {
        case 'd':
          g_dump = true;
          cout << "TranslationUnit dump will be shown.\n";
          break;

        case 'o':
        {
          cout << "option 'options' with:\n";
          splitOpts(optarg, " -", arrOpts);
          for (auto i: arrOpts)
          {
            cout << "  " << i << "\n";
          }
          break;
        }
      }
    }

    while (optind < argc)
    {
      filename = string(argv[optind++]);
    }

    ifstream t(filename);
    strCode = string((istreambuf_iterator<char>(t)), istreambuf_iterator<char>());
  }

  const bool ret = clang::tooling::runToolOnCodeWithArgs(new MyAction, strCode, arrOpts, filename);

  if (ret)
  {
    view::generateDot();
  }

  return ret ? 0 : -1;
}

