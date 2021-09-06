#include "span/Symbol.h"

namespace jc::span {
    // Note[IMPORTANT]: When adding new keyword, update `Interner::isKw`-like methods if needed
    const std::map<Kw, std::string> Symbol::keywords = {
        {Kw::Empty,      ""},
        {Kw::Root,       "[ROOT]"},
        {Kw::Underscore, "_"},

        {Kw::And,        "and"},
        {Kw::As,         "as"},
        {Kw::Async,      "async"},
        {Kw::Await,      "await"},
        {Kw::Break,      "break"},
        {Kw::Const,      "const"},
        {Kw::Continue,   "continue"},
        {Kw::Do,         "do"},
        {Kw::Elif,       "elif"},
        {Kw::Else,       "else"},
        {Kw::Enum,       "enum"},
        {Kw::False,      "false"},
        {Kw::For,        "for"},
        {Kw::Func,       "func"},
        {Kw::If,         "if"},
        {Kw::Impl,       "impl"},
        {Kw::Import,     "import"},
        {Kw::In,         "in"},
        {Kw::Infix,      "infix"},
        {Kw::Init,       "init"},
        {Kw::Loop,       "loop"},
        {Kw::Match,      "match"},
        {Kw::Mod,        "mod"},
        {Kw::Move,       "move"},
        {Kw::Mut,        "mut"},
        {Kw::Not,        "not"},
        {Kw::Of,         "of"},
        {Kw::Or,         "or"},
        {Kw::Return,     "return"},
        {Kw::Party,      "party"},
        {Kw::Pub,        "pub"},
        {Kw::Ref,        "ref"},
        {Kw::Self,       "self"},
        {Kw::Static,     "static"},
        {Kw::Struct,     "struct"},
        {Kw::Super,      "super"},
        {Kw::This,       "this"},
        {Kw::Trait,      "trait"},
        {Kw::True,       "true"},
        {Kw::Type,       "type"},
        {Kw::Use,        "use"},
        {Kw::Let,        "let"},
        {Kw::Where,      "where"},
        {Kw::While,      "while"},
    };

    Symbol Symbol::intern(const std::string & str) {
        return Interner::getInstance().intern(str);
    }

    std::string Symbol::toString() const {
        return Interner::getInstance().getString(*this);
    }

    std::string Symbol::kwToString(Kw kw) {
        return Interner::getInstance().getString(fromKw(kw));
    }

    Interner::Interner() {
        SymbolId::ValueT index {0};
        for (const auto & kw : Symbol::keywords) {
            auto kwSym = intern(kw.second);
            if (kwSym.id.val != static_cast<std::underlying_type<Kw>::type>(kw.first)) {
                log::devPanic(
                    "Invalid KW (keyword) discriminant '",
                    kwSym.id.val,
                    "' vs valid '",
                    index,
                    "' for keyword '",
                    kw.second,
                    "'");
            }
            index++;
        }
    }
}
