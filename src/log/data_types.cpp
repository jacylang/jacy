#include "log/data_types.h"
#include "utils/str.h"

namespace jc::log {
    TrueColor::TrueColor(const std::string & str) {
        using namespace utils::str;

        if (str.empty()) {
            throw std::logic_error("Invalid value for `TrueColor` constructor - empty string");
        }

        if (startsWith(str, "#")) {
            auto strVal = str.substr(1);
            if (strVal.size() == 3) {
                auto val = std::stoul(strVal, nullptr, 16);
                *this = fromHex3(static_cast<uint16_t>(val));
                return;
            } else if (strVal.size() == 6) {
                auto val = std::stoul(strVal, nullptr, 16);
                *this = fromHex6(val);
                return;
            }

            throw std::logic_error(
                "Invalid value for `TrueColor` constructor - hex value must contain 3 or 6 digits, got '"
                    + str + "' with " + std::to_string(strVal.size()) + " digits"
            );
        }

        throw std::logic_error(
            "Invalid value `TrueColor` constructor - " + str
        );
    }

    TrueColor TrueColor::fromHex6(uint32_t value) {
        return TrueColor {
            static_cast<uint8_t>(((value >> 16) & 0xFF)),
            static_cast<uint8_t>(((value >> 8) & 0xFF)),
            static_cast<uint8_t>((value & 0xFF))
        };
    }

    TrueColor TrueColor::fromHex3(uint16_t value) {
        return TrueColor {
            static_cast<uint8_t>((value >> 8) & 0xF),
            static_cast<uint8_t>((value >> 4) & 0xF),
            static_cast<uint8_t>(value & 0xF)
        };
    }
}
