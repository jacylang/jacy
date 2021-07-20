#include "log/utils.h"

// Tables //
namespace jc::log {
    template<table_size_t Cols>
    const std::map<CellKind, std::array<std::string, 3>> Table<Cols>::corners = {
        {CellKind::Value, {"| ", " | ", " |"}},
        {CellKind::Line, {"+-", "-+-", "-+"}},
    };
}
