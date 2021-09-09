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
    // if segment is resolved, but it is private
    struct UnresSeg {
        size_t segIndex;
        DefId::Opt defId{None};
    };

    /// One bit for each `PrimType` variant
    /// Leftmost bit is last `PrimType` variant (`Str`), rightmost is `Bool`
    /// const prim_type_set_t PRIM_TYPES_MASK = 0b1111111111111;

    enum class PrimType : uint8_t {
        Bool = 0,
        Int,
        Uint,
        I8,
        I16,
        I32,
        I64,
        U8,
        U16,
        U32,
        U64,
        Char,
        Str, // Note!!!: Order matters -- keep Str last
    };

    inline Option<PrimType> getPrimType(const span::Symbol & typeName) {
        static const std::map<std::string, PrimType> primTypesNames = {
            {"bool", PrimType::Bool},
            {"int",  PrimType::Int},
            {"uint", PrimType::Uint},
            {"i8",   PrimType::I8},
            {"i16",  PrimType::I16},
            {"i32",  PrimType::I32},
            {"i64",  PrimType::I64},
            {"u8",   PrimType::U8},
            {"u16",  PrimType::U16},
            {"u32",  PrimType::U32},
            {"u64",  PrimType::U64},
            {"char", PrimType::Char},
            {"str",  PrimType::Str},
        };

        const auto & found = primTypesNames.find(typeName.toString());
        if (found == primTypesNames.end()) {
            return None;
        }
        return found->second;
    }

    inline Option<PrimTypeSet> getPrimTypeBitMask(const span::Symbol & typeName) {
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
            {PrimType::Int,  "int"},
            {PrimType::Uint, "uint"},
            {PrimType::I8,   "i8"},
            {PrimType::I16,  "i16"},
            {PrimType::I32,  "i32"},
            {PrimType::I64,  "i64"},
            {PrimType::U8,   "u8"},
            {PrimType::U16,  "u16"},
            {PrimType::U32,  "u32"},
            {PrimType::U64,  "u64"},
            {PrimType::Char, "char"},
            {PrimType::Str,  "str"},
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
            case PrimType::Int:
                return "int";
            case PrimType::Uint:
                return "uint";
            case PrimType::I8:
                return "i8";
            case PrimType::I16:
                return "i16";
            case PrimType::I32:
                return "i32";
            case PrimType::I64:
                return "i64";
            case PrimType::U8:
                return "u8";
            case PrimType::U16:
                return "u16";
            case PrimType::U32:
                return "u32";
            case PrimType::U64:
                return "u64";
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

        friend std::ostream & operator<<(std::ostream & os, const Res & res) {
            os << res.kindStr() << " '";
            switch (res.kind) {
                case ResKind::Def: {
                    os << res.asDef();
                    break;
                }
                case ResKind::Local: {
                    os << res.asLocal();
                    break;
                }
                case ResKind::PrimType: {
                    os << primTypeToString(res.asPrimType());
                    break;
                }
                case ResKind::Error:;
            }
            return os;
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
