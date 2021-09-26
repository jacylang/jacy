#include "log/Logger.h"
#include "suggest/Highlighter.h"

using namespace jc::log;

int main() {
    const auto code = R"(
func foo() {
    print('kek');
}
    )";

    jc::sugg::Highlight highlight;

    Logger::print(highlight.highlight(code));

    return 0;
}
