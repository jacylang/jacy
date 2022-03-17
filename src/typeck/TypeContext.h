#ifndef JACY_SRC_TYPECK_TYPECONTEXT_H
#define JACY_SRC_TYPECK_TYPECONTEXT_H

#include "typeck/type/types.h"
#include "typeck/convert/Converter.h"

namespace jc::typeck {
    using ast::NodeId;

    class TypeContext {
    public:
        using TypeMap = std::map<size_t, Ty>;

    public:
        TypeContext() = default;

        auto & converter() {
            return _converter;
        }

        // Interning //
    public:
        Ty intern(TypeKind::Ptr && kind) {
            auto hash = Type::hashKind(kind);
            auto found = types.find(hash);
            if (found != types.end()) {
                return found->second;
            }
            auto ty = std::make_shared<Type>(std::move(kind));
            types.emplace(hash, ty);
            return ty;
        }

        Ty makeType(TypeKind::Ptr && kind) {
            return intern(std::move(kind));
        }

        Ty makeBottom() {
            return makeType(std::make_unique<Bottom>());
        }

        Infer::Var nextTypeVar() {
            return lastTypeVar++;
        }

        Ty makeInferVar() {
            return makeType(std::make_unique<Infer>(nextTypeVar()));
        }

        Ty makeBool() {
            return makeType(std::make_unique<Bool>());
        }

        Ty makeInt(Int::Kind kind) {
            return makeType(std::make_unique<Int>(kind));
        }

        Ty makeFloat(Float::Kind kind) {
            return makeType(std::make_unique<Float>(kind));
        }

        Ty makeChar() {
            return makeType(std::make_unique<Char>());
        }

        Ty makeStr() {
            return makeType(std::make_unique<Str>());
        }

        Ty makeUnit() {
            return makeType(std::make_unique<Unit>());
        }

        Ty makeDefaultPrimTypeByKind(TypeKind::Kind kind) {
            switch (kind) {
                case TypeKind::Kind::Bottom: {
                    return makeBottom();
                }
                case TypeKind::Kind::Bool: {
                    return makeBool();
                }
                case TypeKind::Kind::Char: {
                    return makeChar();
                }
                case TypeKind::Kind::Int: {
                    return getDefaultIntTy();
                }
                case TypeKind::Kind::Float: {
                    return getDefaultFloatTy();
                }
                case TypeKind::Kind::Str: {
                    return makeStr();
                }
                case TypeKind::Kind::Unit: {
                    return makeUnit();
                }
                default: {
                    log::devPanic("Called `TypeContext::makePrimTypeByKind` with non-primitive type kind");
                }
            }
        }

        Ty makeRef(Region region, Mutability mutability, Ty ty) {
            return makeType(std::make_unique<Ref>(region, mutability, ty));
        }

        Ty makePointer(Mutability mutability, Ty type) {
            return makeType(std::make_unique<Pointer>(mutability, type));
        }

        Ty makeSlice(Ty type) {
            return makeType(std::make_unique<Slice>(type));
        }

        // TODO: Const
        Ty makeArray(Ty type) {
            return makeType(std::make_unique<Array>(type));
        }

        Ty makeTuple(Tuple::Element::List && els) {
            return makeType(std::make_unique<Tuple>(std::move(els)));
        }

        Ty makeFunc(DefId defId, Type::List && inputs, Ty output) {
            return makeType(std::make_unique<Func>(defId, std::move(inputs), output));
        }

        // Defaults //
    public:
        Ty getDefaultIntTy() {
            return makeInt(Int::Kind::I64);
        }

        Ty getDefaultFloatTy() {
            return makeFloat(Float::Kind::F32);
        }

        // Item types //
    public:
        void addItemType(DefId defId, Ty type) {
            utils::map::assertNewEmplace(itemsTypes.emplace(defId, type), "TypeContext::addItemType");
        }

        Ty getItemType(DefId defId) const {
            return utils::map::expectAt(itemsTypes, defId, "TypeContext::getItemType");
        }

        const auto & getItemsTypes() const {
            return itemsTypes;
        }

        // Expr types //
    public:
        void addExprType(NodeId nodeId, Ty type) {
            log::Logger::devDebug("Set type of expression ", nodeId);
            utils::map::assertNewEmplace(exprTypes.emplace(nodeId, type), "TypeContext::addExprType");
        }

        Ty getExprType(NodeId nodeId) const {
            log::Logger::devDebug("Get type of expression ", nodeId);
            return utils::map::expectAt(exprTypes, nodeId, "TypeContext::getExprType");
        }

        const auto & getExprTypes() const {
            return exprTypes;
        }

        // Local types //
    public:
        void addLocalType(NodeId nodeId, Ty type) {
            utils::map::assertNewEmplace(localTypes.emplace(nodeId, type), "TypeContext::addLocalType");
        }

        Ty getLocalType(NodeId nodeId) const {
            return utils::map::expectAt(localTypes, nodeId, "TypeContext::getLocalType");
        }

        const auto & getLocalTypes() const {
            return localTypes;
        }

    private:
        TypeMap types;
        DefId::Map<Ty> itemsTypes;
        NodeId::NodeMap<Ty> exprTypes;
        NodeId::NodeMap<Ty> localTypes;

        Infer::Var lastTypeVar {0};
        std::map<Infer::Var, Ty> typeVars;

        Converter _converter {*this};
    };
}

#endif // JACY_SRC_TYPECK_TYPECONTEXT_H
