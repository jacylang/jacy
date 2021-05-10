cmake_minimum_required(VERSION 3.16)
project(Jacy)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_CXX_FLAGS "-std=c++17")

include_directories("${PROJECT_SOURCE_DIR}/include")
add_executable(${PROJECT_NAME} src/main.cpp include/parser/Parser.h src/parser/Parser.cpp include/parser/Token.h include/parser/Lexer.h src/parser/Lexer.cpp include/common/Error.h src/parser/Token.cpp include/core/Jacy.h src/core/Jacy.cpp include/utils/str.h src/utils/str.cpp include/common/Logger.h include/common/Logger.inl src/common/Logger.cpp include/ast/Node.h include/ast/BaseVisitor.h include/ast/expr/Expr.h include/ast/stmt/Stmt.h include/ast/stmt/ExprStmt.h include/ast/expr/LiteralConstant.h include/ast/expr/Infix.h include/ast/expr/Prefix.h include/ast/expr/Identifier.h include/ast/nodes.h include/common/common.h include/ast/stmt/VarDecl.h include/ast/expr/BreakExpr.h include/ast/expr/ContinueExpr.h include/ast/stmt/ClassDecl.h include/ast/fragments/TypeParams.h include/ast/expr/ThisExpr.h include/ast/expr/SuperExpr.h include/ast/fragments/Delegation.h include/ast/stmt/EnumDecl.h include/ast/stmt/ForStmt.h include/ast/stmt/WhileStmt.h include/ast/stmt/FuncDecl.h include/ast/stmt/ObjectDecl.h include/ast/expr/TryCatchExpr.h include/ast/fragments/Block.h include/ast/expr/IfExpr.h include/ast/expr/ReturnExpr.h include/ast/expr/ThrowExpr.h include/ast/expr/WhenExpr.h include/ast/fragments/Type.h include/ast/fragments/FuncParam.h include/ast/fragments/Attribute.h include/ast/expr/Postfix.h include/ast/expr/Subscript.h include/utils/arr.h include/ast/expr/Invoke.h include/ast/fragments/NamedList.h include/ast/expr/TupleExpr.h include/ast/expr/ListExpr.h include/ast/expr/ParenExpr.h include/ast/expr/SpreadExpr.h include/ast/stmt/Assignment.h include/ast/fragments/TupleType.h include/ast/stmt/TypeAlias.h include/ast/AstPrinter.h src/ast/AstPrinter.cpp include/ast/expr/LoopExpr.h include/ast/expr/UnitExpr.h include/cli/CLI.h src/cli/CLI.cpp include/cli/Config.h include/utils/map.h src/utils/map.cpp src/cli/Config.cpp include/dev/DevConfig.h src/dev/DevConfig.cpp src/utils/arr.cpp include/span/Span.h include/parser/ParserSugg.h include/session/Session.h include/explain/Suggestion.h include/explain/Explain.h include/span/Span.h)

target_include_directories(Jacy PRIVATE ${PROJECT_SOURCE_DIR}/include)
