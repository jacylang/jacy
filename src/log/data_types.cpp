#include "log/data_types.h"
#include "utils/str.h"

namespace jc::log {
    TrueColor TrueColor::fromHex(uint32_t value) {
        return TrueColor {
            static_cast<uint8_t>(((value >> 16) & 0xFF)),
            static_cast<uint8_t>(((value >> 8) & 0xFF)),
            static_cast<uint8_t>((value & 0xFF))
        };
    }
}
