#ifndef JACY_SESSION_DIAGNOSTICS_H
#define JACY_SESSION_DIAGNOSTICS_H

#include <chrono>
#include <memory>
#include <vector>
#include <map>

#include "data_types/Option.h"
#include "log/Logger.h"

namespace jc::sess {
    const auto bench = std::chrono::high_resolution_clock::now;

    enum class MeasUnit {
        NA,
        Char,
        Token,
        Node,
        Def,
    };

    class Step : public std::enable_shared_from_this<Step> {
    public:
        using ptr = std::shared_ptr<Step>;
        using milli_ratio = std::ratio<1, 1000>;
        using bench_t = std::chrono::time_point<std::chrono::high_resolution_clock>;

    public:
        Step(
            Option<ptr> parent,
            const std::string & name,
            MeasUnit measUnit
        ) : parent(parent),
            name(name),
            measUnit(measUnit),
            benchStart(bench()) {}

    public:
        template<class ...Args>
        ptr addChild(Args && ...args) {
            children.emplace_back(std::make_shared<Step>(shared_from_this(), std::forward<Args>(args)...));
            return children.back();
        }

        MeasUnit getUnit() const {
            return measUnit;
        }

        const std::string & getName() const {
            return name;
        }

        auto childrenCount() const {
            return children.size();
        }

        Option<ptr> getParent() const {
            return parent;
        }

        double getBenchmark() const {
            return benchmark.unwrap("`Step::getBenchmark`");
        }

        auto getUnitCount() const {
            return procUnitCount.unwrap("`Step::getUnitCount`");
        }

        const auto & getChildren() const {
            return children;
        }

        auto isComplete() const {
            return complete;
        }

        ptr end(Option<size_t> procUnitCount) {
            this->procUnitCount = procUnitCount;
            benchmark = std::chrono::duration<double, milli_ratio>(bench() - benchStart).count();
            complete = true;
            return parent.unwrap("`Step::end`");
        }

        void endFailed() {
            log::Logger::devDebug("End as failed '", name, "'");
            procUnitCount = None;
            benchmark = std::chrono::duration<double, milli_ratio>(bench() - benchStart).count();
        }

        constexpr const char * unitStr() const {
            switch (measUnit) {
                case MeasUnit::Node: return "node";
                case MeasUnit::Char: return "char";
                case MeasUnit::Token: return "token";
                case MeasUnit::Def: return "def";
                case MeasUnit::NA: return "N/A";
            }
        }

    private:
        Option<ptr> parent{None};
        std::vector<ptr> children{};

        std::string name;

        MeasUnit measUnit;
        Option<size_t> procUnitCount{None};

        bench_t benchStart;
        Option<double> benchmark{None};

        bool complete{false};
    };
}

#endif // JACY_SESSION_DIAGNOSTICS_H
