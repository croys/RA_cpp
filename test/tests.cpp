#include <stdexcept>
#include <sstream>
#include <vector>
#include <array>
#include <iostream>
#include <memory>
#include <memory_resource>

#include <catch2/catch_test_macros.hpp>

#include <RA_cpp/sample_library.hpp>
#include <RA_cpp/storage.h>
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
    {
        std::vector<std::string_view> res;
        res.reserve(tys.size());
        std::transform( tys.cbegin(), tys.cend(), std::back_inserter(res),
            ty_to_string );
        REQUIRE( res == expected );
    }

    {
        std::vector<std::string> res;
        res.reserve(tys.size());
        for( auto ty : tys ) {
            std::ostringstream ss;
            ty_to_stream(ss, ty);
            res.emplace_back( ss.str() );
        }
        std::vector<std::string_view> res_;
        res_.reserve(res.size());
        std::transform( res.cbegin(), res.cend(),
            std::back_inserter(res_),
            [] (auto &s) { return std::string_view(s); }
        );
        REQUIRE( res_ == expected );
    }}

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
    is->reserve( n );
    REQUIRE( cs_int.empty() );
    is->resize( n );
    REQUIRE( cs_int.size() == n );
    REQUIRE( ucs_int.size() == n );
    REQUIRE( is->size() == n );
    REQUIRE( cis->size() == n );
    REQUIRE( size_t(cs_int.cend() - cs_int.cbegin()) == cs_int.size() );
    REQUIRE( size_t(cis->cend() - cis->cbegin()) == cis->size() );


    // const iterator
    {
        int i = 0;
        for( auto it = ucs_int.cbegin(); it != ucs_int.cend(); ++it )
        {
            ++i;
        }
        REQUIRE( i == n );
    }
    {
        int i = 0;
        for( auto it = ucs_int.cend(); it != ucs_int.cbegin(); --it )
        {
            ++i;
        }
        REQUIRE( i == n );
    }
    REQUIRE( ucs_int.cbegin() == ucs_int.cbegin() );
    REQUIRE( ucs_int.cend() == ucs_int.cend() );
    REQUIRE( ucs_int.cend() != ucs_int.cbegin() );
    REQUIRE( ucs_int.cend() > ucs_int.cbegin() );
    REQUIRE( ucs_int.cbegin() < ucs_int.cend() );
    REQUIRE( ucs_int.cbegin() + 0 == ucs_int.cbegin() );
    REQUIRE( 0 + ucs_int.cbegin()== ucs_int.cbegin() );
    REQUIRE( ucs_int.cbegin() + 1 > ucs_int.cbegin() );
    REQUIRE( 1 + ucs_int.cbegin() > ucs_int.cbegin() );
    REQUIRE( ucs_int.cbegin() < ucs_int.cbegin() + 1 );
    REQUIRE( ucs_int.cbegin() < 1 + ucs_int.cbegin() );
    REQUIRE( ucs_int.cbegin() <= ucs_int.cbegin() );
    REQUIRE( ucs_int.cbegin() + n == ucs_int.cend() );
    REQUIRE( n + ucs_int.cbegin() == ucs_int.cend() );
    REQUIRE( ucs_int.cend() - n == ucs_int.cbegin() );
    REQUIRE( ucs_int.cend() - ucs_int.cbegin() == n );

    {
        auto it = ucs_int.cbegin();
        it += 10;
        REQUIRE( it == ucs_int.cbegin() + 10 );
    }

    {
        auto it = ucs_int.cend();
        it -= 10;
        REQUIRE( it == ucs_int.cend() - 10 );
    }

    // iterator
    {
        int i = 0;
        for( auto it = ucs_int.begin(); it != ucs_int.end(); ++it )
        {
            ++i;
        }
        REQUIRE( i == n );
    }
    {
        int i = 0;
        for( auto it = ucs_int.end(); it != ucs_int.begin(); --it )
        {
            ++i;
        }
        REQUIRE( i == n );
    }
    REQUIRE( ucs_int.begin() == ucs_int.begin() );
    REQUIRE( ucs_int.end() == ucs_int.end() );
    REQUIRE( ucs_int.end() != ucs_int.begin() );
    REQUIRE( ucs_int.end() > ucs_int.begin() );
    REQUIRE( ucs_int.begin() < ucs_int.end() );
    REQUIRE( ucs_int.begin() + 0 == ucs_int.begin() );
    REQUIRE( 0 + ucs_int.begin()== ucs_int.begin() );
    REQUIRE( ucs_int.begin() + n == ucs_int.end() );
    REQUIRE( ucs_int.end() - n == ucs_int.begin() );
    REQUIRE( ucs_int.begin() + 0 == ucs_int.begin() );
    REQUIRE( 0 + ucs_int.begin()== ucs_int.begin() );
    REQUIRE( ucs_int.begin() + 1 > ucs_int.begin() );
    REQUIRE( 1 + ucs_int.begin() > ucs_int.begin() );
    REQUIRE( ucs_int.begin() < ucs_int.begin() + 1 );
    REQUIRE( ucs_int.begin() < 1 + ucs_int.begin() );
    REQUIRE( ucs_int.begin() <= ucs_int.begin() );
    REQUIRE( ucs_int.begin() + n == ucs_int.end() );
    REQUIRE( n + ucs_int.begin() == ucs_int.end() );
    REQUIRE( ucs_int.end() - n == ucs_int.begin() );
    REQUIRE( ucs_int.end() - ucs_int.begin() == n );

    {
        auto it = ucs_int.begin();
        it += 10;
        REQUIRE( it == ucs_int.begin() + 10 );
    }

    {
        auto it = ucs_int.end();
        it -= 10;
        REQUIRE( it == ucs_int.end() - 10 );
    }

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

    col_tys_t expected {
        { "A", { Int } }, { "B" , { Float } }, { "C", { Double } }
    };

    const std::vector col_names { "A", "B", "C" };

    // Construction from container
    relation_builder<int, float, double> builder( &rsrc, col_names );

    REQUIRE( builder.type() == expected );
    REQUIRE( builder.size() == 0 );

    const int a = 1;
    const float b = 3.14F;
    const double c = 2.718281828459045;

    builder.push_back( a, b, c );
    builder.dump( std::cout );
    REQUIRE( builder.size() == 1 );
    REQUIRE( builder.at( 0 ) == std::tuple { a, b, c } );

    builder.push_back( 2 * a, 2.0F * b, 2.0 * c );
    builder.dump( std::cout );
    REQUIRE( builder.size() == 2 );
    REQUIRE( builder.at( 1 ) == std::tuple { 2 * a, 2.0F * b, 2.0 * c } );

    builder.push_back( 200, 4.5, 2.3 );
    builder.dump( std::cout );
    REQUIRE( builder.size() == 3 );
    REQUIRE( builder.at( 2 ) == std::tuple { 200, 4.5, 2.3} );

    // Construction from iterators
    const relation_builder<int, float, double> builder2(
        &rsrc, col_names.begin(), col_names.end()
    );

    REQUIRE( builder2.type() == expected );
    REQUIRE( builder2.size() == 0 );
}

TEST_CASE( "relation_builder basics2", "[relation_builder]") {
    std::array< std::uint8_t, 32768 > buffer{};
    std::pmr::monotonic_buffer_resource rsrc( buffer.data(), buffer.size() );

    // Convenience helper structs, want something like:
    //
    // relation_builder builder
    //  { { "A", col<int> }, { "B", col<float> }, { "C", col<double> } }
    //

    /*
    Ideal would be:

    rel_builder rb2(
        { "A", tyInt() },
        { "B", tyFloat() },
        { "C", tyDouble() }
    );

    or:

    rel_builder rb2(
        "A", tyInt(),
        "B", tyFloat(),
        "C", tyDouble()
    );

    Following would be OK:

    rel_builder2 rb2(
        col<int>    ("A"),
        col<float>  ("B"),
        col<double> ("C")
    );

    - arguably the above is more idiomatic C++

    rel_builder rb2(
        col( "A", tyInt() ),
        col( "B", tyFloat() ),
        col( "C", tyDouble() )
    );

    maybe even:

    rel_builder rb2(
        {   "A",        "B",        "C" }
        ,   tyInt(),    tyFloat(),  tyDouble()
    )

    */

    col_tys_t expected {
        { "A", { Int } }, { "B" , { Float } }, { "C", { Double } }
    };

    //auto builder = make_relation_builder(
    //relation_builder builder = make_relation_builder(
    relation_builder builder(
         &rsrc
        ,col_desc( "A", tyInt() )
        ,col_desc( "B", tyFloat() )
        ,col_desc( "C", tyDouble() )
    );

    builder.dump(std::cout);

    REQUIRE( builder.type() == expected );
    REQUIRE( builder.size() == 0 );

    const int a = 1;
    const float b = 3.14F;
    const double c = 2.718281828459045;

    builder.push_back( a, b, c );
    builder.dump( std::cout );
    REQUIRE( builder.size() == 1 );
    REQUIRE( builder.at( 0 ) == std::tuple { a, b, c } );

    builder.push_back( 2 * a, 2.0F * b, 2.0 * c );
    builder.dump( std::cout );
    REQUIRE( builder.size() == 2 );
    REQUIRE( builder.at( 1 ) == std::tuple { 2 * a, 2.0F * b, 2.0 * c } );

    builder.push_back( 200, 4.5, 2.3 );
    builder.dump( std::cout );
    REQUIRE( builder.size() == 3 );
    REQUIRE( builder.at( 2 ) == std::tuple { 200, 4.5, 2.3} );

    const relation_builder builder2(
        &rsrc,
        col_desc<int>(      "A"),
        col_desc<float>(    "B"),
        col_desc<double>(   "C")
    );
    REQUIRE( builder2.type() == expected );
    REQUIRE( builder2.size() == 0 );

    CHECK_THROWS( relation_builder<int>( &rsrc, std::vector { "A", "B" } ) );

    const relation_builder builder3( &rsrc );
    REQUIRE( builder3.type().empty() );
    REQUIRE( builder3.size() == 0 );
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


TEST_CASE( "relation basics", "[relation_builder], [relation]") {
    std::array< std::uint8_t, 32768 > buffer{};
    std::pmr::monotonic_buffer_resource rsrc( buffer.data(), buffer.size() );

    col_tys_t expected {
        { "Z", { Int } }, { "Y" , { Float } }, { "X", { Double } }
    };

    relation_builder builder(
        &rsrc,
        col_desc<int>(      "Z"),
        col_desc<float>(    "Y"),
        col_desc<double>(   "X")
    );

    REQUIRE( builder.type() == expected );
    REQUIRE( builder.size() == 0 );

    const int a = 1;
    const float b = 3.14F;
    const double c = 2.718281828459045;

    builder.push_back( a, b, c );
    builder.push_back( 2 * a, 2.0F * b, 2.0 * c );
    builder.push_back( 200, 4.5, 2.3 );

    builder.dump(std::cout);

    const relation rel( builder.release() );

    REQUIRE( rel.size() == 3 );

    col_tys_t expected_2 {
         { "X",     { Double } }
        ,{ "Y" ,    { Float } }
        ,{ "Z",     { Int } }
    };
    REQUIRE( rel.type() == expected_2 );
    rel.dump( std::cout );

    REQUIRE( builder.size() == 0 );
}


TEST_CASE( "table_view basics", "[relation_builder], [relation], [table_view]") {
    std::array< std::uint8_t, 32768 > buffer{};
    std::pmr::monotonic_buffer_resource rsrc( buffer.data(), buffer.size() );

    const col_tys_t expected {
        { "Z", { Int } }, { "Y" , { Float } }, { "X", { Double } }
    };

    relation_builder builder(
        &rsrc,
        col_desc<int>(      "Z"),
        col_desc<float>(    "Y"),
        col_desc<double>(   "X")
    );

    REQUIRE( builder.type() == expected );
    REQUIRE( builder.size() == 0 );

    const int a = 1;
    const float b = 3.14F;
    const double c = 2.718281828459045;

    builder.push_back( 2 * a, 2.0F * b, 2.0 * c );
    builder.push_back( 200, 4.5, 2.3 );
    builder.push_back( a, b, c );

    builder.dump( std::cout );

    auto rel = std::make_shared<relation>( builder.release() );
    auto irel = static_pointer_cast<IRelation>( rel );

    relation_to_stream( std::cout, irel.get() );

    REQUIRE( *reinterpret_cast<const double*>( irel->at(0,0) ) == 2.0 * c );
    REQUIRE( *reinterpret_cast<const float*>( irel->at(0,1) ) == 2.0F * b );
    REQUIRE( *reinterpret_cast<const int*>( irel->at(0,2) ) == 2 * a );

    const table_view tbl( irel, std::vector { "X", "Y", "Z" } );

    relation_to_stream( std::cout, &tbl );

    // FIXME: need value_ops<>::get
    REQUIRE( *reinterpret_cast<const double*>(  tbl.at(0,0) ) == 2.3 );
    REQUIRE( *reinterpret_cast<const float*>(   tbl.at(0,1) ) == 4.5F );
    REQUIRE( *reinterpret_cast<const int*>(     tbl.at(0,2) ) == 200 );

    REQUIRE( *reinterpret_cast<const double*>(  tbl.at(1,0) ) == c );
    REQUIRE( *reinterpret_cast<const float*>(   tbl.at(1,1) ) == b );
    REQUIRE( *reinterpret_cast<const int*>(     tbl.at(1,2) ) == a );

    REQUIRE( *reinterpret_cast<const double*>(  tbl.at(2,0) ) == 2.0 * c );
    REQUIRE( *reinterpret_cast<const float*>(   tbl.at(2,1) ) == 2.0F * b );
    REQUIRE( *reinterpret_cast<const int*>(     tbl.at(2,2) ) == 2 * a );

}



// NOLINTEND(readability-function-cognitive-complexity)
// NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)
// NOLINTEND(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
