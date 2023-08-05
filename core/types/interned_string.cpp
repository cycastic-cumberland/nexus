//
// Created by cycastic on 7/27/2023.
//

#include "interned_string.h"
#include "../hashfuncs.h"
#include "../../runtime/nexus_output.h"

BinaryMutex InternedString::lock{};
InternedString::StringCell* InternedString::table[InternedString::STRING_TABLE_LEN]{};
bool InternedString::configured = false;

void InternedString::configure() {
    // No need to lock
    if (configured) return;
    for (auto & i : table){
        i = nullptr;
    }
    configured = true;
}

InternedString::InternedString() {
    // InternedString will most likely be configured beforehand anyway, so no need to lock
    if (!InternedString::configured)
        throw StringConstantException("InternedString has not been configured");
}

InternedString::InternedString(const InternedString &p_other) : InternedString() {
    ref(const_cast<InternedString::StringCell*>(p_other.occupying));
}

InternedString::InternedString(const char* p_from) : InternedString(VString(p_from).c_str()) {}
InternedString::InternedString(const VString &p_from) : InternedString(p_from.c_str()) {}
InternedString::InternedString(const wchar_t* p_from) : InternedString() {
    if (!p_from || p_from[0] == 0) {
        occupying = nullptr;
        return; //empty, ignore
    }
    InternedString::lock.lock();
    auto hash = StandardHasher::hash(p_from);
    auto idx = hash & STRING_TABLE_MASK;
    auto current_cell = InternedString::table[idx];
    while (current_cell){
        if (current_cell->hash == hash && current_cell->data == p_from)
            break;
        current_cell = current_cell->next;
    }
    if (current_cell){
        ref(current_cell);
        InternedString::lock.unlock();
        return;
    }
    current_cell = new StringCell{
        .prev = nullptr,
        .next = InternedString::table[idx],
        .hash = hash,
        .refcount{},
        .data{p_from}
    };
    current_cell->refcount.init();
    if (InternedString::table[idx]) {
        InternedString::table[idx]->prev = current_cell;
        current_cell->next = InternedString::table[idx];
    }
    InternedString::table[idx] = current_cell;
    occupying = current_cell;
    InternedString::lock.unlock();
}
InternedString::~InternedString() {
    unref();
}

void InternedString::ref(InternedString::StringCell *cell) {
    unref();
    if (!cell) occupying = nullptr;
    else occupying = cell->refcount.ref() ? cell : nullptr;
}

void InternedString::unref() {
    if (occupying && occupying->refcount.unref()) {
        GUARD(InternedString::lock);
        auto cell_id = occupying->hash % STRING_TABLE_LEN;
        if (InternedString::table[cell_id] == occupying) InternedString::table[cell_id] = occupying->next;
        if (occupying->prev) occupying->prev->next = occupying->next;
        if (occupying->next) occupying->next->prev = occupying->prev;
        delete occupying;
    }
}

InternedString &InternedString::operator=(const InternedString &p_other) {
    ref(const_cast<StringCell*>(p_other.occupying));
    return *this;
}

InternedString &InternedString::operator=(const char *p_from) {
    return *this = InternedString(p_from);
}

InternedString &InternedString::operator=(const wchar_t *p_from) {
    return *this = InternedString(p_from);
}

InternedString &InternedString::operator=(const VString &p_from) {
    return *this = InternedString(p_from);
}

int InternedString::cleanup() {
    GUARD(InternedString::lock);
    auto orphan_count = 0;
    for (auto& cell : InternedString::table){
        auto iterating = cell;
        while (iterating) {
            orphan_count++;
            print_line(VString("Orphan InternedString: ") + iterating->data);
            auto removal_candidate = iterating;
            iterating = iterating->next;
            delete removal_candidate;
        }
//        cell = nullptr;
    }
    if (orphan_count)
        print_line(VString("Total orphan InternedString found during cleanup: ") + uitos(orphan_count));
    InternedString::configured = false;
    return orphan_count;
}

void InternedString::operator+=(const InternedString &p_other) {
    if (occupying) ref(InternedString(occupying->data + p_other.c_str()).occupying);
    else ref(InternedString(p_other.c_str()).occupying);
}

void InternedString::operator+=(const VString &p_other) {
    if (occupying) ref(InternedString(occupying->data + p_other).occupying);
    else ref(InternedString(p_other).occupying);
}

InternedString::operator VString() const {
    return occupying ? occupying->data : L"";
}
