#ifndef JACY_MAP_H
#define JACY_MAP_H

#include <map>
#include <vector>
#include <sstream>
#include <stdexcept>

#include "data_types/Option.h"

namespace jc::utils::map {
    template<class K, class V>
    std::map<K, V> merge(const std::map<K, V> & first, const std::map<K, V> & second) {
        std::map<K, V> result = first;
        for (const auto & pair : second) {
            result[pair.first] = pair.second;
        }
        return result;
    }

    template<class K, class V>
    bool has(const std::map<K, V> & map, const K & value) {
        return map.find(value) != map.end();
    }

    template<class K, class V>
    bool rename(std::map<K, V> & map, const K & replace, const K & with) {
        auto it = map.find(replace);
        if (it != map.end()) {
            std::swap(map[with], it->second);
            map.erase(it);
            return true;
        }
        return false;
    }

    template<class K, class V>
    std::vector<K> keys(const std::map<K, V> & map) {
        std::vector<K> keys;
        for (const auto & pair : map) {
            keys.push_back(pair.first);
        }
        return keys;
    }

    template<class K, class V>
    std::vector<V> values(const std::map<K, V> & map) {
        std::vector<V> values;
        for (const auto & pair : map) {
            values.push_back(pair.second);
        }
        return values;
    }

    template<class K, class V>
    std::map<V, K> reverse(const std::map<K, V> & map) {
        std::map<V, K> reversed;
        for (const auto & pair : map) {
            reversed.emplace(pair.second, pair.first);
        }
        return reversed;
    }

    template<class K, class V>
    Option<V> getOpt(const std::map<K, V> & map, const K & key) {
        const auto & found = map.find(key);
        if (found == map.end()) {
            return None;
        }
        return found->second;
    }

    template<class K, class V>
    const V & expectAt(const std::map<K, V> & map, const K & key, const std::string & place) {
        const auto & found = map.find(key);
        if (found == map.end()) {
            std::stringstream ss;
            ss << "map `expectedAt` '" << key << "' in " << place;
            throw std::logic_error(ss.str());
        }
        return found->second;
    }

    template<class K, class V>
    V & expectAtMut(std::map<K, V> & map, const K & key, const std::string & place) {
        const auto & found = map.find(key);
        if (found == map.end()) {
            std::stringstream ss;
            ss << "map `expectedAt` '" << key << "' in " << place;
            throw std::logic_error(ss.str());
        }
        return found->second;
    }

    template<typename Iter>
    void assertNewEmplace(const std::pair<Iter, bool> & result, const std::string & place) {
        if (not result.second) {
            throw std::logic_error(
                "[map::assertNewEmplace] Tried to replace element in " + place);
        }
    }
}

#endif // JACY_MAP_H
