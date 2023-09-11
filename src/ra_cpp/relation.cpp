#include <RA_cpp/relation.h>

namespace rac
{

// NOLINTBEGIN(readability-identifier-length)

#if 0
IRelationBase::~IRelationBase()
{
}

IRelation::~IRelation()
{
}
#endif

RA_CPP_LIBRARY_EXPORT struct IRelationBase;

RA_CPP_LIBRARY_EXPORT struct IRelation;

RA_CPP_LIBRARY_EXPORT struct relation;

const col_tys_t& relation::type() const noexcept
{
    return m_ty.m_tys;
}

size_t relation::size() const noexcept
{
    return m_cols.empty() ? 0 : m_cols[0]->size();
}

const std::vector<col_tys_t>& relation::keys() const noexcept
{
    return m_keys;
}

const value_t* relation::at( size_t row, size_t col ) const
{
    return m_cols[ col ]->at( row );
}

#ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable:4100)
#else
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wunused-parameter"
#endif


row_slice_t relation::rowSlice( size_t start, size_t end ) const
{
    (void) start; (void) end;  // FIXME: for cppcheck, remove
    throw not_implemented();
}

// NOLINTBEGIN(bugprone-easily-swappable-parameters)
col_slice_t relation::colSlice( size_t col, size_t start, size_t end ) const
{
    (void) col; (void) start; (void) end;   // FIXME: for cppcheck, remove
    throw not_implemented();
}
// NOLINTEND(bugprone-easily-swappable-parameters)

const std::vector<IValue*>& relation::value_ops() const noexcept
{
    return m_ops;
}

#ifdef _MSC_VER
    #pragma warning(pop)
#else
    #pragma GCC diagnostic pop
#endif


relation::relation(
        relation_builder_resources&& res
        // FIXME: keys
) : m_ty( res.m_col_tys )
{
    const size_t n = res.m_col_tys.size();
    if ( n == res.m_ops.size() ) {
        m_ops.resize(n);
    } else {
        throw std::invalid_argument(
            "size of ops doesn't match number of columns");
    }
    if ( n == res.m_resources.size() ) {
        m_resources.resize(n);
    } else {
        throw std::invalid_argument(
            "size of resources doesn't match number of columns");
    }
    if ( n == res.m_cols.size() ) {
        m_cols.resize(n);
    } else {
        throw std::invalid_argument(
            "size of cols doesn't match number of columns");
    }

    // re-arrange columns & take ownership
    for (size_t i = 0; i < n; ++i)
    {
        auto it = std::find(
            m_ty.m_tys.cbegin(), m_ty.m_tys.cend(), res.m_col_tys[i] );
        if( it != m_ty.m_tys.end() )
        {
            const size_t j = size_t(it - m_ty.m_tys.cbegin());
            std::swap( m_ops[j],        res.m_ops[i]);
            std::swap( m_resources[j],  res.m_resources[i]);
            std::swap( m_cols[j],       res.m_cols[i]);
        }
    }
}
 
 std::ostream& relation::dump( std::ostream& os ) const
{
    // dump type

    os << "relation ";
    col_tys_to_stream( os, m_ty.m_tys );
    os << "\n\n";

    return cols_to_stream( os, m_ty.m_tys, m_ops, m_cols );
}


RA_CPP_LIBRARY_EXPORT struct ITable;

RA_CPP_LIBRARY_EXPORT struct table_view;


// ITable interface

const col_tys_t& table_view::type() const noexcept
{
    return m_col_tys;
}

size_t table_view::size() const noexcept
{
    return m_col_tys.size();
}

const value_t* table_view::at( size_t row, size_t col ) const
{
    // FIXME: bounds check
    return m_rel->at( this->m_row_map[ row ], this->m_col_map[ col ] );
}

const std::vector<IValue*>& table_view::value_ops() const noexcept
{
    return m_ops;
}

row_slice_t table_view::rowSlice( size_t start, size_t end ) const
{
    (void)start; (void)end;   // FIXME: for cppcheck, remove
    throw not_implemented();
}

// NOLINTBEGIN(bugprone-easily-swappable-parameters)
col_slice_t table_view::colSlice( size_t col, size_t start, size_t end ) const
{
    (void)col; (void)start; (void)end;  // FIXME: for cppcheck, remove
    throw not_implemented();
}
// NOLINTEND(bugprone-easily-swappable-parameters)


// FIXME: use slices when available
// FIXME: merge with cols_to_stream
std::ostream& relation_to_stream(
     std::ostream&           os
    ,const IRelationBase*   rel
)
{
    const col_tys_t& col_tys    = rel->type();
    const size_t n_cols         = col_tys.size();
    const auto& ops             = rel->value_ops();

    if (n_cols > 0) {
        const size_t n_rows = rel->size();

        // Work out column widths
        std::vector<size_t> col_sizes( n_cols );
        for ( size_t c = 0; c < n_cols; ++c )
        {
            col_sizes[ c ] = col_tys[ c ].first.size();
        }

        // Wiork out colum width and output headers
        std::ostringstream ss;
        size_t total_width = 0;
        for ( size_t c = 0; c < n_cols; ++c )
        {
            size_t m = col_sizes[ c ];
            for( size_t r = 0; r < n_rows; ++r )
            {
                ss.str("");
                ops[ c ]->to_stream( rel->at( r, c ), ss );
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
                ops[ c ]->to_stream( rel->at( r, c ), ss );
                ss.width(0);
                os << ss.str();
            }
            os << "\n";
        }
    }
    return os;
}

// NOLINTEND(readability-identifier-length)

} // namespace rac
