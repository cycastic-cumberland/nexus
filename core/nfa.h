//
// Created by cycastic on 07/08/2023.
//

#ifndef NEXUS_NFA_H
#define NEXUS_NFA_H

#include <functional>
#include "types/priority_queue.h"
#include "types/linked_list.h"
#include "types/hashmap.h"
#include "mathlib.h"

template<class TType>
class NonDeterministicFiniteAutomaton {
public:
    typedef uint32_t ID;
    struct State;
    struct TransitionCondition;
    // Return null if FSM should choose the next state on its own.
    // Return an actual state to override that.
    typedef std::function<const State*(TType&, const State*)> node_callback;
    typedef std::function<bool(TType&, const State*, const TransitionCondition*)> transition_callback;
    struct TransitionCondition {
        transition_callback condition;
        const State* to_node;
    };
    struct State {
        ID id;
        node_callback callback;
        LinkedList<const State*> prev_entries;
        PriorityQueue<TransitionCondition> next_entries;
    };
private:
    HashMap<ID, State*> states_map;
    State* entry;
    State* finish;
    ID id_allocator;
    static const State* finish_callback(TType& ignored_0, const State* ignored_1){
        return nullptr;
    }
public:
    static bool condition_always_true(TType& ignored_0, const State* ignored_1, const TransitionCondition* ignored_2){
        return true;
    }
    static bool condition_always_false(TType& ignored_0, const State* ignored_1, const TransitionCondition* ignored_2){
        return false;
    }
    NonDeterministicFiniteAutomaton() : states_map{}, entry{}, id_allocator{0} {
        finish = new State{
            .id = 0,
            .callback = finish_callback,
            .prev_entries{},
            .next_entries{},
        };
    }
    ~NonDeterministicFiniteAutomaton(){
        auto it = states_map.iterator();
        while (it.move_next()){
            delete it.get_pair().value;
            it.erase();
        }
        delete finish;
    }
    _NO_DISCARD_ _FORCE_INLINE_ const State* operator[](const ID& p_id) const {
        return states_map[p_id];
    }
    const State* get_end() const {
        return finish;
    }
    const State* add_state(node_callback callback){
        auto new_id = ++id_allocator;
        auto re = new State{
            .id = new_id,
            .callback = callback,
            .prev_entries = LinkedList<const State*>(),
            .next_entries = PriorityQueue<TransitionCondition>(),
        };
        states_map[new_id] = re;
        if (!entry) re = entry;
        return re;
    }
    void connect_states(const State* p_from, const State* p_to, transition_callback p_condition, const uint8_t& p_priority){
        auto from = const_cast<State*>(p_from);
        auto to   = const_cast<State*>(p_to);
        // It does not matter whether the link already exist or not
        to->prev_entries.add_last(p_from);
        from->next_entries.push(TransitionCondition{
            .condition = p_condition,
            .to_node = p_to
            }, p_priority);
    }
    void run(TType& p_data, const uint32_t& max_iteration_count = Math::max_u32) const {
        const State* curr = entry;
        uint32_t iteration{};
        while (curr){
            curr = curr->callback(p_data, curr);
            if (++iteration >= max_iteration_count) break;
            if (curr) continue;
            const auto& next_entries = curr->next_entries;
            for (size_t i = 0, s = next_entries.size(); i < s; i++){
                const auto& curr_condition = next_entries[i];
                if (curr_condition.condition(p_data, curr, &curr_condition)){
                    curr = curr_condition.to_node;
                    continue;
                }
            }
            curr = nullptr;
        }
    }
    _NO_DISCARD_ Vector<ID> run_with_tracking(TType& p_data, const uint32_t& max_iteration_count = Math::max_u32) const {
        Vector<ID> track{};
        const State* curr = entry;
        uint32_t iteration{};
        while (curr){
            track.push_back(curr->id);
            curr = curr->callback(p_data, curr);
            if (++iteration >= max_iteration_count) break;
            if (curr) continue;
            const auto& next_entries = curr->next_entries;
            for (size_t i = 0, s = next_entries.size(); i < s; i++){
                const auto& curr_condition = next_entries[i];
                if (curr_condition.condition(p_data, curr, &curr_condition)){
                    curr = curr_condition.to_node;
                    continue;
                }
            }
            curr = nullptr;
        }
        return track;
    }
};

template<class T>
using NFA = NonDeterministicFiniteAutomaton<T>;

#endif //NEXUS_NFA_H
