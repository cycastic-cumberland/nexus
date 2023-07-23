//
// Created by cycastic on 7/22/2023.
//

#ifndef NEXUS_AMBIGOUS_VALUE_H
#define NEXUS_AMBIGOUS_VALUE_H

#include "../language/bytecode.h"

struct StackStructMetadata;

struct AmbiguousValue {
private:
    static constexpr void* null_val = nullptr;
    void* data{};
    NexusSerializedBytecode::DataType data_type = NexusSerializedBytecode::NONE;

    void _FORCE_INLINE_ free_data(){
        switch (data_type){
            case NexusSerializedBytecode::NONE:
                return;
            case NexusSerializedBytecode::UNSIGNED_32_BIT_INTEGER:
                delete (uint32_t*)data;
                break;
            case NexusSerializedBytecode::SIGNED_32_BIT_INTEGER:
                delete (int32_t*)data;
                break;
            case NexusSerializedBytecode::UNSIGNED_64_BIT_INTEGER:
                delete (uint64_t*)data;
                break;
            case NexusSerializedBytecode::SIGNED_64_BIT_INTEGER:
                delete (int64_t*)data;
                break;
            case NexusSerializedBytecode::SINGLE_PRECISION_FLOATING_POINT:
                delete (float*)data;
                break;
            case NexusSerializedBytecode::DOUBLE_PRECISION_FLOATING_POINT:
                delete (double*)data;
                break;
            case NexusSerializedBytecode::STRING:
                delete (VString*)data;
                break;
            case NexusSerializedBytecode::REFERENCE_COUNTED_OBJECT:
                delete (Ref<Object>*)data;
                break;
            case NexusSerializedBytecode::STACK_STRUCT:
                delete (StackStructMetadata*)data;
                break;
            case NexusSerializedBytecode::STRING_LITERAL:
            case NexusSerializedBytecode::METHOD:
                throw Exception("This should not happen...");
        }
        data = nullptr;
        data_type = NexusSerializedBytecode::NONE;
    }
    void set_from(const void* p_data, NexusSerializedBytecode::DataType p_data_type);

    template<class T>
    void _FORCE_INLINE_ init_from(const T& p_val, NexusSerializedBytecode::DataType p_type){
        auto new_data = new T(p_val);
        data = new_data;
        data_type = p_type;
    }
    template<class T>
    T*& _FORCE_INLINE_ get_internal(const NexusSerializedBytecode::DataType& p_type){
        if (p_type != data_type) return *(T**)&null_val;
        return *(T**)&data;
    }
public:
    AmbiguousValue() = default;
    // rvalue
    AmbiguousValue(AmbiguousValue&& p_other) noexcept {
        data = p_other.data;
        data_type = p_other.data_type;
    }
    AmbiguousValue(const AmbiguousValue& p_other) : AmbiguousValue() {
        set_from(p_other.data, p_other.data_type);
    }
    AmbiguousValue(void* p_data, NexusSerializedBytecode::DataType p_type) {
        set_from(p_data, p_type);
    }
    AmbiguousValue(const uint32_t& p_val) { init_from(p_val, NexusSerializedBytecode::UNSIGNED_32_BIT_INTEGER); }
    AmbiguousValue(const int32_t& p_val) { init_from(p_val, NexusSerializedBytecode::SIGNED_32_BIT_INTEGER); }
    AmbiguousValue(const uint64_t& p_val) { init_from(p_val, NexusSerializedBytecode::UNSIGNED_64_BIT_INTEGER); }
    AmbiguousValue(const int64_t& p_val) { init_from(p_val, NexusSerializedBytecode::SIGNED_64_BIT_INTEGER); }
    AmbiguousValue(const float& p_val) { init_from(p_val, NexusSerializedBytecode::SINGLE_PRECISION_FLOATING_POINT); }
    AmbiguousValue(const double& p_val) { init_from(p_val, NexusSerializedBytecode::DOUBLE_PRECISION_FLOATING_POINT); }
    AmbiguousValue(const VString& p_val) { init_from(p_val, NexusSerializedBytecode::STRING); }
    AmbiguousValue(const Ref<Object>& p_val) { init_from(p_val, NexusSerializedBytecode::REFERENCE_COUNTED_OBJECT); }

    _FORCE_INLINE_ AmbiguousValue& operator=(const AmbiguousValue& p_other){
        set_from(p_other.data, p_other.data_type);
        return *this;
    }
    _NO_DISCARD_ _FORCE_INLINE_ NexusSerializedBytecode::DataType get_type() const { return data_type; }
    _NO_DISCARD_ _FORCE_INLINE_ const void* get_raw_pointer() const { return data; }
    _FORCE_INLINE_ uint32_t*& get_u32() { return get_internal<uint32_t>(NexusSerializedBytecode::UNSIGNED_32_BIT_INTEGER); }
    _FORCE_INLINE_ int32_t*& get_i32() { return get_internal<int32_t>(NexusSerializedBytecode::SIGNED_32_BIT_INTEGER); }
    _FORCE_INLINE_ uint64_t*& get_u64() { return get_internal<uint64_t>(NexusSerializedBytecode::UNSIGNED_64_BIT_INTEGER); }
    _FORCE_INLINE_ int64_t*& get_i64() { return get_internal<int64_t>(NexusSerializedBytecode::SIGNED_64_BIT_INTEGER); }
    _FORCE_INLINE_ float*& get_fp32() { return get_internal<float>(NexusSerializedBytecode::SINGLE_PRECISION_FLOATING_POINT); }
    _FORCE_INLINE_ double*& get_fp64() { return get_internal<double>(NexusSerializedBytecode::DOUBLE_PRECISION_FLOATING_POINT); }
    _FORCE_INLINE_ VString*& get_string() { return get_internal<VString>(NexusSerializedBytecode::STRING); }
    _FORCE_INLINE_ Ref<Object>*& get_object() { return get_internal<Ref<Object>>(NexusSerializedBytecode::REFERENCE_COUNTED_OBJECT); }
};

#endif //NEXUS_AMBIGOUS_VALUE_H
