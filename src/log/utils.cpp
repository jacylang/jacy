#include "log/utils.h"

// Tables //
namespace jc::log {
    static const std::map<CellKind, std::array<std::string, 3>> Table::corners = {
        {CellKind::Value, {"| ", " | ", " |"}},
        {CellKind::Line, {"+-", "-+-", "-+"}},
    };
}
