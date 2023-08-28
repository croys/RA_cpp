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

std::ostream& ty_to_stream( std::ostream& os, const type_t& ty )
{
    switch (ty.ty_con) {
        case Void:
        case Bool:
        case Int:
        case Float:
        case Double:
        case String:
        case Date:
        case Time:
        case Object:
            os << ty_to_string( ty );
            break;
        default:
            throw std::invalid_argument( "Unrecognised type" );
    }
    return os;
}


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

std::ostream& col_tys_to_stream( std::ostream& os, const col_tys_t& col_tys )
{
    os << "{ ";
    for (auto it = col_tys.cbegin(); it != col_tys.cend(); ++it) {
        if (it != col_tys.cbegin())
            os << ", ";
        os  << std::get<0>(*it) << " : " << ty_to_string( std::get<1>(*it) );
    }
    os << " }";

    return os;
}

std::string col_tys_to_string( const col_tys_t& col_tys )
{
    std::ostringstream ss;
    col_tys_to_stream( ss, col_tys );
    return ss.str();
}

typedef std::string_view cstring_t;


// Relation type
//
// A relation type is built up of typed and named columns, but has no
// ordering
struct rel_ty_t
{
private:
    void construct()
    {
        // Normal/canonical form is just sorted
        std::sort( m_tys.begin(), m_tys.end() );

        // Check for repeated column names
        if ( m_tys.size() > 1 )
        {
            for ( size_t i=0; i < m_tys.size() - 1; ++i )
            {
                if ( m_tys[ i ].first == m_tys[ i+1 ].first ) {
                    throw_with< std::invalid_argument >(
                        std::ostringstream()
                        << "Column name '" << m_tys[ i ].first << "' repeated"
                    );
                }
            }
        }
    }
public:

    explicit rel_ty_t( const col_tys_t& col_tys )
        : m_tys( col_tys )
    {
        construct();
    }

    explicit rel_ty_t( col_tys_t&& col_tys )
        : m_tys( col_tys )
    {
        construct();
    }

    rel_ty_t( std::initializer_list< col_tys_t::value_type > init )
        : m_tys( init )
    {
        construct();
    }


    // union
    static rel_ty_t union_( const rel_ty_t& a, const rel_ty_t& b )
    {
        col_tys_t col_tys;

        auto a_it = a.m_tys.cbegin();
        auto b_it = b.m_tys.cbegin();

        while ( ( a_it != a.m_tys.cend() ) || ( b_it != b.m_tys.cend() ) )
        {
            if ( a_it == a.m_tys.cend() )
            {
                col_tys.push_back( *b_it );
                ++b_it;
            } else if ( b_it == b.m_tys.cend() )
            {
                col_tys.push_back( *a_it );
                ++a_it;
            } else
            {
                const auto& [ a_name, a_ty ] = *a_it;
                const auto& [ b_name, b_ty ] = *b_it;

                if ( a_name == b_name )
                {
                    if ( a_ty == b_ty )
                    {
                        col_tys.push_back( *a_it );
                    } else {
                        throw_with< std::invalid_argument >(
                            std::ostringstream()
                            << "Types for column '" << a_name 
                            << "' do not match: "
                            << ty_to_string( a_ty ) // FIXME: streaming for types
                            << " and " << ty_to_string( b_ty )
                        );
                    }
                    ++a_it;
                    ++b_it;
                } else {
                    if ( a_name < b_name ) {
                        col_tys.push_back( *a_it );
                        ++a_it;
                    } else {
                        col_tys.push_back( *b_it );
                        ++b_it;
                    }
                }
            }
        }

        // Note: col_tys is sorted by construction
        return rel_ty_t( std::move( col_tys ) );
    }
#ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable:4100)
#else
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wunused-parameter"
#endif

    // project
    template <typename Iterator>
    static rel_ty_t project( const rel_ty_t& a, Iterator begin, Iterator end )
    {
        throw not_implemented();
    }

    // minus

    static rel_ty_t minus( const rel_ty_t& a, const rel_ty_t& b )
    {
        throw not_implemented();
    }

#ifdef _MSC_VER
    #pragma warning(pop)
#else
    #pragma GCC diagnostic pop
#endif

    // intersect
    static rel_ty_t intersect( const rel_ty_t& a, const rel_ty_t& b )
    {
        col_tys_t res;
        std::map< cstring_t, type_t > b_names( b.m_tys.cbegin(), b.m_tys.cend() );

        for( auto a_it = a.m_tys.cbegin(); a_it != a.m_tys.cend(); ++a_it )
        {
            auto b_it = b_names.find( std::get<0>( *a_it ) );
            if ( b_it != b_names.cend() )
            {
                if ( std::get<1>( *a_it ) != std::get<1>( *b_it ) ) {
                    throw_with< std::invalid_argument >(
                        std::ostringstream()
                        << "Types for column '"
                        << std::get<0>(*a_it).data()
                        << "' do not match: "
                        << ty_to_string( std::get<1>(*a_it) )
                        << " and "
                        << ty_to_string( std::get<1>(*b_it) )
                    );
                }
                res.push_back( *a_it );
            }
        }
        return rel_ty_t( std::move( res ) );
    }


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