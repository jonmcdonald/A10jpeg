////////////////////////////////////////////////////////////////////////////////
// Catapult Synthesis
//
// Copyright (c) 2003-2011 Calypto Design Systems, Inc.
//       All Rights Reserved
// 
////////////////////////////////////////////////////////////////////////////////

#ifndef AC_ARRAY_H
#define AC_ARRAY_H

#ifdef AC_ARRAY_ASSERT
#include <ac_assert.h>
#define AC_A_ASSERT(cond) ac::ac_assert(__FILE__, __LINE__, #cond, cond)
#else // !AC_ARRAY_ASSERT
#define AC_A_ASSERT(cond) 
#endif // AC_ARRAY_ASSERT

////////////////////////////////////////////////////////////////////////////////
//  Type:         ac_array
//  Description:  Container class for multi-dimensional arrays, simplifies copy
////////////////////////////////////////////////////////////////////////////////

template <typename T, unsigned D>
class ac_array_b
{
public:
    T d[D]; // data

public:

    typedef T ElemType;

    ac_array_b() {}
    ~ac_array_b() {}

    T &operator [] (unsigned i) {
        AC_A_ASSERT(i < D);
        return d[i];
    }
    const T &operator [] (unsigned i) const {
        AC_A_ASSERT(i < D);
        return d[i];
    }

    unsigned size() const { return D; }

    template <typename Te>
    void set(const Te &ival) {
        for ( unsigned i = 0; i < D; ++i )
            set(i, ival);
    }

    void clearAll(bool dc=false) {
        for ( unsigned i = 0; i < D; ++i )
            clear(i, dc);
    }

    template <typename Te>
    void set(unsigned idx, const Te &ival) {
        d[idx].set(ival);
    }
    void clear(unsigned idx, bool dc=false) {
        d[idx].clear(dc);
    }

};


template <typename T, unsigned D1, unsigned D2=0, unsigned D3=0>
class ac_array : public ac_array_b< ac_array<T,D2,D3>, D1>
{
public:
    ac_array() {}
    ac_array(const T &ival) { set(ival); }
    ~ac_array() {}
};

template <typename T, unsigned D1>
class ac_array<T,D1,0,0> : public ac_array_b<T,D1>
{
    typedef ac_array_b<T,D1> Base;
public:
    ac_array() {}
    ac_array(const T &ival) { set(ival); }
    ~ac_array() {}

    template <typename Te>
    void set(unsigned idx, const Te &ival) {
        Base::d[idx] = ival;
    }
    void clear(unsigned idx, bool dc=false) {
        if ( !dc )
            Base::d[idx] = 0;
        else {
            T v;
            Base::d[idx] = v;
        }
    }
};

#endif // AC_ARRAY_H
