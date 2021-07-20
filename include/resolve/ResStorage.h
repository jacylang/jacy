#ifndef JACY_RESOLVE_RESSTORAGE_H
#define JACY_RESOLVE_RESSTORAGE_H

#include <map>

#include "ast/Node.h"

namespace jc::resolve {
    using ast::node_id;
    using ast::opt_node_id;
    using prim_type_set_t = uint16_t;

    // Unresolved segment
    // Has index in path segments vector and optional definition id,
    // if segment is resolved but it is private
    struct UnresSeg {
        size_t segIndex;
        opt_def_id defId{None};
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

    inline Option<prim_type_set_t> getPrimTypeBitMask(const std::string & typeName) {
        const auto primType = getPrimType(typeName);
        if (primType.none()) {
            return None;
        }
        // Return mask with 1 at found primitive type offset
        return static_cast<prim_type_set_t>(1 << static_cast<prim_type_set_t>(primType.unwrap()));
    }

    /// [Debug noly] get list of shadowed primitive type names by mask
    inline std::vector<std::string> getShadowedPrimTypes(prim_type_set_t mask) {
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
        static const auto leftmostBit = static_cast<prim_type_set_t>(PrimType::Str);

        std::vector<std::string> shadowedTypes;
        for (prim_type_set_t shift = 0; shift < leftmostBit; shift++) {
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
        Res() : kind(ResKind::Error) {}
        Res(const DefId & def) : kind(ResKind::Def), def(def) {}
        Res(node_id nodeId) : kind(ResKind::Local), nodeId(nodeId) {}
        Res(PrimType primType) : kind(ResKind::PrimType), primType(primType) {}

        ResKind kind;
        Option<DefId> def{None};
        Option<node_id> nodeId{None};
        Option<PrimType> primType{None};

        bool isErr() const {
            return kind == ResKind::Error;
        }

        DefId asDef() const {
            return def.unwrap();
        }

        node_id asLocal() const {
            return nodeId.unwrap();
        }

        PrimType asPrimType() const {
            return primType.unwrap();
        }
    };

    /// ResStorage
    /// @brief Collection of {path: node} names resolutions
    class ResStorage {
    public:
        ResStorage() = default;

        Res getRes(node_id name) const {
            if (resolutions.find(name) == resolutions.end()) {
                log::Logger::devPanic("Called `ResStorage::getRes` with non-existent name node id #", name);
            }
            // Note: It is actually a bug if resolution does not exists
            //  as far as unresolved names are stored as Error Res
            return resolutions.at(name);
        }

        void setRes(node_id name, Res res) {
            resolutions.emplace(name, res);
        }

        const std::map<node_id, Res> getResolutions() const {
            return resolutions;
        }

    private:
        /// Map of Identifier node id -> resolution
        std::map<node_id, Res> resolutions;
    };
}

#endif // JACY_RESOLVE_RESSTORAGE_H
