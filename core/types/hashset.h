//
// Created by cycastic on 8/1/2023.
//

#ifndef NEXUS_HASHSET_H
#define NEXUS_HASHSET_H

#include "../exception.h"
#include "../hashfuncs.h"
#include "../comparator.h"

class HashSetException : public Exception {
public:
    explicit HashSetException(const char *p_msg) : Exception(p_msg) {}
};

template <typename T, class Hasher = StandardHasher, class Comparator = StandardComparator<T>>
class StaticHashSet {
public:
    struct Cell {
        T data;
        Cell* next{};
        explicit Cell(const T& p_data) : data(p_data) {}
        explicit Cell(T&& p_data) : data(p_data) {}

        static _FORCE_INLINE_ void cleanup(Cell* p_target) {
            // Anti-recursion gang!
            auto it = p_target;
            while (it){
                auto removal_target = it;
                it = it->next;
                delete removal_target;
            }
        }
    };
private:
    size_t cap;
    size_t entries_count{};
    Cell** entries{};
public:
    _FORCE_INLINE_ uint32_t get_index(const T& p_value) const {
        return Hasher::hash(p_value) % cap;
    }
private:
    Cell* try_get(const T& p_value){
        auto idx = get_index(p_value);
        auto entry = entries[idx];
        if (!entry) {
            return nullptr;
        }
        while (entry){
            if (Comparator::compare(entry->data, p_value)) return entry;
            entry = entry->next;
        }
        return nullptr;
    }
public:
    void try_add(const T& p_value){
        if (try_get(p_value)) return;
        auto idx = get_index(p_value);
        auto old_entry = entries[idx];
        auto new_entry = new Cell(p_value);
        new_entry->next = old_entry;
        entries[idx] = new_entry;
        entries_count++;
    }
    bool try_erase(const T& p_value) noexcept {
        auto idx = get_index(p_value);
        auto hash = Hasher::hash(p_value);
        auto entry = entries[idx];
        Cell* prev = nullptr;
        if (!entry) return false;
        while (entry){
            if (hash == Hasher::hash(entry->data) && Comparator::compare(hash, entry->data)){
                if (prev) prev->next = entry->next;
                else entries[idx] = entry->next;
                delete entry;
                entries_count--;
                return true;
            }
            prev = entry;
            entry = entry->next;
        }
        return false;
    }
    void free_all_cells(){
        for (size_t i = 0; i < cap; i++){
            Cell::cleanup(entries[i]);
        }
        free(entries);
    }
    void reset_table(){
        free_all_cells();
        entries = array_alloc<Cell*>(cap);
        for (size_t i = 0; i < cap; i++){
            entries[i] = nullptr;
        }
        entries_count = 0;
    }
    Cell* get_next_ptr(Cell* p_ptr = nullptr){
        int64_t current_index = -1;
        auto capacity = cap;
        Cell* iterating = p_ptr;
        if (p_ptr) current_index = get_index(p_ptr->data);
        while (current_index < int64_t(capacity)){
            if (iterating == nullptr){
                ++current_index;
                if (current_index >= capacity) return iterating;
                iterating = entries[current_index];
                if (iterating) return iterating;
                else continue;
            }
            iterating = iterating->next;
            if (iterating) return iterating;
        }
        return nullptr;
    }
public:
    class Iterator;
    class ConstIterator;

    friend class Iterator;
    friend class ConstIterator;

    class ConstIterator {
    private:
        StaticHashSet* set;
        Cell* iterating{};
    public:
        explicit ConstIterator(const StaticHashSet* p_set) : set(const_cast<StaticHashSet*>(p_set)) {}
        bool move_next(){
            if (set == nullptr) return false;
            iterating = set->get_next_ptr(iterating);
            if (!iterating) set = nullptr;
            return iterating != nullptr;
        }
        _FORCE_INLINE_ const Cell& get_value() const {
            return *iterating;
        }
    };
    class Iterator {
    private:
        StaticHashSet* set;
        Cell* iterating{};
        Cell* prev{};
    public:
        explicit Iterator(StaticHashSet* p_set) : set(p_set) {}
        bool move_next() {
            if (set == nullptr) return false;
            prev = iterating;
            iterating = iterating->next;
            if (!iterating) set = nullptr;
            return iterating != nullptr;
        }
        void erase(){
            if (!set || !iterating) return;
            if (prev) prev->next = iterating->next;
            else set->entries[set->get_index(iterating->data)] = nullptr;
            delete iterating;
            iterating = prev;
            set->entries_count--;
            if (set->entries_count == 0) set = nullptr;
        }
        _FORCE_INLINE_ Cell& get_value() {
            return *iterating;
        }
    };

    _NO_DISCARD_ _ALWAYS_INLINE_ uint16_t capacity() const { return cap; }
    _NO_DISCARD_ _FORCE_INLINE_ size_t size() const { return entries_count; }
    _NO_DISCARD_ _FORCE_INLINE_ bool empty() const { return size() == 0; }
    _NO_DISCARD_ _ALWAYS_INLINE_ ConstIterator const_iterator() const { return ConstIterator(this); }
    _NO_DISCARD_ _ALWAYS_INLINE_ Iterator iterator() { return Iterator(this); }
private:
    void copy_from(const StaticHashSet& p_other){
        ConstIterator it = p_other.const_iterator();
        while (it.move_next()){
            get_or_create(it.get_value().data) = it.get_value().data;
        }
    }
public:
    _NO_DISCARD_ _FORCE_INLINE_ bool has(const T& p_data) const { return try_get(p_data) != nullptr; }
    _FORCE_INLINE_ void add(const T& p_data) { try_add(p_data); }
    _FORCE_INLINE_ void clear() { reset_table(); }
    _FORCE_INLINE_ StaticHashSet& operator=(const StaticHashSet& p_other){
        reset_table();
        copy_from(p_other);
        return *this;
    }
    _FORCE_INLINE_ void erase(const T& p_data) { try_erase(p_data); }
    explicit StaticHashSet(const size_t& p_starting_capacity = 128){
        cap = p_starting_capacity;
        entries = array_alloc<Cell*>(cap);
        for (size_t i = 0; i < cap; i++)
            entries[i] = nullptr;
    }
    ~StaticHashSet(){
        free_all_cells();
    }
};
template <typename T, class Hasher = StandardHasher, class Comparator = StandardComparator<T>>
class HashSet {
public:
    class EmptyConstIterator : public StaticHashSet<T, Hasher, Comparator>::ConstIterator {
    public:
        EmptyConstIterator() : StaticHashSet<T, Hasher, Comparator>::ConstIterator(nullptr) {}
    };
    class EmptyIterator : public StaticHashSet<T, Hasher, Comparator>::Iterator {
    public:
        EmptyIterator() : StaticHashSet<T, Hasher, Comparator>::Iterator(nullptr) {}
    };
private:
    const size_t initial_capacity;
    float growth_factor;
    StaticHashSet<T, Hasher, Comparator> *current_set = nullptr;

    _FORCE_INLINE_ void initialize(const size_t& p_capacity){
        current_set = new StaticHashSet<T, Hasher, Comparator>(p_capacity);
    }
    _FORCE_INLINE_ void initialize(){
        initialize(initial_capacity);
    }
public:
    _NO_DISCARD_ _ALWAYS_INLINE_ bool is_initialized() const { return current_set != nullptr; }
    _NO_DISCARD_ _FORCE_INLINE_ uint32_t get_index(const T& p_data) {
        if (!is_initialized()) initialize();
        return current_set->get_index(p_data);
    }
    _FORCE_INLINE_ void clear(){
        delete current_set;
        current_set = nullptr;
    }
    _NO_DISCARD_ _FORCE_INLINE_ bool has(const T& p_data) const {
        if (!is_initialized()) return false;
        return current_set->has(p_data);
    }
    _NO_DISCARD_ _FORCE_INLINE_ size_t size() const {
        if (!is_initialized()) return 0;
        return current_set->size();
    }
    _NO_DISCARD_ _FORCE_INLINE_ bool empty() const {
        return !is_initialized() || size() == 0;
    }
    _NO_DISCARD_ _ALWAYS_INLINE_ uint16_t capacity() const {
        if (!is_initialized()) return 0;
        return current_set->capacity();
    }
    _NO_DISCARD_ _ALWAYS_INLINE_ typename StaticHashSet<T, Hasher, Comparator>::ConstIterator const_iterator() const {
        if (!is_initialized()) return EmptyConstIterator();
        return current_set->const_iterator();
    }
    _NO_DISCARD_ _ALWAYS_INLINE_ typename StaticHashSet<T, Hasher, Comparator>::Iterator iterator() {
        if (!is_initialized()) return EmptyIterator();
        return current_set->iterator();
    }
private:
    _FORCE_INLINE_ void copy(const HashSet& p_other){
        auto new_cap = p_other.capacity();
        initialize(new_cap ? new_cap : initial_capacity);
        auto it = p_other.const_iterator();
        while (it.move_next()){
            current_set->add(it.get_value()->data);
        }
    }
public:
    _FORCE_INLINE_ HashSet& operator=(const HashSet& p_other){
        clear();
        copy(p_other);
        return *this;
    }
    _FORCE_INLINE_ void add(const T& p_data){
        if (!is_initialized()) initialize();
        current_set->add(p_data);
        const auto curr_factor = float(size()) / capacity();
        if (curr_factor < growth_factor) return;

        auto new_set = new StaticHashSet<T, Hasher, Comparator>(capacity() * 2);
        auto it = current_set->const_iterator();
        while (it.move_next()) {
            const auto& cell = it.get_value();
            new_set->add(cell.data);
        }
        delete current_set;
        current_set = new_set;
    }
    explicit HashSet(const float& p_growth = 0.75, const size_t& p_initial_capacity = 128, const bool& p_lazy_evaluation = true)
    : growth_factor(p_growth), initial_capacity(p_initial_capacity) {
        if (!p_lazy_evaluation) initialize();
    }
    HashSet(const HashSet& p_other) : growth_factor(p_other.growth_factor), initial_capacity(p_other.initial_capacity) {
        copy(p_other);
    }
    ~HashSet() {
        clear();
    }
};

#endif //NEXUS_HASHSET_H
