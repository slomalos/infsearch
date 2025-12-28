#ifndef MY_HASHMAP_HPP
#define MY_HASHMAP_HPP

#include <vector>
#include <string>

size_t my_string_hash(const std::string& k) {
    std::hash<std::string> hasher;
    return hasher(k);
}

template <typename Key, typename Value>
class MyHashMap {
public:
    struct Node {
        Key key;
        Value value;
    };

private:
    std::vector<std::vector<Node>> buckets;
    size_t table_size;

public:
    MyHashMap(size_t size = 100003) : table_size(size) {
        buckets.resize(size);
    }

    Value& operator[](const Key& key) {
        size_t index = my_string_hash(key) % table_size;
        std::vector<Node>& bucket = buckets[index];

        for (auto& node : bucket) {
            if (node.key == key) {
                return node.value;
            }
        }

        bucket.push_back({key, Value()});
        return bucket.back().value;
    }

    void get_all_pairs(std::vector<Node>& out_vec) {
        for(const auto& bucket : buckets) {
            for(const auto& node : bucket) {
                out_vec.push_back(node);
            }
        }
    }
};

#endif