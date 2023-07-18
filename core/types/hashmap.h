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
public:
    struct KPIterator {
    private:
        KeyPairValue** entries{};
        int64_t capacity{};
        KeyPairValue* iterating{};
        int64_t current_index{-1};
    public:
        KPIterator(KeyPairValue** e, size_t cap){
            entries = e;
            capacity = (int64_t)cap;
        }
        bool move_next(){
            while (current_index < capacity){
                if (iterating && iterating->next) {
                    iterating = iterating->next;
                    return true;
                }
                else {
                    iterating = entries[++current_index];
                    if (current_index < capacity && iterating) return true;
                }
            }
            return false;
        }
        const KeyPairValue& get_pair() const {
            return *iterating;
        }
    };
private:
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
                entries_count--;
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
    _FORCE_INLINE_ Value& operator[](const Key& p_key) { return get_or_create(p_key)->value; }
    _NO_DISCARD_ _FORCE_INLINE_ bool exists(const Key& p_key) const { return try_get(p_key) != nullptr; }
    _NO_DISCARD_ _FORCE_INLINE_ size_t size() const { return entries_count; }
    _NO_DISCARD_ _FORCE_INLINE_ bool empty() const { return size() == 0; }
    _NO_DISCARD_ _ALWAYS_INLINE_ uint16_t capacity() const { return cap; }
    _NO_DISCARD_ _ALWAYS_INLINE_ KPIterator iterator() { return KPIterator(entries, capacity()); }
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

template <typename Key, typename Value, uint16_t StartingCapacity = 32, class Hasher = StandardHasher, class Comparator = StandardComparator<Key>>
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
        const auto curr_factor = (float)size() / capacity();
        if (curr_factor >= growth_factor){
            auto new_map = new StaticHashMap<Key, Value, Hasher, Comparator>(capacity() * 2);
            auto it = current_map->iterator();
            while (it.move_next()){
                const auto& kp = it.get_pair();
                new_map->operator[](kp.key) = kp.value;
            }
            delete current_map;
            current_map = new_map;
        }
        return current_map->operator[](p_key);
    }
    _NO_DISCARD_ _FORCE_INLINE_ bool exists(const Key& p_key) const {
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
    _NO_DISCARD_ _ALWAYS_INLINE_ typename StaticHashMap<Key, Value, Hasher, Comparator>::KPIterator iterator() {
        if (!is_initialized()) throw std::exception();
        return current_map->iterator();
    }
    _FORCE_INLINE_ bool erase(const Key& p_key) {
        if (!is_initialized()) return false;
        return current_map->erase(p_key);
    }
    explicit HashMap(const float& growth = 0.75){
        growth_factor = growth;
    }
};
template <typename Value, class Hasher = StandardHasher, class Comparator = StandardComparator<Value>, uint16_t StartingCapacity = 32>
class HashSet{
private:
    HashMap<Value, uint8_t, StartingCapacity, Hasher, Comparator> inner_map{};
public:
    static _FORCE_INLINE_ HashSet from_array(const Value* p_arr, const size_t& p_len){
        HashSet re{};
        for (size_t i = 0; i < p_len; i++){
            re.add(p_arr[i]);
        }
        return re;
    }

    _FORCE_INLINE_ void add(const Value& p_value) { inner_map[p_value] = 0; }
    _FORCE_INLINE_ bool erase(const Value& p_value) { return inner_map.erase(p_value); }
    _NO_DISCARD_ _FORCE_INLINE_ bool exists(const Value& p_value) const { return inner_map.exists(p_value); }
    _NO_DISCARD_ _FORCE_INLINE_ size_t size() const { return inner_map.size(); }
    _NO_DISCARD_ _FORCE_INLINE_ bool empty() const { return inner_map.empty(); }
};
#endif //NEXUS_HASHMAP_H
