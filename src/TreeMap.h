#ifndef AISDI_MAPS_TREEMAP_H
#define AISDI_MAPS_TREEMAP_H

#include <cstddef>
#include <initializer_list>
#include <stdexcept>
#include <utility>
#include <boost/concept_check.hpp>

namespace aisdi {

    template<typename KeyType, typename ValueType>
    class TreeMap {
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
            Node *parent;
            Node *llink;
            Node *rlink;


            Node(key_type key, mapped_type value, Node *par = nullptr, Node *left = nullptr, Node *right = nullptr) :
                    info(key, value), parent(par), llink(left), rlink(right) {}

        private:
            friend class TreeMap<KeyType, ValueType>::ConstIterator;
        };


        Node *root = nullptr;


        void destroy(Node *&p) // tu moze nie pykac
        {
            if (p != nullptr) {
                destroy(p->llink);
                destroy(p->rlink);
                delete p;
                p = nullptr;
            }
        }

        Node *max_node(Node *other) const {
            if (isEmpty()) return nullptr;
            while (other->rlink != nullptr) {
                other = other->rlink;
            }
            return other;
        }

        Node *min_node(Node *other) const {
            if (isEmpty()) return nullptr;
            while (other->llink != nullptr) {
                other = other->llink;
            }
            return other;
        }

        void add(value_type elem) {
            if (root == nullptr) {
                root = new Node(elem.first, elem.second);
            } else {
                find_place(elem, root, nullptr, false);
            }
        }

        void find_place(value_type elem, Node *x, Node *parnt, bool side) {
            if (x == nullptr) {
                x = new Node(elem.first, elem.second, parnt);
                if (parnt != nullptr) {
                    if (side) { parnt->llink = x; }
                    else parnt->rlink = x;
                }
            } else if (elem.first == x->info.first) x->info.second = elem.second;
            else if (elem.first > x->info.first) {
                auto temp = x;
                x = x->rlink;
                find_place(elem, x, temp, false);
            } else if (elem.first < x->info.first) {
                auto temp = x;
                x = x->llink;
                find_place(elem, x, temp, true);
            }
        }


        Node *find_just_the_place(const key_type key) const {
            Node *temp = root;

            while (temp != nullptr) {
                if (temp->info.first == key) return temp;
                if (temp->info.first > key)
                    temp = temp->llink;
                else
                    temp = temp->rlink;
            }
            return temp;
        }


        void CopyTree(Node *from_node, Node *&new_node, Node *parnt) {
            if (from_node == nullptr)
                new_node = nullptr;
            else {
                new_node = new Node(from_node->info.first, from_node->info.second, parnt, from_node->llink,
                                    from_node->rlink);
                CopyTree(from_node->llink, new_node->llink, new_node);
                CopyTree(from_node->rlink, new_node->rlink, new_node);
            }
        }

        TreeMap() : root(nullptr) {}

        ~TreeMap() {
            destroy(root);
        }

        TreeMap(std::initializer_list<value_type>
                list) {
            for (auto &elem :list) {
                add(elem);
            }
        }

        TreeMap(const TreeMap &other) {

            CopyTree(other.root, root, nullptr);        // jeśli tak nie pyknie, to
            // trzeba zrobić root jako pierwszy - oddzielny przypadek
        }


        TreeMap(TreeMap &&other) {
            root = other.root;
            other.root = nullptr;
        }

        TreeMap &operator=(const TreeMap &other) {
            if (this == &other) return *this;

            destroy(root);
            CopyTree(other.root, root, nullptr);
            return *this;
        }

        TreeMap &operator=(TreeMap &&other) {
            if (this == &other) return *this;

            destroy(root);
            std::swap(root, other.root);        // moze swap, albo po prostu '='
            return *this;
        }

        bool isEmpty() const {
            return root == nullptr;
        }

        mapped_type &operator[](const key_type &key) {
            Node *elem = new Node(key, mapped_type{});
            if (root == nullptr) {
                root = new Node(elem->info.first, elem->info.second);
                delete elem;
                return root->info.second;
            } else {
                find_place(elem->info, root, nullptr, false);
                delete elem;
                return find_just_the_place(key)->info.second;
            }

        }

        const mapped_type &valueOf(const key_type &key) const {
            ConstIterator it(find(key));

            return it.operator*().second;
        }

        mapped_type &valueOf(const key_type &key) {
            Iterator it = find(key);
            return it.operator*().second;
        }

        const_iterator find(const key_type &key) const {
            auto x = find_just_the_place(key);
            //if (x == nullptr) throw std::out_of_range("There is no such key.");
            return ConstIterator(x, root);
        }

        iterator find(const key_type &key) {
            auto x = find_just_the_place(key);
            //if (x == nullptr) throw std::out_of_range("There is no such key.");
            return Iterator(x, root);
        }

        void remove(const key_type &key) {
            remove(find(key));
        }

        void
        remove(const const_iterator &it) {                                                                     // TO DOKOŃCXZYĆ / NAPISAĆ / NAPRAWIĆ BŁĘDY (COŚ Z KOPIUJĄCYM ALBO INNYM KONST)
            if (it == cend()) throw std::out_of_range("Removing non existing element");
            Node *to_remove = it.current;
            Node *its_parent = to_remove->parent;

            if (to_remove->llink == nullptr && to_remove->rlink == nullptr) // we remove leaf
            {
                if (its_parent != nullptr) {
                    if (to_remove == its_parent->llink) { its_parent->llink = nullptr; }
                    else if (its_parent->rlink == to_remove) { its_parent->rlink = nullptr; }
                }
                if (to_remove == root) destroy(root);
                else destroy(to_remove);
            } else if (to_remove->llink != nullptr && to_remove->rlink == nullptr) // have only llink child
            {
                if (its_parent == nullptr) //we delete root
                {
                    root = to_remove->llink;
                    root->parent = nullptr;
                } else if (its_parent->llink == to_remove) //we remove llink child
                {
                    its_parent->llink = to_remove->llink;
                    to_remove->llink->parent = its_parent;
                } else  //we remove rlink child
                {
                    its_parent->rlink = to_remove->llink;
                    to_remove->llink->parent = its_parent;
                }
                delete to_remove;

            } else if (to_remove->llink == nullptr && to_remove->rlink != nullptr) // have only rlink child
            {
                if (its_parent == nullptr) //we delete root
                {
                    root = to_remove->rlink;
                    root->parent = nullptr;
                } else if (its_parent->llink == to_remove) //we remove llink child
                {
                    its_parent->llink = to_remove->rlink;
                    to_remove->rlink->parent = its_parent;
                } else  //we remove rlink child
                {
                    its_parent->rlink = to_remove->rlink;
                    to_remove->rlink->parent = its_parent;
                }
                delete to_remove;
            } else  // have two children
            {

                Node *temp = min_node(to_remove->rlink);
                if (to_remove->parent == nullptr) // we remove root
                {
                    root = temp;
                    root->llink = to_remove->llink;
                    root->parent = nullptr;
                } else // not root
                {
                    temp->llink = to_remove->llink;
                    temp->parent = to_remove->parent;
                }
                delete to_remove;
            }
        }

        size_type getSize() const {
            if (root == nullptr) return 0;
            size_t i = 0;
            Iterator it2(max_node(root), root);
            Iterator it(min_node(root), root);
            for (; it != it2; it++) i++;
            i++;
            return i;
        }

        bool operator==(const TreeMap &other) const {
            if (getSize() != other.getSize()) return false;
            const_iterator it1 = begin();
            const_iterator it2 = other.begin();

            while (it1 != end()) {

                if ((*it1).first != (*it2).first || (*it1).second != (*it2).second)
                    return false;
                it1++;
                it2++;
            }
            return true;
        }


        bool operator!=(const TreeMap &other) const {
            return !(*this == other);
        }


        iterator begin() {
            return Iterator(min_node(root), root);
        }

        iterator end() {
            return Iterator(nullptr, root);
        }

        const_iterator cbegin() const {
            return ConstIterator(min_node(root), root);
        }

        const_iterator cend() const {
            return ConstIterator(nullptr, root);
        }

        const_iterator begin() const {
            return cbegin();
        }

        const_iterator end() const {
            return cend();
        }
    };


    template<typename KeyType, typename ValueType>
    class TreeMap<KeyType, ValueType>::ConstIterator {
    public:
        using reference = typename TreeMap::const_reference;
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = typename TreeMap::value_type;
        using pointer = const typename TreeMap::value_type *;

        Node *current;
        Node *root;

        explicit ConstIterator(Node *x, Node *y) : current(x), root(y) {}

        ConstIterator(const ConstIterator &other) {
            current = other.current;
            root = other.root;
        }

        ConstIterator &operator++() {
            if (current == nullptr) throw std::out_of_range("end++");
            else if (current->rlink != nullptr) {
                current = current->rlink;
                while (current->llink != nullptr) current = current->llink;
                return *this;
            } else {
                while (current->parent != nullptr) {
                    auto temp = current;
                    current = current->parent;
                    if (current->llink == temp) return *this;
                }
            }
            current = nullptr;
            return *this;
        }

        ConstIterator operator++(int) {
            ConstIterator temp(current, root);
            operator++();
            return temp;
        }

        ConstIterator &operator--() {
            if (current == nullptr) {
                current = GiveMax();
                return *this;
            }
            if (current == GiveMin()) throw std::out_of_range("begin--");
            if (current->llink != nullptr) {
                current = current->llink;
                while (current->rlink != nullptr) current = current->rlink;
                return *this;
            } else {
                while (current->parent != nullptr) {
                    auto temp = current;
                    current = current->parent;
                    if (current->rlink == temp) return *this;
                }
            }
            current = nullptr;
            return *this;
        }

        ConstIterator operator--(int) {
            ConstIterator temp(current, root);
            operator--();
            return temp;
        }

        Node *GiveMax() {
            if (root == nullptr) throw std::out_of_range("oor");
            ConstIterator temp(root, root);
            while (temp.current->rlink != nullptr) temp.current = temp.current->rlink;
            return temp.current;
        }

        Node *GiveMin() {
            if (root == nullptr) throw std::out_of_range("oor");
            ConstIterator temp(root, root);
            while (temp.current->llink != nullptr) temp.current = temp.current->llink;
            return temp.current;
        }

        reference operator*() const {
            if (current == nullptr) throw std::out_of_range("Referencing from end");
            return current->info;
        }

        pointer operator->() const {
            return &this->operator*();
        }

        bool operator==(const ConstIterator &other) const {
            return current == other.current;
        }

        bool operator!=(const ConstIterator &other) const {
            return !(*this == other);
        }
    };

    template<typename KeyType, typename ValueType>
    class TreeMap<KeyType, ValueType>::Iterator : public TreeMap<KeyType, ValueType>::ConstIterator {
    public:
        using reference = typename TreeMap::reference;
        using pointer = typename TreeMap::value_type *;

        explicit Iterator(Node *x, Node *y) : ConstIterator(x, y) {}

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

#endif /* AISDI_MAPS_MAP_H */
