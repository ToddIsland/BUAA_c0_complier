#pragma once

#include "error/error.h"
#include "instruction/instruction.h"
#include "tokenizer/token.h"

#include <vector>
#include <optional>
#include <utility>
#include <map>
#include <cstdint>
#include <cstddef> // for std::size_t

namespace miniplc0 {
    struct LocalVars {
    public:
        LocalVars(int32_t _localVarsIndex, std::string value) :
                _localVarsIndex(_localVarsIndex), value(value) {}

    private:
        int32_t _localVarsIndex;
        std::string value;
    };

    struct FunctionsBodyInstructions {
    public:
        FunctionsBodyInstructions(int32_t _funcsBodyIndex, std::string opcode, std::string operands) :
                _funcsBodyIndex(_funcsBodyIndex), opcode(opcode), operands(operands) {}

        FunctionsBodyInstructions() {}

    private:
        int32_t _funcsBodyIndex;
        std::string opcode;
        std::string operands;
    };

    struct Functions {
    public:
        Functions(std::string value, int32_t _funcsTableIndex, int32_t name_index, int32_t params_size, int32_t level) :
                value(value), _funcsTableIndex(_funcsTableIndex), name_index(name_index), params_size(params_size),
                level(level),
                _functionsBodyInstructions({}), _localVars({}), _localVarsIndex(0),
                _functionInstructionIndex(0) {}

        Functions() {}

        std::string getValue() { return value; }

        void setParamsSize(int32_t number) { params_size = number; }

        void addLocalVars(std::string ident) {
            auto lv = LocalVars(_localVarsIndex, value);
            _localVarsIndex++;
            _localVars.emplace_back(lv);
        }

        void addFunctionInstructions(std::string opcode,std::string operands){
            auto ins = FunctionsBodyInstructions(_functionInstructionIndex,opcode,operands);
            _functionInstructionIndex++;
        }

    private:
        int32_t _funcsTableIndex;
        int32_t name_index;
        int32_t params_size;
        int32_t level;
        int32_t _localVarsIndex;
        int32_t _functionInstructionIndex;
        std::string value;//便于搜索
        std::vector<LocalVars> _localVars;
        //函数体指令列表
        std::vector<FunctionsBodyInstructions> _functionsBodyInstructions;
    };

    struct Constants {
    public:
        Constants(int32_t _constsTableIndex, std::string type, std::string value) :
                _constsTableIndex(_constsTableIndex), type(type), value(value) {}

        Constants() {}

        std::string getValue() { return value; }

        int32_t getIndex() { return _constsTableIndex; }

    private:
        int32_t _constsTableIndex;
        std::string type;
        std::string value;
    };

    struct StartCode {
    public:
        StartCode(int32_t _startCodeIndex, std::string opcode, std::string operands) :
                _startCodeIndex(_startCodeIndex), opcode(opcode), operands(operands) {}

        StartCode() {}

    private:
        int32_t _startCodeIndex;
        std::string opcode;
        std::string operands;
        //下面两个都采用string
    };

    class Assemble {
    public:
        Assemble(std::map<std::string, int32_t> _const,
                 std::map<std::string, int32_t> _funcs,
                 std::map<std::string, int32_t> _vars,
                 std::map<std::string, int32_t> _uninitialized_vars) :
                _conststable(_conststable), _funcs(_funcs), _vars(_vars), _uninitialized_vars(_uninitialized_vars) {}

        Assemble() {}

        std::map<std::string, int32_t> GetConsts() { return _consts; }

        std::map<std::string, int32_t> GetConstsTable() { return _conststable; }

        std::map<std::string, int32_t> GetFuncs() { return _funcs; }

        std::map<std::string, int32_t> GetVars() { return _vars; }

        std::map<std::string, int32_t> GetUninitializedVars() { return _uninitialized_vars; }

    private:
        std::map<std::string, int32_t> _consts;
        std::map<std::string, int32_t> _funcs;
        std::map<std::string, int32_t> _vars;
        std::map<std::string, int32_t> _uninitialized_vars;
        std::map<std::string, int32_t> _conststable;
    };

    class Analyser final {
    private:
        using uint64_t = std::uint64_t;
        using int64_t = std::int64_t;
        using uint32_t = std::uint32_t;
        using int32_t = std::int32_t;
    public:
        Analyser(std::vector<Token> v)
                : _tokens(std::move(v)), _offset(0), _instructions({}), _current_pos(0, 0),
                  _uninitialized_vars({}), _vars({}), _consts({}), _funcs({}), _nextTokenIndex(0),
                  _startCodeIndex(0), _constsTableIndex(0), _startCodeTable({}), _funcsBodyIndex(0),
                  _funcsTableIndex(0) {}

        Analyser(Analyser &&) = delete;

        Analyser(const Analyser &) = delete;

        Analyser &operator=(Analyser) = delete;

        // 唯一接口
        std::pair<Assemble, std::optional<CompilationError>> Analyse();

    private:
        // 所有的递归子程序

        // <程序>
        std::optional<CompilationError> analyseProgram();

        // <主过程>
        std::optional<CompilationError> analyseMain();

        //分析函数定义
        std::optional<CompilationError> analyseFunctionDefinition();

        //分析函数参数定义
        std::optional<CompilationError> analyseParameterDeclaration(std::string ident);

        //分析函数体
        std::optional<CompilationError> analyseCompoundStatment();

        //分析变量初始化
        std::optional<CompilationError> analyseInitDeclarator(bool isConst);

        //分析条件申明
        std::optional<CompilationError> analyseConditionStatement();

        //分析跳转语句（基础只有return）
        std::optional<CompilationError> analyseJumpStatement();

        //分析SCAN语句
        std::optional<CompilationError> analyseScanStatement();

        //分析函数调用
        std::optional<CompilationError> analyseFunctionCall();

        //分析条件
        std::optional<CompilationError> analyseCondition();

        //分析乘法表达式
        std::optional<CompilationError> analyseMultiplicativeExpression();

        //分析一元表达式
        std::optional<CompilationError> analyseUnaryExpression();

        // <常量声明>
        std::optional<CompilationError> analyseConstantDeclaration();

        // <变量声明>
        std::optional<CompilationError> analyseVariableDeclaration();

        // <语句序列>
        std::optional<CompilationError> analyseStatementSequence();

        // <常表达式>
        // 这里的 out 是常表达式的值
        std::optional<CompilationError> analyseConstantExpression(int32_t &out);

        // <表达式>
        std::optional<CompilationError> analyseExpression();

        // <赋值语句>
        std::optional<CompilationError> analyseAssignmentStatement();

        // <输出语句>
        std::optional<CompilationError> analyseOutputStatement();

        // <项>
        std::optional<CompilationError> analyseItem();

        // <因子>
        std::optional<CompilationError> analyseFactor();

        // Token 缓冲区相关操作

        // 返回下一个 token
        std::optional<Token> nextToken();

        // 回退一个 token
        void unreadToken();

        // 下面是符号表相关操作

        // helper function
        void _add(const Token &, std::map<std::string, int32_t> &);

        // 添加变量、常量、未初始化的变量
        void addVariable(const Token &);

        void addConstant(const Token &);

        void addUninitializedVariable(const Token &);

        void addFunc(const Token &);

        void addConstsTable(const Token &);

        void addGlobalConsts(const Token &);

        void addGlobalVars(const Token &);

        //是否为函数名
        bool isFunc(const std::string &s);

        // 是否被声明过
        bool isDeclared(const std::string &);

        // 是否是未初始化的变量
        bool isUninitializedVariable(const std::string &);

        // 是否是已初始化的变量
        bool isInitializedVariable(const std::string &);

        // 是否是常量
        bool isConstant(const std::string &);

        //函数名在常量表中的下标
        int32_t getFuncsNameIndex(const std::string &);

        void changeFuncsParamsSize(std::string ident, int32_t number);

        //给函数添加局部变量包括参数
        void addLocalVarsToFunc(std::string ident,std::string value);


        // 获得 {变量，常量} 在栈上的偏移
        int32_t getIndex(const std::string &);

    private:
        std::vector<Token> _tokens;
        std::size_t _offset;
        std::vector<Instruction> _instructions;
        std::pair<uint64_t, uint64_t> _current_pos;

        // 为了简单处理，我们直接把符号表耦合在语法分析里
        // 变量                   示例
        // _uninitialized_vars    int a;
        // _vars                  int a=1;
        // _consts                const a=1;
        // _funcs                 int fun();
        std::map<std::string, int32_t> _uninitialized_vars;
        std::map<std::string, int32_t> _vars;
        std::map<std::string, int32_t> _consts;
        std::map<std::string, int32_t> _funcs;//函数名
//        std::map<std::string,int32_t > _constsTable;//文本汇编中的常量表
        //全局的三种情况
        std::map<std::string, int32_t> _globalConsts;
        std::map<std::string, int32_t> _globalVars;
        //没有初始化应该没有影响
//        std::map<std::string,int32_t > _globalUninitializedVars;
        int32_t _startCodeIndex;//start表中的index
        std::vector<StartCode> _startCodeTable;
        std::vector<Constants> _constantsTable;
        std::vector<Functions> _functionsTable;//函数表，里面包含了函数体的表
        // 下一个 token 在栈的偏移
        int32_t _nextTokenIndex;
        int32_t _constsTableIndex;
        int32_t _funcsBodyIndex;//有很多 待考虑
        int32_t _funcsTableIndex;

        bool isGlobal = true;//是全局的情况
        std::string func_name;//当前位于的函数名称
    };
}
