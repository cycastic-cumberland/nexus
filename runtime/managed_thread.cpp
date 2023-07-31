//
// Created by cycastic on 7/25/2023.
//
#include "managed_thread.h"
#include "nexus_output.h"

ManagedThread::ID ManagedThread::main_thread_id = thread_id_hash(std::this_thread::get_id());
static thread_local ManagedThread::ID caller_id = 0;
static thread_local bool caller_id_cached = false;

uint64_t ManagedThread::thread_id_hash(const std::thread::id &p_native_id) {
    static std::hash<std::thread::id> hasher{};
    return hasher(p_native_id);
}


bool ManagedThread::is_started() const {
    R_GUARD(lock);
    return id != thread_id_hash(std::thread::id());
}

bool ManagedThread::is_alive() const {
    R_GUARD(lock);
    return !thread.joinable();
}

bool ManagedThread::is_finished() const {
    R_GUARD(lock);
    return !is_alive();
}

void ManagedThread::join() {
    W_GUARD(lock);
    if (id != thread_id_hash(std::thread::id())) {
        if (id == ManagedThread::get_caller_id()){
            print_error(VString("ManagedThread ") + uitos(id) + " can't join itself.");
            return;
        }
        thread.join();
        std::thread empty_thread;
        thread.swap(empty_thread);
        id = thread_id_hash(std::thread::id());
    }
}

ManagedThread::ID ManagedThread::get_caller_id() {
    if (likely(caller_id_cached)) {
        return caller_id;
    } else {
        caller_id = thread_id_hash(std::this_thread::get_id());
        caller_id_cached = true;
        return caller_id;
    }
}


ManagedThread::~ManagedThread(){
    if (id != thread_id_hash(std::thread::id())){
        print_warning(VString("ManagedThread object with id ") + uitos(id)
            + " has been terminated without being joined. This thread will now be detached");
        thread.detach();
    }
}
