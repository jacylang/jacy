cmake_minimum_required(VERSION 3.16)
project(Jacy)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories("${PROJECT_SOURCE_DIR}/include")
add_executable(${PROJECT_NAME} src/main.cpp include/parser/Parser.h src/parser/Parser.cpp include/parser/Token.h include/parser/Lexer.h src/parser/Lexer.cpp include/common/Error.h src/parser/Token.cpp include/core/Jacy.h src/core/Jacy.cpp include/utils/string.h src/utils/string.cpp include/common/Logger.h include/common/Logger.inl src/common/Logger.cpp include/tree/Node.h include/tree/BaseVisitor.h include/tree/expr/Expr.h include/tree/stmt/Stmt.h include/tree/stmt/ExprStmt.h include/tree/expr/LiteralConstant.h include/tree/expr/Infix.h include/tree/expr/Prefix.h include/tree/expr/Identifier.h include/tree/nodes.h include/common/common.h include/tree/stmt/VarDecl.h include/tree/expr/BreakExpr.h include/tree/expr/ContinueExpr.h include/tree/stmt/ClassDecl.h include/tree/fragments/TypeParams.h include/tree/expr/ThisExpr.h include/tree/expr/SuperExpr.h include/tree/fragments/Delegation.h include/tree/stmt/EnumDecl.h include/tree/stmt/ForStmt.h include/tree/stmt/WhileStmt.h include/tree/stmt/FuncDecl.h include/tree/stmt/DoWhileStmt.h include/tree/stmt/ObjectDecl.h include/tree/expr/TryCatchExpr.h include/tree/fragments/Block.h include/tree/expr/IfExpr.h include/tree/expr/ReturnExpr.h include/tree/expr/ThrowExpr.h include/tree/expr/WhenExpr.h include/tree/fragments/Type.h include/tree/fragments/FuncParam.h include/tree/fragments/Attribute.h include/tree/expr/Postfix.h include/tree/expr/Subscript.h include/utils/array.h include/tree/expr/Invoke.h include/tree/fragments/NamedList.h include/tree/expr/TupleExpr.h include/tree/expr/ListExpr.h include/tree/expr/ParenExpr.h include/tree/expr/SpreadExpr.h include/tree/stmt/Assignment.h include/tree/fragments/TupleType.h include/tree/stmt/TypeAlias.h)

target_include_directories(Jacy PRIVATE ${PROJECT_SOURCE_DIR}/include)
