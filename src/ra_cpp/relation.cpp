#include <RA_cpp/relation.h>

namespace rac
{

// NOLINTBEGIN(readability-identifier-length)

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
