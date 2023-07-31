//
// Created by cycastic on 7/19/2023.
//

#include "object.h"

HashMap<uint64_t, Object*> ObjectDB::objects_registry = HashMap<uint64_t, Object*>();
SafeNumeric<uint64_t> ObjectDB::refcount = SafeNumeric<uint64_t>(0);
RWLock ObjectDB::lock = RWLock();

void ObjectDB::register_object(Object *obj) {
    W_GUARD(lock);
    obj->object_id = refcount.increment();
    objects_registry[obj->object_id] = obj;
}

void ObjectDB::remove_object(Object *obj) {
    W_GUARD(lock);
    objects_registry.erase(obj->object_id);
}
Ref<Object> ObjectDB::get_instance(const uint64_t &p_id) {
    R_GUARD(lock);
    if (!objects_registry.has(p_id)) return Ref<Object>::null();
    auto ptr = objects_registry[p_id];
    return Ref<Object>::from_initialized_object(ptr);
}

Vector<uint64_t> ObjectDB::get_all_objects_id() {
    R_GUARD(lock);
    Vector<uint64_t> re(objects_registry.size());
    auto it = objects_registry.const_iterator();
    while (it.move_next()){
        re.push_back(it.get_pair().key);
    }
    return re;
}

size_t ObjectDB::get_object_count() {
    R_GUARD(lock);
    return objects_registry.size();
}

#ifdef DEBUG_ENABLED
Vector<Object *> ObjectDB::get_all_instances() {
    R_GUARD(lock);
    Vector<Object *> re{};
    auto it = objects_registry.const_iterator();
    while (it.move_next()){
        re.push_back(it.get_pair().value);
    }
    return re;
}
#endif

Object::Object(){
    ObjectDB::register_object(this);
}

Object::~Object() {
    ObjectDB::remove_object(this);
}
