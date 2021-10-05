#include "log/Logger.h"
#include "message/Highlighter.h"

using namespace jc::log;

int main() {
    const auto code = R"(
func foo() {
    print('kek' + 123);
}
    )";

    jc::message::Highlighter highlight;

    Logger::print(highlight.highlight(code));

    return 0;
}
