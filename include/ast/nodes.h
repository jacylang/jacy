#ifndef JACY_NODES_H
#define JACY_NODES_H

// Expressions //
#include "ast/expr/Assignment.h"
#include "ast/expr/Block.h"
#include "ast/expr/BorrowExpr.h"
#include "ast/expr/BreakExpr.h"
#include "ast/expr/ContinueExpr.h"
#include "ast/expr/DerefExpr.h"
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
#include "ast/expr/ThisExpr.h"
#include "ast/expr/TupleExpr.h"
#include "ast/expr/UnitExpr.h"
#include "ast/expr/WhenExpr.h"

// Statements //
#include "ast/stmt/ExprStmt.h"
#include "ast/stmt/ForStmt.h"
#include "ast/stmt/ItemStmt.h"
#include "ast/stmt/VarStmt.h"
#include "ast/stmt/WhileStmt.h"

// Items //
#include "ast/item/Enum.h"
#include "ast/item/Func.h"
#include "ast/item/Impl.h"
#include "ast/item/Item.h"
#include "ast/item/Mod.h"
#include "ast/item/Struct.h"
#include "ast/item/Trait.h"
#include "ast/item/TypeAlias.h"

#include "ast/Node.h"

#endif // JACY_NODES_H
