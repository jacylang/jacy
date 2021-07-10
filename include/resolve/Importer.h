#ifndef JACY_RESOLVE_IMPORTER_H
#define JACY_RESOLVE_IMPORTER_H

#include "ast/StubVisitor.h"
#include "suggest/SuggInterface.h"

namespace jc::resolve {
    class Imported : public ast::StubVisitor, public sugg::SuggInterface {
    public:
        Imported() : StubVisitor("Imported") {}
        ~Imported() override = default;
    }
}

#endif // JACY_RESOLVE_IMPORTER_H
