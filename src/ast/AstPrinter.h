#ifndef JACY_ASTPRINTER_H
#define JACY_ASTPRINTER_H

#include <cstdint>
#include <iostream>

#include "ast/BaseVisitor.h"
#include "ast/nodes.h"
#include "log/Logger.h"
#include "session/Session.h"

namespace jc::ast {
    using Color = log::Color;

    enum class AstPrinterMode {
        Parsing, // Print tree after parsing
        Names, // Print tree after name resolution
    };

    class AstPrinter : public BaseVisitor {
    public:
        AstPrinter();

        void print(const sess::Session::Ptr & sess, const Party & party, AstPrinterMode mode = AstPrinterMode::Parsing);

        void visit(const ErrorNode & errorNode) override;

        // Items //
        void visit(const Enum & enumDecl) override;

        void visit(const Variant & variant) override;

        void visit(const Func & func) override;

        void visit(const FuncParam & funcParam) override;

        void visit(const Impl & impl) override;

        void visit(const Mod & mod) override;

        void visit(const Struct & st) override;

        void visit(const Trait & trait) override;

        void visit(const TypeAlias & typeAlias) override;

        void visit(const UseDecl & useDecl) override;

        void visit(const UseTree & useTree) override;

        void visit(const Init & init) override;

        // Statements //
        void visit(const ExprStmt & exprStmt) override;

        void visit(const ItemStmt & itemStmt) override;

        void visit(const LetStmt & letStmt) override;

        // Expressions //
        void visit(const Assign & assignment) override;

        void visit(const Block & block) override;

        void visit(const BorrowExpr & borrowExpr) override;

        void visit(const BreakExpr & breakExpr) override;

        void visit(const ContinueExpr & continueExpr) override;

        void visit(const ForExpr & forStmt) override;

        void visit(const IfExpr & ifExpr) override;

        void visit(const Infix & infix) override;

        void visit(const Invoke & invoke) override;

        void visit(const Lambda & lambdaExpr) override;

        void visit(const LambdaParam & param) override;

        void visit(const ListExpr & listExpr) override;

        void visit(const LitExpr & literalConstant) override;

        void visit(const LoopExpr & loopExpr) override;

        void visit(const FieldExpr & memberAccess) override;

        void visit(const ParenExpr & parenExpr) override;

        void visit(const PathExpr & pathExpr) override;

        void visit(const Prefix & prefix) override;

        void visit(const Postfix & questExpr) override;

        void visit(const ReturnExpr & returnExpr) override;

        void visit(const SpreadExpr & spreadExpr) override;

        void visit(const Subscript & subscript) override;

        void visit(const SelfExpr & selfExpr) override;

        void visit(const TupleExpr & tupleExpr) override;

        void visit(const UnitExpr & unitExpr) override;

        void visit(const MatchExpr & matchExpr) override;

        void visit(const MatchArm & matchArm) override;

        void visit(const WhileExpr & whileStmt) override;

        // Types //
        void visit(const ParenType & parenType) override;

        void visit(const TupleType & tupleType) override;

        void visit(const FuncType & funcType) override;

        void visit(const SliceType & listType) override;

        void visit(const ArrayType & arrayType) override;

        void visit(const TypePath & typePath) override;

        void visit(const UnitType & unitType) override;

        // Generics //
        void visit(const GenericParam & param) override;
        void visit(const GenericArg & arg) override;

        // Fragments //
        void visit(const Attr & attr) override;

        void visit(const Ident & ident) override;

        void visit(const Path & path) override;

        void visit(const PathSeg & seg) override;

        void visit(const SimplePath & path) override;

        void visit(const SimplePathSeg & seg) override;

        void visit(const AnonConst & anonConst) override;

        // Patterns //
        void visit(const MultiPat & pat) override;

        void visit(const ParenPat & pat) override;

        void visit(const LitPat & pat) override;

        void visit(const IdentPat & pat) override;

        void visit(const RefPat & pat) override;

        void visit(const PathPat & pat) override;

        void visit(const WildcardPat & pat) override;

        void visit(const RestPat & pat) override;

        void visit(const StructPat & pat) override;

        void visit(const TuplePat & pat) override;

        void visit(const SlicePat & pat) override;

    private:
        sess::Session::Ptr sess;
        log::Logger log {"ast-printer"};
        AstPrinterMode mode {AstPrinterMode::Parsing};

        // Helpers //
    private:
        void printVis(const Vis & vis);

        void printAttributes(const Attr::List & attributes);

        void printModifiers(const parser::Token::List & modifiers);

        /// Print generic parameters (for items, e.g. `struct`)
        void printGenericParams(const GenericParam::OptList & optGenerics);

        /// Print generic arguments (for path segments)
        void printGenericArgs(const GenericArg::OptList & optArgs);

        void printFuncHeader(const FuncHeader & header);

        void printFuncSig(const FuncSig & sig);

        template<class N>
        void printNamedNodeList(
            const typename NamedNode<N, Ident::OptPR>::List & els,
            const std::string & opening,
            const std::string & closing
        ) {
            log.raw(opening);

            for (size_t i = 0; i < els.size(); i++) {
                const auto & el = els.at(i);
                el.name.then([&](const Ident::PR & name) {
                    name.autoAccept(*this);
                    log.raw(": ");
                });

                el.node.autoAccept(*this);

                if (i < els.size() - 1) {
                    log.raw(", ");
                }
            }

            log.raw(closing);
        }

        static constexpr uint8_t DEFAULT_CHOP_THRESHOLD = 5;

        template<class T>
        void basePrintDelim(
            const std::vector<T> & elements,
            const std::string & begin,
            const std::string & end,
            const std::string & delim,
            uint8_t chopTH,
            const std::function<void(const T &)> & cb
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
                cb(elements.at(i));
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

        template<class T>
        void printDelim(
            const std::vector<T> & elements,
            const std::string & begin = "",
            const std::string & end = "",
            const std::string & delim = ", ",
            uint8_t chopTH = DEFAULT_CHOP_THRESHOLD
        ) {
            basePrintDelim<T>(elements, begin, end, delim, chopTH, [&](const T & el) -> void {
                if constexpr (dt::is_ptr_like<T>::value) {
                    el->accept(*this);
                } else {
                    el.accept(*this);
                }
            });
        }

        template<typename T>
        void printDelim(
            const std::vector<PR<T>> & elements,
            const std::string & begin = "",
            const std::string & end = "",
            const std::string & delim = ",",
            uint8_t chopTH = DEFAULT_CHOP_THRESHOLD
        ) {
            basePrintDelim<PR<T>>(elements, begin, end, delim, chopTH, [&](const PR<T> & el) {
                el.autoAccept(*this);
            });
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

        uint16_t indent {0};

        // NodeMap mode //
    private:
        bool printAstNodeMap {false};

        void printNodeId(NodeId id) const;

        void printNodeId(const Node & node) const;

        // Names mode //
    private:
        const std::vector<Color> allowedNamesColors = {
            Color::Magenta,
            Color::Orange,
            Color::Blue,
            Color::Green,
            Color::Cyan,
            Color::Red,
            Color::Pink,
            Color::Yellow,
        };
        const static Color noneNodeColor = Color::LightGray;
        std::map<NodeId, Color> namesColors;
        uint8_t lastColor;

        void colorizeNameDecl(NodeId nodeId, const Ident::PR & ident);

        void colorizePathName(NodeId pathNodeId);

        void resetNameColor();

        Color getNameColor(NodeId nodeId);

        Option<Color> getNameColorChecked(NodeId nodeId);

        template<class T>
        void tryPrintColorized(Option<Color> color, const T & node) {
            if (color.some()) {
                log.raw(color);
            }
            node.accept(*this);
            resetNameColor();
        }

        void tryPrintStringColorized(Option<Color> color, const std::string & str) {
            if (color.some()) {
                log.raw(color.unwrap());
            }
            log.raw(str);
            resetNameColor();
        }

        template<class T>
        void printColorizedByNodeId(const T & node) {
            if (mode != AstPrinterMode::Names) {
                node.accept(*this);
                return;
            }
            log.raw(getNameColor(node.id));
            node.accept(*this);
            resetNameColor();
        }

    private:
        // DEBUG //
        bool precedenceDebug = false;
    };
}

#endif // JACY_ASTPRINTER_H
