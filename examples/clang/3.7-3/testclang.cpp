#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/GraphWriter.h"

namespace view
{
  typedef std::map<std::string, std::string> Classes; // Id-Name
  typedef std::multimap<std::string, std::string> Inheritance; // Id-BaseId

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
      Out << i.first;
      Out << " [ shape=\"box\", label=\"" << llvm::DOT::EscapeString(i.second);
      Out << "\"];\n";
    }
    for (auto i : inheritance)
    {
      Out << "  ";
      Out << llvm::DOT::EscapeString(i.first);
      Out << " -> ";
      Out << llvm::DOT::EscapeString(i.second);
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
    D->dump();
    return true;
  }

  bool VisitCXXRecordDecl (CXXRecordDecl *D)
  {
    const std::string qName = D->getQualifiedNameAsString();

    llvm::outs() << "CXXRecord: " << D->getKindName();
    if (D->getIdentifier())
    {
      llvm::outs() << ", id: " << D->getIdentifier()->getName();
      llvm::outs() << " " << D;
    }
    llvm::outs() << " | " << qName;
    llvm::outs() << "\n";

    view::classes[view::nameToId(qName)] = qName;

    // List base classes
    {
      if (D->getNumBases())
      {
        llvm::outs() << "- Bases (" << D->getNumBases() << ")\n";
        for (const auto &base : D->bases())
        {
          llvm::outs() << "- - ";
          const QualType type = base.getType();
          //const QualType canonType = D->getASTContext().getCanonicalType(type);
          const RecordType *recType = type->getAs<RecordType>();
          const CXXRecordDecl *cxxDecl = cast_or_null<CXXRecordDecl>(recType->getDecl()->getDefinition());
          assert(cxxDecl);

          const std::string qNameBase = cxxDecl->getQualifiedNameAsString();
          llvm::outs() << type.getAsString() << " | " << qNameBase;
          if (base.isVirtual())
          {
            llvm::outs() << " (virtual)";
          }
          llvm::outs() << "\n";

          view::inheritance.insert(view::Inheritance::value_type(view::nameToId(qName), view::nameToId(qNameBase)));
        }
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

int main (int argc, char *argv[])
{
  const bool ret = clang::tooling::runToolOnCode(new MyAction,
                                                 "struct MyStruct {};"
                                                 "class MyClass {};"
                                                 "namespace Nspace {"
                                                 "class MyDerived : public MyStruct {};"
                                                 "class MyDerived2 : public MyDerived, public virtual MyClass {};"
                                                 "}");

  view::generateDot();

  return ret ? 0 : -1;
}

