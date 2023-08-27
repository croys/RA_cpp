#pragma once

#include <memory_resource>
#include <vector>
#include <algorithm>
#include <execution>
#include <memory>
#include <ostream>
#include <sstream>
//#include <concepts>

#include "base.h"
#include "types.h"
#include "storage.h"

namespace rac
{


// Convenience function to help with deducing template types
//
// FIXME: merge with col_ty stuff and move to types.h?

template<typename T>
struct col_desc
{
    template< typename S >
    col_desc( S name , type_t_traits<T> type ) :
        m_name( name ), m_type( type )
    {}

    std::string_view    m_name;
    type_t_traits<T>    m_type;
};


// relation builder
//
// Design considerations - the only reason we are using dynamically typed
// storage here is so we can pass ownership to a relation later.
// We could keep this fully statically typed and only create
// IStorage instances when the relation is created....
//
// Abstract over resource type?
//
// Note: order of columns is preserved, so this is really a table builder
// also, relation builder needs to check key validity
template< typename... Types >
struct relation_builder
{
private:
    typedef std::shared_ptr<std::pmr::memory_resource> resource_ptr_t;


    template< typename T>
    void make_cols( col_desc<T> c )
    {
        m_col_tys.emplace_back( c.m_name, type_t_traits<T>::ty() );
    }

    template<typename T, typename...Ts>
    void make_cols( col_desc<T> c, Ts... args )
    {
        m_col_tys.emplace_back( c.m_name, type_t_traits<T>::ty() );
        make_cols( args... );
    }

    void make_storage( std::pmr::memory_resource* rsrc )
    {
        for (const auto& op : m_ops ) {
            // we could use memory directly allocated from monotonic_buffer_resource
            // instead of a std::vector
            //resource_ptr_t r = std::make_shared<std::pmr::monotonic_buffer_resource>( rsrc );
            resource_ptr_t r  =
                std::make_shared<std::pmr::unsynchronized_pool_resource>( rsrc );
            m_cols.emplace_back( op->make_storage( r.get() ) );
            m_resources.emplace_back( r );
        }
    }

public:
    // FIXME: we probably should take ownership of the resource
    template<typename Iter>
    explicit relation_builder(
        std::pmr::memory_resource* rsrc, Iter nb, Iter ne
    ) {
        m_ops = get_ops<Types...>();

        // FIXME: refactor - extract out
        if ( size_t(ne - nb) != m_ops.size() ) {
            throw_with< std::invalid_argument >(
                std::ostringstream()
                << "Size of column names and types do not match, "
                << "names: " << ne-nb << ", types: " << m_ops.size()
            );
        }
        Iter ni = nb;
        for (auto oi = m_ops.cbegin(); ni < ne; ++ni, ++oi) {
            m_col_tys.push_back( std::make_pair(*ni, (*oi)->type() ) );
        }
        make_storage(rsrc);
    }

    // FIXME: concepts not working, figure out what flags are needed...
    //template<std::Container C>
    template<typename C>
    explicit relation_builder( std::pmr::memory_resource* rsrc, const C& names )
        : relation_builder( rsrc, names.begin(), names.end() )
    {
    }

    template<typename T, typename...Ts>
    explicit relation_builder(
         std::pmr::memory_resource* rsrc
        ,col_desc<T> arg0
        ,Ts... args
    ) {
        // Note:
        // `Types...` doesn't expand without a deduction guide
        // C++ allows instantiation with emptry/unknown parameter packs.
        //
        // This seems questionable...
        //
        m_ops = get_ops<Types...>();
        make_cols( arg0, args... );
        make_storage( rsrc );
    }

    constexpr const col_tys_t& type() const noexcept
    {
        return m_col_tys;
    }

    size_t constexpr size() const noexcept
    {
        return m_cols.size() > 0 ? m_cols[0]->size() : 0;
    }


    // push_back
private:
    template<typename T>
    constexpr void _push_back( size_t col, const T& v )
    {
        this->m_cols[col]->push_back( reinterpret_cast<const value_t*>( &v ) );
    }

    template<typename T, typename... Ts>
    constexpr void _push_back( size_t col, const T& v, Ts... vs )
    {
        this->m_cols[col]->push_back( reinterpret_cast<const value_t*>( &v ) );
        this->_push_back( col + 1, vs...);
    }
public:
    constexpr void push_back( Types... vs )
    {
        this->_push_back( 0, vs... );
    }

#if 0
    constexpr void push_back( const std::tuple<Types...>& v )
    {
    }
#endif
#if 0
    constexpr void push_back( const std::tuple<Types...>&& v )
    {
    }
#endif

    // FIXME: emplace_back


public:
    constexpr std::tuple<Types...> at( size_t idx ) const
    {
        return col_helper<Types...>::row( m_cols, 0, idx );
    }

    std::ostream& dump( std::ostream& os ) const
    {
        // dump type

        os << "relation_builder ";
        col_tys_to_stream( os, m_col_tys );
        os << "\n\n";

        return cols_to_stream( os, m_col_tys, m_ops, m_cols );
    }

    col_tys_t                           m_col_tys;
    // FIXME: combine following into tuple?
    std::vector<IValue*>                m_ops;
    std::vector<resource_ptr_t>         m_resources;
    std::vector<IValue::storage_ptr_t>  m_cols;

    // FIXME: move this over to being fully statically typed
    // use a tuple of shared_ptr to each column_storage class
    // can then construct IValue for each and pass off to relation
    // if we just add a typed_storage() method to untyped_column_storage,
    // then we can just keep untype_column_storage instances, and
    // use these to build the relation...
};

// Deduction guide
template<typename R, typename... Ts>
relation_builder( R rsrc, col_desc<Ts>... )
-> relation_builder<Ts...>;

// Helper function works...
template<typename... Ts>
relation_builder<Ts...>
make_relation_builder(
     std::pmr::memory_resource* rsrc
    ,col_desc<Ts>...            args
) {
    return relation_builder<Ts...>( rsrc, args... );
}


#ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable:4100)
#else
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wunused-parameter"
#endif


#ifdef _MSC_VER
    #pragma warning(push)
#else
    #pragma GCC diagnostic push
#endif

// FIXME: Have IRowSlice/IColSlize and row_slice_t/col_slice_t over there
// to allow abstraction over iteration?
struct row_slice_t
{
    // FIXME: implement
};
struct col_slice_t
{
    // FIXME: implement
};

//
// IRelation
//
// column slices/iterators
// row slice/iteratos
//  separete iterator type for column/row slice iterators
//  asking for slice returns std compatible [begin, end) for each
//  or slice type has ForwardIterator/etc

struct IRelation
{
    virtual const rel_ty_t&                 type() const noexcept = 0;
    virtual size_t                          size() const noexcept = 0;
    virtual const std::vector<col_tys_t>&   keys() const noexcept = 0;
    virtual const value_t*                  at( size_t row, size_t col )
        const = 0;
    virtual row_slice_t rowSlice( size_t start, size_t end ) const = 0;
    virtual col_slice_t colSlice( size_t col, size_t start, size_t end )
        const = 0;

    virtual ~IRelation() {}
};


// FIXME: Separate out mutable/immutable interfaces?


// FIXME: move out of header
//
// relations - monotyped
// - relation columns are unordered
// - do we want to guarantee ordering of rows on first key?
// - do keys belong in relation type, or just to relations?
struct relation : IRelation
{
    // IRelation

    const rel_ty_t& type() const noexcept override
    {
        return m_ty;
    }

    size_t size() const noexcept override
    {
        return m_cols.size() > 0 ? m_cols[0]->size() : 0;
    }

    // FIXME: pmr?
    const std::vector<col_tys_t>& keys() const noexcept override
    {
        return m_keys;
    }

#ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable:4100)
#else
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wunused-parameter"
#endif

    const value_t* at( size_t row, size_t col ) const override
    {
        throw not_implemented();
    }

    row_slice_t rowSlice( size_t start, size_t end ) const override
    {
        throw not_implemented();
    }
    col_slice_t colSlice( size_t col, size_t start, size_t end ) const override
    {
        throw not_implemented();
    }

#ifdef _MSC_VER
    #pragma warning(pop)
#else
    #pragma GCC diagnostic pop
#endif

    typedef std::shared_ptr<std::pmr::memory_resource> resource_ptr_t;

    ///
    ///
    ///

    // construction

    // from builder

    // build relation type

    // re-arrange columns

    // keys - do we want a primary, physical key?
    // all other keys need to be checked at construction time, but do
    // we need to maintain structures (e.g. map) for all?
    //   - probably worthwhile for e.g. project/join etc...
    // map can be implicit - just a sorted list of row indices...

    // primary/physical key could be optional
    // if specified, could allow specifying the data should be sorted
    // then only need to check the key, not do any sort.
    // can also specify if columns should be compressed
    // - primary key can be stored via RLE/tree stucture
    // - probably only worthwhile for heavier types?

    rel_ty_t                            m_ty;
    std::vector<col_tys_t>              m_keys; // FIXME: pmr
    std::vector<IValue*>                m_ops;
    std::vector<resource_ptr_t>         m_resources;
    std::vector<IValue::storage_ptr_t>  m_cols;
};


// relation operations

// table views


// table_view - monotyped view onto a relation
// In table_view columns and rows have ordering

struct ITable
{
    virtual const col_tys_t&    type() const noexcept = 0;
    virtual size_t              size() const noexcept = 0;
    virtual const value_t*      at( size_t row, size_t col ) const = 0;

    virtual row_slice_t rowSlice( size_t start, size_t end ) const = 0;
    virtual col_slice_t colSlice( size_t col, size_t start, size_t end )
        const = 0;

    virtual ~ITable() {}
};



// table_view_t - statically typed view onto a relation
// In table_view_t columns and rows have ordering

// want iteration over rows and std::get<> to allow
// interop with std contaiers

// column and row slices/iterators
// column ordering generally fully specified
// row ordering via function over specified columns
//   - but can also have explicit ordering just vector of indices
//   - to be stable, needs source to be ordered (i.e. a Table)
//  - best to leave that for presentation/UI

// note: want default ordering for unspecified columns to ensure
// table is stable, otherwise might get weird UI behaviour



}