#ifndef JACY_RESOLVE_RESSTORAGE_H
#define JACY_RESOLVE_RESSTORAGE_H

#include <map>

#include "ast/Node.h"
#include "resolve/Definition.h"

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
        /// DefRes - definition
        /// PrimType - primitive type
        using ValueType = std::variant<NodeId, DefRes, PrimType>;

        Res() : kind(ResKind::Error) {}
        Res(const DefRes & defRes) : kind(ResKind::Def), val{defRes} {}
        Res(NodeId nodeId) : kind(ResKind::Local), val{nodeId} {}
        Res(PrimType primType) : kind(ResKind::PrimType), val{primType} {}

        ResKind kind;
        ValueType val;

        bool isErr() const {
            return kind == ResKind::Error;
        }

        const auto & asDef() const {
            return std::get<DefRes>(val);
        }

        auto asLocal() const {
            return std::get<NodeId>(val);
        }

        auto asPrimType() const {
            return std::get<PrimType>(val);
        }
    };

    struct Name {
        Name(NodeId nodeId) : nodeId{nodeId} {}

        NodeId nodeId;

        bool operator==(const Name & other) const {
            return nodeId == other.nodeId;
        }

        bool operator<(const Name & other) const {
            return nodeId < other.nodeId;
        }

        friend std::ostream & operator<<(std::ostream & os, const Name & name) {
            return os << log::Color::Cyan << "#" << name.nodeId << log::Color::Reset;
        }
    };

    /// ResStorage
    /// @brief Collection of {path: node} names resolutions
    class ResStorage {
    public:
        ResStorage() = default;

        Res getRes(const Name & name) const {
            if (resolutions.find(name) == resolutions.end()) {
                log::Logger::devPanic("Called `ResStorage::getRes` with non-existent name ", name);
            }
            // Note: It is actually a bug if resolution does not exists
            //  as far as unresolved names are stored as Error Res
            return resolutions.at(name);
        }

        void setRes(NodeId name, Res res) {
            resolutions.emplace(name, res);
        }

        const auto & getResolutions() const {
            return resolutions;
        }

        const auto & getDefRes(NodeId nodeId) const {
            // TODO!: Error resolutions recovery
            return resolutions.at(nodeId).asDef();
        }

    private:
        std::map<Name, Res> resolutions;
    };
}

#endif // JACY_RESOLVE_RESSTORAGE_H
