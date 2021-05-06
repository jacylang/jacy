#ifndef JACY_NODES_H
#define JACY_NODES_H

// Fragments //
#include "tree/fragments/Attribute.h"
#include "tree/fragments/Block.h"
#include "tree/fragments/Delegation.h"
#include "tree/fragments/FuncParam.h"
#include "tree/fragments/Type.h"
#include "tree/fragments/TypeParams.h"

// Expressions //
#include "tree/expr/BreakExpr.h"
#include "tree/expr/ContinueExpr.h"
#include "tree/expr/Identifier.h"
#include "tree/expr/IfExpr.h"
#include "tree/expr/Infix.h"
#include "tree/expr/Invoke.h"
#include "tree/expr/ListExpr.h"
#include "tree/expr/LiteralConstant.h"
#include "tree/expr/ParenExpr.h"
#include "tree/expr/Postfix.h"
#include "tree/expr/Prefix.h"
#include "tree/expr/ReturnExpr.h"
#include "tree/expr/SpreadExpr.h"
#include "tree/expr/Subscript.h"
#include "tree/expr/SuperExpr.h"
#include "tree/expr/ThisExpr.h"
#include "tree/expr/ThrowExpr.h"
#include "tree/expr/TryCatchExpr.h"
#include "tree/expr/TupleExpr.h"
#include "tree/expr/WhenExpr.h"

// Statements //
#include "tree/stmt/Assignment.h"
#include "tree/stmt/ClassDecl.h"
#include "tree/stmt/DoWhileStmt.h"
#include "tree/stmt/EnumDecl.h"
#include "tree/stmt/ExprStmt.h"
#include "tree/stmt/ForStmt.h"
#include "tree/stmt/FuncDecl.h"
#include "tree/stmt/ObjectDecl.h"
#include "tree/stmt/TypeAlias.h"
#include "tree/stmt/VarDecl.h"
#include "tree/stmt/WhileStmt.h"

#include "tree/Node.h"

#endif // JACY_NODES_H
