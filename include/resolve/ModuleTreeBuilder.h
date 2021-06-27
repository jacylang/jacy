#ifndef JACY_RESOLVE_MODULETREEBUILDER_H
#define JACY_RESOLVE_MODULETREEBUILDER_H

#include "ast/StubVisitor.h"
#include "session/Session.h"
#include "suggest/SuggInterface.h"

#include "data_types/Option.h"
#include "data_types/SuggResult.h"

namespace jc::resolve {
    class ModuleTreeBuilder : public ast::StubVisitor, public sugg::SuggInterface {
    public:
        ModuleTreeBuilder() : StubVisitor("ScopeTreeBuilder") {}

        dt::SuggResult<dt::none_t> build(sess::sess_ptr sess, const ast::Party & party);

        void visit(const ast::File & file) override;
        void visit(const ast::Dir & dir) override;

        void visit(const ast::Enum & _enum) override;
        void visit(const ast::EnumEntry & enumEntry) override;
        void visit(const ast::Func & func) override;
        void visit(const ast::Impl & impl) override;
        void visit(const ast::Mod & mod) override;
        void visit(const ast::Struct & _struct) override;
        void visit(const ast::Trait & trait) override;
        void visit(const ast::TypeAlias & typeAlias) override;

        void visit(const ast::Block & block) override;

    private:
        using ast::StubVisitor::visit;

    private:
        sess::sess_ptr sess;
        common::Logger log{"ModuleTreeBuilder"};

        // Definitions //
    private:
        DefStorage defStorage;

        def_id define(const ast::id_ptr & ident, DefKind defKind);
        void defineGenerics(const ast::opt_type_params & maybeGenerics);

        // Modules //
    private:
        module_ptr mod;
        void enterBlock(node_id nodeId);
        void enterModule(const ast::id_ptr & ident, def_id defId);
        void enterFictiveModule(const std::string & name, DefKind defKind);
        void enterChildModule(module_ptr child);
        void exitMod();

        // Suggestions //
    private:
        void suggestCannotRedeclare(
            const std::string & name,
            const std::string & as,
            const std::string & declaredAs,
            node_id nodeId,
            node_id declaredHere
        );

        // Debug //
    private:
        dt::Option<std::string> curModuleName{dt::None};
    };
}

#endif // JACY_RESOLVE_MODULETREEBUILDER_H
