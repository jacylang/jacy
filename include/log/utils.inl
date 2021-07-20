// Common data structures std::ostream overloads //
namespace jc::log {
    template<class T>
    std::ostream & operator<<(std::ostream & os, const std::vector<T> & vec) {
        os << "[";
        for (size_t i = 0; i < vec.size(); ++i) {
            os << vec.at(i);
            if (i < vec.size() - 1) {
                os << ", ";
            }
        }
        return os << "]";
    }

    template<class K, class V>
    std::ostream & operator<<(std::ostream & os, const std::map<K, V> & map) {
        os << "{";
        for (auto it = map.begin(); it != map.end(); it++) {
            os << it->first << ": " << it->second;
            if (it != std::prev(map.end())) {
                os << ", ";
            }
        }
        return os << "}";
    }

    template<class K, class V>
    std::ostream & operator<<(std::ostream & os, const std::unordered_map<K, V> & map) {
        os << "{";
        size_t i = 0;
        for (const auto & el : map) {
            os << el.first << ": " << el.second;
            if (i != map.size()) {
                os << ", ";
            }
            i++;
        }
        return os << "}";
    }
}
