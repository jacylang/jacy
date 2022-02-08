#include "hir/nodes/exprs.h"

namespace jc::hir {
    std::string binOpStr(BinOpKind binOp) {
        switch (binOp) {
            case BinOpKind::Add:
                return "+";
            case BinOpKind::Sub:
                return "-";
            case BinOpKind::Mul:
                return "*";
            case BinOpKind::Div:
                return "/";
            case BinOpKind::Rem:
                return "%";
            case BinOpKind::Pow:
                return "**";
            case BinOpKind::And:
                return "and";
            case BinOpKind::Or:
                return "or";
            case BinOpKind::Xor:
                return "^";
            case BinOpKind::BitAnd:
                return "&";
            case BinOpKind::BitOr:
                return "|";
            case BinOpKind::Shl:
                return "<<";
            case BinOpKind::Shr:
                return ">>";
            case BinOpKind::Eq:
                return "==";
            case BinOpKind::LT:
                return "<";
            case BinOpKind::LE:
                return "<=";
            case BinOpKind::GE:
                return ">=";
            case BinOpKind::GT:
                return ">";
            case BinOpKind::Spaceship:
                return "<=>";
            case BinOpKind::NE:
                return "!=";
        }
    }
}

