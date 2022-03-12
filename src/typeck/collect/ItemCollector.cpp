#include "typeck/collect/ItemCollector.h"

namespace jc::typeck {
    void ItemCollector::visitFunc(const hir::Func & func, const hir::Item::ItemData & data) {
        return tyCtx.addItemType(
            data.defId,
            tyCtx.makeFunc(
                data.defId,
                tyCtx.converter().convertTypeList(func.sig.inputs),
                tyCtx.converter().convert(func.sig.returnType.asSome())
            )
        );
    }
}
