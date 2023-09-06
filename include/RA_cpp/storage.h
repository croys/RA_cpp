#pragma once

#include <compare>
#include <memory_resource>
#include <vector>
#include <algorithm>
#include <execution>
#include <memory>
#include <ostream>
#include <sstream>
#include <cmath>

#include "base.h"
#include "types.h"

namespace rac
{

struct value_t{};


// FIXME: use std::iterator

// Note: we implement as much of the stdlib iterators as makes sense
// As we are abstracting over variable sized storage of unknown type
// we cannot provide anything where the type leaks
// - no dereferencing, so std::copy and std::move cannot work with
// these iterators
struct const_value_iterator
{
    //typedef const value_t*  value_type;
    typedef int64_t         difference_type;
    //typedef const value_t*  reference_type;

    explicit constexpr const_value_iterator( const value_t* ptr, size_t size )
        : m_ptr( ptr ), m_size( size )
    {}


    // Iterator

    // operator* purposefully left out
    constexpr const_value_iterator& operator++() noexcept
    {
        m_ptr += m_size;
        return *this;
    }

    // ForwardIterator
    constexpr const_value_iterator operator++(int) noexcept // post increment
    {
        return const_value_iterator( m_ptr + m_size, m_size );
    }

    // BidirectionalIterator

    constexpr const_value_iterator& operator--() noexcept
    {
        m_ptr -= m_size;
        return *this;
    }

    constexpr const_value_iterator operator--(int) noexcept // post-decrement
    {
        return const_value_iterator( m_ptr - m_size, m_size );
    }

    // RandomAccessIterator
#if 0
    constexpr reference_type operator[](size_t idx) noexcept
    {
        return m_ptr + ( idx * m_size );
    }
#endif

    constexpr const_value_iterator& operator+=(size_t n) noexcept
    {
        m_ptr += n * m_size;
        return *this;
    }

    constexpr const_value_iterator& operator-=(size_t n) noexcept
    {
        m_ptr -= n * m_size;
        return *this;
    }

    constexpr const value_t* get() const noexcept
    {
        return m_ptr;
    }

    constexpr size_t elem_size() const noexcept
    {
        return m_size;
    }

private:
    const value_t*  m_ptr;
    size_t          m_size;
};


template<typename T>
constexpr const_value_iterator operator+(
     const const_value_iterator&    it
    ,const T                        d
) noexcept {
    return const_value_iterator(
         it.get()
            + static_cast<const_value_iterator::difference_type>(d)
            * static_cast<long>(it.elem_size())
        ,it.elem_size()
    );
}

template<typename T>
constexpr const_value_iterator operator+(
     const T                        d
    ,const const_value_iterator&    it
) noexcept {
    return const_value_iterator(
         it.get()
            + static_cast<const_value_iterator::difference_type>(d)
            * static_cast<long>(it.elem_size())
        ,it.elem_size()
    );
}

constexpr const_value_iterator::difference_type operator-(
     const const_value_iterator& a
    ,const const_value_iterator& b
) noexcept {
    return ( a.get() - b.get() ) / static_cast<long>(a.elem_size());
}

template<typename T>
constexpr const_value_iterator operator-(
     const const_value_iterator&    a
    ,const T                        b
) noexcept {
    return const_value_iterator(
         a.get()
            - ( static_cast<const_value_iterator::difference_type>(b)
                * static_cast<long>(a.elem_size()) )
        ,a.elem_size()
    );
}

constexpr bool operator==(
     const const_value_iterator& a
    ,const const_value_iterator& b
) {
    return a.get() == b.get();
}

constexpr auto operator<=>(
     const const_value_iterator& a
    ,const const_value_iterator& b
) {
    return a.get() <=> b.get();
}



struct value_iterator
{
    typedef int64_t         difference_type;

    constexpr explicit value_iterator( value_t* ptr, size_t size )
        : m_ptr( ptr ), m_size( size )
    {}

    // Iterator

    // operator* purposefully left out
    constexpr value_iterator& operator++() noexcept
    {
        m_ptr += m_size;
        return *this;
    }

    // ForwardIterator
    constexpr value_iterator operator++(int) noexcept // post increment
    {
        return value_iterator( m_ptr + m_size, m_size );
    }

    // BidirectionalIterator

    constexpr value_iterator& operator--() noexcept
    {
        m_ptr -= m_size;
        return *this;
    }

    constexpr value_iterator operator--(int) noexcept // post-decrement
    {
        return value_iterator( m_ptr - m_size, m_size );
    }

    // RandomAccessIterator

    // operator[] purposefully left out

    constexpr value_iterator& operator+=( size_t n ) noexcept
    {
        m_ptr += n * m_size;
        return *this;
    }

    constexpr value_iterator& operator-=( size_t n ) noexcept
    {
        m_ptr -= n * m_size;
        return *this;
    }

    constexpr value_t* get() const noexcept
    {
        return m_ptr;
    }

    constexpr size_t elem_size() const noexcept
    {
        return m_size;
    }

private:
    value_t*    m_ptr;
    size_t      m_size;
};


template<typename T>
constexpr value_iterator operator+(
     const value_iterator&  it
    ,const T                d
) noexcept {
    return value_iterator(
         it.get()
            + static_cast<value_iterator::difference_type>(d)
            * static_cast<long>(it.elem_size())
        ,it.elem_size()
    );
}

template<typename T>
constexpr value_iterator operator+(
     const T                d
    ,const value_iterator&  it
) noexcept {
    return value_iterator(
         it.get()
            + static_cast<value_iterator::difference_type>(d)
            * static_cast<long>(it.elem_size())
        ,it.elem_size()
    );
}

constexpr value_iterator::difference_type operator-(
     const value_iterator& a
    ,const value_iterator& b
) noexcept {
    return ( a.get() - b.get() ) / static_cast<long>(a.elem_size());
}

template<typename T>
constexpr value_iterator operator-(
     const value_iterator&  a
    ,const T                b
) noexcept {
    return value_iterator(
         a.get()
            - ( static_cast<value_iterator::difference_type>(b)
                * static_cast<long>(a.elem_size()) )
        ,a.elem_size()
    );
}

constexpr bool operator==( const value_iterator& a, const value_iterator& b )
{
    return a.get() == b.get();
}

constexpr auto operator<=>( const value_iterator& a, const value_iterator& b )
{
    return a.get() <=> b.get();
}




// untyped access to aligned, contiguous storage of monotyped values
//
// tempting to split out interfaces into pure and mutable, but all
// implementations will implement both, so we can just stick
// with const methods and use IStorage/const IStorage as appropriate
struct IStorage
{
    typedef const_value_iterator    const_iterator;
    typedef value_iterator          iterator;

    // immutable access
    virtual const value_t*  at( size_t idx ) const = 0;
    virtual size_t          size() const noexcept = 0;
    virtual bool            empty() const noexcept = 0;
    virtual const_iterator  cbegin() const noexcept = 0;
    virtual const_iterator  cend() const noexcept = 0;

    // mutable access
    virtual value_t*        at( size_t idx ) = 0;
    virtual iterator        begin() noexcept = 0;
    virtual iterator        end() noexcept = 0;

    // reserve
    virtual void reserve( size_t sz ) = 0;

    // resize
    virtual void resize( size_t sz ) = 0;

    // push_back
    virtual void push_back( const value_t* v ) = 0;

    // insert - limit to extend?


    // copy
    virtual void copy(   const const_iterator&  fromb
                        ,const const_iterator&  frome
                        ,iterator               to
                        ) = 0;

    // move
    virtual void move(   iterator  fromb
                        ,iterator  frome
                        ,iterator  to
                        ) = 0;

    virtual ~IStorage() {}
};

// We use one monotonic_buffer_resource per column
// to get good locality without fragmentation

// Make this a base class, so we can specialise, e.g. std::string -> std::pmr::string?
// Or just have a bunch of helper functions for std::string, etc..
// and always use std::pmr::string?
// deconstruction should be string_view...
// - have a floor size - do reserve on first alloc/store
// - might want two resources for strings
//   - one for string
//   - another for data and/or pooled strings
// - separate resource for each of strings and data
// - one more for string_views into pool
// might want to share pool across the whole relation, or even across relations
// => need pool to use shared pointers

// FIXME: use std::pmr::string and allow separate allocators for strings
// (and perhaps per column?)

template<typename T>
struct column_storage_base
{
    // types

    typedef std::pmr::vector<T> vec_t;

    typedef typename vec_t::value_type       value_type;
    typedef typename vec_t::size_type        size_type;
    typedef typename vec_t::const_reference  const_reference;
    typedef typename vec_t::reference        reference;
    typedef typename vec_t::const_iterator   const_iterator;
    typedef typename vec_t::iterator         iterator;

    //typedef std::shared_ptr<std::pmr::memory_resource> resource_ptr_t;
    typedef std::pmr::memory_resource* resource_ptr_t;

    explicit column_storage_base( resource_ptr_t rsrc )
        : m_rsrc( rsrc ), m_vec( rsrc ) {
        assert( rsrc );
    }

    // immutable deconstruction

    constexpr bool empty() const noexcept
    {
        return this->m_vec.empty();
    }

    constexpr size_type size() const noexcept
    {
        return this->m_vec.size();
    }

    constexpr const_iterator cbegin() const noexcept
    {
        return this->m_vec.cbegin();
    }

    constexpr const_iterator cend() const noexcept
    {
        return this->m_vec.cend();
    }

    constexpr const_reference at( size_type i ) const
    {
        return this->m_vec.at( i );
    }

    constexpr const_reference operator[]( size_type i ) const
    {
        return this->m_vec[i];
    }

    constexpr const T* data() const noexcept
    {
        return this->m_vec.data();
    }

    // mutation

    constexpr iterator begin() noexcept
    {
        return this->m_vec.begin();
    }

    constexpr iterator end() noexcept
    {
        return this->m_vec.end();
    }

    constexpr reference at( size_type i )
    {
        return this->m_vec.at( i );
    }

    constexpr reference operator[]( size_type i )
    {
        return this->m_vec[i];
    }

    constexpr T* data() noexcept
    {
        return this->m_vec.data();
    }

    constexpr void reserve( size_t sz )
    {
        this->m_vec.reserve( sz );
    }

    constexpr void resize( size_type sz )
    {
         this->m_vec.resize( sz );
    }

    constexpr void push_back( const T& v )
    {
        this->m_vec.push_back( v );
    }

    constexpr void push_back( const T&& v )
    {
        this->m_vec.push_back( v );
    }


protected:
    resource_ptr_t  m_rsrc;
    vec_t           m_vec;
};


template<typename T>
struct column_storage : public column_storage_base< T >
{
    using typename column_storage_base< T >::resource_ptr_t;

    explicit column_storage( resource_ptr_t rsrc )
        : column_storage_base< T >( rsrc )
    {
    }

    virtual ~column_storage() {}
};


template<typename T>
struct untyped_column_storage : public IStorage
{
    typedef std::shared_ptr< column_storage< T > > storage_ptr_t;
    //typedef column_storage< T >* storage_ptr_t;

    explicit untyped_column_storage( storage_ptr_t storage ) :
        m_storage( storage )
    {
    }

    virtual ~untyped_column_storage() {}

private:

    // Convenience
    // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
    static constexpr value_t* v( T* x ) noexcept
    {
        return reinterpret_cast<value_t*>(x);
    }

    static constexpr const value_t* cv( const T* x ) noexcept
    {
        return reinterpret_cast<const value_t*>(x);
    }

    static constexpr T* t( value_t * x ) noexcept
    {
        return reinterpret_cast<T*>(x);
    }

    static constexpr const T* ct( const value_t  * x ) noexcept
    {
        return reinterpret_cast<const T*>(x);
    }
    // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)

public:

    // IStorage interface

    const value_t* at( size_t idx ) const override
    {
        return cv( &( m_storage->at( idx ) ) );
    }

    size_t size() const noexcept override
    {
        return m_storage->size();
    }

    bool empty() const noexcept override
    {
        return m_storage->empty();
    }

    const_iterator cbegin() const noexcept override
    {
        return const_value_iterator( cv( m_storage->data() ), sizeof( T ) ); // FIXME: sizeof?
    }

    const_iterator cend() const noexcept override
    {
        return const_value_iterator( cv( m_storage->data() + m_storage->size() ), sizeof( T ) ); // FIXME: sizeof?
    }

    value_t* at( size_t idx ) override
    {
        return v( &( m_storage->at( idx ) ) );
    }


    iterator begin() noexcept override
    {
        return value_iterator( v( m_storage->data() ), sizeof( T ) ); // FIXME: sizeof?
    }

    iterator end() noexcept override
    {
        return value_iterator( v( m_storage->data() + m_storage->size() ), sizeof( T ) ); // FIXME: sizeof
    }

    void reserve( size_t sz ) override
    {
        m_storage->reserve( sz );
    }

    void resize( size_t sz ) override
    {
        m_storage->resize( sz );
    }

    void push_back( const value_t* v ) override
    {
        const T* v_ = ct( v );
        m_storage->push_back( *v_ );
    }

    void copy(   const const_iterator&  fromb
                ,const const_iterator&  frome
                ,iterator               to
    ) override
    {
        const T* fb = ct( fromb.get() );
        const T* fe = ct( frome.get() );
        T* to_      = t( to.get() );
        // abstract out execution policy?
        // select based on size?
        // std::copy( std::execution::par_unseq, fb, fe, to_ );
        std::copy( fb, fe, to_ );
    }

    void move(   iterator   fromb
                ,iterator   frome
                ,iterator   to
    ) override
    {
        T* fb   = t( fromb.get() );
        T* fe   = t( frome.get() );
        T* to_  = t( to.get() );

        // std::move( std::execution::par_unseq, fb, fe, to_ );
        // std::move( std::execution::unseq, fb, fe, to_ );
        std::move( fb, fe, to_ );
    } 

private:
    storage_ptr_t m_storage;
};



template<typename T>
struct value_ops_base
{
    typedef std::shared_ptr<IStorage> storage_ptr_t;

    static storage_ptr_t make_storage( std::pmr::memory_resource* rsrc )
    {
        // FIXME: use another memory resource for the objects themselves
        auto s = std::make_shared< column_storage< T > >( rsrc );
        auto us =  std::make_shared< untyped_column_storage< T > >( s );
        return us;
    }

};


template<typename T>
struct value_ops : public value_ops_base< T >
{
};

// FIXME: std::vector<bool> is specialised, so this needs to be too...
template<>
struct value_ops<bool> : public value_ops_base<bool>
{
    static constexpr const type_t type() noexcept {
        return type_t( { Bool } );
    }
};

template<>
struct value_ops<int> : public value_ops_base<int>
{
    static constexpr const type_t type() noexcept {
        return type_t( { Int } );
    }

};

template<>
struct value_ops<float> : public value_ops_base<float>
{
    static constexpr const type_t type() noexcept {
        return type_t( { Float } );
    }

};

template<>
struct value_ops<double> : public value_ops_base<double>
{
    static constexpr const type_t type() noexcept {
        return type_t( { Double } );
    }

};






// IValue - Dynamically/monotyped value operations
// Arguably could be merged with IStorage, though later we will have
// comparison operations, etc...
struct IValue
{
    virtual constexpr type_t type() const noexcept = 0;

    // FIXME:

    // init,clear,move,copy/asign,swap

    // compare/lt,eq - stability requirements?
    // partial vs total

    // total - stability requirements?
    virtual std::strong_ordering cmp( const value_t* a, const value_t* b )
        const noexcept = 0;

    virtual std::ostream& to_stream( const value_t* v, std::ostream& os )
        const = 0;

    // really belongs in IColumn/IColumnBuilder/etc...
    typedef std::shared_ptr<IStorage> storage_ptr_t;

    virtual storage_ptr_t make_storage(
        std::pmr::memory_resource* rsrc
    ) const = 0;

protected:
    ~IValue() {}
};


template<typename T>
struct strong_ordering
{
    static constexpr std::strong_ordering cmp(
         const T* a
        ,const T* b
    ) noexcept
    {
        return *a <=> *b;
    }
};

template<>
struct strong_ordering<float>
{
    static /* constexpr */ std::strong_ordering cmp(
         const float* a
        ,const float* b
    ) noexcept
    {
        // Note: std::partial_ordering not enum/int, can't use switch :-()
        auto c = *a <=> *b;
        if ( c == std::partial_ordering::less )
            return std::strong_ordering::less;
        if ( c == std::partial_ordering::equivalent )
            return std::strong_ordering::equivalent;
        if ( c == std::partial_ordering::greater )
            return std::strong_ordering::greater;
        if ( c == std::partial_ordering::unordered )
        {
            if ( std::isnan( *a ) ) {
                if ( std::isnan( *b ) )
                    return std::strong_ordering::equivalent;
                else
                    return std::strong_ordering::greater;
            } else {
                if ( std::isnan( *b ) )
                    return std::strong_ordering::less;
                else
                    // not possible
                    return std::strong_ordering::less;
            }
        }
        // not possible
        return std::strong_ordering::less;
    }
};

// FIXME: refactor and share with above?
template<>
struct strong_ordering<double>
{
    static /* constexpr */ std::strong_ordering cmp(
         const double* a
        ,const double* b
    ) noexcept
    {
        // Note: std::partial_ordering not enum/int, can't use switch :-()
        auto c = *a <=> *b;
        if ( c == std::partial_ordering::less )
            return std::strong_ordering::less;
        if ( c == std::partial_ordering::equivalent )
            return std::strong_ordering::equivalent;
        if ( c == std::partial_ordering::greater )
            return std::strong_ordering::greater;
        if ( c == std::partial_ordering::unordered )
        {
            if ( std::isnan( *a ) ) {
                if ( std::isnan( *b ) )
                    return std::strong_ordering::equivalent;
                else
                    return std::strong_ordering::greater;
            } else {
                if ( std::isnan( *b ) )
                    return std::strong_ordering::less;
                else
                    // not possible
                    return std::strong_ordering::less;
            }
        }
        // not possible
        return std::strong_ordering::less;
    }
};


template<typename T>
struct untyped_value_ops : public IValue
{
private:
    typedef value_ops<T> val_t;

    // Convenience
    // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
    static constexpr value_t* v( T* x ) noexcept
    {
        return reinterpret_cast<value_t*>(x);
    }

    static constexpr const value_t* cv( const T* x ) noexcept
    {
        return reinterpret_cast<const value_t*>(x);
    }

    static constexpr T* t( value_t * x ) noexcept
    {
        return reinterpret_cast<T*>(x);
    }

    static constexpr const T* ct( const value_t  * x ) noexcept
    {
        return reinterpret_cast<const T*>(x);
    }
    // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)

public:

    // IValue
    using typename IValue::storage_ptr_t;

    constexpr type_t type() const noexcept override
    {
        return val_t::type();
    }

    std::strong_ordering cmp( const value_t* a, const value_t* b )
        const noexcept override
    {
        return strong_ordering<T>::cmp( ct( a ), ct( b ) );
    }

    //
    std::ostream& to_stream( const value_t* v, std::ostream& os ) const override
    {
        return os << *ct( v );
    }

    storage_ptr_t make_storage(
        std::pmr::memory_resource* rsrc
    ) const override
    {
        return val_t::make_storage( rsrc );
    }

    // Note that multiple instantiations will result in multiple statics.
    // We don't care, as this class has no storage, and all instances are canonical.
    // Also, use of this function should be limited.
    static IValue* ops() noexcept
    {
        static untyped_value_ops<T> ops;
        return &ops;
    }

    virtual ~untyped_value_ops() {}
};

// FIXME:
struct column_storage_t
{
    rel_ty_t m_ty;

    typedef std::shared_ptr<IStorage> storage_ptr_t;

    std::vector<storage_ptr_t> m_cols;
};

// Typed operations over multiple columns
// FIXME: these are just folds
template<typename T, typename... Ts>
struct col_helper
{
    static constexpr std::tuple<T, Ts...> row(
         const std::vector<IValue::storage_ptr_t>&  cols
        ,size_t                                     col
        ,size_t                                     row
    )
    {
        return std::tuple_cat(
            // FIXME: this is wrong, should be using
            // value_ops::get<> or similar
            std::tuple<T>( *(reinterpret_cast<const T*>( cols[ col ]->at( row ) ) ) ),
            col_helper<Ts...>::row( cols, col + 1, row )
        );
    }
};


template<typename T>
struct col_helper<T>
{
    static constexpr std::tuple<T> row(
         const std::vector<IValue::storage_ptr_t>&  cols
        ,size_t                                     col
        ,size_t                                     row
    )
    {
        return std::tuple<T>( *(reinterpret_cast<const T*>( cols[ col ]->at( row ) ) ) );
    }
};

template<typename ... Ts>
std::vector<IValue*> get_ops()
{
    return std::vector { untyped_value_ops<Ts>::ops()... };
}


std::ostream& cols_to_stream(
     std::ostream&                              os
    ,const col_tys_t&                           col_tys
    ,const std::vector<IValue*>&                ops
    ,const std::vector<IValue::storage_ptr_t>&  cols
)
{
    const size_t n_cols = cols.size();
    if (n_cols > 0) {
        const size_t n_rows = cols[ 0 ]->size();

        // Work out column widths
        std::vector<size_t> col_sizes( n_cols );
        for ( size_t c = 0; c < n_cols; ++c )
        {
            col_sizes[ c ] = col_tys[ c ].first.size();
        }

        std::ostringstream ss;
        size_t total_width = 0;
        for ( size_t c = 0; c < n_cols; ++c )
        {
            size_t m = col_sizes[ c ];
            for (auto it = cols[ c ]->cbegin(); it != cols[ c ]->cend(); ++it )
            {
                ss.str("");
                ops[ c ]->to_stream( it.get(), ss );
                m = std::max( size_t( ss.tellp() ), m );
            }
            col_sizes[ c ] = m;

            if (c > 0) {
                os << "  ";
                total_width += 2;
            }
            ss.str("");
            ss.width( static_cast<long>(m) );
            ss << col_tys[ c ].first;
            ss.width( 0 );
            os << ss.str();
            total_width += m;
        }
        os << "\n";

        for ( size_t i=0; i < total_width; ++i )
        {
            os << "-";
        }
        os << "\n";

        // values

        for ( size_t r = 0; r < n_rows; ++r )
        {
            for( size_t c = 0; c < n_cols; ++c )
            {
                if (c > 0) {
                    os << "  ";
                }
                ss.str("");
                ss.width( static_cast<long>(col_sizes[ c ]) );
                ops[ c ]->to_stream( cols[ c ]->at( r ), ss );
                ss.width(0);
                os << ss.str();
            }
            os << "\n";
        }
    }
    return os;
}

}