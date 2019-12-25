#include "analyser.h"

#include <climits>
#include <sstream>

namespace miniplc0 {
    std::pair<Assemble, std::optional<CompilationError>> Analyser::Analyse() {
        auto err = analyseProgram();
        if (err.has_value())
            return std::make_pair(Assemble(), err);
        else {
            auto ass = Assemble(_consts, _funcs, _vars, _uninitialized_vars);
            return std::make_pair(ass, std::optional<CompilationError>());
        }
//            return std::make_pair(_instructions, std::optional<CompilationError>());
    }

    // <C0-program> ::=
    //{<variable-declaration>}{<function-definition>}
    std::optional<CompilationError> Analyser::analyseProgram() {

        // 'begin'
        //auto bg = nextToken();
        //if (!bg.has_value() || bg.value().GetType() != TokenType::BEGIN)
        //return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoBegin);

        // <主过程>
        //auto err = analyseMain();
        //if (err.has_value())
        //return err;

        // 'end'
        //auto ed = nextToken();
        //if (!ed.has_value() || ed.value().GetType() != TokenType::END)
        //return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoEnd);
        //return {};

        //全局变量申明
        auto err = analyseVariableDeclaration();
        if (err.has_value())
            return err;

        //函数内部就不是全局
        isGlobal = false;

        //函数定义
        err = analyseFunctionDefinition();
        if (err.has_value())
            return err;
    }

    // <主过程> ::= <常量声明><变量声明><语句序列>
    // 需要补全
    //std::optional<CompilationError> Analyser::analyseMain() {
    // 完全可以参照 <程序> 编写

    // <常量声明>
    //auto err = analyseConstantDeclaration();
    //if (err.has_value())
    //return err;

    // <变量声明>
    //err = analyseVariableDeclaration();
    //if (err.has_value())
    //return err;

    // <语句序列>
    //err = analyseStatementSequence();
    //if (err.has_value())
    //return err;

    //return {};
    //}

    //<variable-declaration> ::=
    //[<const-qualifier>]<type-specifier><init-declarator-list>';'
    //<init-declarator-list> ::=
    //<init-declarator>{','<init-declarator>}
    //<init-declarator> ::=
    //<identifier>[<initializer>]
    //<initializer> ::=
    //'='<expression>
    std::optional<CompilationError> Analyser::analyseVariableDeclaration() {
        while (true) {
            auto next = nextToken();
            bool isConst = false;
            //进行常量的说明
            //是否有CONST修饰符
            if (next.value().GetType() == TokenType::CONST) {
                isConst = true;
                next = nextToken();
            }
                //转向函数分析
            else if (next.value().GetType() == TokenType::VOID) {
                if (isGlobal) {
                    unreadToken();
                    return {};
                }
                    //函数中的申明语句使用void
                else {
                    return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidAssignment);
                }
            }

            //变量或者常量不需要VOID修饰
            //是否有INT修饰符
//            if (next.value().GetType() == VOID)
//                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedTypeSpecifier);

            if (next.value().GetType() != TokenType::INT || next.value().GetType() == VOID)
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedTypeSpecifier);

            while (true) {
                auto err = analyseInitDeclarator(isConst);
                //跳转到函数分析
                if (err.has_value()) {
                    if (err.value().GetCode() == ErrorCode::ErrGoToFuncDef)
                        return {};
                    else
                        return err;
                }

                next = nextToken();
                if (next.value().GetType() == TokenType::SEMICOLON)
                    return {};
                else if (next.value().GetType() == TokenType::COMMA)
                    continue;
                else
                    return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);
            }
        }
        return {};
    }

    //<function-definition> ::=
    //<type-specifier><identifier><parameter-clause><compound-statement>
    std::optional<CompilationError> Analyser::analyseFunctionDefinition() {
        //需要跳出这个循环
        while (true) {
            auto next = nextToken();


            //只有VOID或者INT修饰
            if (!next.has_value() || next.value().GetType() != TokenType::VOID ||
                next.value().GetType() != TokenType::INT)
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedTypeSpecifier);

            next = nextToken();
            if (!next.has_value() || next.value().GetType() != TokenType::IDENTIFIER)
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedIdentifier);
            else {
                //函数名加入常量表
                auto con = Constants(_constsTableIndex, "S", next.value().GetValueString());
                _constsTableIndex++;
                _constantsTable.emplace_back(con);
                //函数加入函数表
                auto func = Functions(next.value().GetValueString(), _funcsTableIndex,
                                      getFuncsNameIndex(next.value().GetValueString()), 0, 1);
                _funcsTableIndex++;
                _functionsTable.emplace_back(func);
            }

            //保存函数名
            auto ident = next.value().GetValueString();
            //全局保存函数名
            func_name = ident;

            next = nextToken();
            if (!next.has_value() || next.value().GetType() != TokenType::LEFT_BRACKET)
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedBracket);

            auto err = analyseParameterDeclaration(ident);
            if (err.has_value())
                return err;

            next = nextToken();
            if (!next.has_value() || next.value().GetType() != TokenType::RIGHT_BRACKET)
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedBracket);

            err = analyseCompoundStatment();
            if (err.has_value())
                return err;
        }
    }


    //<init-declarator> ::=
    //<identifier>[<initializer>]
    //<initializer> ::=
    //'='<expression>
    std::optional<CompilationError> Analyser::analyseInitDeclarator(bool isConst) {
        auto next = nextToken();

        if (next.value().GetType() != TokenType::IDENTIFIER)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedIdentifier);

        auto val = next;//记录变量或常量名
        next = nextToken();
        if (next.value().GetType() == TokenType::LEFT_BRACKET) {
            unreadToken();
            unreadToken();
            unreadToken();//回朔三个token
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrGoToFuncDef);
        }

        //没有初始化和初始化的情况
        //局部变量是否为const的情况！！！
        if (isConst == false) {
            if (next.value().GetType() == TokenType::EQUAL_SIGN) {
                if (isGlobal) {
                    next = nextToken();
                    addGlobalVars(val.value());
                    //赋值的情况 直接push栈中 保存在常量表中
                    auto stcode = StartCode(_startCodeIndex, "ipush", next.value().GetValueString());
                    _startCodeIndex++;
                    _startCodeTable.emplace_back(stcode);
                } else {
                    next = nextToken();
                    addLocalVarsToFunc(func_name, val.value().GetValueString());
                }
            } else {
                //没有赋值的情况
                if (isGlobal) {
                    unreadToken();
                    addGlobalVars(val.value());
                    //为变量分配空间
                    auto stcode = StartCode(_startCodeIndex, "snew", "1");
                    _startCodeIndex++;
                    _startCodeTable.emplace_back(stcode);
                    auto con = Constants(_constsTableIndex, "I", val.value().GetValueString());
                    _constsTableIndex++;
                    _constantsTable.emplace_back(con);
                }
                else{
                    unreadToken();
                    //默认赋值为0？
                    addLocalVarsToFunc(func_name,"0");
                }
            }
        } else if (isConst == true) {
            //CONST修饰必须被显式初始化
            if (!next.has_value() || next.value().GetType() != TokenType::EQUAL_SIGN)
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrConstantNeedValue);
            else {
                next = nextToken();
                if(isGlobal) {
                    //赋值的情况 直接push栈中 保存在常量表中
                    addGlobalConsts(val.value());//不能改变他的值
                    auto stcode = StartCode(_startCodeIndex, "ipush", val.value().GetValueString());
                    _startCodeIndex++;
                    _startCodeTable.emplace_back(stcode);
                    auto con = Constants(_constsTableIndex, "I", val.value().GetValueString());
                    _constsTableIndex++;
                    _constantsTable.emplace_back(con);
                }
                else{
                    addLocalVarsToFunc(func_name,next.value().GetValueString());
                }
            }
        }
        return {};
    }

    //参数中只有int的情况
    std::optional<CompilationError> Analyser::analyseParameterDeclaration(std::string ident) {

        auto next = nextToken();
        int32_t number = 0;
        while (true) {
            //没有参数的情况
            if (next.value().GetType() == TokenType::RIGHT_BRACKET) {
                if (number > 0) {
                    changeFuncsParamsSize(ident, number);
                    //改变参数占用的slot数
                    //因为只有int类型
                }
                unreadToken();
                return {};
            } else if (next.value().GetType() == TokenType::IDENTIFIER) {
                number++;//参数占用slot数
                //添加局部变量
                addLocalVarsToFunc(ident, next.value().GetValueString());
            } else {
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedIdentifier);
            }

            next = nextToken();
            if (next.value().GetType() == TokenType::COMMA) {
                next = nextToken();
                continue;
            } else
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedComma);
        }
    }

    std::optional<CompilationError> Analyser::analyseCompoundStatment() {
        auto next = nextToken();
        if (next.value().GetType() != TokenType::LEFT_BRACE)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedBrace);

        next = nextToken();
        if (next.value().GetType() == TokenType::INT || next.value().GetType() == TokenType::CONST) {
            unreadToken();
            auto err = analyseVariableDeclaration();
            if (err.has_value())
                return err;
        }

        auto err = analyseStatementSequence();
        if (err.has_value())
            return err;


        next = nextToken();
        if (next.value().GetType() != TokenType::RIGHT_BRACE)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedBrace);
    }

    //比较主要的部分
    std::optional<CompilationError> Analyser::analyseStatementSequence() {
        while (true) {
            auto next = nextToken();
            switch (next.value().GetType()) {
                case TokenType::LEFT_BRACE : {
                    auto err = analyseStatementSequence();
                    if (err.has_value())
                        return err;
                    next = nextToken();
                    if (next.value().GetType() != TokenType::RIGHT_BRACE)
                        return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedBrace);
                    break;
                }
                case TokenType::IF : {//不回朔
                    auto err = analyseConditionStatement();
                    if (err.has_value())
                        return err;
                    break;
                }
                case TokenType::RETURN : {
                    auto err = analyseJumpStatement();
                    if (err.has_value())
                        return err;
                    break;
                }
                case TokenType::PRINT : {
                    auto err = analyseOutputStatement();
                    if (err.has_value())
                        return err;
                    break;
                }
                case TokenType::SCAN : {
                    auto err = analyseScanStatement();
                    if (err.has_value())
                        return err;
                    break;
                    //区分函数调用和赋值语句
                }
                case TokenType::IDENTIFIER :
                    if (isDeclared(next.value().GetValueString())) {
                        auto err = analyseAssignmentStatement();
                        if (err.has_value())
                            return err;
                        break;
                    } else if (isFunc(next.value().GetValueString())) {
                        auto err = analyseFunctionCall();
                        if (err.has_value())
                            return err;
                        break;
                    }
                case TokenType::SEMICOLON :
                    return {};
            }
        }
    }

    std::optional<CompilationError> Analyser::analyseConditionStatement() {
        auto next = nextToken();
        if (next.value().GetType() != TokenType::LEFT_BRACKET)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedBracket);

        auto err = analyseCondition();
        if (err.has_value())
            return err;

        next = nextToken();
        if (next.value().GetType() != TokenType::RIGHT_BRACKET)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedBracket);

        err = analyseStatementSequence();
        if (err.has_value())
            return err;

        next = nextToken();
        if (next.value().GetType() == TokenType::ELSE) {
            auto err = analyseStatementSequence();
            if (err.has_value())
                return err;
        }

        return {};
    }

    std::optional<CompilationError> Analyser::analyseJumpStatement() {
        auto err = analyseExpression();
        if (err.has_value())
            return err;

        auto next = nextToken();
        if (next.value().GetType() != TokenType::SEMICOLON)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);

        return {};
    }

    //std::optional<CompilationError> Analyser::analysePrintStatement() {
    //auto next = nextToken();
    //if(next.value().GetType() != TokenType::LEFT_BRACKET)
    //return std::make_optional<CompilationError>(_current_pos,ErrorCode::ErrNeedBracket);

    //analyseExpression

    //next = nextToken();
    //if(next.value().GetType() != TokenType::SEMICOLON)
    //return std::make_optional<CompilationError>(_current_pos,ErrorCode::ErrNoSemicolon);
    //}

    //<scan-statement> ::=
    //'scan' '(' <identifier> ')' ';'
    std::optional<CompilationError> Analyser::analyseScanStatement() {
        auto next = nextToken();

        //'('
        if (next.value().GetType() != TokenType::LEFT_BRACKET)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedBracket);

        //<identifier>
        next = nextToken();
        if (next.value().GetType() != TokenType::IDENTIFIER)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedIdentifier);

        //')'
        next = nextToken();
        if (next.value().GetType() != TokenType::RIGHT_BRACKET)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedBracket);

        //';'
        next = nextToken();
        if (next.value().GetType() != TokenType::SEMICOLON)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);
    }


    // <常量声明> ::= {<常量声明语句>}
    // <常量声明语句> ::= 'const'<标识符>'='<常表达式>';'
    std::optional<CompilationError> Analyser::analyseConstantDeclaration() {
        // 示例函数，示例如何分析常量声明

        // 常量声明语句可能有 0 或无数个
        while (true) {
            // 预读一个 token，不然不知道是否应该用 <常量声明> 推导
            auto next = nextToken();
            if (!next.has_value())
                return {};
            // 如果是 const 那么说明应该推导 <常量声明> 否则直接返回
            if (next.value().GetType() != TokenType::CONST) {
                unreadToken();
                return {};
            }

            // <常量声明语句>
            next = nextToken();
            if (!next.has_value() || next.value().GetType() != TokenType::IDENTIFIER)
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedIdentifier);
            if (isDeclared(next.value().GetValueString()))
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrDuplicateDeclaration);
            //二次声明
            addConstant(next.value());

            // '='
            next = nextToken();
            if (!next.has_value() || next.value().GetType() != TokenType::EQUAL_SIGN)
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrConstantNeedValue);

            // <常表达式>
            int32_t val;
            auto err = analyseConstantExpression(val);
            if (err.has_value())
                return err;

            // ';'
            next = nextToken();
            if (!next.has_value() || next.value().GetType() != TokenType::SEMICOLON)
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);
            // 生成一次 LIT 指令加载常量
//            _instructions.emplace_back(Operation::LIT, val);
        }
        return {};
    }

    // <变量声明> ::= {<变量声明语句>}
    // <变量声明语句> ::= 'var'<标识符>['='<表达式>]';'
    // 需要补全
    //std::optional<CompilationError> Analyser::analyseVariableDeclaration() {
    //变量声明语句可能有一个或者多个
    //while (true) {
    //auto next = nextToken();
    //if (!next.has_value())
    //return {};
    //如果不是VAR则直接返回
    //if (next.value().GetType() != TokenType::VAR) {
    //unreadToken();
    //return {};
    //}
    //标识符
    //next = nextToken();
    //if (!next.has_value() || next.value().GetType() != TokenType::IDENTIFIER)
    //return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedIdentifier);
    //if (isDeclared(next.value().GetValueString()))
    //return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrDuplicateDeclaration);

    //			addVariable(next.value());
    //auto ident = next;//上一个标识符
    //next = nextToken();
    //if (!next.has_value())
    //return {};
    //初值为0 便于Expression中的操作
    //_instructions.emplace_back(Operation::LIT, 0);

    //if (next.value().GetType() == TokenType::EQUAL_SIGN) {
    //auto err = analyseExpression();
    //if (err.has_value())
    //return err;
    //addVariable(ident.value());
    //int32_t index = getIndex(ident.value().GetValueString());
    //_instructions.emplace_back(Operation::STO, index);//栈顶是计算完的值

    //next = nextToken();
    //if (!next.has_value() || next.value().GetType() != TokenType::SEMICOLON)
    //return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);
    //} else if (next.value().GetType() == TokenType::SEMICOLON) {
    //{
    //addUninitializedVariable(ident.value());
    //continue;}
    //} else
    //return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);
    //}
    // 预读？

    // 'var'

    // <标识符>

    // 变量可能没有初始化，仍然需要一次预读

    // '='

    // '<表达式>'

    // ';'
    //return {};
    //}

    // <语句序列> ::= {<语句>}
    // <语句> :: = <赋值语句> | <输出语句> | <空语句>
    // <赋值语句> :: = <标识符>'='<表达式>';'
    // <输出语句> :: = 'print' '(' <表达式> ')' ';'
    // <空语句> :: = ';'
    // 需要补全
    //std::optional<CompilationError> Analyser::analyseStatementSequence() {
    //while (true) {
    // 预读
    //auto next = nextToken();
    //if (!next.has_value())
    //return {};
    //unreadToken();
    //if (next.value().GetType() != TokenType::IDENTIFIER &&
    //next.value().GetType() != TokenType::PRINT &&
    //next.value().GetType() != TokenType::SEMICOLON) {
    //return {};
    //}
    //std::optional<CompilationError> err;
    //switch (next.value().GetType()) {
    // 这里需要你针对不同的预读结果来调用不同的子程序
    // 注意我们没有针对空语句单独声明一个函数，因此可以直接在这里返回
    //case TokenType::IDENTIFIER : {
    //auto err = analyseAssignmentStatement();
    //if (err.has_value())
    //return err;
    //break;
    //}
    //case TokenType::PRINT : {
    //err = analyseOutputStatement();
    //if (err.has_value())
    //return err;
    //break;
    //}
    //case TokenType::SEMICOLON :
    //nextToken();
    //break;//如果是分号就进入下一个循环
    //default:
    //break;
    //}
    //}
    //return {};
    //}

    // <常表达式> ::= [<符号>]<无符号整数>
    // 需要补全
    std::optional<CompilationError> Analyser::analyseConstantExpression(int32_t &out) {
        // out 是常表达式的结果
        // 这里你要分析常表达式并且计算结果
        // 注意以下均为常表达式
        // +1 -1 1
        // 同时要注意是否溢出
        bool isNegative = false;
        auto next = nextToken();
        auto type = next.value().GetType();
        if (type == MINUS_SIGN) {
            isNegative = true;
        } else if (type == PLUS_SIGN) {
            isNegative = false;
        } else {
            unreadToken();
        }

        next = nextToken();
        int len = next.value().GetValueString().length();
        int64_t result_num;
        std::stringstream strstr(next.value().GetValueString());
        strstr >> result_num;
        //        if (isNegative)
        //            result_num *= -1;
        //        if (len > 10 || result_num > 2147483647 || result_num < -2147483648)
        if (len > 10 || result_num > 4294967295)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIntegerOverflow);
        else
            out = result_num;
        if (isNegative)
            out *= -1;

        return {};
    }

    // <表达式> ::= <项>{<加法型运算符><项>}
    // <expression> ::=
    //<additive-expression>
    //<additive-expression> ::=
    //<multiplicative-expression>{<additive-operator><multiplicative-expression>}
    //<multiplicative-expression> ::=
    //<cast-expression>{<multiplicative-operator><cast-expression>}
    //<cast-expression> ::=
    //{'('<type-specifier>')'}<unary-expression>
    //<unary-expression> ::=
    //[<unary-operator>]<primary-expression>
    //<primary-expression> ::=
    //'('<expression>')'
    //|<identifier>
    //|<integer-literal>
    //|<function-call>
    std::optional<CompilationError> Analyser::analyseExpression() {
        //<multiplicative-expression>
        auto err = analyseMultiplicativeExpression();
        if (err.has_value())
            return err;

        // {<加法型运算符><项>}
        // {<additive-operator><multiplicative-expression>}
        while (true) {
            // 预读
            auto next = nextToken();
            if (!next.has_value())
                return {};
            auto type = next.value().GetType();
            if (type != TokenType::PLUS_SIGN && type != TokenType::MINUS_SIGN) {
                unreadToken();
                return {};
            }

            // <项>
            err = analyseMultiplicativeExpression();
            if (err.has_value())
                return err;

            // 根据结果生成指令
            //if (type == TokenType::PLUS_SIGN)
            //_instructions.emplace_back(Operation::ADD, 0);
            //else if (type == TokenType::MINUS_SIGN)
            //_instructions.emplace_back(Operation::SUB, 0);
        }
        return {};
    }

    // <赋值语句> ::= <标识符>'='<表达式>';'
    // 需要补全
    // <assignment-expression> ::=
    //<identifier><assignment-operator><expression>
    //<assignment-operator>     ::= '='
    std::optional<CompilationError> Analyser::analyseAssignmentStatement() {
        // 这里除了语法分析以外还要留意
        // 标识符声明过吗？
        // 标识符是常量吗？
        // 需要生成指令吗？
        auto next = nextToken();
        if (!next.has_value() || next.value().GetType() != TokenType::IDENTIFIER)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedIdentifier);
        if (!isDeclared(next.value().GetValueString()))//没有被声明过
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrDuplicateDeclaration);
        if (isConstant(next.value().GetValueString()))
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrAssignToConstant);

        auto ident = next;//保存标识符

        next = nextToken();
        if (!next.has_value() || next.value().GetType() != TokenType::EQUAL_SIGN)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedEqualSign);


        auto err = analyseExpression();
        if (err.has_value())
            return err;

        next = nextToken();
        if (!next.has_value() || next.value().GetType() != TokenType::SEMICOLON)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);

        //从未初始化的集合中删除
        //if (isUninitializedVariable(ident.value().GetValueString())) {
        //_vars[ident.value().GetValueString()] = _uninitialized_vars[ident.value().GetValueString()];
        //_uninitialized_vars.erase(ident.value().GetValueString());
        //}

        //_instructions.emplace_back(Operation::STO, getIndex(ident.value().GetValueString()));//进行赋值

        return {};
    }

    // <输出语句> ::= 'print' '(' <表达式> ')' ';'
    std::optional<CompilationError> Analyser::analyseOutputStatement() {
        // 如果之前 <语句序列> 的实现正确，这里第一个 next 一定是 TokenType::PRINT
        auto next = nextToken();

        // '('
        next = nextToken();
        if (!next.has_value() || next.value().GetType() != TokenType::LEFT_BRACKET)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidPrint);

        // <表达式>
        auto err = analyseExpression();
        if (err.has_value())
            return err;

        // ')'
        next = nextToken();
        if (!next.has_value() || next.value().GetType() != TokenType::RIGHT_BRACKET)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidPrint);

        // ';'
        next = nextToken();
        if (!next.has_value() || next.value().GetType() != TokenType::SEMICOLON)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);

        // 生成相应的指令 WRT
//        _instructions.emplace_back(Operation::WRT, 0);
        return {};
    }

    //<multiplicative-expression> ::=
    //<cast-expression>{<multiplicative-operator><cast-expression>}
    //<cast-expression> ::=
    //{'('<type-specifier>')'}<unary-expression>
    //<unary-expression> ::=
    //[<unary-operator>]<primary-expression>
    //<primary-expression> ::=
    //'('<expression>')'
    //|<identifier>
    //|<integer-literal>
    //|<char-literal>
    //|<floating-literal>
    //|<function-call>
    // <项> :: = <因子>{ <乘法型运算符><因子> }
    // 需要补全
    std::optional<CompilationError> Analyser::analyseMultiplicativeExpression() {
        // 可以参考 <表达式> 实现
        auto err = analyseUnaryExpression();
        if (err.has_value())
            return err;

        //{ <乘法型运算符><因子> }
        while (true) {
            //预读
            auto next = nextToken();
            if (!next.has_value())
                return {};
            auto type = next.value().GetType();
            if (type != TokenType::MULTIPLICATION_SIGN && type != TokenType::DIVISION_SIGN) {
                unreadToken();
                return {};
            }

            //<因子>
            err = analyseUnaryExpression();
            if (err.has_value())
                return err;

            //根据结果生成指令
            //if (type == TokenType::MULTIPLICATION_SIGN)
            //_instructions.emplace_back(Operation::MUL, 0);
            //else if (type == TokenType::DIVISION_SIGN)
            //_instructions.emplace_back(Operation::DIV, 0);
        }

        return {};
    }

    //<function-call> ::=
    //<identifier> '(' [<expression-list>] ')'
    //<expression-list> ::=
    //<expression>{','<expression>}
    std::optional<CompilationError> Analyser::analyseFunctionCall() {
        //已经将function读入

        auto next = nextToken();
        if (!next.has_value() || next.value().GetType() != TokenType::LEFT_BRACKET)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedBracket);

        while (true) {
            auto next = nextToken();
            auto type = next.value().GetType();
            //没有 <expression-list>
            if (type != TokenType::MINUS_SIGN ||
                type != TokenType::PLUS_SIGN ||
                type != TokenType::IDENTIFIER ||
                type != TokenType::UNSIGNED_INTEGER ||
                type != TokenType::LEFT_BRACKET)
                break;

            auto err = analyseExpression();
            if (err.has_value())
                return err;

            next = nextToken();
            if (!next.has_value() || next.value().GetType() != TokenType::COMMA)
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedComma);
        }
    }

    //<unary-expression> ::=
    //[<unary-operator>]<primary-expression>
    //<primary-expression> ::=
    //'('<expression>')'
    //|<identifier>
    //|<integer-literal>
    //|<function-call>
    // <因子> ::= [<符号>]( <标识符> | <无符号整数> | '('<表达式>')' )
    // 需要补全
    // 因子是指令最重要的一步
    std::optional<CompilationError> Analyser::analyseUnaryExpression() {
        // [<unary-operator>]
        auto next = nextToken();
        auto prefix = 1;
        if (!next.has_value())
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);
        if (next.value().GetType() == TokenType::PLUS_SIGN)
            prefix = 1;
        else if (next.value().GetType() == TokenType::MINUS_SIGN) {
            prefix = -1;
//            _instructions.emplace_back(Operation::LIT, 0);
        } else
            unreadToken();

        // 预读
        next = nextToken();
        if (!next.has_value())
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);
        switch (next.value().GetType()) {
            // 这里和 <语句序列> 类似，需要根据预读结果调用不同的子程序
            // 但是要注意 default 返回的是一个编译错误
            case TokenType::IDENTIFIER: {
                //没有被声明
                if (!isDeclared(next.value().GetValueString()))
                    return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNotDeclared);
                //没有被初始化
                if (isUninitializedVariable(next.value().GetValueString()))
                    return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNotInitialized);

                if (isInitializedVariable(next.value().GetValueString())) {

                } else(isFunc(next.value().GetValueString()))
                {
                    auto err = analyseFunctionCall();
                    if (err.has_value())
                        return err;

                }
                //_instructions.emplace_back(Operation::LOD, getIndex(next.value().GetValueString()));//加载标识符的值
                break;
            }
            case TokenType::UNSIGNED_INTEGER: {
                //int len = next.value().GetValueString().length();
                //int64_t result_num = std::stol(next.value().GetValueString());
                //if (len > 10 || result_num > 4294967295)
                //return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIntegerOverflow);//溢出
                //_instructions.emplace_back(Operation::LIT, result_num);


                break;
            }
            case TokenType::LEFT_BRACKET: {
                auto err = analyseExpression();
                if (err.has_value())
                    return err;

                next = nextToken();
                if (!next.has_value() || next.value().GetType() != TokenType::RIGHT_BRACKET)
                    return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);
                break;//忘了
            }
            default:
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);
        }

        //取负
        //if (prefix == -1)
        //_instructions.emplace_back(Operation::SUB, 0);
        return {};
    }

    std::optional<Token> Analyser::nextToken() {
        if (_offset == _tokens.size())
            return {};
        // 考虑到 _tokens[0..._offset-1] 已经被分析过了
        // 所以我们选择 _tokens[0..._offset-1] 的 EndPos 作为当前位置
        _current_pos = _tokens[_offset].GetEndPos();
        return _tokens[_offset++];
    }

    void Analyser::unreadToken() {
        if (_offset == 0)
            DieAndPrint("analyser unreads token from the begining.");
        _current_pos = _tokens[_offset - 1].GetEndPos();
        _offset--;
    }

    void Analyser::_add(const Token &tk, std::map<std::string, int32_t> &mp) {
        if (tk.GetType() != TokenType::IDENTIFIER)
            DieAndPrint("only identifier can be added to the table.");
        mp[tk.GetValueString()] = _nextTokenIndex;//索引值每次增加
        _nextTokenIndex++;
    }

    void Analyser::addVariable(const Token &tk) {
        _add(tk, _vars);
    }

    void Analyser::addConstant(const Token &tk) {
        _add(tk, _consts);
    }

    void Analyser::addUninitializedVariable(const Token &tk) {
        _add(tk, _uninitialized_vars);
    }

//    void Analyser::addConstsTable(const Token &tk) {
//        _add(tk, _constsTable);
//    }

    void Analyser::addFunc(const Token &tk) {
        _add(tk, _funcs);
    }

    void Analyser::addGlobalConsts(const Token &tk) {
        _add(tk, _globalConsts);
    }

    void Analyser::addGlobalVars(const Token &tk) {
        _add(tk, _globalVars);
    }


    int32_t Analyser::getIndex(const std::string &s) {
        if (_uninitialized_vars.find(s) != _uninitialized_vars.end())
            return _uninitialized_vars[s];
        else if (_vars.find(s) != _vars.end())
            return _vars[s];
        else if (_funcs.find(s) != _funcs.end())
            return _funcs[s];
        else
            return _consts[s];
    }

    int32_t Analyser::getFuncsNameIndex(const std::string &s) {
        for (std::vector<Constants>::iterator iter = _constantsTable.begin(); iter != _constantsTable.end(); iter++) {
            if ((*iter).getValue() == s) { return (*iter).getIndex(); }
        }
    }

    void Analyser::changeFuncsParamsSize(std::string ident, int32_t number) {
        for (std::vector<Functions>::iterator iter = _functionsTable.begin(); iter != _functionsTable.end(); iter++) {
            if ((*iter).getValue() == ident) { (*iter).setParamsSize(number); }
        }
    }


    void Analyser::addLocalVarsToFunc(std::string ident, std::string value) {
        for (std::vector<Functions>::iterator iter = _functionsTable.begin(); iter != _functionsTable.end(); iter++) {
            if ((*iter).getValue() == ident) { (*iter).addLocalVars(value); }
        }
    }

    bool Analyser::isDeclared(const std::string &s) {
        return isConstant(s) || isUninitializedVariable(s) || isInitializedVariable(s);
    }

    bool Analyser::isUninitializedVariable(const std::string &s) {
        return _uninitialized_vars.find(s) != _uninitialized_vars.end();
    }

    bool Analyser::isInitializedVariable(const std::string &s) {
        return _vars.find(s) != _vars.end();
    }

    bool Analyser::isConstant(const std::string &s) {
        return _consts.find(s) != _consts.end();
    }

    bool Analyser::isFunc(const std::string &s) {
        return _funcs.find(s) != _consts.end();
    }
}
