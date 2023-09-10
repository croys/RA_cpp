#pragma once

#include <vector>
//#include <string>
#include <string_view>
#include <tuple>
#include <map>
#include <stdexcept>
#include <algorithm>
#include <utility>
#include <ostream>
#include <sstream>
#include <initializer_list>

#include "base.h"

#ifndef RA_CPP_LIBRARY_HPP
#define RA_CPP_LIBRARY_HPP

#include <RA_cpp/ra_cpp_library_export.hpp>

#endif


namespace rac
{

// Types of basic objects that can be stored in a relation

// FIXME: uint32_t, uint64_t, size_t, pointers

typedef enum {
    Void, Bool, Int, Float, Double, String, Date, Time,
    Object,
} ty_con_t;


// FIXME: extend to tycons with args
struct type_t
{
    ty_con_t ty_con;

};
constexpr bool operator==(const type_t& ta, const type_t& tb)
{
    return ta.ty_con == tb.ty_con;
}

constexpr auto operator<=>(const type_t& ta, const type_t& tb)
{
    return ta.ty_con <=> tb.ty_con;
}

constexpr std::string_view ty_to_string( const type_t& ty )
{
    using namespace std::string_view_literals;
    switch (ty.ty_con) {
        case Void:      return "Void"sv;
        case Bool:      return "Bool"sv;
        case Int:       return "Int"sv;
        case Float:     return "Float"sv;
        case Double:    return "Double"sv;
        case String:    return "String"sv;
        case Date:      return "Date"sv;
        case Time:      return "Time"sv;
        case Object:    return "Object"sv;
    }
    throw std::invalid_argument( "Unrecognised type" );
}

RA_CPP_LIBRARY_EXPORT
std::ostream& ty_to_stream( std::ostream& os, const type_t& ty );


// statically determined type convenience traits (mainly for witness)

template<typename T>
struct type_t_traits
{
};


template<> struct type_t_traits<void>
{
    static constexpr type_t ty() { return type_t { ty_con_t::Void }; }
};

template<> struct type_t_traits<bool>
{
    static constexpr type_t ty() { return type_t { ty_con_t::Bool }; }
};

template<> struct type_t_traits<int>
{
    static constexpr type_t ty() { return type_t { ty_con_t::Int }; }
};

template<> struct type_t_traits<float>
{
    static constexpr type_t ty() { return type_t { ty_con_t::Float }; }
};

template<> struct type_t_traits<double>
{
    static constexpr type_t ty() { return type_t { ty_con_t::Double }; }
};

template<> struct type_t_traits<std::string_view>
{
    static constexpr type_t ty() { return type_t { ty_con_t::String }; }
};


// FIXME: date, time, object, std:optional, etc..

// Convenience

constexpr auto tyVoid()     { return type_t_traits<void>(); }
constexpr auto tyBool()     { return type_t_traits<bool>(); }
constexpr auto tyInt()      { return type_t_traits<int>(); }
constexpr auto tyFloat()    { return type_t_traits<float>(); }
constexpr auto tyDouble()   { return type_t_traits<double>(); }
constexpr auto tyString()   { return type_t_traits<std::string_view>(); }



// columns are typed and have names and ordering
// used in relation_builder and table_view
typedef std::vector< std::pair< std::string, type_t > > col_tys_t;

RA_CPP_LIBRARY_EXPORT
std::ostream& col_tys_to_stream( std::ostream& os, const col_tys_t& col_tys );

RA_CPP_LIBRARY_EXPORT std::string col_tys_to_string( const col_tys_t& col_tys );

typedef std::string_view cstring_t;


// Relation type
//
// A relation type is built up of typed and named columns, but has no
// ordering
RA_CPP_LIBRARY_EXPORT struct rel_ty_t
{
private:
    void construct();

public:

    explicit inline rel_ty_t( const col_tys_t& col_tys )
    : m_tys( col_tys )
    {
        construct();
    }

    explicit inline rel_ty_t( col_tys_t&& col_tys )
    : m_tys( col_tys )
    {
        construct();
    }

    explicit inline rel_ty_t(
        std::initializer_list< col_tys_t::value_type > init
    ) : m_tys( init )
    {
        construct();
    }


    // union
    [[nodiscard]] static
    rel_ty_t union_( const rel_ty_t& a, const rel_ty_t& b );

#ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable:4100)
#else
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wunused-parameter"
#endif

    // project
    template <typename Iterator>
    [[nodiscard]] static
    rel_ty_t project( const rel_ty_t& a, Iterator begin, Iterator end )
    {
        throw not_implemented();
    }

    // minus

    [[nodiscard]] static inline
    rel_ty_t minus( const rel_ty_t& a, const rel_ty_t& b )
    {
        throw not_implemented();
    }

#ifdef _MSC_VER
    #pragma warning(pop)
#else
    #pragma GCC diagnostic pop
#endif

    // intersect
    [[nodiscard]] static
    rel_ty_t intersect( const rel_ty_t& a, const rel_ty_t& b );

    // all_but
    // FIXME: "exclude"/"excluding"?

    col_tys_t m_tys;
};


inline bool operator==(const rel_ty_t& ta, const rel_ty_t& tb)
{
    return ta.m_tys == tb.m_tys;
}

inline auto operator<=>(const rel_ty_t& ta, const rel_ty_t& tb)
{
    return ta.m_tys <=> tb.m_tys;
}


}