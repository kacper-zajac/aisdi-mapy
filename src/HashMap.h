#ifndef AISDI_MAPS_HASHMAP_H
#define AISDI_MAPS_HASHMAP_H

#include <cstddef>
#include <initializer_list>
#include <stdexcept>
#include <utility>

namespace aisdi {

    template<typename KeyType, typename ValueType>
    class HashMap {
    public:
        using key_type = KeyType;
        using mapped_type = ValueType;
        using value_type = std::pair<const key_type, mapped_type>;
        using size_type = std::size_t;
        using reference = value_type &;
        using const_reference = const value_type &;

        class ConstIterator;

        class Iterator;

        using iterator = Iterator;
        using const_iterator = ConstIterator;

        class Node {
        public:
            value_type info;
            size_type hash;
            Node *previous;
            Node *next;

            Node(key_type x, mapped_type y, size_type h, Node *prev = nullptr, Node *nex = nullptr) :
                    info(std::make_pair(x, y)), hash(h), previous(prev), next(nex) {};
        };

        size_type hashFun(const key_type &key) const {
            auto temp = std::hash<key_type>{}(key) % HASH_SIZE;
            return temp;
        }

        size_t HASH_SIZE = 2;
        size_t size = 0;
        Node **arr;

        Node *findNode(key_type key) const {
            size_type hash_num = hashFun(key);
            auto hashed_nodes = arr[hash_num];

            while (hashed_nodes != nullptr) {
                if (hashed_nodes->info.first == key) return hashed_nodes;
                hashed_nodes = hashed_nodes->next;
            }
            return nullptr;
        }

        Node *add(value_type new_info) {
            Node *x = findNode(new_info.first);
            if (x != nullptr) {
                x->info.second = new_info.second;
                return x;
            } else {
                if (size == HASH_SIZE - 1) rehash();
                auto new_hash = hashFun(new_info.first);
                auto new_node = new Node(new_info.first, new_info.second, new_hash);
                if (arr[new_hash] == nullptr) arr[new_hash] = new_node;
                else {
                    arr[new_hash]->previous = new_node;
                    new_node->next = arr[new_hash];
                    arr[new_hash] = new_node;
                }
                size++;
                return new_node;
            }
        }

        void rehash() {
            auto new_max = HASH_SIZE * 2;
            Node **new_arr = new Node *[new_max + 1]{nullptr};

            for (auto it = begin(); it != end(); it++) {
                auto new_hash = std::hash<key_type>{}(it.current->info.first) % new_max;
                if (new_arr[new_hash] == nullptr) {
                    auto new_node = new Node(it.current->info.first, it.current->info.second, new_hash, nullptr,
                                             nullptr);
                    new_arr[new_hash] = new_node;
                } else {
                    auto new_node = new Node(it.current->info.first, it.current->info.second, new_hash, nullptr,
                                             new_arr[new_hash]);
                    new_arr[new_hash]->previous = new_node;
                    arr[new_hash] = new_node;
                }
            }
            clear();
            delete[] arr;
            size = (new_max / 2) - 1;
            arr = new_arr;
            HASH_SIZE = new_max;
        };

        void clear() {
            if (size != 0) {
                for (size_t i = 0; i < HASH_SIZE + 1; ++i) {
                    if (arr[i] != nullptr) {
                        Node *temp = arr[i];
                        while (temp != nullptr) {
                            Node *remove = temp;
                            temp = temp->next;
                            delete remove;
                        }
                        arr[i] = nullptr;
                    }
                }
            }
            size = 0;
            HASH_SIZE = 2;
        };

        HashMap() {
            arr = new Node *[HASH_SIZE + 1]{nullptr};
        }

        ~HashMap() {
            clear();
            delete[] arr;
        }

        HashMap(std::initializer_list<value_type> list) {
            arr = new Node *[HASH_SIZE + 1]{nullptr};
            for (auto &elem :list) {
                add(elem);
            }
        }

        HashMap(const HashMap &other) {
            HASH_SIZE = other.HASH_SIZE;
            size = 0;
            arr = new Node *[HASH_SIZE + 1]{nullptr};

            for (auto i = other.begin(); i != other.end(); i++)
                add(*i);
        }

        HashMap(HashMap &&other) {
            HASH_SIZE = other.HASH_SIZE;
            size = other.size;
            arr = other.arr;
            other.arr = nullptr;
            other.size = 0;
        }

        HashMap &operator=(const HashMap &other) {
            if (this == &other) return *this;
            clear();
            for (auto it = other.begin(); it != other.end(); ++it) {
                add(it.current->info);                           // może trzeba będzie po kolei żeby ta sama kolejnośc była
            }
            return *this;
        }

        HashMap &operator=(HashMap &&other) {
            if (this == &other) return *this;
            clear();
            std::swap(arr, other.arr);
            std::swap(size, other.size);
            std::swap(HASH_SIZE, other.HASH_SIZE);
            return *this;
        }

        bool isEmpty() const {
            return (size == 0);
        }

        mapped_type &operator[](const key_type &key) {
            auto x = add(std::make_pair(key, mapped_type{}));
            return x->info.second;
        }

        const mapped_type &valueOf(const key_type &key) const {
            auto node = findNode(key);
            if (node == nullptr) throw std::out_of_range("nullptr");
            return node->info.second;
        }

        mapped_type &valueOf(const key_type &key) {
            auto node = findNode(key);
            if (node == nullptr) throw std::out_of_range("nullptr");
            return node->info.second;
        }

        const_iterator find(const key_type &key) const {
            auto node = findNode(key);
            return ConstIterator(node, arr, HASH_SIZE);
        }

        iterator find(const key_type &key) {
            auto node = findNode(key);
            return Iterator(node, arr, HASH_SIZE);
        }

        void remove(const key_type &key) {
            auto to_remove = findNode(key);
            if (to_remove == nullptr) throw std::out_of_range("remove end");
            auto temp = arr[to_remove->hash];
            while (temp->info.first != to_remove->info.first) temp = temp->next;
            if (to_remove->previous != nullptr && to_remove->next != nullptr) {
                to_remove->previous->next = to_remove->next;
                to_remove->next->previous = to_remove->previous;
            } else if (to_remove->previous != nullptr) {
                to_remove->previous->next = nullptr;
            } else if (to_remove->next != nullptr) {
                to_remove->next->previous = nullptr;
            } else arr[to_remove->hash] = nullptr;
            delete to_remove;
            size--;
        }

        void remove(const const_iterator &it) {
            if (it == end()) throw std::out_of_range("remove end");
            remove(it.current->info.first);
        }

        size_type getSize() const {
            return size;
        }

        bool operator==(const HashMap &other) const {
            if (!(size + other.size)) return true;
            if (size != other.size) return false;
            auto it = begin();
            auto it_other = other.begin();
            while (it != end() && it_other != end()) {
                if (it.current->info != it_other.current->info) return false;
                it++;
                it_other++;
            }
            return true;
        }

        bool operator!=(const HashMap &other) const {
            return !(*this == other);
        }

        iterator begin() {
            if (isEmpty()) return end();
            int i = 0;
            for (; arr[i] == nullptr; ++i) {}
            return Iterator(arr[i], arr, HASH_SIZE);
        }

        iterator end() {
            return Iterator(nullptr, arr, HASH_SIZE);
        }

        const_iterator cbegin() const {
            if (isEmpty()) return cend();
            int i = 0;
            for (; arr[i] == nullptr; ++i) {}
            return ConstIterator(arr[i], arr, HASH_SIZE);
        }

        const_iterator cend() const {
            return Iterator(nullptr, arr, HASH_SIZE);
        }

        const_iterator begin() const {
            return cbegin();
        }

        const_iterator end() const {
            return cend();
        }
    };

    template<typename KeyType, typename ValueType>
    class HashMap<KeyType, ValueType>::ConstIterator {
    public:
        using reference = typename HashMap::const_reference;
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = typename HashMap::value_type;
        using pointer = const typename HashMap::value_type *;

        friend class HashMap;

    private:
        Node *current;
        Node **arr;
        size_t HASH_SIZE;
    public:
        explicit ConstIterator(Node *current = nullptr, Node **arr = nullptr, size_t HASH_SIZE = 0) :
                current(current), arr(arr), HASH_SIZE(HASH_SIZE) {}

        ConstIterator(const ConstIterator &other) {
            current = other.current;
            arr = other.arr;
            HASH_SIZE = other.HASH_SIZE;
        }

        ConstIterator &operator++() {
            if (current == nullptr) throw std::out_of_range("end++");
            if (current->next != nullptr) current = current->next;
            else {
                auto new_hash = ++current->hash;
                while (arr[new_hash] == nullptr) {
                    if (new_hash >= HASH_SIZE) {
                        current = nullptr;
                        return *this;
                    }
                    new_hash++;
                }
                current = arr[new_hash];
            }
            return *this;
        }

        ConstIterator operator++(int) {
            auto temp = *this;
            operator++();
            return temp;
        }

        ConstIterator &operator--() {
            if (current == nullptr) {
                auto new_hash = HASH_SIZE - 1;
                while (arr[new_hash] == nullptr) {
                    new_hash--;
                    if (new_hash <= 0) throw std::out_of_range("begin--");
                }
                current = arr[new_hash];
                while (current->next != nullptr) current = current->next;
            } else if (current->hash == 0) throw std::out_of_range("begin--");
            else if (current->previous != nullptr) current = current->previous;
            else {
                auto new_hash = current->hash--;
                while (arr[new_hash] == nullptr) {
                    new_hash--;
                    if (new_hash < 0) throw std::out_of_range("begin--");
                }
                current = arr[new_hash];
                while (current->next != nullptr) current = current->next;
            }
            return *this;
        }

        ConstIterator operator--(int) {
            auto temp = *this;
            operator--();
            return temp;
        }

        reference operator*() const {
            if (current == nullptr) throw std::out_of_range("*nullptr");
            return current->info;
        }

        pointer operator->() const {
            return &this->operator*();
        }

        bool operator==(const ConstIterator &other) const {
            return (current == other.current);              // tutaj chyba trzeb ->info ale jak będize nullptr to błąd
        }

        bool operator!=(const ConstIterator &other) const {
            return !(*this == other);
        }
    };

    template<typename KeyType, typename ValueType>
    class HashMap<KeyType, ValueType>::Iterator : public HashMap<KeyType, ValueType>::ConstIterator {
    public:
        using reference = typename HashMap::reference;
        using pointer = typename HashMap::value_type *;

        explicit Iterator(Node *current, Node **arr, size_t HASH_SIZE = 0) : ConstIterator(current, arr, HASH_SIZE) {}

        Iterator(const ConstIterator &other)
                : ConstIterator(other) {}

        Iterator &operator++() {
            ConstIterator::operator++();
            return *this;
        }

        Iterator operator++(int) {
            auto result = *this;
            ConstIterator::operator++();
            return result;
        }

        Iterator &operator--() {
            ConstIterator::operator--();
            return *this;
        }

        Iterator operator--(int) {
            auto result = *this;
            ConstIterator::operator--();
            return result;
        }

        pointer operator->() const {
            return &this->operator*();
        }

        reference operator*() const {
            // ugly cast, yet reduces code duplication.
            return const_cast<reference>(ConstIterator::operator*());
        }
    };

}

#endif /* AISDI_MAPS_HASHMAP_H */
