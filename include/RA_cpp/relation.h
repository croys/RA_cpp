#pragma once

#include <memory_resource>
#include <vector>
#include <algorithm>
#include <execution>
#include <memory>
#include <ostream>
#include <sstream>

#include "base.h"
#include "types.h"
#include "storage.h"

namespace rac
{

// relation builder
//
// Design considerations - the only reason we are using dynamically typed
// storage here is so we can pass ownership to a relation later.
// We could keep this fully statically typed and only create
// IStorage instances when the relation is created....
//
// Abstract over resource type?
template< typename... Types >
struct relation_builder
{
private:
    typedef std::shared_ptr<std::pmr::memory_resource> resource_ptr_t;

public:
    // FIXME: we probably should take ownership of the resource
    template<typename Iter>
    explicit relation_builder( std::pmr::memory_resource* rsrc, Iter nb, Iter ne )
    {
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


// relations - monotyped
// - relation columns are unordered
// - do we want to guarantee ordering of rows on first key?
// - do keys belong in relation type, or just to relations?
struct relation
{
    typedef std::shared_ptr<std::pmr::memory_resource> resource_ptr_t;

    // construction

    // from components

    // build relation type

    // re-arrange columns

    rel_ty_t                            m_ty;
    std::vector<IValue*>                m_ops;
    std::vector<resource_ptr_t>         m_resources;
    std::vector<IValue::storage_ptr_t>  m_cols;
};


// relation operations

// table views

// table_view - monotyped view onto a relation
// In table_view columns and rows have ordering

// table_view_t - statically typed view onto a relation
// In table_view_t columns and rows have ordering

// want iteration over rows and std::get<> to allow
// interop with std contaiers

}