#include <RA_cpp/types.h>


namespace rac
{

// NOLINTBEGIN(readability-identifier-length)


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


std::ostream& col_tys_to_stream( std::ostream& os, const col_tys_t& col_tys )
{
    os << "{ ";
    for (auto it = col_tys.cbegin(); it != col_tys.cend(); ++it) {
        if (it != col_tys.cbegin()) {
            os << ", ";
        }
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

void rel_ty_t::construct()
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


rel_ty_t rel_ty_t::union_( const rel_ty_t& a, const rel_ty_t& b )
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
            // FIXME: WTH cppcheck?
            // cppcheck-suppress unassignedVariable
            const auto& [ a_name, a_ty ] = *a_it;
            // cppcheck-suppress unassignedVariable
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


rel_ty_t rel_ty_t::intersect( const rel_ty_t& a, const rel_ty_t& b )
{
    col_tys_t res;
    std::map< cstring_t, type_t > b_names( b.m_tys.cbegin(), b.m_tys.cend() );

    for ( const auto& a_col : a.m_tys ) {
        auto b_it = b_names.find( std::get<0>( a_col ) );
        if ( b_it != b_names.cend() )
        {
            if ( std::get<1>( a_col ) != std::get<1>( *b_it ) ) {
                throw_with< std::invalid_argument >(
                    std::ostringstream()
                    << "Types for column '"
                    << std::get<0>(a_col)
                    << "' do not match: "
                    << ty_to_string( std::get<1>( a_col ) )
                    << " and "
                    << ty_to_string( std::get<1>(*b_it) )
                );
            }
            res.push_back( a_col );
        }
    }
    return rel_ty_t( std::move( res ) );
}


// NOLINTEND(readability-identifier-length)

} // namespace rac
