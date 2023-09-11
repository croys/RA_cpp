#pragma once

#include <memory_resource>
#include <vector>
#include <algorithm>
#include <execution>
#include <memory>
#include <ostream>
#include <sstream>
#include <ranges>

#include "base.h"
#include "types.h"
#include "storage.h"

#ifndef RA_CPP_LIBRARY_HPP
#define RA_CPP_LIBRARY_HPP

#include <RA_cpp/ra_cpp_library_export.hpp>

#endif

namespace rac
{


// Convenience function to help with deducing template types
//
// FIXME: merge with col_ty stuff and move to types.h?

template<typename T>
struct col_desc
{
    template< typename S >
    explicit col_desc( S name ) : m_name( name ) {}

    template< typename S >
    col_desc( S name , type_t_traits<T> /* phantom arg */ ) : m_name( name ) {}

    constexpr std::string_view  name() const noexcept { return m_name; }
    constexpr type_t            ty() const { return type_t_traits<T>::ty(); }
    constexpr std::pair<std::string_view, type_t> col_ty() const
    {
        return std::pair { this->name(), this->ty() };
    }
private:
    std::string_view m_name;
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


typedef
    std::shared_ptr<std::pmr::memory_resource>
relation_builder_resource_ptr_t;

// Use to pass ownership of resources
struct relation_builder_resources
{
    col_tys_t                                       m_col_tys;
    std::vector<IValue*>                            m_ops;
    std::vector<relation_builder_resource_ptr_t>    m_resources;
    std::vector<IValue::storage_ptr_t>              m_cols;
};


template< typename... Types >
struct relation_builder
{
private:
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

    typedef relation_builder_resource_ptr_t resource_ptr_t;

    // FIXME: we probably should take ownership of the resource
    template<typename Iter>
    explicit relation_builder(
        std::pmr::memory_resource* rsrc, Iter nb, Iter ne
    ) : m_ops( get_ops<Types...>() )
    {
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

    // FIXME: concepts not working...
    //template<SequenceContainer C>
    template<typename C>
    explicit relation_builder( std::pmr::memory_resource* rsrc, const C& names )
        : relation_builder( rsrc, names.begin(), names.end() )
    {
    }

    explicit relation_builder( std::pmr::memory_resource* /* unused */ )
    {
    }

    // explicit relation_builder(
    //      std::pmr::memory_resource* rsrc
    //     ,std::ranges::range auto& names
    // ) : relation_builder( rsrc, names.begin(), names.end() )
    // {
    // }

    template<typename...Ts>
    explicit relation_builder(
         std::pmr::memory_resource* rsrc
        ,const col_desc<Ts>&...     args
    ) : m_ops( get_ops<Types...>() )
    {
        // Note:
        // `Types...` doesn't expand without a deduction guide
        // C++ allows instantiation with emptry/unknown parameter packs.
        //
        // This seems questionable...
        //

        for( auto [ name, ty ] : { args.col_ty()... } )
        {
            m_col_tys.emplace_back( name, ty );
        }

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
        // FIXME: use assign
        this->m_cols[col]->push_back( reinterpret_cast<const value_t*>( &v ) );
    }

    template<typename T, typename... Ts>
    constexpr void _push_back( size_t col, const T& v, Ts... vs )
    {
        // FIXME: use assign
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

    void clear()
    {
        m_col_tys.clear();
        m_ops.clear();
        m_resources.clear();
        m_cols.clear();
    }

    auto release()
    {
        relation_builder_resources res;

        std::swap( res.m_col_tys, m_col_tys );
        std::swap( res.m_ops, m_ops );
        std::swap( res.m_resources, m_resources );
        std::swap( res.m_cols, m_cols );
        this->clear();

        return res;
    }

    col_tys_t                           m_col_tys;
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


// FIXME: dynamically typed relation builder
// might want to rename above to relation_builder_t to be in line
// with all statically typed classes


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



RA_CPP_LIBRARY_EXPORT struct IRelationBase
{
    // FIXME: add Relation type to type_t?
    // Note: Tutorial D calls this the "header"
    // FIXME: col_types?
    virtual const col_tys_t&                type() const noexcept = 0;
    virtual size_t                          size() const noexcept = 0;
    virtual const value_t*                  at( size_t row, size_t col )
        const = 0;
    virtual row_slice_t rowSlice( size_t start, size_t end ) const = 0;
    virtual col_slice_t colSlice( size_t col, size_t start, size_t end )
        const = 0;

    // FIXME: Will need type_t -> IValue* function, for e.g.
    // constructing relation values dynamically from a stream
    // combine with rel_ty/col_tys_t?
    virtual const std::vector<IValue*>&     value_ops() const noexcept = 0;

    virtual ~IRelationBase() = default;
};


//
// IRelation
//
// column slices/iterators
// row slice/iteratos
//  separete iterator type for column/row slice iterators
//  asking for slice returns std compatible [begin, end) for each
//  or slice type has ForwardIterator/etc

RA_CPP_LIBRARY_EXPORT struct IRelation : IRelationBase
{
    virtual const std::vector<col_tys_t>&   keys() const noexcept = 0;

    virtual ~IRelation() = default;
};


// FIXME: Separate out mutable/immutable interfaces?


// FIXME: move out of header
//
// relations - monotyped
// - relation columns are unordered
// - do we want to guarantee ordering of rows on first key?
// - do keys belong in relation type, or just to relations?
RA_CPP_LIBRARY_EXPORT struct relation : IRelation
{
    // IRelation

    const col_tys_t&    type() const noexcept override;
    size_t              size() const noexcept override;

    // FIXME: pmr?
    const std::vector<col_tys_t>& keys() const noexcept override;


    const value_t* at( size_t row, size_t col ) const override;

    row_slice_t rowSlice( size_t start, size_t end ) const override;
    col_slice_t colSlice( size_t col, size_t start, size_t end ) const override;

    const std::vector<IValue*>&     value_ops() const noexcept override;

    typedef std::shared_ptr<std::pmr::memory_resource> resource_ptr_t;

    ///
    ///
    ///

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


    // construction from relation_builder
    // relation takes ownership of storage
    //
    explicit relation(
        relation_builder_resources&& res
        // FIXME: keys
    );

    virtual ~relation() = default;

    std::ostream& dump( std::ostream& os ) const;


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

RA_CPP_LIBRARY_EXPORT struct ITable : IRelationBase
{
    virtual ~ITable() = default;
};

// FIXME: ITableM for dynamically typed updates

// table_view for dynamically typed view onto an IRelation

RA_CPP_LIBRARY_EXPORT struct table_view : ITable
{
    // FIXME: custom comparison function over multiple columns - later
    // FIXME: vector of column name and ascending/descending
    explicit table_view(
         std::shared_ptr<IRelation>& rel
        ,std::ranges::forward_range auto col_names
    ) : m_rel( rel )
    {
        // FIXME: do this with std::views
        m_col_map.reserve( col_names.size() );
        for( const auto cn : col_names ) {
            auto it = m_rel->type().cbegin();
            for ( ; it != m_rel->type().cend(); ++it ) {
                if ( it->first == cn ) {
                    break;
                }
            }
            if (it == m_rel->type().cend()) {
                throw_with<std::invalid_argument>(
                    std::ostringstream()
                    << "Unknown column '" << cn << "'"
                );
            } else {
                auto j = it - m_rel->type().cbegin();
                m_col_map.push_back( size_t( j ) );
            }
        }

        // build m_col_tys and m_ops
        const size_t n = m_col_map.size();
        m_col_tys.reserve( n );
        m_ops.reserve( n );
        for ( const auto c : m_col_map ) {
            m_col_tys.emplace_back( m_rel->type()[ c ] );
            m_ops.emplace_back( m_rel->value_ops()[ c ] );
        }

        // Start with identity map of rows
        m_row_map.resize( m_rel->size() );
        std::iota(m_row_map.begin(), m_row_map.end(), 0 );

        // sort through the mapping
        auto row_cmp = [&]( size_t a, size_t b )
        {
            for ( const auto c : m_col_map ) {
                auto cmp = m_rel->value_ops()[ c ]->cmp(
                            m_rel->at( a, c )
                            ,m_rel->at( b, c )
                );

                if (cmp == std::strong_ordering::less)      { return true; }
                if (cmp == std::strong_ordering::greater)   { return false; }
            }
            return false;
        };
        std::sort( m_row_map.begin(), m_row_map.end(), row_cmp );
    }

    virtual ~table_view() = default;

    // ITable interface

    const col_tys_t&    type() const noexcept override;
    size_t              size() const noexcept override;
    const value_t*      at( size_t row, size_t col ) const override;
    const std::vector<IValue*>& value_ops() const noexcept override;

    row_slice_t rowSlice( size_t start, size_t end ) const override;
    col_slice_t colSlice( size_t col, size_t start, size_t end ) const override;


private:
    std::vector<size_t>         m_col_map;  // column map
    std::vector<size_t>         m_row_map;  // row ordering map
    std::shared_ptr<IRelation>  m_rel;

    // ephemeral/dervied
    col_tys_t                   m_col_tys;
    std::vector<IValue*>        m_ops;
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


RA_CPP_LIBRARY_EXPORT std::ostream& relation_to_stream(
     std::ostream&           os
    ,const IRelationBase*   rel
);

}