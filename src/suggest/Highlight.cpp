#include "suggest/Highlight.h"

namespace jc::sugg {
    std::string Highlight::highlight(const std::string & source) {
        parser::Lexer lexer;
        auto tokens = lexer.lexInternal(source);

        for (const auto & t : tokens) {
            switch (t.kind) {
                case parser::TokenKind::Eof: break;
                case parser::TokenKind::Lit: {

                    break;
                }
                case parser::TokenKind::Id:
                    break;
                case parser::TokenKind::Assign:
                    break;
                case parser::TokenKind::AddAssign:
                    break;
                case parser::TokenKind::SubAssign:
                    break;
                case parser::TokenKind::MulAssign:
                    break;
                case parser::TokenKind::DivAssign:
                    break;
                case parser::TokenKind::ModAssign:
                    break;
                case parser::TokenKind::PowerAssign:
                    break;
                case parser::TokenKind::ShlAssign:
                    break;
                case parser::TokenKind::ShrAssign:
                    break;
                case parser::TokenKind::BitAndAssign:
                    break;
                case parser::TokenKind::BitOrAssign:
                    break;
                case parser::TokenKind::XorAssign:
                    break;
                case parser::TokenKind::Add:
                    break;
                case parser::TokenKind::Sub:
                    break;
                case parser::TokenKind::Mul:
                    break;
                case parser::TokenKind::Div:
                    break;
                case parser::TokenKind::Rem:
                    break;
                case parser::TokenKind::Power:
                    break;
                case parser::TokenKind::Or:
                    break;
                case parser::TokenKind::And:
                    break;
                case parser::TokenKind::Shl:
                    break;
                case parser::TokenKind::Shr:
                    break;
                case parser::TokenKind::Ampersand:
                    break;
                case parser::TokenKind::BitOr:
                    break;
                case parser::TokenKind::Xor:
                    break;
                case parser::TokenKind::Inv:
                    break;
                case parser::TokenKind::Eq:
                    break;
                case parser::TokenKind::NotEq:
                    break;
                case parser::TokenKind::LAngle:
                    break;
                case parser::TokenKind::RAngle:
                    break;
                case parser::TokenKind::LE:
                    break;
                case parser::TokenKind::GE:
                    break;
                case parser::TokenKind::Spaceship:
                    break;
                case parser::TokenKind::RefEq:
                    break;
                case parser::TokenKind::RefNotEq:
                    break;
                case parser::TokenKind::Range:
                    break;
                case parser::TokenKind::RangeEQ:
                    break;
                case parser::TokenKind::Dot:
                    break;
                case parser::TokenKind::Path:
                    break;
                case parser::TokenKind::Spread:
                    break;
                case parser::TokenKind::Pipe:
                    break;
                case parser::TokenKind::Dollar:
                    break;
                case parser::TokenKind::At:
                    break;
                case parser::TokenKind::Backslash:
                    break;
                case parser::TokenKind::Semi:
                    break;
                case parser::TokenKind::Arrow:
                    break;
                case parser::TokenKind::DoubleArrow:
                    break;
                case parser::TokenKind::LParen:
                    break;
                case parser::TokenKind::RParen:
                    break;
                case parser::TokenKind::LBrace:
                    break;
                case parser::TokenKind::RBrace:
                    break;
                case parser::TokenKind::LBracket:
                    break;
                case parser::TokenKind::RBracket:
                    break;
                case parser::TokenKind::Comma:
                    break;
                case parser::TokenKind::Colon:
                    break;
                case parser::TokenKind::Quest:
                    break;
                case parser::TokenKind::Backtick: {

                    break;
                }
                case parser::TokenKind::None: break;
            }
        }
    }
}
