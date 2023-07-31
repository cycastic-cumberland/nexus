//
// Created by cycastic on 7/27/2023.
//

#include "string_literal.h"
#include "../hashfuncs.h"
#include "../../runtime/nexus_output.h"

BinaryMutex StringLiteral::lock{};
StringLiteral::StringCell* StringLiteral::table[StringLiteral::STRING_TABLE_LEN]{};
bool StringLiteral::configured = false;

void StringLiteral::configure() {
    // No need to lock
    if (configured) return;
    for (auto & i : table){
        i = nullptr;
    }
    configured = true;
}

StringLiteral::StringLiteral() {
    // StringLiteral will most likely be configured beforehand anyway, so no need to lock
    if (!StringLiteral::configured)
        throw StringLiteralException("StringLiteral has not been configured");
}

StringLiteral::StringLiteral(const StringLiteral &p_other) : StringLiteral() {
    ref(const_cast<StringLiteral::StringCell*>(p_other.occupying));
}

StringLiteral::StringLiteral(const char* p_from) : StringLiteral(VString(p_from).c_str()) {}
StringLiteral::StringLiteral(const VString &p_from) : StringLiteral(p_from.c_str()) {}
StringLiteral::StringLiteral(const wchar_t* p_from) : StringLiteral() {
    if (!p_from || p_from[0] == 0) {
        return; //empty, ignore
    }
    StringLiteral::lock.lock();
    auto hash = StandardHasher::hash(p_from);
    auto idx = hash & STRING_TABLE_MASK;
    auto current_cell = StringLiteral::table[idx];
    while (current_cell){
        if (current_cell->hash == hash && current_cell->data == p_from)
            break;
        current_cell = current_cell->next;
    }
    if (current_cell){
        ref(current_cell);
        StringLiteral::lock.unlock();
        return;
    }
    current_cell = new StringCell{
        .prev = nullptr,
        .next = StringLiteral::table[idx],
        .hash = hash,
        .refcount{},
        .data{p_from}
    };
    current_cell->refcount.init();
    if (StringLiteral::table[idx]) {
        StringLiteral::table[idx]->prev = current_cell;
        current_cell->next = StringLiteral::table[idx];
    }
    StringLiteral::table[idx] = current_cell;
    occupying = current_cell;
    StringLiteral::lock.unlock();
}
StringLiteral::~StringLiteral() {
    unref();
}

void StringLiteral::ref(StringLiteral::StringCell *cell) {
    unref();
    occupying = cell->refcount.ref() ? cell : nullptr;
}

void StringLiteral::unref() {
    if (occupying && occupying->refcount.unref()) {
        // TODO: do unref-ing
        GUARD(StringLiteral::lock);
        auto cell_id = occupying->hash % STRING_TABLE_LEN;
        if (StringLiteral::table[cell_id] == occupying) StringLiteral::table[cell_id] = occupying->next;
        if (occupying->prev) occupying->prev->next = occupying->next;
        if (occupying->next) occupying->next->prev = occupying->prev;
        delete occupying;
    }
}

StringLiteral &StringLiteral::operator=(const StringLiteral &p_other) {
    ref(const_cast<StringCell*>(p_other.occupying));
    return *this;
}

StringLiteral &StringLiteral::operator=(const char *p_from) {
    return *this = StringLiteral(p_from);
}

StringLiteral &StringLiteral::operator=(const wchar_t *p_from) {
    return *this = StringLiteral(p_from);
}

StringLiteral &StringLiteral::operator=(const VString &p_from) {
    return *this = StringLiteral(p_from);
}

int StringLiteral::cleanup() {
    GUARD(StringLiteral::lock);
    auto orphan_count = 0;
    for (auto& cell : StringLiteral::table){
        auto iterating = cell;
        while (iterating) {
            orphan_count++;
            print_line(VString("Orphan StringLiteral: ") + iterating->data);
            auto removal_candidate = iterating;
            iterating = iterating->next;
            delete removal_candidate;
        }
//        cell = nullptr;
    }
    if (orphan_count)
        print_line(VString("Total orphan StringLiteral found during cleanup: ") + uitos(orphan_count));
    StringLiteral::configured = false;
    return orphan_count;
}
