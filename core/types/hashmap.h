//
// Created by cycastic on 7/18/2023.
//

#ifndef NEXUS_HASHMAP_H
#define NEXUS_HASHMAP_H

#include "../hashfuncs.h"
#include "../comparator.h"

template <typename Key, typename Value, class Hasher = StandardHasher, class Comparator = StandardComparator<Key>>
class StaticHashMap{
private:
    struct KeyPairValue {
        Key key;
        Value value;
        KeyPairValue* next{};

        explicit KeyPairValue(const Key& p_key) { key = p_key; }
        ~KeyPairValue(){
            delete next;
        }
    };
private:
    size_t cap{};
    size_t entries_count{};
    KeyPairValue** entries{};
    KeyPairValue* try_get(const Key& p_key) const {
        auto hash = get_index(p_key);
        auto entry = entries[hash];
        if (!entry) {
            return nullptr;
        }
        while (entry){
            if (Comparator::compare(entry->key, p_key)) return entry;
            entry = entry->next;
        }
        return nullptr;
    }
    const KeyPairValue* get(const Key& p_key) const {
        auto result = try_get(p_key);
        if (!result)
            throw std::exception();
        return result;
    }
    KeyPairValue* get_or_create(const Key& p_key){
        auto trial = try_get(p_key);
        if (trial) return trial;
        auto hash = get_index(p_key);
        auto old_entry = entries[hash];
        auto new_entry = new KeyPairValue(p_key);
        new_entry->next = old_entry;
        entries[hash] = new_entry;
        entries_count++;
        return new_entry;
    }
    bool try_erase(const Key& p_key) noexcept {
        auto hash = get_index(p_key);
        auto entry = entries[hash];
        KeyPairValue* prev = nullptr;
        if (!entry) return false;
        while (entry){
            // Check hash first before comparing
            if (Hasher::hash(p_key) == Hasher::hash(entry->key) && Comparator::compare(entry->key, p_key)) {
                if (prev) prev->next = entry->next;
                else entries[hash] = entry->next;
                delete entry;
                entries_count--;
                return true;
            }
            prev = entry;
            entry = entry->next;
        }
        return false;
    }
    void reset_table(){
        for (size_t i = 0; i < cap; i++){
            delete entries[i];
        }
        free(entries);
        entries = array_alloc<KeyPairValue*>(cap);
        for (size_t i = 0; i < cap; i++){
            entries[i] = nullptr;
        }
    }
    KeyPairValue* get_next_ptr(KeyPairValue* p_ptr = nullptr){
        int64_t current_index = -1;
        auto capacity = cap;
        KeyPairValue* iterating = p_ptr;
        if (p_ptr) current_index = get_index(p_ptr->key);
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
        StaticHashMap* map;
        KeyPairValue* iterating{};
    protected:
        ConstIterator() = default;
    public:
        explicit ConstIterator(const StaticHashMap* p_map) : map(const_cast<StaticHashMap*>(p_map)) {}
        bool move_next() {
            if (map == nullptr) return false;
            iterating = map->get_next_ptr(iterating);
            if (!iterating) map = nullptr;
            return iterating != nullptr;
        }
        _FORCE_INLINE_ const KeyPairValue& get_pair() const {
            return *iterating;
        }
    };
    class Iterator {
    private:
        StaticHashMap* map;
        KeyPairValue* iterating{};
        KeyPairValue* prev_iterator{};
    protected:
        Iterator() = default;
    public:
        explicit Iterator(StaticHashMap* p_map) : map(p_map) {}
        bool move_next() {
            if (map == nullptr) return false;
            prev_iterator = iterating;
            iterating = map->get_next_ptr(prev_iterator);
            if (!iterating) map = nullptr;
            return iterating != nullptr;
        }
        _FORCE_INLINE_ void erase(){
            if (map == nullptr) return;
            if (prev_iterator){
                prev_iterator->next = iterating->next;
            } else {
                map->entries[map->get_index(iterating->key)] = nullptr;
            }
            delete iterating;
            iterating = prev_iterator;
            map->entries_count--;
            if (map->entries_count == 0) map = nullptr;
        }
        _FORCE_INLINE_ const KeyPairValue& get_pair() const {
            return *iterating;
        }
    };
    _NO_DISCARD_ _ALWAYS_INLINE_ uint16_t capacity() const { return cap; }
    _NO_DISCARD_ _ALWAYS_INLINE_ ConstIterator const_iterator() const { return ConstIterator(this); }
    _NO_DISCARD_ _ALWAYS_INLINE_ Iterator iterator() { return Iterator(this); }
private:
    void copy_from(const StaticHashMap& p_other){
        ConstIterator it = p_other.const_iterator();
        while (it.move_next()){
            get_or_create(it.get_pair().key) = it.get_pair().value;
        }
    }
public:
    _FORCE_INLINE_ uint32_t get_index(const Key& p_key) const {
        return Hasher::hash(p_key) % capacity();
    }
    _FORCE_INLINE_ const Value& operator[](const Key& p_key) const { return get(p_key)->value; }
    _FORCE_INLINE_ Value& operator[](const Key& p_key) { return get_or_create(p_key)->value; }
    _NO_DISCARD_ _FORCE_INLINE_ bool exists(const Key& p_key) const { return try_get(p_key) != nullptr; }
    _NO_DISCARD_ _FORCE_INLINE_ size_t size() const { return entries_count; }
    _NO_DISCARD_ _FORCE_INLINE_ bool empty() const { return size() == 0; }
    _FORCE_INLINE_ StaticHashMap& operator=(const StaticHashMap& p_other){
        clear();
        copy_from(p_other);
        return *this;
    }
    _FORCE_INLINE_ bool erase(const Key& p_key) noexcept { return try_erase(p_key); }
    _FORCE_INLINE_ void clear() { reset_table(); }
    explicit StaticHashMap(const size_t& starting_capacity = 150){
        cap = starting_capacity;
        entries = array_alloc<KeyPairValue*>(cap);
        for (size_t i = 0; i < cap; i++){
            entries[i] = nullptr;
        }
    }
    ~StaticHashMap(){
        for (size_t i = 0; i < cap; i++){
            delete entries[i];
        }
        free(entries);
    }
};

template <typename Key, typename Value, uint16_t StartingCapacity = 32, class Hasher = StandardHasher, class Comparator = StandardComparator<Key>>
class HashMap {
public:
    class EmptyConstIterator : public StaticHashMap<Key, Value, Hasher, Comparator>::ConstIterator {
    public:
        EmptyConstIterator() : StaticHashMap<Key, Value, Hasher, Comparator>::ConstIterator(nullptr) {}
    };
    class EmptyIterator : public StaticHashMap<Key, Value, Hasher, Comparator>::Iterator {
    public:
        EmptyIterator() : StaticHashMap<Key, Value, Hasher, Comparator>::Iterator(nullptr) {}
    };
private:
    float growth_factor;
    StaticHashMap<Key, Value, Hasher, Comparator> *current_map = nullptr;
    _FORCE_INLINE_ void initialize(){
        current_map = new StaticHashMap<Key, Value, Hasher, Comparator>(StartingCapacity);
    }
public:
    _NO_DISCARD_ _ALWAYS_INLINE_ bool is_initialized() const { return current_map != nullptr; }
    _FORCE_INLINE_ uint32_t get_index(const Key& p_key) const {
        if (!is_initialized()) initialize();
        return current_map->get_index(p_key);
    }
    _FORCE_INLINE_ void clear(){
        if (!is_initialized()) return;
        delete current_map;
        current_map = nullptr;
    }
private:
    _FORCE_INLINE_ void copy(const HashMap& p_other){
        clear(); initialize();
        auto it = p_other.const_iterator();
        while (it.move_next()){
            operator[](it.get_pair().key) = it.get_pair().value;
        }
    }
public:
    _FORCE_INLINE_ const Value& operator[](const Key& p_key) const {
        // TODO: Custom Exception
        if (!is_initialized()) throw std::exception();
        return current_map->operator[](p_key);
    }
    _FORCE_INLINE_ Value& operator[](const Key& p_key) {
        // TODO: Custom Exception
        // Resize
        if (!is_initialized()) initialize();
        const auto curr_factor = (float)size() / capacity();
        if (curr_factor >= growth_factor){
            auto new_map = new StaticHashMap<Key, Value, Hasher, Comparator>(capacity() * 2);
            auto it = current_map->const_iterator();
            while (it.move_next()){
                const auto& kp = it.get_pair();
                new_map->operator[](kp.key) = kp.value;
            }
            delete current_map;
            current_map = new_map;
        }
        return current_map->operator[](p_key);
    }
    _FORCE_INLINE_ HashMap& operator=(const HashMap& p_other){
        copy(p_other);
        return *this;
    }
    _NO_DISCARD_ _FORCE_INLINE_ bool has(const Key& p_key) const {
        if (!is_initialized()) return false;
        return current_map->exists(p_key);
    }
    _NO_DISCARD_ _FORCE_INLINE_ size_t size() const {
        if (!is_initialized()) return 0;
        return current_map->size();
    }
    _NO_DISCARD_ _FORCE_INLINE_ bool empty() const {
        return !is_initialized() || size() == 0;
    }
    _NO_DISCARD_ _ALWAYS_INLINE_ uint16_t capacity() const {
        if (!is_initialized()) return 0;
        return current_map->capacity();
    }
    _NO_DISCARD_ _ALWAYS_INLINE_ typename StaticHashMap<Key, Value, Hasher, Comparator>::ConstIterator const_iterator() const {
        if (!is_initialized()) return EmptyConstIterator();
        return current_map->const_iterator();
    }
    _NO_DISCARD_ _ALWAYS_INLINE_ typename StaticHashMap<Key, Value, Hasher, Comparator>::Iterator iterator() {
        if (!is_initialized()) return EmptyIterator();
        return current_map->iterator();
    }
    _FORCE_INLINE_ bool erase(const Key& p_key) noexcept {
        if (!is_initialized()) return false;
        return current_map->erase(p_key);
    }
    explicit HashMap(const float& growth = 0.75){
        growth_factor = growth;
    }
    HashMap(const HashMap& p_other) : HashMap() {
        copy(p_other);
    }
};

#endif //NEXUS_HASHMAP_H
