#include "catch2/catch.hpp"

#include "instruction/instruction.h"
#include "tokenizer/tokenizer.h"
#include "analyser/analyser.h"

/*
	不要忘记写测试用例喔。
*/
#include "catch2/catch.hpp"

#include "instruction/instruction.h"
#include "tokenizer/tokenizer.h"
#include "analyser/analyser.h"
#include "simple_vm.hpp"

#include <sstream>
#include <vector>

TEST_CASE("missing_begin_end", "[invalid]") {
std::string input =
        "var a = 1;\n"
        "print(a);\n"
;
std::stringstream ss;
ss.str(input);
miniplc0::Tokenizer tkz(ss);
auto tokens = tkz.AllTokens();
if (tokens.second.has_value())
FAIL();
miniplc0::Analyser analyser(tokens.first);
auto instructions = analyser.Analyse();
REQUIRE( (instructions.second.has_value()) );
}

TEST_CASE("missing_semicolon", "[invalid]") {
std::string input =
        "begin\n"
        "const A = 1;\n"
        "var B = A;\n"
        "print(A)\n"
        "end\n"
;
std::stringstream ss;
ss.str(input);
miniplc0::Tokenizer tkz(ss);
auto tokens = tkz.AllTokens();
if (tokens.second.has_value())
FAIL();
miniplc0::Analyser analyser(tokens.first);
auto instructions = analyser.Analyse();
REQUIRE( (instructions.second.has_value()) );
}

TEST_CASE("redeclaration", "[invalid]") {
std::string input =
        "begin\n"
        "const A = 1;\n"
        "var A;\n"
        "end\n"
;
std::stringstream ss;
ss.str(input);
miniplc0::Tokenizer tkz(ss);
auto tokens = tkz.AllTokens();
if (tokens.second.has_value())
FAIL();
miniplc0::Analyser analyser(tokens.first);
auto instructions = analyser.Analyse();
REQUIRE( (instructions.second.has_value()) );
}

TEST_CASE("uninit", "[invalid]") {
std::string input =
        "begin\n"
        "var a;\n"
        "print(a);\n"
        "end\n"
;
std::stringstream ss;
ss.str(input);
miniplc0::Tokenizer tkz(ss);
auto tokens = tkz.AllTokens();
if (tokens.second.has_value())
FAIL();
miniplc0::Analyser analyser(tokens.first);
auto instructions = analyser.Analyse();
REQUIRE( (instructions.second.has_value()) );
}

TEST_CASE("var_const", "[invalid]") {
std::string input =
        "begin\n"
        "var a;\n"
        "const b = 1;\n"
        "end\n"
;
std::stringstream ss;
ss.str(input);
miniplc0::Tokenizer tkz(ss);
auto tokens = tkz.AllTokens();
if (tokens.second.has_value())
FAIL();
miniplc0::Analyser analyser(tokens.first);
auto instructions = analyser.Analyse();
REQUIRE( (instructions.second.has_value()) );
}

TEST_CASE("assign", "[valid]") {
std::string input =
        "begin\n"
        "var a = 1;\n"
        "var b;\n"
        "var c;\n"
        "var d;\n"
        "var e;\n"
        "b = a;\n"
        "e = b;\n"
        "d = e;\n"
        "c = a;\n"
        "print(c);\n"
        "end\n"
;
std::stringstream ss;
ss.str(input);
miniplc0::Tokenizer tkz(ss);
auto tokens = tkz.AllTokens();
if (tokens.second.has_value())
FAIL();
miniplc0::Analyser analyser(tokens.first);
auto instructions = analyser.Analyse();
if (instructions.second.has_value())
FAIL();
miniplc0::VM vm(instructions.first);
std::vector<int32_t> writes = vm.Run();
std::vector<int32_t> output = {};
output.emplace_back(1);
REQUIRE( (writes == output) );
}

TEST_CASE("declaration", "[valid]") {
std::string input =
        "begin\n"
        "const abc = 123;\n"
        "var ABC = 456;\n"
        "print(abc);\n"
        "print(ABC);\n"
        "end\n"
;
std::stringstream ss;
ss.str(input);
miniplc0::Tokenizer tkz(ss);
auto tokens = tkz.AllTokens();
if (tokens.second.has_value())
FAIL();
miniplc0::Analyser analyser(tokens.first);
auto instructions = analyser.Analyse();
if (instructions.second.has_value())
FAIL();
miniplc0::VM vm(instructions.first);
std::vector<int32_t> writes = vm.Run();
std::vector<int32_t> output = {};
output.emplace_back(123);
output.emplace_back(456);
REQUIRE( (writes == output) );
}

TEST_CASE("init", "[valid]") {
std::string input =
        "begin\n"
        "var a = 0;\n"
        "var b = 1;\n"
        "var c = a+b;\n"
        "a = b;\n"
        "c = c;\n"
        "c = a+b;\n"
        "a = b;\n"
        "b = c;\n"
        "print(a);\n"
        "print(b);\n"
        "print(c);\n"
        "end\n"
;
std::stringstream ss;
ss.str(input);
miniplc0::Tokenizer tkz(ss);
auto tokens = tkz.AllTokens();
if (tokens.second.has_value())
FAIL();
miniplc0::Analyser analyser(tokens.first);
auto instructions = analyser.Analyse();
if (instructions.second.has_value()) {
std::cout << instructions.second.value().GetCode() << "\n";
FAIL();
}
miniplc0::VM vm(instructions.first);
std::vector<int32_t> writes = vm.Run();
std::vector<int32_t> output = {};
output.emplace_back(1);
output.emplace_back(2);
output.emplace_back(2);
REQUIRE( (writes == output) );
}

TEST_CASE("no_begin", "[invalid]") {
std::string input =
        "end"
;
std::stringstream ss;
ss.str(input);
miniplc0::Tokenizer tkz(ss);
auto tokens = tkz.AllTokens();
if (tokens.second.has_value())
FAIL();
miniplc0::Analyser analyser(tokens.first);
auto instructions = analyser.Analyse();
REQUIRE( (instructions.second.has_value()) );
}

TEST_CASE("no_end", "[invalid]") {
std::string input =
        "begin"
;
std::stringstream ss;
ss.str(input);
miniplc0::Tokenizer tkz(ss);
auto tokens = tkz.AllTokens();
if (tokens.second.has_value())
FAIL();
miniplc0::Analyser analyser(tokens.first);
auto instructions = analyser.Analyse();
REQUIRE( (instructions.second.has_value()) );
}

TEST_CASE("no_semicolon", "[invalid]") {
std::string input =
        "begin\n"
        "   const a = 1\n"
        "end"
;
std::stringstream ss;
ss.str(input);
miniplc0::Tokenizer tkz(ss);
auto tokens = tkz.AllTokens();
if (tokens.second.has_value())
FAIL();
miniplc0::Analyser analyser(tokens.first);
auto instructions = analyser.Analyse();
REQUIRE( (instructions.second.has_value()) );
}

TEST_CASE("const_no_value", "[invalid]") {
std::string input =
        "begin\n"
        "   const a;\n"
        "   const b;\n"
        "end"
;
std::stringstream ss;
ss.str(input);
miniplc0::Tokenizer tkz(ss);
auto tokens = tkz.AllTokens();
if (tokens.second.has_value())
FAIL();
miniplc0::Analyser analyser(tokens.first);
auto instructions = analyser.Analyse();
REQUIRE( (instructions.second.has_value()) );
}

TEST_CASE("const_duplicate", "[invalid]") {
std::string input =
        "begin\n"
        "   const a = 1;\n"
        "   const a = 2;\n"
        "end"
;
std::stringstream ss;
ss.str(input);
miniplc0::Tokenizer tkz(ss);
auto tokens = tkz.AllTokens();
if (tokens.second.has_value())
FAIL();
miniplc0::Analyser analyser(tokens.first);
auto instructions = analyser.Analyse();
REQUIRE( (instructions.second.has_value()) );
}

TEST_CASE("var_duplicate", "[invalid]") {
std::string input =
        "begin\n"
        "   var a;\n"
        "   var a;\n"
        "end"
;
std::stringstream ss;
ss.str(input);
miniplc0::Tokenizer tkz(ss);
auto tokens = tkz.AllTokens();
if (tokens.second.has_value())
FAIL();
miniplc0::Analyser analyser(tokens.first);
auto instructions = analyser.Analyse();
REQUIRE( (instructions.second.has_value()) );
}

TEST_CASE("assign_to_const", "[invalid]") {
std::string input =
        "begin\n"
        "   const a = 1;\n"
        "   a = 2;\n"
        "end"
;
std::stringstream ss;
ss.str(input);
miniplc0::Tokenizer tkz(ss);
auto tokens = tkz.AllTokens();
if (tokens.second.has_value())
FAIL();
miniplc0::Analyser analyser(tokens.first);
auto instructions = analyser.Analyse();
REQUIRE( (instructions.second.has_value()) );
}

TEST_CASE("assign_no_value", "[invalid]") {
std::string input =
        "begin\n"
        "   var a;\n"
        "   a = ;\n"
        "end"
;
std::stringstream ss;
ss.str(input);
miniplc0::Tokenizer tkz(ss);
auto tokens = tkz.AllTokens();
if (tokens.second.has_value())
FAIL();
miniplc0::Analyser analyser(tokens.first);
auto instructions = analyser.Analyse();
REQUIRE( (instructions.second.has_value()) );
}

TEST_CASE("print_no_value", "[invalid]") {
std::string input =
        "begin\n"
        "   print();\n"
        "end"
;
std::stringstream ss;
ss.str(input);
miniplc0::Tokenizer tkz(ss);
auto tokens = tkz.AllTokens();
if (tokens.second.has_value())
FAIL();
miniplc0::Analyser analyser(tokens.first);
auto instructions = analyser.Analyse();
REQUIRE( (instructions.second.has_value()) );
}

TEST_CASE("valid_empty", "[valid]") {
std::string input =
        "begin\n"
        "end"
;
std::stringstream ss;
ss.str(input);
miniplc0::Tokenizer tkz(ss);
auto tokens = tkz.AllTokens();
if (tokens.second.has_value())
FAIL();
miniplc0::Analyser analyser(tokens.first);
auto instructions = analyser.Analyse();
if (instructions.second.has_value())
FAIL();
miniplc0::VM vm(instructions.first);
std::vector<int32_t> writes = vm.Run();
std::vector<int32_t> output = {};
REQUIRE( (writes == output) );
}

TEST_CASE("valid_empty_statement", "[valid]") {
std::string input =
        "begin\n"
        "   ;;;;;;;;;;;;\n"
        "   ;;;;;\n"
        "end"
;
std::stringstream ss;
ss.str(input);
miniplc0::Tokenizer tkz(ss);
auto tokens = tkz.AllTokens();
if (tokens.second.has_value())
FAIL();
miniplc0::Analyser analyser(tokens.first);
auto instructions = analyser.Analyse();
if (instructions.second.has_value())
FAIL();
miniplc0::VM vm(instructions.first);
std::vector<int32_t> writes = vm.Run();
std::vector<int32_t> output = {};
REQUIRE( (writes == output) );
}

TEST_CASE("valid_overview", "[valid]") {
std::string input =
        "begin\n"
        "   const a = 1;\n"
        "   const b = -2147483647;\n"
        "   var c;\n"
        "   var d = a + b + 6;\n"
        "   c = 3;\n"
        "   d = 5;\n"
        "   print((-d + c) * a);\n"
        "   ;\n"
        "end\n"
;
std::stringstream ss;
ss.str(input);
miniplc0::Tokenizer tkz(ss);
auto tokens = tkz.AllTokens();
if (tokens.second.has_value())
FAIL();
miniplc0::Analyser analyser(tokens.first);
auto instructions = analyser.Analyse();
if (instructions.second.has_value())
FAIL();
miniplc0::VM vm(instructions.first);
std::vector<int32_t> writes = vm.Run();
std::vector<int32_t> output = {};
output.emplace_back(-2);
REQUIRE( (writes == output) );
}
