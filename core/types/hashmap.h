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

        ~KeyPairValue(){
            delete next;
        }
    };
    struct KPIterator {
    private:
        KeyPairValue** entries{};
        size_t capacity{};
        KeyPairValue* iterating{};
        int64_t current_index{-1};
    public:
        KPIterator(KeyPairValue** e, size_t cap){
            entries = e;
            capacity = cap;
        }
        bool move_next(){
            auto loop_ran = false;
            while (!iterating && current_index < capacity){
                loop_ran = true;
                iterating = entries[++current_index];
            }
            if (current_index >= capacity) {
                iterating = nullptr; return false;
            }
            if (!loop_ran) iterating = iterating->next;
            return true;
        }
        const KeyPairValue& get_pair() const {
            return iterating;
        }
    };
    size_t cap{};
    size_t entries_count{};
    KeyPairValue** entries{};
    const KeyPairValue* get(const Key& p_key) const {
        auto hash = get_index(p_key);
        auto entry = entries[hash];
        // TODO: Custom exception
        // No such entry exist
        if (!entry) throw std::exception();
        while (entry){
            if (Comparator::compare(entry->key, p_key)) return entry;
            entry = entry->next;
        }
        throw std::exception();
    }
    KeyPairValue* try_get(const Key& p_key) {
        auto hash = get_index(p_key);
        auto entry = entries[hash];
        // Create if not exist
        if (!entry) {
            auto new_entry = new KeyPairValue();
            new_entry->key = p_key;
            entries[hash] = new_entry;
            entries_count++;
            return new_entry;
        }
        while (entry){
            if (Comparator::compare(entry->key, p_key)) return entry;
            entry = entry->next;
        }
        // Insert at front if not exist
        auto new_entry = new KeyPairValue();
        new_entry->key = p_key;
        new_entry->next = entries[hash];
        entries[hash] = new_entry;
        entries_count++;
        return new_entry;
    }
    bool try_erase(const Key& p_key){
        auto hash = get_index(p_key);
        auto entry = entries[hash];
        KeyPairValue* prev = nullptr;
        if (!entry) return false;
        while (entry){
            if (Comparator::compare(entry->key, p_key)) {
                if (prev) prev->next = entry->next;
                else entries[hash] = entry->next;
                delete entry;
                return true;
            }
            prev = entry;
            entry = entry->next;
        }
        return false;
    }
public:
    _FORCE_INLINE_ uint32_t get_index(const Key& p_key) const {
        return Hasher::hash(p_key) % capacity();
    }
    _FORCE_INLINE_ const Value& operator[](const Key& p_key) const { return get(p_key)->value; }
    _FORCE_INLINE_ Value& operator[](const Key& p_key) { return try_get(p_key)->value; }
    _NO_DISCARD_ _FORCE_INLINE_ size_t size() const { return entries_count; }
    _NO_DISCARD_ _ALWAYS_INLINE_ uint16_t capacity() const { return cap; }
    _NO_DISCARD_ _ALWAYS_INLINE_ KPIterator iterator() { return new KPIterator(entries, capacity()); }
    _FORCE_INLINE_ bool erase(const Key& p_key) { return try_erase(p_key); }
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
    }
};

template <typename Key, typename Value, class Hasher = StandardHasher, class Comparator = StandardComparator<Key>, uint16_t StartingCapacity = 32>
class HashMap{
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
    _FORCE_INLINE_ const Value& operator[](const Key& p_key) const {
        // TODO: Custom exception
        if (!is_initialized()) throw std::exception();
        return current_map->operator[](p_key);
    }
    _FORCE_INLINE_ Value& operator[](const Key& p_key) {
        // TODO: Custom exception
        // Resize
        if (!is_initialized()) initialize();
        if (size() / capacity() >= growth_factor){
            auto new_map = new StaticHashMap<Key, Value, Hasher, Comparator>(capacity() * (1 + growth_factor));
            auto it = current_map->iterator();
            while (it.move_next()){
                const auto& kp = it.get_pair();
                new_map[kp.key] = kp.value;
            }
            delete current_map;
            current_map = new_map;
        }
        return current_map->operator[](p_key);
    }
    _NO_DISCARD_ _FORCE_INLINE_ size_t size() const {
        if (!is_initialized()) return 0;
        return current_map->size();
    }
    _NO_DISCARD_ _ALWAYS_INLINE_ uint16_t capacity() const {
        if (!is_initialized()) return 0;
        return current_map->capacity();
    }
    _FORCE_INLINE_ bool erase(const Key& p_key) {
        if (!is_initialized()) return false;
        return current_map->erase(p_key);
    }
    explicit HashMap(const float& growth = 0.75){
        growth_factor = growth;
    }
};

#endif //NEXUS_HASHMAP_H
