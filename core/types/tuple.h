//
// Created by cycastic on 7/19/2023.
//

#ifndef NEXUS_TUPLE_H
#define NEXUS_TUPLE_H

template <typename T1, typename T2>
class TupleT2 {
private:
    T1 t1;
    T2 t2;
public:
    TupleT2(const T1& p_t1, const T2& p_t2){
        t1 = p_t1;
        t2 = p_t2;
    }
    TupleT2(const TupleT2& p_other) : t1(p_other.t1), t2(p_other.t2) {}
    virtual void unpack(T1& p_t1, T2& p_t2) const {
        p_t1 = t1;
        p_t2 = t2;
    }
};

template <typename T1, typename T2, typename T3>
class TupleT3 : public TupleT2<T1, T2> {
private:
    T3 t3;
public:
    TupleT3(const T1& p_t1, const T2& p_t2, const T3& p_t3) : TupleT2<T1, T2>(p_t1, p_t2){
        t3 = p_t3;
    }
    TupleT3(const TupleT3& p_other) : TupleT2<T1, T2>(p_other), t3(p_other.t3) {}
    virtual void unpack(T1& p_t1, T2& p_t2, T3& p_t3) const {
        TupleT2<T1, T2>::unpack(p_t1, p_t2);
        p_t3 = t3;
    }
    void unpack(T1& p_t1, T2& p_t2) const override = delete;
};

template <typename T1, typename T2, typename T3, typename T4>
class TupleT4 : public TupleT3<T1, T2, T3> {
private:
    T4 t4;
public:
    TupleT4(const T1& p_t1, const T2& p_t2, const T3& p_t3, const T4& p_t4) : TupleT3<T1, T2, T3>(p_t1, p_t2, p_t3){
        t4 = p_t4;
    }
    TupleT4(const TupleT4& p_other) : TupleT3<T1, T2, T3>(p_other), t4(p_other.t4) {}
    virtual void unpack(T1& p_t1, T2& p_t2, T3& p_t3, T4& p_t4) const {
        TupleT3<T1, T2, T3>::unpack(p_t1, p_t2, p_t3);
        p_t4 = t4;
    }
    void unpack(T1& p_t1, T2& p_t2, T3& p_t3) const override = delete;
};

#endif //NEXUS_TUPLE_H
