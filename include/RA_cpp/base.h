#pragma once

#include <stdexcept>
#include <sstream>
#include <iostream>

namespace rac
{

template< typename Container, typename... Values>
auto make_container( auto* resource, Values&&... values) {
    Container result{resource};
    result.reserve( sizeof...(values) );
    (result.emplace_back(std::forward<Values>(values)), ...);
    return result;
}

struct not_implemented : public std::logic_error
{
    not_implemented() : std::logic_error( "Not yet implemented" ) { }
};

template<typename T, typename U>
void throw_with( U s )
{
    // FIXME: do this statically
    std::ostringstream* ss = dynamic_cast<std::ostringstream*>(&s);
    if (ss) {
        throw T( ss->str() );
    } else {
        throw std::logic_error( "Guru meditation: ostringstream expected!" );
    }
}

template< typename T >
void throw_lambda( auto l )
{
    std::ostringstream ss;
    l(ss);
    throw T( ss.str() );
}

}