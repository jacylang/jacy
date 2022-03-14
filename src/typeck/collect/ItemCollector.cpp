#include "typeck/collect/ItemCollector.h"

namespace jc::typeck {
    void ItemCollector::visitFunc(const hir::Func & func, const hir::Item::ItemData & data) {
        auto defId = data.defId;
        auto inputs = tyCtx.converter().convertTypeList(func.sig.inputs);
        auto output = tyCtx.converter().convertFuncReturnType(func.sig.returnType);
        auto ty = tyCtx.makeFunc(
            defId,
            std::move(inputs),
            output
        );

        return tyCtx.addItemType(
            defId,
            ty
        );
    }
}
