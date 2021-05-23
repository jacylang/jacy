#ifndef JACY_RESOLVE_BASERESOLVER_H
#define JACY_RESOLVE_BASERESOLVER_H

#include "ast/StubVisitor.h"
#include "suggest/BaseSugg.h"
#include "span/Span.h"
#include "resolve/Name.h"

namespace jc::resolve {
    using span::Span;
    using sugg::SuggKind;
    using sugg::eid_t;
    using common::Logger;

    class BaseResolver : public ast::StubVisitor {
    public:
        BaseResolver(const std::string & name, sess::sess_ptr sess)
            : sess(sess), StubVisitor(name, ast::StubVisitorMode::Stub) {}
        friend class NameResolver;

    public:
        sugg::sugg_list extractSuggestions();

        // Ribs //
    protected:
        rib_ptr rib;
        void acceptRib(rib_ptr newRib);

        // Suggestions //
    protected:
        sess::sess_ptr sess;
        sugg::sugg_list suggestions;
        void suggest(sugg::sugg_ptr suggestion);
        void suggest(const std::string & msg, ast::node_id nodeId, SuggKind kind, eid_t eid = sugg::NoneEID);
        void suggestErrorMsg(const std::string & msg, ast::node_id nodeId, eid_t eid = sugg::NoneEID);
        void suggestWarnMsg(const std::string & msg, ast::node_id nodeId, eid_t eid = sugg::NoneEID);
        void suggestHelp(const std::string & helpMsg, sugg::sugg_ptr sugg);
        void suggestCannotRedeclare(
            const std::string & name,
            const std::string & as,
            const std::string & declaredAs,
            ast::node_id nodeId,
            ast::node_id declaredHere
        );
    };
}

#endif // JACY_RESOLVE_BASERESOLVER_H
