if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
    message("Applied debug options")
    set(CMAKE_VERBOSE_MAKEFILE on)
endif ()

cmake_minimum_required(VERSION 3.16)
project(Jacy)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O0")

add_compile_options(
        # WARNINGS #
        -Wall
    #   Uncomment this line when want to suffer a little bit
    #    -Werror
        -pedantic
        -pedantic-errors
        -Wextra
        -Wpedantic
        -Wcast-align
        -Wcast-qual
        -Wconversion
        -Wctor-dtor-privacy
    #   Stupid thing that just warns mostly about things that are right
    #    -Weffc++
        -Wenum-compare
        -Wfloat-equal
        -Wnon-virtual-dtor
        -Wold-style-cast
        -Woverloaded-virtual
        -Wredundant-decls
    #    I always shadow names of fields in constructors, so... meow
    #    -Wshadow
        -Wsign-conversion
        -Wsign-promo
        -Wstrict-aliasing
        -ansi
        -Wunreachable-code
        -Wunused
        -Wunused-parameter

        # For dev
        -Wno-unused-function

        -Wvariadic-macros
        -Wwrite-strings

        #
#        -ftime-report
)

if (${CMAKE_BUILD_TYPE} MATCHES "Release")
    add_compile_options(
            -Werror
    )
endif ()

include_directories("${PROJECT_SOURCE_DIR}/include")
add_executable(${PROJECT_NAME} src/main.cpp include/parser/Parser.h src/parser/Parser.cpp include/parser/Token.h include/parser/Lexer.h src/parser/Lexer.cpp include/common/Error.h src/parser/Token.cpp include/core/Jacy.h src/core/Jacy.cpp include/utils/str.h src/utils/str.cpp include/common/Logger.h include/common/Logger.inl src/common/Logger.cpp include/ast/Node.h include/ast/BaseVisitor.h include/ast/expr/Expr.h include/ast/stmt/Stmt.h include/ast/stmt/ExprStmt.h include/ast/expr/LiteralConstant.h include/ast/expr/Infix.h include/ast/expr/Prefix.h include/ast/fragments/Identifier.h include/ast/nodes.h include/ast/stmt/LetStmt.h include/ast/expr/BreakExpr.h include/ast/expr/ContinueExpr.h include/ast/fragments/TypeParams.h include/ast/expr/SelfExpr.h include/ast/item/Enum.h include/ast/stmt/ForStmt.h include/ast/stmt/WhileStmt.h include/ast/item/Func.h include/ast/expr/Block.h include/ast/expr/IfExpr.h include/ast/expr/ReturnExpr.h include/ast/expr/WhenExpr.h include/ast/fragments/Type.h include/ast/fragments/Attribute.h include/ast/expr/Subscript.h include/utils/arr.h include/ast/expr/Invoke.h include/ast/fragments/NamedList.h include/ast/expr/TupleExpr.h include/ast/expr/ListExpr.h include/ast/expr/ParenExpr.h include/ast/expr/SpreadExpr.h include/ast/expr/Assignment.h include/ast/item/TypeAlias.h include/ast/AstPrinter.h src/ast/AstPrinter.cpp include/ast/expr/LoopExpr.h include/ast/expr/UnitExpr.h include/cli/CLI.h src/cli/CLI.cpp include/cli/Args.h include/utils/map.h src/utils/map.cpp src/cli/Args.cpp src/utils/arr.cpp include/span/Span.h include/parser/ParserSugg.h include/session/Session.h include/suggest/BaseSugg.h include/suggest/Explain.h include/span/Span.h include/ast/Validator.h src/ast/Validator.cpp include/suggest/Suggester.h src/suggest/Suggester.cpp include/data_types/Option.h include/ast/Party.h include/data_types/Result.h include/data_types/SuggResult.h include/ast/item/Struct.h include/ast/item/Impl.h include/ast/item/Trait.h include/ast/item/Item.h include/suggest/BaseSuggester.h include/suggest/SuggDumper.h src/suggest/SuggDumper.cpp include/ast/expr/BorrowExpr.h include/ast/expr/DerefExpr.h include/ast/expr/QuestExpr.h include/ast/expr/MemberAccess.h include/ast/expr/Lambda.h include/resolve/NameResolver.h include/ast/StubVisitor.h src/ast/StubVisitor.cpp include/resolve/Rib.h src/resolve/NameResolver.cpp src/resolve/Rib.cpp include/ast/stmt/ItemStmt.h include/ast/item/Mod.h include/core/Interface.h src/core/Interface.cpp include/common/Config.h src/common/Config.cpp src/session/Session.cpp include/parser/ParseSess.h include/session/SourceMap.h src/session/SourceMap.cpp include/utils/rand.h include/utils/hash.h include/ast/NodeMap.h src/ast/NodeMap.cpp include/ast/fragments/Pattern.h include/resolve/Module.h include/fs/Entry.h src/fs/fs.cpp include/ast/item/UseDecl.h include/ast/fragments/SimplePath.h include/parser/ParseResult.h include/ast/DirTreePrinter.h src/ast/DirTreePrinter.cpp include/resolve/ModuleTreeBuilder.h src/resolve/ModuleTreeBuilder.cpp src/resolve/Module.cpp include/suggest/SuggInterface.h src/suggest/SuggInterface.cpp include/platform/signals.h include/resolve/ResStorage.h include/resolve/Definition.h)

#target_include_directories(Jacy PRIVATE ${PROJECT_SOURCE_DIR}/include)

message("Running on ${CMAKE_SYSTEM_NAME}")

set(CMAKE_CXX_FLAGS "-std=c++17")
