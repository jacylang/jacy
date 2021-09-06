#include "span/Symbol.h"

namespace jc::span {
    // Note[IMPORTANT]: When adding new keyword, update `Interner::isKw`-like methods if needed
    const std::map<KW, std::string> Symbol::keywords = {
        {KW::Empty,      ""},
        {KW::Root,       "[ROOT]"},
        {KW::Underscore, "_"},

        {KW::And,        "and"},
        {KW::As,         "as"},
        {KW::Async,      "async"},
        {KW::Await,      "await"},
        {KW::Break,      "break"},
        {KW::Const,      "const"},
        {KW::Continue,   "continue"},
        {KW::Do,         "do"},
        {KW::Elif,       "elif"},
        {KW::Else,       "else"},
        {KW::Enum,       "enum"},
        {KW::False,      "false"},
        {KW::For,        "for"},
        {KW::Func,       "func"},
        {KW::If,         "if"},
        {KW::Impl,       "impl"},
        {KW::Import,     "import"},
        {KW::In,         "in"},
        {KW::Infix,      "infix"},
        {KW::Init,       "init"},
        {KW::Loop,       "loop"},
        {KW::Match,      "match"},
        {KW::Mod,        "mod"},
        {KW::Move,       "move"},
        {KW::Mut,        "mut"},
        {KW::Not,        "not"},
        {KW::Of,         "of"},
        {KW::Or,         "or"},
        {KW::Return,     "return"},
        {KW::Party,      "party"},
        {KW::Pub,        "pub"},
        {KW::Ref,        "ref"},
        {KW::Self,       "self"},
        {KW::Static,     "static"},
        {KW::Struct,     "struct"},
        {KW::Super,      "super"},
        {KW::This,       "this"},
        {KW::Trait,      "trait"},
        {KW::True,       "true"},
        {KW::Type,       "type"},
        {KW::Use,        "use"},
        {KW::Let,        "let"},
        {KW::Where,      "where"},
        {KW::While,      "while"},
    };

    auto Symbol::intern(const std::string & str) {
        return Interner::getInstance().intern(str);
    }

    std::string Symbol::toString() const {
        return Interner::getInstance().getString(*this);
    }

    std::string Symbol::kwToString(KW kw) {
        return Interner::getInstance().getString(fromKw(kw));
    }

    Interner::Interner() {
        SymbolId::ValueT index {0};
        for (const auto & kw : Symbol::keywords) {
            auto kwSym = intern(kw.second);
            if (kwSym.id.val != static_cast<std::underlying_type<KW>::type>(kw.first)) {
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
