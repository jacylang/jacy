#ifndef JACY_NODES_H
#define JACY_NODES_H

// Fragments //
#include "ast/fragments/Attribute.h"
#include "ast/fragments/Block.h"
#include "ast/fragments/Delegation.h"
#include "ast/fragments/FuncParam.h"
#include "ast/fragments/Type.h"
#include "ast/fragments/TypeParams.h"

// Expressions //
#include "ast/expr/BreakExpr.h"
#include "ast/expr/ContinueExpr.h"
#include "ast/expr/Identifier.h"
#include "ast/expr/IfExpr.h"
#include "ast/expr/Infix.h"
#include "ast/expr/Invoke.h"
#include "ast/expr/ListExpr.h"
#include "ast/expr/LiteralConstant.h"
#include "ast/expr/LoopExpr.h"
#include "ast/expr/ParenExpr.h"
#include "ast/expr/Postfix.h"
#include "ast/expr/Prefix.h"
#include "ast/expr/ReturnExpr.h"
#include "ast/expr/SpreadExpr.h"
#include "ast/expr/Subscript.h"
#include "ast/expr/SuperExpr.h"
#include "ast/expr/ThisExpr.h"
#include "ast/expr/ThrowExpr.h"
#include "ast/expr/TryCatchExpr.h"
#include "ast/expr/TupleExpr.h"
#include "ast/expr/UnitExpr.h"
#include "ast/expr/WhenExpr.h"

// Statements //
#include "ast/stmt/Assignment.h"
#include "ast/stmt/ClassDecl.h"
#include "ast/stmt/EnumDecl.h"
#include "ast/stmt/ExprStmt.h"
#include "ast/stmt/ForStmt.h"
#include "ast/stmt/FuncDecl.h"
#include "ast/stmt/ObjectDecl.h"
#include "ast/stmt/TypeAlias.h"
#include "ast/stmt/VarDecl.h"
#include "ast/stmt/WhileStmt.h"

#include "ast/Node.h"

#endif // JACY_NODES_H
