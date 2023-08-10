//
// Created by cycastic on 7/19/2023.
//

#ifndef NEXUS_OBJECT_H
#define NEXUS_OBJECT_H

#include "safe_refcount.h"
#include "../lock.h"
#include "vector.h"
#include "hashmap.h"
#include "reference.h"

class ObjectDB;

class BaseObject {
public:
    virtual void init_ref() const = 0;
    virtual bool ref() const = 0;
    virtual bool unref() const = 0;
    _NO_DISCARD_ virtual uint32_t get_reference_count() const = 0;

    virtual ~BaseObject() = default;
};

// Decentralized, Thread safe Object
class ThreadSafeObject : public BaseObject {
private:
    mutable SafeRefCount refcount{};
public:
    void init_ref() const override { refcount.init(); }
    bool ref() const override { return refcount.ref(); }
    bool unref() const override { return refcount.unref(); }
    uint32_t get_reference_count() const override { return refcount.get(); }

    ~ThreadSafeObject() override = default;
};

// Decentralized, Thread unsafe Object
class ThreadUnsafeObject : public BaseObject {
private:
    mutable uint32_t refcount{};
public:
    void init_ref() const override { refcount = 1; }
    bool ref() const override { return (refcount == 0 ? 0 : ++refcount) != 0; }
    bool unref() const override { return --refcount == 0; }
    uint32_t get_reference_count() const override { return refcount; }

    ~ThreadUnsafeObject() override = default;
};

// Centralized, thread safe Object
class ManagedObject : public ThreadSafeObject {
private:
    uint64_t object_id{};

    friend class ObjectDB;
public:
    uint64_t get_object_id() const { return object_id; }
    ManagedObject();
    ~ManagedObject() override;
};

class ObjectDB {
private:
    static HashMap<uint64_t, ManagedObject*> objects_registry;
    static SafeNumeric<uint64_t> refcount;
    static RWLock lock;

    friend class ManagedObject;
private:
    static void register_object(ManagedObject* obj);
    static void remove_object(ManagedObject* obj);
public:
    static Ref<ManagedObject> get_instance(const uint64_t& p_id);
    static size_t get_object_count();
    static Vector<uint64_t> get_all_objects_id();

#ifdef DEBUG_ENABLED
    static Vector<ManagedObject*> get_all_instances();
#endif
};

#endif //NEXUS_OBJECT_H
