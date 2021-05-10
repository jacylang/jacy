#ifndef JACY_HIR_LINTER_H
#define JACY_HIR_LINTER_H

#include "common/Logger.h"
#include "ast/BaseVisitor.h"

namespace jc::hir {
    class Linter : public ast::BaseVisitor {
        Linter();



    private:
        common::Logger log{"linter", {}};
    };
}

#endif //JACY_HIR_LINTER_H
