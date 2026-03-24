#include <cctype>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/FormatVariadic.h>

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

// ------------------------------------------------------------
// Error helper
// ------------------------------------------------------------
void error(const std::string &message,
           const char *sourceFile = nullptr,
           int lineNumber = 0)
{
    if (sourceFile)
    {
        llvm::outs() << sourceFile << ":";
        if (lineNumber)
            llvm::outs() << lineNumber << ":";
    }
    llvm::outs() << "error: " << message << "\n";
    exit(1);
}

llvm::LLVMContext llvmContext;
static std::string g_sourceFileName;

// ------------------------------------------------------------
// Lexer
// ------------------------------------------------------------
enum class TokKind {
    Eof,
    KwInt,
    KwReturn,
    Ident,
    Number,

    LParen, RParen,
    LBrace, RBrace,
    Semicolon,
    Comma,

    Plus, Minus,
    Star, Slash,

    Assign, // '='
};

struct Token {
    TokKind kind = TokKind::Eof;
    std::string text;
    int64_t number = 0;
    int line = 1;
};

class Lexer {
public:
    explicit Lexer(const std::string& input) : s(input) {}

    Token next() {
        skipWS();

        Token t;
        t.line = line;

        if (pos >= s.size()) {
            t.kind = TokKind::Eof;
            return t;
        }

        char c = s[pos];

        // identifier / keyword
        if (std::isalpha((unsigned char)c) || c == '_') {
            size_t start = pos++;
            while (pos < s.size()) {
                char cc = s[pos];
                if (std::isalnum((unsigned char)cc) || cc == '_') pos++;
                else break;
            }

            std::string w = s.substr(start, pos - start);
            t.text = w;

            if (w == "int") t.kind = TokKind::KwInt;
            else if (w == "return") t.kind = TokKind::KwReturn;
            else t.kind = TokKind::Ident;

            return t;
        }

        // number
        if (std::isdigit((unsigned char)c)) {
            size_t start = pos++;
            while (pos < s.size() && std::isdigit((unsigned char)s[pos])) pos++;
            std::string num = s.substr(start, pos - start);
            t.kind = TokKind::Number;
            t.number = std::stoll(num);
            return t;
        }

        // single-char tokens
        pos++;
        switch (c) {
            case '(': t.kind = TokKind::LParen; return t;
            case ')': t.kind = TokKind::RParen; return t;
            case '{': t.kind = TokKind::LBrace; return t;
            case '}': t.kind = TokKind::RBrace; return t;
            case ';': t.kind = TokKind::Semicolon; return t;
            case ',': t.kind = TokKind::Comma; return t;

            case '+': t.kind = TokKind::Plus; return t;
            case '-': t.kind = TokKind::Minus; return t;
            case '*': t.kind = TokKind::Star; return t;
            case '/': t.kind = TokKind::Slash; return t;

            case '=': t.kind = TokKind::Assign; return t;

            default:
                error(
                    llvm::formatv("Unexpected character '{0}'", std::string(1, c)).str(),
                    g_sourceFileName.empty() ? nullptr : g_sourceFileName.c_str(),
                    line
                );
        }

        // unreachable (error() exits), but avoids warning
        t.kind = TokKind::Eof;
        return t;
    }

private:
    const std::string& s;
    size_t pos = 0;
    int line = 1;

    void skipWS() {
        while (pos < s.size()) {
            char c = s[pos];
            if (c == '\n') { line++; pos++; continue; }
            if (c == ' ' || c == '\t' || c == '\r') { pos++; continue; }

            // // comments
            if (c == '/' && pos + 1 < s.size() && s[pos + 1] == '/') {
                pos += 2;
                while (pos < s.size() && s[pos] != '\n') pos++;
                continue;
            }

            break;
        }
    }
};

// ------------------------------------------------------------
// Parser + LLVM codegen (Step 4: calls)
// ------------------------------------------------------------
class Parser {
public:
    Parser(const std::string& input, llvm::Module* m)
        : lex(input), mod(m), builder(llvmContext)
    {
        cur = lex.next();
    }

    void parseProgram() {
        while (cur.kind != TokKind::Eof) {
            parseFunction();
        }
    }

private:
    Lexer lex;
    Token cur;

    llvm::Module* mod;
    llvm::IRBuilder<> builder;

    // Per-function symbol table: name -> alloca (args and locals)
    std::unordered_map<std::string, llvm::AllocaInst*> sym;

    void consume(TokKind k, const char* expected) {
        if (cur.kind != k) {
            error(llvm::formatv("Expected {0}", expected).str(),
                  g_sourceFileName.c_str(),
                  cur.line);
        }
        cur = lex.next();
    }

    llvm::Value* constI32(int64_t n) {
        return llvm::ConstantInt::get(llvm::Type::getInt32Ty(llvmContext), n, true);
    }

    llvm::AllocaInst* createAllocaInEntry(llvm::Function* fn, const std::string& name) {
        llvm::IRBuilder<> entryBuilder(&fn->getEntryBlock(), fn->getEntryBlock().begin());
        return entryBuilder.CreateAlloca(llvm::Type::getInt32Ty(llvmContext), nullptr, name);
    }

    // function := int name( [params] ) { {stmt} return expr; }
    void parseFunction() {
        consume(TokKind::KwInt, "'int'");

        if (cur.kind != TokKind::Ident)
            error("Expected function name", g_sourceFileName.c_str(), cur.line);

        std::string fname = cur.text;
        cur = lex.next();

        consume(TokKind::LParen, "'('");

        // Parse params
        std::vector<std::string> paramNames;
        std::unordered_set<std::string> seenParams;

        if (cur.kind != TokKind::RParen) {
            while (true) {
                consume(TokKind::KwInt, "'int'");

                if (cur.kind != TokKind::Ident)
                    error("Expected parameter name", g_sourceFileName.c_str(), cur.line);

                std::string pname = cur.text;
                if (seenParams.count(pname)) {
                    error(llvm::formatv("Duplicate argument '{0}'", pname).str(),
                          g_sourceFileName.c_str(),
                          cur.line);
                }
                seenParams.insert(pname);

                paramNames.push_back(pname);
                cur = lex.next();

                if (cur.kind == TokKind::Comma) {
                    cur = lex.next();
                    continue;
                }
                break;
            }
        }

        consume(TokKind::RParen, "')'");
        consume(TokKind::LBrace, "'{'");

        // Create LLVM function
        std::vector<llvm::Type*> argTypes(paramNames.size(), llvm::Type::getInt32Ty(llvmContext));
        auto* ft = llvm::FunctionType::get(llvm::Type::getInt32Ty(llvmContext), argTypes, false);
        auto* fn = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, fname, mod);

        auto* entry = llvm::BasicBlock::Create(llvmContext, "entry", fn);
        builder.SetInsertPoint(entry);

        // Args -> allocas
        sym.clear();
        size_t i = 0;
        for (auto &arg : fn->args()) {
            arg.setName(paramNames[i]);
            auto* a = createAllocaInEntry(fn, paramNames[i]);
            builder.CreateStore(&arg, a);
            sym[paramNames[i]] = a;
            i++;
        }

        // Parse statements until return
        while (cur.kind == TokKind::KwInt || cur.kind == TokKind::Ident) {
            parseStatement(fn);
        }

        consume(TokKind::KwReturn, "'return'");
        llvm::Value* retVal = parseExpr();
        consume(TokKind::Semicolon, "';'");
        consume(TokKind::RBrace, "'}'");

        builder.CreateRet(retVal);
    }

    // stmt :=
    //   decl | assign | callstmt
    //
    // decl := int declitem {, declitem}* ;
    // declitem := ident [= expr]
    //
    // assign := ident = expr ;
    //
    // callstmt := ident ( [args] ) ;
    void parseStatement(llvm::Function* fn) {
        // ---- declaration(s) ----
        if (cur.kind == TokKind::KwInt) {
            cur = lex.next(); // consume 'int'

            while (true) {
                if (cur.kind != TokKind::Ident)
                    error("Expected local variable name", g_sourceFileName.c_str(), cur.line);

                std::string name = cur.text;
                int declLine = cur.line;
                cur = lex.next();

                if (sym.count(name)) {
                    error(llvm::formatv("Duplicate symbol '{0}'", name).str(),
                          g_sourceFileName.c_str(),
                          declLine);
                }

                auto* a = createAllocaInEntry(fn, name);
                sym[name] = a;

                if (cur.kind == TokKind::Assign) {
                    cur = lex.next(); // '='
                    llvm::Value* init = parseExpr();
                    builder.CreateStore(init, a);
                } else {
                    builder.CreateStore(constI32(0), a);
                }

                if (cur.kind == TokKind::Comma) {
                    cur = lex.next(); // ','
                    continue;
                }
                break;
            }

            consume(TokKind::Semicolon, "';'");
            return;
        }

        // ---- starts with identifier: assignment OR call statement ----
        if (cur.kind == TokKind::Ident) {
            std::string name = cur.text;
            int nameLine = cur.line;
            cur = lex.next();

            if (cur.kind == TokKind::Assign) {
                // assignment
                auto it = sym.find(name);
                if (it == sym.end()) {
                    error(llvm::formatv("Unknown identifier '{0}'", name).str(),
                          g_sourceFileName.c_str(),
                          nameLine);
                }

                cur = lex.next(); // consume '='
                llvm::Value* rhs = parseExpr();
                consume(TokKind::Semicolon, "';'");
                builder.CreateStore(rhs, it->second);
                return;
            }

            if (cur.kind == TokKind::LParen) {
                // call statement
                llvm::Value* callVal = parseCallAfterName(name, nameLine);
                (void)callVal; // ignore return value
                consume(TokKind::Semicolon, "';'");
                return;
            }

            error("Expected '=' or '(' after identifier", g_sourceFileName.c_str(), cur.line);
        }

        error("Expected statement", g_sourceFileName.c_str(), cur.line);
    }

    // expr := add
    llvm::Value* parseExpr() { return parseAdd(); }

    // add := mul { (+|-) mul }
    llvm::Value* parseAdd() {
        llvm::Value* lhs = parseMul();
        while (cur.kind == TokKind::Plus || cur.kind == TokKind::Minus) {
            TokKind op = cur.kind;
            cur = lex.next();
            llvm::Value* rhs = parseMul();
            lhs = (op == TokKind::Plus)
                    ? builder.CreateAdd(lhs, rhs, "addtmp")
                    : builder.CreateSub(lhs, rhs, "subtmp");
        }
        return lhs;
    }

    // mul := unary { (*|/) unary }
    llvm::Value* parseMul() {
        llvm::Value* lhs = parseUnary();
        while (cur.kind == TokKind::Star || cur.kind == TokKind::Slash) {
            TokKind op = cur.kind;
            cur = lex.next();
            llvm::Value* rhs = parseUnary();
            lhs = (op == TokKind::Star)
                    ? builder.CreateMul(lhs, rhs, "multmp")
                    : builder.CreateSDiv(lhs, rhs, "divtmp");
        }
        return lhs;
    }

    // unary := (+|-) unary | primary
    llvm::Value* parseUnary() {
        if (cur.kind == TokKind::Plus) {
            cur = lex.next();
            return parseUnary();
        }
        if (cur.kind == TokKind::Minus) {
            cur = lex.next();
            llvm::Value* v = parseUnary();
            return builder.CreateSub(constI32(0), v, "negtmp");
        }
        return parsePrimary();
    }

    // primary := number | ident | ident(args) | (expr)
    llvm::Value* parsePrimary() {
        if (cur.kind == TokKind::Number) {
            int64_t n = cur.number;
            cur = lex.next();
            return constI32(n);
        }

        if (cur.kind == TokKind::Ident) {
            std::string name = cur.text;
            int nameLine = cur.line;
            cur = lex.next();

            if (cur.kind == TokKind::LParen) {
                // call expression
                return parseCallAfterName(name, nameLine);
            }

            // variable
            auto it = sym.find(name);
            if (it == sym.end()) {
                error(llvm::formatv("Unknown identifier '{0}'", name).str(),
                      g_sourceFileName.c_str(),
                      nameLine);
            }
            return builder.CreateLoad(llvm::Type::getInt32Ty(llvmContext), it->second, "loadtmp");
        }

        if (cur.kind == TokKind::LParen) {
            cur = lex.next();
            llvm::Value* v = parseExpr();
            consume(TokKind::RParen, "')'");
            return v;
        }

        error("Expected number, identifier, or '('", g_sourceFileName.c_str(), cur.line);
        return nullptr;
    }

    // We are called when we've already consumed the function name,
    // and cur is currently '('.
    // Parses: '(' [expr {, expr}*] ')'
    // Emits a call to an external or existing function returning i32.
    llvm::Value* parseCallAfterName(const std::string& fname, int callLine) {
        consume(TokKind::LParen, "'('");

        std::vector<llvm::Value*> args;
        if (cur.kind != TokKind::RParen) {
            while (true) {
                llvm::Value* a = parseExpr();
                args.push_back(a);

                if (cur.kind == TokKind::Comma) {
                    cur = lex.next();
                    continue;
                }
                break;
            }
        }

        consume(TokKind::RParen, "')'");

        // All args are i32 in this assignment subset
        std::vector<llvm::Type*> argTypes(args.size(), llvm::Type::getInt32Ty(llvmContext));
        auto* calleeTy = llvm::FunctionType::get(llvm::Type::getInt32Ty(llvmContext), argTypes, false);

        // Create external declaration if missing (needed across separate .c compilations)
        llvm::FunctionCallee callee = mod->getOrInsertFunction(fname, calleeTy);

        // If the symbol exists but isn't a function, LLVM will assert later; keep it simple here.
        return builder.CreateCall(callee, args, "calltmp");
    }
};

// ------------------------------------------------------------
// Compile
// ------------------------------------------------------------
llvm::Module *compile(const std::string &input, const char *llFileName)
{
    auto* mod = new llvm::Module(llFileName, llvmContext);
    Parser p(input, mod);
    p.parseProgram();
    return mod;
}

// ------------------------------------------------------------
// Main
// ------------------------------------------------------------
int main(int argc, char** argv)
{
    if (argc != 2)
    {
        llvm::outs() << "Usage: compiler <file>.c\n";
        return 1;
    }

    std::string cFileName(argv[1]);
    if (cFileName.rfind(".c", cFileName.size()-2) == std::string::npos)
        error(llvm::formatv("File name {0} does not end with .c", cFileName));

    g_sourceFileName = cFileName;

    std::string llFileName = cFileName.substr(0,cFileName.size()-2) + ".ll";

    auto inFile = llvm::MemoryBuffer::getFile(cFileName);
    if (inFile.getError())
        error("Cannot read input file", cFileName.c_str());

    llvm::Module *module = compile((*inFile)->getBuffer().str(), llFileName.c_str());

    std::error_code err;
    llvm::raw_fd_ostream output(module->getName(), err, llvm::sys::fs::OF_Text);
    if (err)
        error(err.message(), cFileName.c_str());

    module->print(output, nullptr);
    return 0;
}
