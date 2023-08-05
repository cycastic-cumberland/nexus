//
// Created by cycastic on 8/1/2023.
//

#ifndef NEXUS_HASH_SET_H
#define NEXUS_HASH_SET_H

#include "../hashfuncs.h"
#include "../comparator.h"

template <typename T, class Hasher = StandardHasher, class Comparator = StandardComparator<T>>
class StaticHashSet {
private:
    struct Cell {
        T data;
        Cell* next{};
        explicit Cell(const T& p_data) : data(p_data) {}
        ~Cell() { delete next; }
    };
    size_t cap;
    size_t entries_count{};
    Cell** entries{};
public:
    _FORCE_INLINE_ uint32_t get_index(const T& p_value) const {
        return Hasher::idx(p_value) % cap;
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
    const Cell* get(const T& p_value) const {
        auto result = try_get(p_value);
        if (!result)
            throw std::exception();
        return result;
    }
    Cell* get_or_create(const T& p_value){
        auto trial = try_get(p_value);
        if (trial) return trial;
        auto idx = get_index(p_value);
        auto old_entry = entries[idx];
        auto new_entry = new Cell(p_value);
        new_entry->next = old_entry;
        entries[idx] = new_entry;
        entries_count++;
        return new_entry;
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
    void reset_table(){
        for (size_t i = 0; i < cap; i++){
            delete entries[i];
        }
        free(entries);
        entries = array_alloc<Cell*>(cap);
        for (size_t i = 0; i < cap; i++){
            entries[i] = nullptr;
        }
        entries_count = 0;
    }
};

#endif //NEXUS_HASH_SET_H
