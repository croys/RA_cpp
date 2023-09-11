#include <RA_cpp/relation.h>

namespace rac
{

RA_CPP_LIBRARY_EXPORT struct IStorage;

RA_CPP_LIBRARY_EXPORT struct IValue;

// NOLINTBEGIN(readability-identifier-length)

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



// NOLINTEND(readability-identifier-length)

} // namespace rac
