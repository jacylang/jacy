#ifndef JACY_RESOLVE_RESSTORAGE_H
#define JACY_RESOLVE_RESSTORAGE_H

#include <map>

#include "ast/Node.h"
#include "resolve/Definition.h"
#include "utils/map.h"

namespace jc::resolve {
    using ast::NodeId;
    using PrimTypeSet = uint16_t;

    // Unresolved segment
    // Has index in path segments vector and optional definition id,
    // if segment is resolved but it is private
    struct UnresSeg {
        size_t segIndex;
        DefId::Opt defId{None};
    };

    /// One bit for each `PrimType` variant
    /// Leftmost bit is last `PrimType` variant (`Str`), rightmost is `Bool`
    /// const prim_type_set_t PRIM_TYPES_MASK = 0b1111111111111;

    enum class PrimType : uint8_t {
        Bool = 0,
        Int8,
        Int16,
        Int32,
        Int, // Alias for `Int32`
        Int64,
        Uint8,
        Uint16,
        Uint32,
        Uint, // Alias for `Uint32`
        Uint64,
        Char,
        Str, // Note!!!: Order matters -- keep Str last
    };

    inline Option<PrimType> getPrimType(const std::string & typeName) {
        static const std::map<std::string, PrimType> primTypesNames = {
            {"bool", PrimType::Bool},
            {"int8", PrimType::Int8},
            {"int16", PrimType::Int16},
            {"int32", PrimType::Int32},
            {"int", PrimType::Int},
            {"int64", PrimType::Int64},
            {"uint8", PrimType::Uint8},
            {"uint16", PrimType::Uint16},
            {"uint32", PrimType::Uint32},
            {"uint", PrimType::Uint},
            {"uint64", PrimType::Uint64},
            {"char", PrimType::Char},
            {"str", PrimType::Str},
        };

        const auto & found = primTypesNames.find(typeName);
        if (found == primTypesNames.end()) {
            return None;
        }
        return found->second;
    }

    inline Option<PrimTypeSet> getPrimTypeBitMask(const std::string & typeName) {
        const auto primType = getPrimType(typeName);
        if (primType.none()) {
            return None;
        }
        // Return mask with 1 at found primitive type offset
        return static_cast<PrimTypeSet>(1 << static_cast<PrimTypeSet>(primType.unwrap()));
    }

    /// [Debug noly] get list of shadowed primitive type names by mask
    inline std::vector<std::string> getShadowedPrimTypes(PrimTypeSet mask) {
        static const std::map<PrimType, std::string> primTypesNames = {
            {PrimType::Bool, "bool"},
            {PrimType::Int8, "int8"},
            {PrimType::Int16, "int16"},
            {PrimType::Int32, "int32"},
            {PrimType::Int, "int"},
            {PrimType::Int64, "int64"},
            {PrimType::Uint8, "uint8"},
            {PrimType::Uint16, "uint16"},
            {PrimType::Uint32, "uint32"},
            {PrimType::Uint, "uint"},
            {PrimType::Uint64, "uint64"},
            {PrimType::Char, "char"},
            {PrimType::Str, "str"},
        };
        static const auto leftmostBit = static_cast<PrimTypeSet>(PrimType::Str);

        std::vector<std::string> shadowedTypes;
        for (PrimTypeSet shift = 0; shift < leftmostBit; shift++) {
            if (mask << shift & 1) {
                shadowedTypes.emplace_back(primTypesNames.at(static_cast<PrimType>(shift)));
            }
        }
        return shadowedTypes;
    }

    inline std::string primTypeToString(PrimType primType) {
        switch (primType) {
            case PrimType::Bool:
                return "bool";
            case PrimType::Int8:
                return "int8";
            case PrimType::Int16:
                return "int16";
            case PrimType::Int32:
                return "int32";
            case PrimType::Int:
                return "int";
            case PrimType::Int64:
                return "int64";
            case PrimType::Uint8:
                return "uint8";
            case PrimType::Uint16:
                return "uint16";
            case PrimType::Uint32:
                return "uint32";
            case PrimType::Uint:
                return "uint";
            case PrimType::Uint64:
                return "uint64";
            case PrimType::Char:
                return "char";
            case PrimType::Str:
                return "str";
        }
    }

    enum class ResKind {
        Def,
        Local,
        PrimType,
        Error,
    };

    struct Res {
        /// Resolution type
        /// NodeId - local variable
        /// DefId - definition id
        /// PrimType - primitive type
        using ValueType = std::variant<NodeId, DefId, PrimType>;

        Res() : kind(ResKind::Error) {}
        Res(const DefId & defId) : kind(ResKind::Def), val{defId} {}
        Res(NodeId nodeId) : kind(ResKind::Local), val{nodeId} {}
        Res(PrimType primType) : kind(ResKind::PrimType), val{primType} {}

        ResKind kind;
        ValueType val;

        std::string kindStr() const {
            switch (kind) {
                case ResKind::Def: return "def";
                case ResKind::Local: return "local";
                case ResKind::PrimType: return "prim_type";
                case ResKind::Error: return "[ERROR]";
            }
        }

        bool isErr() const {
            return kind == ResKind::Error;
        }

        const auto & asDef() const {
            return std::get<DefId>(val);
        }

        auto asLocal() const {
            return std::get<NodeId>(val);
        }

        auto asPrimType() const {
            return std::get<PrimType>(val);
        }
    };

    /// `Name` is a pointer to path-like node.
    /// In `Resolutions` it is mapped to specific `Res` and can point to
    /// - local variable
    /// - definition id
    /// - primitive type
    struct NamePath {
        NamePath(NodeId nodeId) : nodeId{nodeId} {}

        NodeId nodeId;

        bool operator==(const NamePath & other) const {
            return nodeId == other.nodeId;
        }

        bool operator<(const NamePath & other) const {
            return nodeId < other.nodeId;
        }

        friend std::ostream & operator<<(std::ostream & os, const NamePath & name) {
            return os << name.nodeId;
        }
    };

    /// Resolutions
    /// @brief Collection of {path: node} names resolutions
    class Resolutions {
    public:
        Resolutions() = default;

        Res getRes(const NamePath & path) const {
            if (resolutions.find(path) == resolutions.end()) {
                log::devPanic("Called `ResStorage::getRes` with non-existent name ", path);
            }
            // Note: It is actually a bug if resolution does not exists
            //  as far as unresolved names are stored as Error Res
            return resolutions.at(path);
        }

        void setRes(NamePath path, Res res) {
            resolutions.emplace(path, res);
        }

        const auto & getResolutions() const {
            return resolutions;
        }

        const auto & getDefRes(NamePath path) const {
            // TODO!: Error resolutions recovery
            return utils::map::expectAt(
                resolutions,
                path,
                log::fmt("`Resolutions::getDefRes`, resolutions: ", resolutions)).asDef();
        }

    private:
        std::map<NamePath, Res> resolutions;
    };
}

#endif // JACY_RESOLVE_RESSTORAGE_H
