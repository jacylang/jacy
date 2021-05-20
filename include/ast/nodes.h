#ifndef JACY_NODES_H
#define JACY_NODES_H

// Fragments //
#include "ast/fragments/Attribute.h"
#include "ast/fragments/Delegation.h"
#include "ast/fragments/FuncParam.h"
#include "ast/fragments/Type.h"
#include "ast/fragments/TypeParams.h"

// Expressions //
#include "ast/expr/Assignment.h"
#include "ast/expr/Block.h"
#include "ast/expr/BorrowExpr.h"
#include "ast/expr/BreakExpr.h"
#include "ast/expr/ContinueExpr.h"
#include "ast/expr/DerefExpr.h"
#include "ast/expr/Identifier.h"
#include "ast/expr/IfExpr.h"
#include "ast/expr/Infix.h"
#include "ast/expr/Invoke.h"
#include "ast/expr/Lambda.h"
#include "ast/expr/ListExpr.h"
#include "ast/expr/LiteralConstant.h"
#include "ast/expr/LoopExpr.h"
#include "ast/expr/MemberAccess.h"
#include "ast/expr/ParenExpr.h"
#include "ast/expr/PathExpr.h"
#include "ast/expr/Prefix.h"
#include "ast/expr/QuestExpr.h"
#include "ast/expr/ReturnExpr.h"
#include "ast/expr/SpreadExpr.h"
#include "ast/expr/Subscript.h"
#include "ast/expr/SuperExpr.h"
#include "ast/expr/ThisExpr.h"
#include "ast/expr/TupleExpr.h"
#include "ast/expr/UnitExpr.h"
#include "ast/expr/WhenExpr.h"

// Statements //
#include "ast/stmt/EnumDecl.h"
#include "ast/stmt/ExprStmt.h"
#include "ast/stmt/ForStmt.h"
#include "ast/stmt/FuncDecl.h"
#include "ast/stmt/Impl.h"
#include "ast/stmt/Item.h"
#include "ast/stmt/Struct.h"
#include "ast/stmt/Trait.h"
#include "ast/stmt/TypeAlias.h"
#include "ast/stmt/VarDecl.h"
#include "ast/stmt/WhileStmt.h"

#include "ast/Node.h"

#endif // JACY_NODES_H
