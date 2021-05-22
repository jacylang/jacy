#include "resolve/BaseResolver.h"

namespace jc::resolve {
    void BaseResolver::setSession(sess::sess_ptr sess) {
        this->sess = sess;
    }

    sugg::sugg_list BaseResolver::extractSuggestions() {
        return std::move(suggestions);
    }

    // Ribs //
    void BaseResolver::acceptRib(rib_ptr newRib) {
        rib = newRib;
    }

    // Suggestion //
    void BaseResolver::suggest(sugg::sugg_ptr suggestion) {
        suggestions.emplace_back(std::move(suggestion));
    }

    void BaseResolver::suggest(const std::string & msg, ast::node_id nodeId, SuggKind kind, eid_t eid) {
        suggest(std::make_unique<sugg::MsgSugg>(msg, ast::Node::nodeMap.getNodeSpan(nodeId), kind, eid));
    }

    void BaseResolver::suggestErrorMsg(const std::string & msg, ast::node_id nodeId, eid_t eid) {
        suggest(msg, nodeId, SuggKind::Error, eid);
    }

    void BaseResolver::suggestWarnMsg(const std::string & msg, ast::node_id nodeId, eid_t eid) {
        suggest(msg, nodeId, SuggKind::Warn, eid);
    }

    void BaseResolver::suggestHelp(const std::string & helpMsg, sugg::sugg_ptr sugg) {
        suggest(std::make_unique<sugg::HelpSugg>(helpMsg, std::move(sugg)));
    }

    void BaseResolver::suggestCannotRedeclare(
        const std::string & name,
        const std::string & as,
        const std::string & declaredAs,
        ast::node_id nodeId
    ) {
        suggestErrorMsg(
            Logger::format(
                "Cannot redeclare '" + name + "' as", as + ", because it is already declared as", declaredAs, "here"
            ),
            nodeId
        );
    }
}
