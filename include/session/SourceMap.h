#ifndef JACY_SESSION_SOURCEMAP_H
#define JACY_SESSION_SOURCEMAP_H

#include <map>
#include <vector>
#include <random>

#include "session/Session.h"
#include "parser/Token.h"
#include "common/Error.h"

namespace jc::span {
    struct Span;
}

namespace jc::sess {

    static struct SourceMap {
        std::map<file_id_t, source_t> sources;

        file_id_t addSource() {
            // TODO!: Hash-generated `fileId`
            const auto & rand = []() {
                std::random_device dev;
                std::mt19937 rng(dev());
                std::uniform_int_distribution<std::mt19937::result_type> range(1, UINT16_MAX);
                return range(rng);
            };
            file_id_t fileId = rand();
            while (sources.find(fileId) != sources.end()) {
                fileId = rand();
            }
            std::cout << "added source to fileId " << fileId << std::endl;
            sources.emplace(fileId, source_t{});
            return fileId;
        }

        void setSource(file_id_t fileId, source_t && source) {
            sources.emplace(fileId, std::move(source));
        }

        std::string sliceBySpan(const span::Span & span, sess_ptr sess);
    } sourceMap;
}

#endif // JACY_SESSION_SOURCEMAP_H
