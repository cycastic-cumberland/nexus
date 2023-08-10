//
// Created by cycastic on 8/3/2023.
//

#ifndef NEXUS_NEXUS_STACK_H
#define NEXUS_NEXUS_STACK_H

#include "../core/exception.h"
#include "../language/bytecode.h"
#include "../core/types/stack.h"
#include "../core/types/hashmap.h"
#include "../core/types/linked_list.h"
#include "../core/types/box.h"

class InternedString;
struct StackItemMetadata;
struct StackStructItemMetadata;

typedef void (*constructor_cb)(const StackItemMetadata*, void*);
typedef void (*copy_constructor_cb)(const StackItemMetadata*, void*, const void*);
typedef void (*destructor_cb)(const StackItemMetadata*, void*);
typedef void (*op_assign_cb)(const StackItemMetadata*, void*, const void*);
typedef void (*op_equal_cb)(const StackItemMetadata*, const void*, const void*, bool*);
typedef void (*op_greater_cb)(const StackItemMetadata*, const void*, const void*, bool*);
typedef void (*op_greater_or_equal_cb)(const StackItemMetadata*, const void*, const void*, bool*);
typedef void (*op_lesser_cb)(const StackItemMetadata*, const void*, const void*, bool*);
typedef void (*op_lesser_or_equal_cb)(const StackItemMetadata*, const void*, const void*, bool*);

template<class T>
static _FORCE_INLINE_ void generic_constructor(const StackItemMetadata* ignored, void* p_data){
    new (p_data) T();
}

template<class T>
static _FORCE_INLINE_ void numeric_constructor(const StackItemMetadata* ignored, void* p_data) {
    *(T*)p_data = 0;
}

template<class T>
static _FORCE_INLINE_ void generic_copy_constructor(const StackItemMetadata* ignored, void* p_data, const void* p_copy_target) {
    new (p_data) T(*(T*)p_copy_target);
}

template<class T>
static _FORCE_INLINE_ void generic_copy_assignment(const StackItemMetadata* ignored, void* p_data, const void* p_copy_target) {
    *(T*)p_data = *(T*)p_copy_target;
}

template<class T>
static _FORCE_INLINE_ void generic_destructor(const StackItemMetadata* ignored, void* p_data){
    auto casted = (T*)p_data;
    casted->~T();
}

struct NexusBaseVtable {
    typedef uint64_t ID;

    const NexusBaseVtable* parent_vtable{};

    const constructor_cb constructor;
    const copy_constructor_cb copy_constructor;
    const destructor_cb destructor;

    const op_assign_cb op_assign;
    const op_equal_cb op_equal{};
    const op_greater_cb op_greater{};
    const op_greater_or_equal_cb op_greater_or_equal{};
    const op_lesser_cb op_lesser{};
    const op_lesser_or_equal_cb op_lesser_or_equal{};

    virtual ~NexusBaseVtable() = default;
    NexusBaseVtable(constructor_cb p_con,
                    copy_constructor_cb p_copy_con,
                    op_assign_cb p_op_assign,
                    destructor_cb p_destructor,
                    const NexusBaseVtable* p_parent = nullptr)
            : constructor(p_con), copy_constructor(p_copy_con),
              op_assign(p_op_assign), destructor(p_destructor), parent_vtable(p_parent) {}

};

struct StackItemMetadata {
    typedef uint64_t ID;

    // this value is opaque, does not affect the add process
    ID type_id;

    NexusStandardType type;
    size_t data_size;
    const NexusBaseVtable* vtable;

    virtual ~StackItemMetadata() = default;
    StackItemMetadata(const ID& p_id,
                      const NexusStandardType& p_type,
                      const size_t& p_total_size,
                      const NexusBaseVtable* p_vtable)
                      : type_id(p_id),
                        type(p_type),
                        data_size(p_total_size),
                        vtable(p_vtable) {}
};

struct StackStructItemMetadata : public StackItemMetadata {
private:
    void build_cache();
public:
    LinkedList<const StackItemMetadata*> struct_items{};
    ~StackStructItemMetadata() override = default;

    static void default_struct_constructor(const StackItemMetadata* p_metadata, void* p_mem_region);
    static void default_struct_copy_constructor(const StackItemMetadata* p_metadata, void* p_mem_region, const void* p_copy_target_region);
    static void default_assignment_operator(const StackItemMetadata* p_metadata, void* p_mem_region, const void* p_copy_target_region);
    static void default_struct_destructor(const StackItemMetadata* p_metadata, void* p_mem_region);

    StackStructItemMetadata(const ID& p_id,
                            const NexusStandardType& p_type,
                            const size_t& p_total_size,
                            const NexusBaseVtable* p_vtable,
                            const LinkedList<const StackItemMetadata*>& p_struct_items)
                            : StackItemMetadata(p_id, p_type, p_total_size, p_vtable),
                            struct_items(p_struct_items) { build_cache(); }
    StackStructItemMetadata(const ID& p_id,
                            const NexusStandardType& p_type,
                            const size_t& p_total_size,
                            const NexusBaseVtable* p_vtable,
                            LinkedList<const StackItemMetadata*>&& p_struct_items)
                            : StackItemMetadata(p_id, p_type, p_total_size, p_vtable),
                            struct_items(p_struct_items) { build_cache(); }
};

class NexusTypeInfoServer {
private:
    mutable RWLock* lock{};
    LinkedList<StackItemMetadata*> metadata_record{};
    LinkedList<NexusBaseVtable*> vtable_record{};
    HashMap<StackItemMetadata::ID, const StackItemMetadata*> metadata_map{};
    HashMap<NexusBaseVtable::ID, const NexusBaseVtable*> vtable_map{};
    SafeNumeric<StackItemMetadata::ID> metadata_id_allocator{};
    SafeNumeric<NexusBaseVtable::ID> vtable_id_allocator{};

    void add_builtin_types();
    StackItemMetadata::ID add_metadata(StackItemMetadata* p_metadata, const StackItemMetadata::ID& p_id);
    NexusBaseVtable::ID add_vtable(NexusBaseVtable* p_vtable, const NexusBaseVtable::ID& p_id);
public:
    explicit NexusTypeInfoServer(bool p_is_multi_threaded = true);
    ~NexusTypeInfoServer();

    template<class T, class ...Args>
    _FORCE_INLINE_ StackItemMetadata::ID add_derived_type(Args&& ...args){
        W_GUARD(lock);
        auto new_metadata = new T(args...);
        return add_metadata(new_metadata, metadata_id_allocator.increment());
    }
    template<class ...Args>
    _FORCE_INLINE_ StackItemMetadata::ID add_struct_type(Args&& ...args){
        return add_derived_type<StackStructItemMetadata>(0, NexusStandardType::STACK_STRUCT, 0, args...);
    }
    template<class T, class ...Args>
    _FORCE_INLINE_ NexusBaseVtable::ID add_custom_vtable(Args&& ...args){
        W_GUARD(lock);
        auto new_vtable = new T(args...);
        return add_vtable(new_vtable, vtable_id_allocator.increment());
    }
    template<class ...Args>
    _FORCE_INLINE_ NexusBaseVtable::ID add_struct_based_vtable(Args&& ...args){
        W_GUARD(lock);
        auto new_vtable = new NexusBaseVtable(StackStructItemMetadata::default_struct_constructor,
                                              StackStructItemMetadata::default_struct_copy_constructor,
                                              StackStructItemMetadata::default_assignment_operator,
                                              StackStructItemMetadata::default_struct_destructor,
                                              args...);
        return add_vtable(new_vtable, vtable_id_allocator.increment());
    }
    _FORCE_INLINE_ const StackItemMetadata* get_metadata_by_id(const StackItemMetadata::ID& p_id) const {
        const StackItemMetadata* re = nullptr;
        R_GUARD(lock);
        metadata_map.try_get(p_id, re);
        return re;
    }
    _FORCE_INLINE_ const StackItemMetadata* get_primitive_metadata(NexusStandardType p_data_type) const {
        return get_metadata_by_id((StackItemMetadata::ID)p_data_type);
    }
    _FORCE_INLINE_ const NexusBaseVtable* get_vtable_by_id(const NexusBaseVtable::ID& p_id) const {
        const NexusBaseVtable* re = nullptr;
        R_GUARD(lock);
        vtable_map.try_get(p_id, re);
        return re;
    }
//    _FORCE_INLINE_ NexusBaseVtable* get_vtable_by_id(const NexusBaseVtable::ID& p_id) {
//        const NexusBaseVtable* re = nullptr;
//        R_GUARD(lock);
//        vtable_map.try_get(p_id, re);
//        return const_cast<NexusBaseVtable*>(re);
//    }
    _FORCE_INLINE_ const NexusBaseVtable* get_primitive_vtable(NexusStandardType p_data_type) const {
        return get_vtable_by_id((NexusBaseVtable::ID)p_data_type);
    }
};

class NexusStackException : public Exception {
public:
    explicit NexusStackException(const char *p_msg = nullptr) : Exception(p_msg) {}
};


class NexusStack {
public:
    struct ObjectInfo {
        const StackItemMetadata* type;
        void* data;
        size_t index;

        _NO_DISCARD_ _FORCE_INLINE_ bool is_valid() const {
            return type && data;
        }
        _FORCE_INLINE_ void set_primitive(const void* p_from_data) const {
            type->vtable->op_assign(type, data, p_from_data);
        }
    };
    class Frame {
        NexusStack *parent;
        size_t object_offset;
        void* memory_offset;
        size_t current_object_count;

        friend class NexusStack;

        template<class T>
        void push_primitive(const NexusStandardType& p_type, const T& p_data);
        void push_object(const StackItemMetadata* p_metadata, const void* p_data);

        template<class T>
        void set_primitive(const int64_t &p_idx, const NexusStandardType &p_type, const T &p_data) const;

        static void set_object(const ObjectInfo& p_info, const StackItemMetadata* p_metadata, const void* p_data);
        void register_object(const StackItemMetadata* p_metadata);
    public:
        explicit Frame(NexusStack* p_stack);
        Frame(const Frame& p_other) = delete;
        Frame(Frame&& p_other) noexcept;
        ~Frame();

        void push(const ObjectInfo& p_info);
        void push_empty(const StackItemMetadata* p_metadata);
        void push(const StackItemMetadata* p_metadata, const void* p_copy_from);
        void push(const uint32_t& p_u32);
        void push(const int32_t& p_i32);
        void push(const uint64_t& p_u64);
        void push(const int64_t& p_i64);
        void push(const float& p_f32);
        void push(const double & p_f64);
        void push(const wchar_t* p_str);
        void push(const InternedString& p_str);
        void push(const VString& p_string);
        void push(const Ref<ManagedObject>& p_ref_counted);

        void set(const int64_t& p_idx, const ObjectInfo& p_info) const;
        void set(const int64_t& p_idx, const StackItemMetadata* p_metadata, const void* p_copy_from) const;
        void set(const int64_t& p_idx, const uint32_t& p_u32) const;
        void set(const int64_t& p_idx, const int32_t& p_i32) const;
        void set(const int64_t& p_idx, const uint64_t& p_u64) const;
        void set(const int64_t& p_idx, const int64_t& p_i64) const;
        void set(const int64_t& p_idx, const float& p_f32) const;
        void set(const int64_t& p_idx, const double & p_f64) const;
        void set(const int64_t& p_idx, const wchar_t* p_str) const;
        void set(const int64_t& p_idx, const InternedString& p_str) const;
        void set(const int64_t& p_idx, const VString& p_string) const;
        void set(const int64_t& p_idx, const Ref<ManagedObject>& p_ref_counted) const;

        void pop();
        void copy_to_top(const int64_t& p_idx);

        _NO_DISCARD_ _FORCE_INLINE_ size_t object_count() const { return current_object_count; }
        _NO_DISCARD_ _FORCE_INLINE_ bool empty() const { return object_count() == 0; }

        _FORCE_INLINE_ void clear() {
            while (!empty()) pop();
        }
        _NO_DISCARD_ _FORCE_INLINE_ ObjectInfo get_at(const int64_t& p_idx) const {
            int64_t normalized_position = (p_idx >= 0 ? p_idx : int64_t(object_count()) + p_idx) + int64_t(object_offset);
            return (normalized_position < 0 || normalized_position >= parent->object_count()) ?
                    ObjectInfo{
                        .type = nullptr,
                        .data = nullptr,
                        .index = 0
                    } : parent->object_info[normalized_position];
        }
        _NO_DISCARD_ _FORCE_INLINE_ ObjectInfo top() const {
            return get_at(-1);
        }
    };

    friend class Frame;
private:
    const size_t max_stack_size;
    const NexusTypeInfoServer* type_info_server;
    void* stack_begin;
    size_t allocated;
    size_t current_object_count;
    VectorStack<Box<Frame, ThreadUnsafeObject>> stack_frames;
    Vector<ObjectInfo> object_info;
public:
    NexusStack(const NexusTypeInfoServer* p_type_info_server, const size_t& p_stack_size, const size_t& p_initial_frame_capacity);
    NexusStack(const NexusStack& p_other) = delete;
    ~NexusStack();

    // Returning a reference? And coming from a Vector no less...
    // This is potentially unsafe, but as long as you are careful, there should be no problem
    // Plus, allocating the stack frame itself on the Vector's heap is more efficient.
    // Just make sure not to push any frame onto the stack while holding a reference
    Box<NexusStack::Frame, ThreadUnsafeObject>& push_stack_frame();
    bool pop_stack_frame();
    _NO_DISCARD_ Box<Frame, ThreadUnsafeObject>& get_last_frame();
    _NO_DISCARD_ const Box<Frame, ThreadUnsafeObject>& get_last_frame() const;
    // DO NOT const_cast this Frame&, pushing onto a frame that is not at the top of the stack
    // lead to undefined behavior. There will be no check so do it at your discretion
    _NO_DISCARD_ const Box<Frame, ThreadUnsafeObject>& get_frame_at(const int64_t& p_idx) const;

    _NO_DISCARD_ _FORCE_INLINE_ size_t frame_count() const { return stack_frames.size(); }
    _NO_DISCARD_ _FORCE_INLINE_ size_t object_count() const { return object_info.size(); }
    _NO_DISCARD_ _FORCE_INLINE_ bool empty() const { return stack_frames.empty(); }
};

template<class T>
void NexusStack::Frame::push_primitive(const NexusStandardType &p_type, const T &p_data) {
    auto metadata = parent->type_info_server->get_primitive_metadata(p_type);
    if (!metadata) throw NexusStackException("Can not find metadata for primitive type, "
                                             "NexusTypeInfoServer might not have been initialized");
    if (parent->allocated + metadata->data_size > parent->max_stack_size) throw NexusStackException("Stack overflow");
    push_object(metadata, &p_data);
}

template<class T>
void NexusStack::Frame::set_primitive(const int64_t &p_idx, const NexusStandardType &p_type, const T &p_data) const {
    auto metadata = parent->type_info_server->get_primitive_metadata(p_type);
    if (!metadata) throw NexusStackException("Can not find metadata for primitive type, "
                                             "NexusTypeInfoServer might not have been initialized");
    auto object = get_at(p_idx);
    if (!object.is_valid()) throw NexusStackException("Invalid stack frame index");
    set_object(object, metadata, &p_data);
}

#endif //NEXUS_NEXUS_STACK_H
