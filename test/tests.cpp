#include <stdexcept>
#include <sstream>
#include <vector>
#include <array>
#include <iostream>
#include <memory_resource>

// #define CATCH_CONFIG_MAIN
// #include "catch.hpp"

#include <catch2/catch_test_macros.hpp>

#include <RA_cpp/sample_library.hpp>
#include <RA_cpp/relation.h>

using namespace rac;


// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
// NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
// NOLINTBEGIN(readability-function-cognitive-complexity)

TEST_CASE( "base basics" , "[base]" ) {
    CHECK_THROWS_AS( throw_with< std::runtime_error >( std::ostringstream() << "Test" ), std::runtime_error );

    CHECK_THROWS_AS( throw_lambda< std::invalid_argument >(
        [=]( std::ostringstream& ss ) { ss
            << "Lambda throw";
        } ),
        std::invalid_argument
    );

#if 0
    // FIXME: Not working with this version of Catch2...
    CHECK_THROWS_WITH( throw_with< std::runtime_error >( std::ostringstream() << "Test" ), "Test" );

    CHECK_THROWS_WITH( throw_lambda< std::invalid_argument >(
        [=]( std::ostringstream& ss ) { ss
            << "Lambda throw";
        } ),
        "Lambda throw"
    );
#endif
}

TEST_CASE( "type_t basics", "[type_t]" ) {
    const std::vector<type_t> tys( {
         { Void }, { Bool }, { Int }, { Float }, { Double }, { String },
         { Date }, { Time }, { Object }
        } );
    const std::vector<std::string_view> expected(
        {
             "Void", "Bool", "Int", "Float", "Double", "String"
            ,"Date", "Time", "Object"
        } );
    std::vector<std::string_view> res;
    res.reserve(tys.size());
    for( const auto & ty : tys ) {
        res.push_back( ty_to_string(ty) );
    }
    REQUIRE( res == expected );
}

TEST_CASE( "col_tys_t basics", "[col_tys_t]") {
    const col_tys_t col_tys( {
         { "A", { Void } }, { "B", { Bool } }, { "C", { Int } }
        ,{ "D", { Float } }, { "E", { Double } }, { "F", { String } }
        ,{ "G", { Date } }, { "H", { Time } }, { "I", { Object } }
        } );

    const std::string_view expected =
        "{ A : Void, B : Bool, C : Int, D : Float, E : Double, F : String, "
        "G : Date, H : Time, I : Object }";

    REQUIRE( col_tys_to_string( col_tys ) == expected );

}

TEST_CASE( "column_storage basics", "[column_storage] [untyped_column_storage]") {
    std::array< std::uint8_t, 32768 > buffer{};
    std::pmr::monotonic_buffer_resource rsrc( buffer.data(), buffer.size() );

    auto cs_int_ = std::make_shared< column_storage< int > >( &rsrc );
    auto& cs_int = *cs_int_;
    untyped_column_storage< int > ucs_int( cs_int_ );
    auto* is = static_cast<IStorage*>( &ucs_int );
    const auto* cis = static_cast<const IStorage*>( &ucs_int );

    REQUIRE( cs_int.empty() );

    REQUIRE( cs_int.cend() - cs_int.cbegin() == 0 );
    REQUIRE( cis->cend() - cis->cbegin()  == 0 );

    REQUIRE( cs_int.empty() );
    REQUIRE( ucs_int.empty() );
    REQUIRE( is->empty() );
    REQUIRE( cis->empty() );

    const size_t n = 100;
    is->resize( n );
    REQUIRE( cs_int.size() == n );
    REQUIRE( ucs_int.size() == n );
    REQUIRE( is->size() == n );
    REQUIRE( cis->size() == n );
    REQUIRE( size_t(cs_int.cend() - cs_int.cbegin()) == cs_int.size() );
    REQUIRE( size_t(cis->cend() - cis->cbegin()) == cis->size() );


    column_storage<int>::iterator it;
    {
      int  i = 0;
      for ( it = cs_int.begin(); it != cs_int.end(); ++it )
      {
          *it = i++;
      }
    }

    auto as_cint = []( const value_t* v )
    {
        return reinterpret_cast<const int *>(v);
    };

    REQUIRE( cs_int.at(0) == 0 );
    REQUIRE( cs_int.at(99) == 99 );
    REQUIRE( *as_cint( cis->at(0) ) == 0 );
    REQUIRE( *as_cint( cis->at(99) ) == 99 );

    auto cs_int2_ = std::make_shared< column_storage< int > >( &rsrc );
    auto& cs_int2 = *cs_int2_;
    untyped_column_storage< int > ucs_int2( cs_int2_ );
    auto* is2 = static_cast<IStorage*>( &ucs_int2 );
    const auto* cis2 = static_cast<const IStorage*>( &ucs_int2 );

    is2->resize( n );

    REQUIRE( cs_int2.size() == n );
    REQUIRE( ucs_int2.size() == n );
    REQUIRE( is2->size() == n );
    REQUIRE( cis2->size() == n );

    is2->move( is->begin(), is->end(), is2->begin() );
    REQUIRE( *as_cint( cis2->at( 0 ) ) == 0 );
    REQUIRE( *as_cint( cis2->at( 99 ) ) == 99 );

    for ( it = cs_int.begin(); it != cs_int.end(); ++it ) {
        *it = 0;
    }

    REQUIRE( *as_cint( cis->at( 0 ) ) == 0 );
    REQUIRE( *as_cint( cis->at( 99 ) ) == 0 );

    is->copy( cis2->cbegin(), cis2->cend(), is->begin() );

    REQUIRE( *as_cint( cis->at( 0 ) ) == 0 );
    REQUIRE( *as_cint( cis->at( 99 ) ) == 99 );

    for (size_t i = 0; i<n; ++i) {
        cs_int2.at( i ) = int(n-i);
    }
    REQUIRE( cs_int2.at( 0 ) == n );
    REQUIRE( cs_int2.at( 99 ) == n - 99 );

    for (size_t i = 0; i<n; ++i) {
        cs_int2[i] = int(i);
    }

    REQUIRE( cs_int2[ 0 ] == 0 );
    REQUIRE( cs_int2.at( 0 ) == 0 );
    REQUIRE( cs_int2[ 99 ] == 99 );
    REQUIRE( cs_int2.at( 99 ) == 99 );

}


TEST_CASE( "relation_builder basics", "[relation_builder]") {
    std::array< std::uint8_t, 32768 > buffer{};
    std::pmr::monotonic_buffer_resource rsrc( buffer.data(), buffer.size() );

    //std::vector<std::string> col_names { "A", "B", "C" };
    std::vector col_names { "A", "B", "C" };
    relation_builder<int, float, double> builder( &rsrc, col_names.begin(), col_names.end() );

    col_tys_t expected { { "A", { Int } }, { "B" , { Float } }, { "C", { Double } } };
    REQUIRE( builder.type() == expected );

    const int a = 1;
    const float b = 3.14F;
    const double c = 2.718281828459045;

    builder.push_back( a, b, c );

    builder.dump( std::cout );

    REQUIRE( builder.size() == 1 );
    REQUIRE( builder.at( 0 ) == std::tuple<int, float, double>( a, b, c ) );

    builder.push_back( 2 * a, 2.0F * b, 2.0 * c );

    REQUIRE( builder.size() == 2 );
    REQUIRE( builder.at( 1 ) == std::tuple<int, float, double>( 2 * a, 2.0F * b, 2.0 * c ) );

    builder.push_back( 200, 4.5, 2.3 );

    builder.dump( std::cout );
}


TEST_CASE( "rel_ty_t basics", "[rel_ty_t]" ) {

    rel_ty_t rel_ty_empty   {};
    rel_ty_t rel_ty_a       { { { "A" }, { Int } } };
    rel_ty_t rel_ty_b       { { { "B" }, { Int } } };
    rel_ty_t rel_ty_a_      { { { "A" }, { Double } } };
    rel_ty_t rel_ty_ab      { { { "A" }, { Int } }, { { "B" }, { Int } } };
    rel_ty_t rel_ty_ba      { { { "B" }, { Int } }, { { "A" }, { Int } } };

    REQUIRE( rel_ty_empty.m_tys.empty() );
    REQUIRE( rel_ty_empty == rel_ty_empty );
    REQUIRE( rel_ty_empty == rel_ty_t { } );

    REQUIRE( rel_ty_a == rel_ty_a );
    REQUIRE( rel_ty_a == rel_ty_t { { "A", { Int } } } );
    REQUIRE( rel_ty_a != rel_ty_b );
    REQUIRE( rel_ty_a != rel_ty_a_ );
    REQUIRE( rel_ty_a  != rel_ty_empty );
    REQUIRE( rel_ty_ab  == rel_ty_ba );
    CHECK_THROWS( rel_ty_t { { "A", { Int } }, { "A", { Double } } } );

    REQUIRE( rel_ty_t::union_( rel_ty_empty, rel_ty_empty ) == rel_ty_empty );
    REQUIRE( rel_ty_t::union_( rel_ty_a, rel_ty_empty ) == rel_ty_a );
    REQUIRE( rel_ty_t::union_( rel_ty_empty, rel_ty_a ) == rel_ty_a );
    CHECK_THROWS( rel_ty_t::union_( rel_ty_a, rel_ty_a_ ) );
    REQUIRE( rel_ty_t::union_( rel_ty_a, rel_ty_a ) == rel_ty_a );
    REQUIRE( rel_ty_t::union_( rel_ty_b, rel_ty_b ) == rel_ty_b );
    REQUIRE( rel_ty_t::union_( rel_ty_a, rel_ty_b ) == rel_ty_ab );
    REQUIRE( rel_ty_t::union_( rel_ty_b, rel_ty_a ) == rel_ty_ab );
    REQUIRE( rel_ty_t::union_( rel_ty_ab, rel_ty_a ) == rel_ty_ab );
    REQUIRE( rel_ty_t::union_( rel_ty_a, rel_ty_ab ) == rel_ty_ab );
    REQUIRE( rel_ty_t::union_( rel_ty_ab, rel_ty_b ) == rel_ty_ab );
    REQUIRE( rel_ty_t::union_( rel_ty_b, rel_ty_ab ) == rel_ty_ab );

    REQUIRE( rel_ty_t::intersect( rel_ty_empty, rel_ty_empty ) == rel_ty_empty );
    REQUIRE( rel_ty_t::intersect( rel_ty_empty, rel_ty_a ) == rel_ty_empty );
    REQUIRE( rel_ty_t::intersect( rel_ty_a, rel_ty_empty ) == rel_ty_empty );
    REQUIRE( rel_ty_t::intersect( rel_ty_a, rel_ty_a ) == rel_ty_a );
    REQUIRE( rel_ty_t::intersect( rel_ty_b, rel_ty_b ) == rel_ty_b );
    REQUIRE( rel_ty_t::intersect( rel_ty_a, rel_ty_b ) == rel_ty_empty );
    REQUIRE( rel_ty_t::intersect( rel_ty_b, rel_ty_a ) == rel_ty_empty );
    CHECK_THROWS( rel_ty_t::intersect( rel_ty_a, rel_ty_a_ ) );
    REQUIRE( rel_ty_t::intersect( rel_ty_ab, rel_ty_ab ) == rel_ty_ab );
    REQUIRE( rel_ty_t::intersect( rel_ty_ba, rel_ty_ba ) == rel_ty_ab );
    REQUIRE( rel_ty_t::intersect( rel_ty_ab, rel_ty_ba ) == rel_ty_ab );
    REQUIRE( rel_ty_t::intersect( rel_ty_ba, rel_ty_ab ) == rel_ty_ab );
    REQUIRE( rel_ty_t::intersect( rel_ty_a, rel_ty_ab ) == rel_ty_a );
    REQUIRE( rel_ty_t::intersect( rel_ty_ab, rel_ty_a ) == rel_ty_a );
    REQUIRE( rel_ty_t::intersect( rel_ty_b, rel_ty_ab ) == rel_ty_b );
    REQUIRE( rel_ty_t::intersect( rel_ty_ab, rel_ty_b ) == rel_ty_b );
}

// NOLINTEND(readability-function-cognitive-complexity)
// NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)
// NOLINTEND(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
