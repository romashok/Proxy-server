#pragma once

#include <sstream>
#include <unordered_map>
#include <memory>

template<typename K, typename V>
struct lru_cache {
private:
    struct node;
    struct fast_remove_list;

public:
    lru_cache(int capacity):
        capacity(capacity),
        size(0),
        list(),
        map()
    {}

    int get_size() const noexcept {
        return size;
    }

    int get_capacity() const noexcept {
        return capacity;
    }

    void push(K key, V value) {
        auto new_node = std::make_shared<node>(key, value);
        auto it = map.find(key);

        if (it != map.end()) {
            it->second->detach();
            map.erase(it);
        } else {
            if (size == capacity) {
                K key_last = list.peek_back()->key;
                list.pop_back();
                it = map.find(key_last);
                it->second->detach();
                map.erase(it);
            } else {
                ++size;
            }
        }

        map[key] = new_node;
        list.push_front(new_node);
    }

    std::pair<bool, V> peek(K key) {
        auto it = map.find(key);
        if (it != map.end()) {
            list.sift_up(it->second);
            return std::make_pair(true, it->second->item.second);
        } else {
            return std::make_pair(false, V());
        }
    }

    std::string to_string() const {
        return list.to_string();
    }

private:
    struct node {
        node()
        {}

        node(K key, V value):
            key(key),
            item(std::make_pair(key, value)),
            prev(),
            next()
        {}

        void detach() {
            prev->next = next;
            next->prev = prev;
        }

        K key;
        std::pair<K,V> item;

        std::shared_ptr<node> prev, next;
    };

    struct fast_remove_list {
        fast_remove_list()
        {
            fake_first = std::make_shared<node>();
            fake_last = std::make_shared<node>();
            fake_first->next = fake_last;
            fake_last->prev = fake_first;
        }

        void push_front(std::shared_ptr<node> new_node) noexcept {
            auto next = fake_first->next;

            fake_first->next = new_node;
            new_node->prev = fake_first;

            new_node->next = next;
            next->prev = new_node;
        }

        std::shared_ptr<node> peek_back() noexcept {
            return fake_last->prev;
        }

        void pop_back() noexcept {
            fake_last->prev->detach();
        }

        void sift_up(std::shared_ptr<node> cur) noexcept {
            cur->detach();
            push_front(cur);
        }

        std::string to_string() const {
            std::stringstream ss;
            ss << "{";

            auto it = fake_first->next;
            while (it != fake_last) {
                ss << it->key << ": " << it->item.second;
                it = it->next;

                if (it != fake_last) {
                    ss << ", ";
                }
            }
            ss << "}";
            return ss.str();
        }

        // expected memory leak (cyclic links)
        std::shared_ptr<node> fake_first, fake_last;
    };


    int capacity, size;

    fast_remove_list list;
    std::unordered_map<K, std::shared_ptr<node>> map;
};
