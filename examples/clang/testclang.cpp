#include <clang/AST/ASTConsumer.h>
#include <clang/AST/ASTContext.h>
#include <clang/AST/DeclVisitor.h>
#include <clang/Basic/Builtins.h>
#include <clang/Basic/Diagnostic.h>
#include <clang/Basic/FileManager.h>
#include <clang/Basic/IdentifierTable.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Basic/TargetInfo.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Frontend/TextDiagnosticBuffer.h>
#include <clang/Lex/HeaderSearch.h>
#include <clang/Lex/Preprocessor.h>
#include <clang/Parse/ParseAST.h>
#include <clang/Parse/Parser.h>
#include <clang/Sema/Sema.h>
#include <llvm/LLVMContext.h>
#include <llvm/Support/Casting.h>

#include <gtest/gtest.h>

#include <string>

using std::auto_ptr;

///////////////////////////////////////////////////////////////////////////////

struct AstInfo
{
  AstInfo () :
    _classCount(0),
    _fieldCount(0),
    _methodCount(0),
    _constructorCount(0)
  {
  }

  size_t _classCount;
  size_t _fieldCount;
  size_t _methodCount;
  size_t _constructorCount;
};

///////////////////////////////////////////////////////////////////////////////

class MyDeclVisitor : public clang::DeclVisitor<MyDeclVisitor>
{
  llvm::raw_ostream &_out;
  unsigned int _indent;
  AstInfo &_astinfo;

public:
  MyDeclVisitor (llvm::raw_ostream &out, AstInfo &astinfo) :
    _out(out),
    _indent(0),
    _astinfo(astinfo)
  {
  }

  void VisitDeclContext (clang::DeclContext *DC)
  {
    clang::DeclContext::decl_iterator DBegin = DC->decls_begin();
    clang::DeclContext::decl_iterator DEnd = DC->decls_end();

    for (clang::DeclContext::decl_iterator i = DBegin; i != DEnd; ++i)
    {
      // Skip over implicit declarations
      if ((*i)->isImplicit())
      {
        continue;
      }

      // FIXME: The following was copied and pasted from clang sources.
      // Ugly hack so we don't pretty-print the builtin declaration of
      // __builtin_va_list or __[u]int128_t.  There should be some other way to
      // check that.
      if (clang::NamedDecl *ND = llvm::dyn_cast<clang::NamedDecl>(*i))
      {
        if (clang::IdentifierInfo *II = ND->getIdentifier())
        {
          if (II->isStr("__builtin_va_list")
              || II->isStr("__int128_t")
              || II->isStr("__uint128_t"))
          {
            continue;
          }
        }
      }

      _out.indent(_indent);
      _out.changeColor(llvm::raw_ostream::GREEN);
      _out << __FUNCTION__ << ": " << (*i)->getDeclKindName() << "\n";
      _out.resetColor();

      _indent += 2;
      Visit(*i);
      _indent -= 2;
    }

  }

  void VisitTranslationUnitDecl (clang::TranslationUnitDecl *D)
  {
    _out.indent(_indent);
    _out << D->Decl::getDeclKindName() << "\n";
    _indent += 2;
    VisitDeclContext(D);
    _indent -= 2;
  }

  void VisitTypedefDecl (clang::TypedefDecl *D)
  {
    //_out.indent(_indent);
    //_out << "typedef " << D->getUnderlyingType().getAsString() << " "
    //    <<  D->getNameAsString() << "\n";
  }

  void VisitCXXRecordDecl (clang::CXXRecordDecl *D)
  {
    _astinfo._classCount += 1;

    _out.indent(_indent);
    _out << "CXXRecord: ";
    _out << D->getKindName();

    if (D->getIdentifier())
    {
      _out << ' ' << D;
    }

    _out << "\n";

    _indent += 2;
    VisitDeclContext(D);
    _indent -= 2;
  }

  void VisitFieldDecl (clang::FieldDecl *D)
  {
    _astinfo._fieldCount += 1;

    //_out.indent(_indent);
    //if (D->isMutable())
    //{
    //  _out << "mutable ";
    //}

    //_out << D->getType().getAsString() << " " << D->getNameAsString();

    //if (D->isBitField()) {
    //  _out << " : ";
    //  D->getBitWidth()->printPretty(_out, Context, 0, Policy, Indentation);
    //}

    //_out << "\n";
  }

  void VisitCXXConstructorDecl (clang::CXXConstructorDecl *D)
  {
    _astinfo._constructorCount += 1;
  }

  void VisitCXXMethodDecl (clang::CXXMethodDecl *D)
  {
    _astinfo._methodCount += 1;
  }
};

///////////////////////////////////////////////////////////////////////////////

class MyAstConsumer : public clang::ASTConsumer
{
public:
  AstInfo _astinfo;

  virtual void HandleTranslationUnit(clang::ASTContext &ctx)
  {
    llvm::raw_ostream &out = llvm::errs();
    clang::TranslationUnitDecl *translationUnit = ctx.getTranslationUnitDecl();

    MyDeclVisitor(out, _astinfo).Visit(translationUnit);
  }
};

///////////////////////////////////////////////////////////////////////////////

class TestClang : public ::testing::Test
{
protected:
  auto_ptr<clang::Diagnostic> diagnostic;
  clang::TextDiagnosticBuffer *diagnosticBuffer;

  void SetUp ()
  {
    diagnosticBuffer = new clang::TextDiagnosticBuffer;
    diagnostic.reset(new clang::Diagnostic(diagnosticBuffer));
  }

  void TearDown ()
  {
    diagnostic.release();
  }

#define EXPECT_DIAGNOSTIC_IS_CLEAN \
  { \
    EXPECT_FALSE(diagnostic->hasErrorOccurred()); \
    EXPECT_FALSE(diagnostic->hasFatalErrorOccurred()); \
    EXPECT_EQ(0, diagnostic->getNumErrors()); \
    EXPECT_EQ(0, diagnostic->getNumErrorsSuppressed()); \
    EXPECT_EQ(0, diagnostic->getNumWarnings()); \
    \
    clang::TextDiagnosticBuffer::const_iterator i; \
    for (i = diagnosticBuffer->err_begin(); \
         i != diagnosticBuffer->err_end(); \
         ++i) \
    { \
      const std::string &errorMessage = (*i).second; \
      EXPECT_EQ("", errorMessage);\
    } \
  }
};

///////////////////////////////////////////////////////////////////////////////

TEST_F(TestClang, testCommandLineArguments)
{
  clang::CompilerInstance compiler;

  //compiler.createDiagnostics(0, NULL); // to stdout
  compiler.setDiagnostics(diagnostic.get());
  ASSERT_TRUE(compiler.hasDiagnostics());

  const char *args[] =
  {
    "--version",
    "--help",
    "--my_unknown_arg"
  };

  clang::CompilerInvocation::CreateFromArgs(
    compiler.getInvocation(), args, args + 3, compiler.getDiagnostics());

  EXPECT_TRUE(compiler.getFrontendOpts().ShowHelp);
  EXPECT_TRUE(compiler.getFrontendOpts().ShowVersion);

  EXPECT_EQ(1, diagnostic->getNumErrors());

  const std::string &errorMessage = (*diagnosticBuffer->err_begin()).second;
  EXPECT_EQ("unknown argument: '--my_unknown_arg'", errorMessage);
}

///////////////////////////////////////////////////////////////////////////////

TEST_F(TestClang, testAstPrintXMLAction)
{
  using clang::FrontendAction;
  using clang::ASTPrintXMLAction;

  clang::CompilerInstance compiler;
  ASSERT_FALSE(compiler.hasDiagnostics());

  //compiler.createDiagnostics(0, NULL); // to stdout
  compiler.setDiagnostics(diagnostic.get());
  ASSERT_TRUE(compiler.hasDiagnostics());

  const char *args[] =
  {
    "-cc1",
    "a.cpp"
  };

  clang::CompilerInvocation::CreateFromArgs(
    compiler.getInvocation(), args, args + 2, compiler.getDiagnostics());
  EXPECT_EQ(0, diagnostic->getNumErrors());

  auto_ptr<FrontendAction> action(new ASTPrintXMLAction);
  ASSERT_TRUE(0 != action.get());

  bool actionSuccessful = compiler.ExecuteAction(*action);
  ASSERT_TRUE(actionSuccessful);

  EXPECT_DIAGNOSTIC_IS_CLEAN;
}

///////////////////////////////////////////////////////////////////////////////

TEST_F(TestClang, testAstPrintXMLActionManual)
{
  using clang::ASTPrintXMLAction;
  using clang::FileEntry;
  using clang::FileManager;
  using clang::FrontendAction;
  using clang::SourceManager;

  clang::CompilerInstance compiler;
  ASSERT_FALSE(compiler.hasDiagnostics());

  //compiler.createDiagnostics(0, NULL); // to stdout
  compiler.setDiagnostics(diagnostic.get());
  ASSERT_TRUE(compiler.hasDiagnostics());

  compiler.getTargetOpts().Triple = "x86_64"; // x86, alpha, ppc, ppc64, ...
  compiler.getLangOpts().CPlusPlus = 1;
  //compiler.getHeaderSearchOpts().Verbose = true;

  std::pair<clang::InputKind, std::string> input(
    clang::IK_CXX, "a.cpp");
  compiler.getFrontendOpts().Inputs.push_back(input);

  compiler.getFrontendOpts().OutputFile = "./a.xml";

  auto_ptr<FrontendAction> action(new ASTPrintXMLAction);
  bool actionSuccessful = compiler.ExecuteAction(*action);

  EXPECT_DIAGNOSTIC_IS_CLEAN;
  ASSERT_TRUE(actionSuccessful);
}

///////////////////////////////////////////////////////////////////////////////

TEST_F(TestClang, testBareMinimum)
{
  using clang::FileManager;
  using clang::HeaderSearch;
  using clang::LangOptions;
  using clang::Preprocessor;
  using clang::SourceManager;
  using clang::TargetInfo;
  using clang::TargetOptions;

  LangOptions langOptions;

  TargetOptions targetOptions;
  targetOptions.Triple = "x86_64";
  auto_ptr<TargetInfo> targetInfo(TargetInfo::CreateTargetInfo(*diagnostic,
                                                               targetOptions));
  EXPECT_TRUE(0 != targetInfo.get());
  EXPECT_DIAGNOSTIC_IS_CLEAN;

  SourceManager sourceManager(*diagnostic);
  EXPECT_DIAGNOSTIC_IS_CLEAN;

  FileManager fileManager;
  HeaderSearch headerSearch(fileManager);

  Preprocessor preprocessor(*diagnostic,
                            langOptions,
                            *targetInfo,
                            sourceManager,
                            headerSearch);
  EXPECT_DIAGNOSTIC_IS_CLEAN;
}

///////////////////////////////////////////////////////////////////////////////

TEST_F(TestClang, testParseAstFunction)
{
  using clang::ASTContext;
  using clang::FileEntry;
  using clang::FileManager;
  using clang::HeaderSearch;
  using clang::IdentifierTable;
  using clang::LangOptions;
  using clang::Preprocessor;
  using clang::SelectorTable;
  using clang::SourceManager;
  using clang::TargetInfo;
  using clang::TargetOptions;

  // Create objects needed to create a preprocessor.
  LangOptions langOptions;
  langOptions.CPlusPlus = 1; // We will parse a C++ file.

  TargetOptions targetOptions;
  targetOptions.Triple = "x86_64"; // x86, ppc, ...
  auto_ptr<TargetInfo> targetInfo(
    TargetInfo::CreateTargetInfo(*diagnostic, targetOptions));
  EXPECT_DIAGNOSTIC_IS_CLEAN;
  ASSERT_TRUE(0 != targetInfo.get());

  SourceManager sourceManager(*diagnostic);
  FileManager fileManager;
  HeaderSearch headerSearch(fileManager);

  // Create a preprocessor.
  Preprocessor preprocessor(*diagnostic,
                            langOptions,
                            *targetInfo,
                            sourceManager,
                            headerSearch);
  EXPECT_DIAGNOSTIC_IS_CLEAN;

  // Create an ASTContext.
  IdentifierTable identifierTable(langOptions);
  SelectorTable selectorTable;
  clang::Builtin::Context builtinContext(*targetInfo);
  const unsigned int NumReserved = 10;

  ASTContext astContext(langOptions,
                        sourceManager,
                        *targetInfo,
                        identifierTable,
                        selectorTable,
                        builtinContext,
                        NumReserved);

  // Find a source file and add the source file to the source manager.
  const FileEntry* file = fileManager.getFile("a.cpp");
  ASSERT_TRUE(0 != file);
  sourceManager.createMainFileID(file);

  // Create the AST consumer.
  MyAstConsumer consumer;

  // Parse the tree.
  clang::ParseAST(preprocessor, &consumer, astContext);
  EXPECT_DIAGNOSTIC_IS_CLEAN;

  // Check AST stats.
  EXPECT_EQ(1, consumer._astinfo._classCount);
  EXPECT_EQ(2, consumer._astinfo._fieldCount);
  EXPECT_EQ(1, consumer._astinfo._methodCount);
  EXPECT_EQ(1, consumer._astinfo._constructorCount);
}

///////////////////////////////////////////////////////////////////////////////

TEST_F(TestClang, testBareParser)
{
  using clang::ASTContext;
  using clang::ASTConsumer;
  using clang::FileEntry;
  using clang::FileManager;
  using clang::HeaderSearch;
  using clang::IdentifierTable;
  using clang::LangOptions;
  using clang::Parser;
  using clang::Preprocessor;
  using clang::SelectorTable;
  using clang::Sema;
  using clang::SourceManager;
  using clang::TargetInfo;
  using clang::TargetOptions;

  // Create C++ language options.
  LangOptions langOptions;
  langOptions.CPlusPlus = 1;

  // Create target options.
  // There is no default target, we need to set a target manually.
  TargetOptions targetOptions;
  targetOptions.Triple = "x86_64";
  auto_ptr<TargetInfo> targetInfo(TargetInfo::CreateTargetInfo(*diagnostic,
                                                               targetOptions));
  EXPECT_TRUE(0 != targetInfo.get());
  EXPECT_DIAGNOSTIC_IS_CLEAN;

  // Create a source manager.
  SourceManager sourceManager(*diagnostic);
  EXPECT_DIAGNOSTIC_IS_CLEAN;

  // Create a file manager.
  FileManager fileManager;

  // Create a header searcher.
  HeaderSearch headerSearch(fileManager);

  // Create a preprocessor.
  Preprocessor preprocessor(*diagnostic,
                            langOptions,
                            *targetInfo,
                            sourceManager,
                            headerSearch);
  EXPECT_DIAGNOSTIC_IS_CLEAN;

  // Find a source file.
  const FileEntry* file = fileManager.getFile("a.cpp");
  ASSERT_TRUE(0 != file);

  // Add the source file to the source manager.
  sourceManager.createMainFileID(file);
  preprocessor.EnterMainSourceFile();
  EXPECT_DIAGNOSTIC_IS_CLEAN;

  // Create an ASTContext.
  IdentifierTable identifierTable(langOptions);
  SelectorTable selectorTable;
  clang::Builtin::Context builtinContext(*targetInfo);
  const unsigned int NumReserved = 10;

  ASTContext astContext(langOptions,
                        sourceManager,
                        *targetInfo,
                        identifierTable,
                        selectorTable,
                        builtinContext,
                        NumReserved);

  // Create an AST consumer.
  ASTConsumer consumer;

  // Create a semantic analyzer and an AST builder.
  Sema sema(preprocessor, astContext, consumer);
  EXPECT_DIAGNOSTIC_IS_CLEAN;

  // Create a parser.
  Parser parser(preprocessor, sema);
  EXPECT_DIAGNOSTIC_IS_CLEAN;

  // Parse the file with the parser.
  parser.ParseTranslationUnit();
  EXPECT_DIAGNOSTIC_IS_CLEAN;
}

///////////////////////////////////////////////////////////////////////////////

TEST_F(TestClang, testParseAstManual)
{
  using clang::ASTContext;
  using clang::ASTConsumer;
  using clang::FileEntry;
  using clang::FileManager;
  using clang::HeaderSearch;
  using clang::IdentifierTable;
  using clang::LangOptions;
  using clang::Parser;
  using clang::Preprocessor;
  using clang::SelectorTable;
  using clang::Sema;
  using clang::SourceManager;
  using clang::TargetInfo;
  using clang::TargetOptions;

  // Create C++ language options.
  LangOptions langOptions;
  langOptions.CPlusPlus = 1;

  // Create target options.
  // There is no default target, we need to set a target manually.
  TargetOptions targetOptions;
  targetOptions.Triple = "x86_64";
  auto_ptr<TargetInfo> targetInfo(TargetInfo::CreateTargetInfo(*diagnostic,
                                                               targetOptions));
  EXPECT_TRUE(0 != targetInfo.get());
  EXPECT_DIAGNOSTIC_IS_CLEAN;

  // Create a source manager.
  SourceManager sourceManager(*diagnostic);
  EXPECT_DIAGNOSTIC_IS_CLEAN;

  // Create a file manager.
  FileManager fileManager;

  // Create a header searcher.
  HeaderSearch headerSearch(fileManager);

  // Create a preprocessor.
  Preprocessor preprocessor(*diagnostic,
                            langOptions,
                            *targetInfo,
                            sourceManager,
                            headerSearch);
  EXPECT_DIAGNOSTIC_IS_CLEAN;

  // Create an ASTContext.
  IdentifierTable identifierTable(langOptions);
  SelectorTable selectorTable;
  clang::Builtin::Context builtinContext(*targetInfo);
  const unsigned int NumReserved = 10;

  ASTContext astContext(langOptions,
                        sourceManager,
                        *targetInfo,
                        identifierTable,
                        selectorTable,
                        builtinContext,
                        NumReserved);

  // Create an AST consumer.
  clang::ASTConsumer consumer;

  // Create a semantic analyzer and an AST builder.
  Sema sema(preprocessor, astContext, consumer);
  EXPECT_DIAGNOSTIC_IS_CLEAN;

  // Create a parser.
  Parser parser(preprocessor, sema);
  EXPECT_DIAGNOSTIC_IS_CLEAN;

  // Find a source file.
  const FileEntry* file = fileManager.getFile("a.cpp");
  ASSERT_TRUE(0 != file);

  // Add the source file to the source manager.
  //sourceManager.createMainFileID(file);
  parser.getActions().getSourceManager().createMainFileID(file);
  preprocessor.EnterMainSourceFile();
  EXPECT_DIAGNOSTIC_IS_CLEAN;

  // Parse the file with the parser.
  parser.ParseTranslationUnit();
  EXPECT_DIAGNOSTIC_IS_CLEAN;

  // Handle the whole tree.
  AstInfo astinfo;
  MyDeclVisitor visitor(llvm::errs(), astinfo);
  visitor.Visit(astContext.getTranslationUnitDecl());

  // Check AST stats.
  EXPECT_EQ(1, astinfo._classCount);
  EXPECT_EQ(2, astinfo._fieldCount);
  EXPECT_EQ(1, astinfo._methodCount);
  EXPECT_EQ(1, astinfo._constructorCount);
}

///////////////////////////////////////////////////////////////////////////////

