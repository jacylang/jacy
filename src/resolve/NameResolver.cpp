#include "resolve/NameResolver.h"

namespace jc::resolve {
    dt::SuggResult<dt::none_t> NameResolver::resolve(const sess::sess_ptr & sess, const ast::Party & party) {
        this->sess = sess;
        rootMod = sess->modTreeRoot.unwrap();

        party.getRootModule()->accept(*this);

        return {dt::None, extractSuggestions()};
    }

    void NameResolver::visit(const ast::FileModule & fileModule) {
        for (const auto & item : fileModule.getFile()->items) {
            item.accept(*this);
        }
    }

    void NameResolver::visit(const ast::DirModule & dirModule) {
        for (const auto & module : dirModule.getModules()) {
            module->accept(*this);
        }
    }

    void NameResolver::visit(const ast::Func & func) {
        uint32_t prevDepth = getDepth();
        visitTypeParams(func.typeParams);

        for (const auto & param : func.params) {
            param->type.accept(*this);
        }

        if (func.returnType) {
            func.returnType.unwrap().accept(*this);
        }

        for (const auto & param : func.params) {
            declare(param->name.unwrap()->getValue(), Name::Kind::Param, param->name.unwrap()->id);
        }

        if (func.oneLineBody) {
            func.oneLineBody.unwrap().accept(*this);
        } else {
            func.body.unwrap()->accept(*this);
        }

        liftToDepth(prevDepth);
    }

    void NameResolver::visit(const ast::Mod & mod) {
        enterRib();
        visitItems(mod.items);
        exitRib();
    }

    void NameResolver::visit(const ast::Struct & _struct) {
        visitTypeParams(_struct.typeParams);

        enterRib(); // -> (item rib)

        for (const auto & field : _struct.fields) {
            field->type.accept(*this);
        }

        exitRib();
    }

    void NameResolver::visit(const ast::UseDecl & useDecl) {
        resolveUseTree(useDecl.useTree);
    }

    void NameResolver::resolveUseTree(const ast::use_tree_ptr & maybeUseTree) {
        // TODO: Everything goes harder
    }

    // Statements //
    void NameResolver::visit(const ast::VarStmt & varStmt) {
        enterRib();
        declare(varStmt.name.unwrap()->getValue(), Name::Kind::Local, varStmt.id);
    }

    // Expressions //
    void NameResolver::visit(const ast::Block & block) {
        const auto prevDepth = getDepth();
        enterRib(); // -> block rib
        for (const auto & stmt : block.stmts) {
            stmt.accept(*this);
        }

        liftToDepth(prevDepth);
    }

    void NameResolver::visit(const ast::Lambda & lambdaExpr) {
        enterRib(); // -> (lambda params)

        for (const auto & param : lambdaExpr.params) {
            // TODO: Param name
            if (param->type) {
                param->type.unwrap().accept(*this);
            }
        }

        if (lambdaExpr.returnType) {
            lambdaExpr.returnType.unwrap().accept(*this);
        }

        lambdaExpr.body.accept(*this);

        exitRib(); // <- (lambda params)
    }

    void NameResolver::visit(const ast::PathExpr & pathExpr) {
        // TODO: global

//        if (pathExpr.segments.size() == 1) {
//            // Simplest case, we just got an identifier
//
//            resStorage.setRes(pathExpr.id, )
//        }
    }

    // Types //
    void NameResolver::visit(const ast::TypePath & typePath) {
        // TODO: !!!
    }

    // Extended visitors //
    void NameResolver::visitItems(const ast::item_list & members) {
        // At first we need to forward all declarations.
        // This is the work for ItemResolver.
        for (const auto & maybeMember : members) {
            const auto & member = maybeMember.unwrap();
            std::string name;
            Name::Kind kind;
            switch (member->kind) {
                case ast::ItemKind::Func: {
                    name = std::static_pointer_cast<ast::Func>(member)->name.unwrap()->getValue();
                    kind = Name::Kind::Func;
                    break;
                }
                case ast::ItemKind::Enum: {
                    name = std::static_pointer_cast<ast::Enum>(member)->name.unwrap()->getValue();
                    kind = Name::Kind::Enum;
                    break;
                }
                case ast::ItemKind::Struct: {
                    name = std::static_pointer_cast<ast::Struct>(member)->name.unwrap()->getValue();
                    kind = Name::Kind::Struct;
                    break;
                }
                case ast::ItemKind::TypeAlias: {
                    name = std::static_pointer_cast<ast::TypeAlias>(member)->name.unwrap()->getValue();
                    kind = Name::Kind::TypeAlias;
                    break;
                }
                case ast::ItemKind::Trait: {
                    name = std::static_pointer_cast<ast::Trait>(member)->name.unwrap()->getValue();
                    kind = Name::Kind::Trait;
                    break;
                }
                default: continue;
            }
            declare(name, kind, member->id);
        }

        // Then we resolve the signatures and bodies
        // This is done here -- in NameResolver
        for (const auto & member : members) {
            member.accept(*this);
        }
    }

    void NameResolver::visitTypeParams(const ast::opt_type_params & maybeTypeParams) {
        if (!maybeTypeParams) {
            return;
        }
        const auto & typeParams = maybeTypeParams.unwrap();
        for (const auto & typeParam : typeParams) {
            if (typeParam->kind == ast::TypeParamKind::Type) {
                declare(
                    std::static_pointer_cast<ast::GenericType>(typeParam)->name.unwrap()->getValue(),
                    Name::Kind::TypeParam,
                    typeParam->id
                );
            }
        }
        for (const auto & typeParam : typeParams) {
            if (typeParam->kind == ast::TypeParamKind::Lifetime) {
                declare(
                    std::static_pointer_cast<ast::Lifetime>(typeParam)->name.unwrap()->getValue(),
                    Name::Kind::Lifetime,
                    typeParam->id
                );
            }
        }
        for (const auto & typeParam : typeParams) {
            if (typeParam->kind == ast::TypeParamKind::Const) {
                declare(
                    std::static_pointer_cast<ast::ConstParam>(typeParam)->name.unwrap()->getValue(),
                    Name::Kind::ConstParam,
                    typeParam->id
                );
            }
        }
    }

    // Ribs //
    uint32_t NameResolver::getDepth() const {
        return depth;
    }

    const rib_ptr & NameResolver::curRib() const {
        try {
            return ribStack.at(depth);
        } catch (std::exception & e) {
            common::Logger::devPanic("Called `NameResolver::curRib` with depth out of `ribStack` bounds:", depth);
        }
    }

    opt_rib NameResolver::ribAt(size_t d) const {
        if (d >= ribStack.size()) {
            return dt::None;
        }
        return ribStack.at(d);
    }

    void NameResolver::enterRib(Rib::Kind kind) {
        ribStack.push_back(std::make_unique<Rib>(kind));
        if (depth == UINT32_MAX) {
            Logger::devPanic("Maximum ribStack depth limit exceeded");
        }
        depth = static_cast<uint32_t>(ribStack.size() - 1);
    }

    void NameResolver::exitRib() {
        if (depth == 0) {
            Logger::devPanic("NameResolver: Tried to exit top-level rib");
        }
        depth--;
    }

    void NameResolver::liftToDepth(size_t prevDepth) {
        if (prevDepth > depth) {
            common::Logger::devPanic("Called `NameResolver::lifeToDepth` with `prevDepth` > `depth`");
        }

        // Note: Save depth when we started, because it will be changed in `exitRib`
        const auto curDepth = depth;
        for (size_t i = prevDepth; i < curDepth; i++) {
            exitRib();
        }
    }

    // Declarations //
    void NameResolver::declare(const std::string & name, Name::Kind kind, ast::node_id nodeId) {
        const auto & redecl = curRib()->declare(name, kind, nodeId);

        if (redecl) {
            suggestCannotRedeclare(
                name,
                Name::kindStr(kind),
                redecl.unwrap()->kindStr(),
                nodeId,
                redecl.unwrap()->nodeId
            );
        }
    }

    // Resolution //
    void NameResolver::resolveSimplePath(const ast::simple_path_ptr & simplePath) {
        // TODO
    }

    // Suggestions //
    void NameResolver::suggestCannotRedeclare(
        const std::string & name,
        const std::string & as,
        const std::string & declaredAs,
        ast::node_id nodeId,
        ast::node_id declaredHere
    ) {
        suggest(
            std::make_unique<sugg::MsgSpanLinkSugg>(
                "Cannot redeclare '" + name + "' as " + as,
                sess->nodeMap.getNodeSpan(nodeId),
                "Because it is already declared as " + declaredAs + " here",
                sess->nodeMap.getNodeSpan(declaredHere),
                SuggKind::Error
            )
        );
    }
}
