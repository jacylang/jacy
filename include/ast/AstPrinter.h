#ifndef JACY_ASTPRINTER_H
#define JACY_ASTPRINTER_H

#include <cstdint>
#include <iostream>

#include "ast/BaseVisitor.h"
#include "ast/nodes.h"
#include "common/Logger.h"
#include "session/Session.h"

namespace jc::ast {
    using Color = common::Color;

    enum class AstPrinterMode {
        Parsing, // Print tree after parsing
        Names, // Print tree after name resolution
    };

    class AstPrinter : public BaseVisitor {
    public:
        AstPrinter();

        void print(const sess::sess_ptr & sess, const Party & party, AstPrinterMode mode = AstPrinterMode::Parsing);

        void visit(const ErrorNode & errorNode) override;
        void visit(const File & file) override;

        void visit(const RootModule & rootModule) override;
        void visit(const FileModule & fileModule) override;
        void visit(const DirModule & dirModule) override;

        // Items //
        void visit(const Enum & enumDecl) override;
        void visit(const EnumEntry & enumEntry) override;
        void visit(const Func & func) override;
        void visit(const FuncParam & funcParam) override;
        void visit(const Impl & impl) override;
        void visit(const Mod & mod) override;
        void visit(const Struct & _struct) override;
        void visit(const StructField & field) override;
        void visit(const Trait & trait) override;
        void visit(const TypeAlias & typeAlias) override;
        void visit(const UseDecl & useDecl) override;
        void visit(const UseTreeRaw & useTree) override;
        void visit(const UseTreeSpecific & useTree) override;
        void visit(const UseTreeRebind & useTree) override;
        void visit(const UseTreeAll & useTree) override;

        // Statements //
        void visit(const ExprStmt & exprStmt) override;
        void visit(const ForStmt & forStmt) override;
        void visit(const ItemStmt & itemStmt) override;
        void visit(const VarStmt & varStmt) override;
        void visit(const WhileStmt & whileStmt) override;

        // Expressions //
        void visit(const Assignment & assignment) override;
        void visit(const Block & block) override;
        void visit(const BorrowExpr & borrowExpr) override;
        void visit(const BreakExpr & breakExpr) override;
        void visit(const ContinueExpr & continueExpr) override;
        void visit(const DerefExpr & derefExpr) override;
        void visit(const IfExpr & ifExpr) override;
        void visit(const Infix & infix) override;
        void visit(const Invoke & invoke) override;
        void visit(const Lambda & lambdaExpr) override;
        void visit(const LambdaParam & param) override;
        void visit(const ListExpr & listExpr) override;
        void visit(const LiteralConstant & literalConstant) override;
        void visit(const LoopExpr & loopExpr) override;
        void visit(const MemberAccess & memberAccess) override;
        void visit(const ParenExpr & parenExpr) override;
        void visit(const PathExpr & pathExpr) override;
        void visit(const PathExprSeg & seg) override;
        void visit(const Prefix & prefix) override;
        void visit(const QuestExpr & questExpr) override;
        void visit(const ReturnExpr & returnExpr) override;
        void visit(const SpreadExpr & spreadExpr) override;
        void visit(const StructExpr & structExpr) override;
        void visit(const StructExprField & field) override;
        void visit(const Subscript & subscript) override;
        void visit(const ThisExpr & thisExpr) override;
        void visit(const TupleExpr & tupleExpr) override;
        void visit(const UnitExpr & unitExpr) override;
        void visit(const WhenExpr & whenExpr) override;
        void visit(const WhenEntry & entry) override;

        // Types //
        void visit(const ParenType & parenType) override;
        void visit(const TupleType & tupleType) override;
        void visit(const TupleTypeEl & el) override;
        void visit(const FuncType & funcType) override;
        void visit(const SliceType & listType) override;
        void visit(const ArrayType & arrayType) override;
        void visit(const TypePath & typePath) override;
        void visit(const TypePathSeg & seg) override;
        void visit(const UnitType & unitType) override;

        // Generics //
        void visit(const GenericType & genericType) override;
        void visit(const Lifetime & lifetime) override;
        void visit(const ConstParam & constParam) override;

        // Fragments //
        void visit(const Attribute & attr) override;
        void visit(const Identifier & id) override;
        void visit(const NamedElement & el) override;
        void visit(const SimplePath & path) override;
        void visit(const SimplePathSeg & seg) override;

    private:
        sess::sess_ptr sess;
        common::Logger log{"ast_printer"};
        AstPrinterMode mode{AstPrinterMode::Parsing};

        void printAttributes(const attr_list & attributes);
        void printModifiers(const parser::token_list & modifiers);
        void printTypeParams(const opt_type_params & optTypeParams, bool pathPrefix = false);

        // Helpers //
    private:
        static constexpr uint8_t DEFAULT_CHOP_THRESHOLD = 5;

        template<class T>
        void printDelim(
            const std::vector<T> & elements,
            const std::string & begin = "",
            const std::string & end = "",
            const std::string & delim = ", ",
            uint8_t chopTH = DEFAULT_CHOP_THRESHOLD
        ) {
            const auto chop = elements.size() > chopTH;
            if (not begin.empty()) {
                log.raw(begin);
            }
            if (chop) {
                log.nl();
                incIndent();
            }
            for (size_t i = 0; i < elements.size(); i++) {
                if (chop) {
                    printIndent();
                }
                elements.at(i)->accept(*this);
                if (i < elements.size() - 1) {
                    log.raw(delim);
                }
            }
            if (chop) {
                log.nl();
                decIndent();
                printIndent();
            }
            if (not end.empty()) {
                log.raw(end);
            }
        }

        template<typename T>
        void printDelim(
            const std::vector<PR<T>> & elements,
            const std::string & begin = "",
            const std::string & end = "",
            const std::string & delim = ",",
            uint8_t chopTH = DEFAULT_CHOP_THRESHOLD
        ) {
            std::vector<T> unwrappedEls;
            for (const auto & el : elements) {
                unwrappedEls.emplace_back(el.unwrap());
            }
            printDelim(unwrappedEls, begin, end, delim, chopTH);
        }

        template<class T>
        void printBodyLike(
            const std::vector<T> & elements,
            const std::string & delim = ","
        ) {
            printDelim(elements, " {", "}", delim, 0);
        }

        template<class T>
        void printBodyLike(
            const std::vector<PR<T>> & elements,
            const std::string & delim = ","
        ) {
            printDelim(elements, " {", "}", delim, 0);
        }

        // Indentation //
    private:
        void printIndent() const;
        void incIndent();
        void decIndent();
        uint16_t indent{0};

        // NodeMap mode //
    private:
        bool printAstNodeMap{false};
        void printNodeId(const Node & node) const;

        // Names mode //
    private:
        const std::vector<Color> allowedNamesColors = {
            Color::DarkBlue,
            Color::DarkGreen,
            Color::LightBlue,
            Color::DarkRed,
            Color::Magenta,
            Color::Orange,
            Color::Gray,
            Color::Blue,
            Color::Green,
            Color::Cyan,
            Color::Red,
            Color::Pink,
            Color::Yellow,
        };
        std::map<node_id, Color> namesColors;
        uint8_t lastColor;
        void colorizeDef(node_id nodeId);
        void colorizeName(node_id nodeId);
        void resetNameColor();
        Color getNameColor(node_id nodeId);

    private:
        // DEBUG //
        bool precedenceDebug = false;
    };
}

#endif // JACY_ASTPRINTER_H
