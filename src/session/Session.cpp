#include "session/Session.h"

namespace jc::sess {
    Session::Session() {
        step = std::make_shared<Step>(None, "compilation", MeasUnit::NA);
    }

    // Debug info //
    void Session::beginStep(const std::string & name, MeasUnit measUnit) {
        step = step->addChild(name, measUnit);
    }

    void Session::endStep(Option<size_t> procUnitCount) {
        const auto unit = step->getUnit();
        if (procUnitCount.none()) {
            // Check if unit exists globally (e.g. node) and not bound to something specific like file, etc.
            switch (unit) {
                case MeasUnit::Node: {
                    procUnitCount = nodeStorage.size().val;
                    break;
                }
                case MeasUnit::Def: {
                    procUnitCount = defTable.size();
                    break;
                }
                case MeasUnit::NA: {
                    // Don't use this value, check for `MeasUnit::NA`
                    procUnitCount = 0;
                    break;
                }
                default: {
                    log::Logger::devPanic(
                        "Called `Interface::endStep` with step containing non-global measurement unit [",
                        step->unitStr(),
                        "] without `procUnitCount`");
                }
            }
        }

        step = step->end(procUnitCount.unwrap());
    }

    void Session::printSteps() {
        // Unwind steps if compiler crashed
        log::Logger::devDebug(
            "Unwind steps, current step [",
            step->getName(),
            "] is ", step->isComplete() ? "complete" : "incomplete",
            " and its parent is ",
            step->getParent().some() ? step->getParent().unwrap()->getName() : "None");

        while (true) {
            for (const auto & child : step->getChildren()) {
                if (not child->isComplete()) {
                    child->endFailed();
                }
            }

            // Note!: If `compile-depth` cli flag used, that is, compilation was interrupted
            //  the main step (compilation) will be ended as incomplete, so don't be surprised
            if (not step->isComplete()) {
                step->endFailed();
            }

            if (step->getParent().some()) {
                step = step->getParent().unwrap();
            } else {
                break;
            }
        }

        printStepsDevMode();

        // Print final benchmark
        log::Logger::print(
            "Full compilation done in ",
            step->getBenchmark(),
            "ms"
        );
        log::Logger::nl();
    }

    void Session::printStepsDevMode() {
        if (not common::Config::getInstance().checkDev()) {
            return;
        }

        // Table
        // | Benchmark name | Processed entity (e.g. AST) | time | speed
        // Wrap it to ~120 chars (limit was found by typing), so the layout is following
        // | 55 | 20 | 15 | 30 (there can be pretty long entity names) |
        // Note: Choose the shortest names for benchmarks!!!

        constexpr uint8_t BNK_NAME_WRAP_LEN = 60;
        constexpr uint8_t ENTITY_NAME_WRAP_LEN = 20;
        constexpr uint8_t TIME_WRAP_LEN = 15;
        constexpr uint8_t SPEED_WRAP_LEN = 25;

        log::Table<4> table {
            {BNK_NAME_WRAP_LEN, ENTITY_NAME_WRAP_LEN, TIME_WRAP_LEN, SPEED_WRAP_LEN},
            {log::Align::Left, log::Align::Center, log::Align::Center, log::Align::Center}
        };

        std::function<void(const Step::Ptr&, uint8_t)> printStep;

        printStep = [&table, &printStep](const Step::Ptr & step, uint8_t depth) -> void {
            for (const auto & child : step->getChildren()) {
                printStep(child, depth + 1);
            }

            if (depth == 0) {
                return;
            }

            const auto time = std::to_string(step->getBenchmark()) + "ms";
            std::string speed = "N/A";
            if (step->getUnit() != MeasUnit::NA and step->isComplete()) {
                speed =
                    std::to_string(static_cast<double>(step->getUnitCount()) / step->getBenchmark()) + " " + step->unitStr() +
                    "/ms";
            }

            std::string preparedName;

            if (depth > 0) {
                preparedName += utils::str::repeat(" ", depth - 1) + "> ";
            } else {
                table.addLine(true);
            }

            preparedName += step->getName();

            if (not step->isComplete()) {
                preparedName += " 🔥";
            }

            table.addRow(preparedName, step->unitStr(), time, speed);

            if (depth == 1) {
                table.addLine(true);
            }
        };

        table.addSectionName("Summary");
        table.addHeader("Name", "Entity", "Time", "Speed");

        printStep(step, 0);

        table.addLine(true);

        log::Logger::print(table);
    }
}
