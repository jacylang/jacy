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

        def_id define(const ast::id_ptr & ident, DefKind defKind) {
            const auto defId = defStorage.define(defKind, ident.span());
            const auto ns = Def::getNS(defStorage.getDef(defId).kind);
            log.dev(
                "Define '",
                ident.unwrap()->getValue(),
                "' in module with defId [",
                defId,
                "] in ",
                Module::nsToString(ns));
            const auto & name = ident.unwrap()->getValue();
            auto & map = mod->getNS(ns);
            if (utils::map::has(map, name)) {
                suggestErrorMsg("'" + name + "' has been already declared", ident.unwrap()->span);
            }
            map.emplace(name, defId);
            return defId;
        }

        void defineGenerics(const ast::opt_type_params & maybeGenerics);

        // Modules //
    private:
        module_ptr mod;
        void enterAnonMod(node_id nodeId, dt::Option<def_id> defId);
        void enterMod(const std::string & name, const dt::Option<ast::Span> & nameSpan, def_id defId);
        void exitMod();
    };
}

#endif // JACY_RESOLVE_MODULETREEBUILDER_H
