#ifndef JACY_RESOLVE_IMPORTER_H
#define JACY_RESOLVE_IMPORTER_H

#include "ast/StubVisitor.h"
#include "suggest/SuggInterface.h"

namespace jc::resolve {
    class Importer : public ast::StubVisitor, public sugg::SuggInterface {
    public:
        Importer() : StubVisitor("Importer") {}
        ~Importer() override = default;

        void visit(const ast::UseDecl & useDecl) override;
        void visit(const ast::UseTreeRaw & useTree) override;
    };
}

#endif // JACY_RESOLVE_IMPORTER_H
