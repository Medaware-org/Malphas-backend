#pragma once

#include <map>
#include <functional>
#include <utility>

template<typename K, typename V>
class ordered_map {
        std::map<K, V> map;
        std::vector<K> keys;

        public:
                ordered_map() = default;

                V operator[](K key)
                {
                        return map.at(key);
                }

                size_t size() const
                {
                        return map.size();
                }

                V &n(int n) const
                {
                        if (n >= keys.size())
                                return NULL;

                        return map.at(keys.at(n));
                }

                std::pair<K, V> pair_n(int n) const
                {
                        K key = keys.at(n);
                        V val = map.at(key);
                        return std::make_pair(key, val);
                }

                void clear()
                {
                        map.clear();
                        keys.clear();
                }

                void emplace(K key, V value)
                {
                        if (map.contains(key))
                                return;

                        map.emplace(key, value);
                        keys.push_back(key);
                }

                bool empty() const
                {
                        return map.empty();
                }

                std::vector<std::pair<K, V> > ordered_pairs() const
                {
                        std::vector<std::pair<K, V> > pairs;
                        for (auto const &key: this->keys) {
                                pairs.emplace_back(key, this->map.at(key));
                        }
                }

                ordered_map filter(std::function<bool(K, V)> predicate) const
                {
                        ordered_map filtered;
                        for (const auto &key: keys) {
                                const auto &value = map.at(key);
                                if (!predicate(key, value))
                                        continue;
                                filtered.emplace(key, value);
                        }
                        return filtered;
                }

                void for_each(std::function<void(const K &k, const V &v)> cb) const
                {
                        for (const auto &key: keys) {
                                const auto &value = map.at(key);
                                cb(key, value);
                        }
                }

                const std::vector<K> &getKeys() const
                {
                        return keys;
                }
};
