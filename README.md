# RA_cpp

[![ci](https://github.com/croys/RA_cpp/actions/workflows/ci.yml/badge.svg)](https://github.com/croys/RA_cpp/actions/workflows/ci.yml)
[![codecov](https://codecov.io/gh/croys/RA_cpp/branch/main/graph/badge.svg)](https://codecov.io/gh/croys/RA_cpp)
[![CodeQL](https://github.com/croys/RA_cpp/actions/workflows/codeql-analysis.yml/badge.svg)](https://github.com/croys/RA_cpp/actions/workflows/codeql-analysis.yml)

## About RA_cpp
Relational Algebra implementation in C++20 as an exercise.

The overall design follows the relational algebra `Tutorial D` as described in Date[1]

The design is centered around providing dynamically typed core, with statically typed views onto this storage.

The basic types of Tutorial D are extended with parametric types - maybe/optional, either, vectors, maps, objects and ADTS (algebraic data types). Serialization will be provided via `protobuf`` or similar.

The goal is to provide an implementation that can be used by other languages, such as Python, but can be used in idiomatic C++ without dynamic types becoming intrusive. 

## Roadmap

* Core types
    * basic types - done
    * parameterized types (std::optional, either, vector, map, functions)
    * ADTs
* Core values
    * basic types - done
    * values of parameterized types
    * may want to support `Shared` to wrap values in shared_ptr<> or similar.
    * may use types to express storage/lifecycle choices (e.g. PooledString, GlobalStorage, ThreadStorage, etc..)
    * dynamicaly typed function application
* Column storage
    * pmr support by default
    * basic types - done
    * objects
    * ADTs
* Main operations
    * construction - done
    * dee, dum, product, filter, project, join
    * copy
* Utility
    * table view
    * serialization

[1] "An Introduction to Database Systems", eighth edition by C.J. Date. Addison-Wesley.

## More Details

 * [Dependency Setup](README_dependencies.md)
 * [Building Details](README_building.md)
 * [Troubleshooting](README_troubleshooting.md)
 * [Docker](README_docker.md)
