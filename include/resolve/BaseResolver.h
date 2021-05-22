#ifndef JACY_RESOLVE_BASERESOLVER_H
#define JACY_RESOLVE_BASERESOLVER_H

#include "ast/StubVisitor.h"
#include "suggest/BaseSugg.h"
#include "span/Span.h"

namespace jc::resolve {
    using span::Span;
    using sugg::SuggKind;
    using sugg::eid_t;

    class BaseResolver : public ast::StubVisitor {
    public:
        BaseResolver(const std::string & name) : StubVisitor(name, ast::StubVisitorMode::Stub) {}

        sugg::sugg_list extractSuggestions();

        // Suggestions //
    private:
        sess::sess_ptr sess;
        sugg::sugg_list suggestions;
        void suggest(sugg::sugg_ptr suggestion);
        void suggest(const std::string & msg, const Span & span, SuggKind kind, eid_t eid = sugg::NoneEID);
        void suggestErrorMsg(const std::string & msg, const Span & span, eid_t eid = sugg::NoneEID);
        void suggestWarnMsg(const std::string & msg, const Span & span, eid_t eid = sugg::NoneEID);
        void suggestHelp(const std::string & helpMsg, sugg::sugg_ptr sugg);
    };
}

#endif // JACY_RESOLVE_BASERESOLVER_H
