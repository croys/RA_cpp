include(GenerateExportHeader)



add_library(ra_cpp_library types.cpp storage.cpp relation.cpp)

add_library(RA_cpp::ra_cpp_library ALIAS ra_cpp_library)

target_link_libraries(ra_cpp_library PRIVATE RA_cpp_options RA_cpp_warnings)

target_include_directories(ra_cpp_library
  ${WARNING_GUARD} PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
)

target_compile_features(ra_cpp_library PUBLIC cxx_std_20)

set_target_properties(
  ra_cpp_library
  PROPERTIES VERSION ${PROJECT_VERSION}
             CXX_VISIBILITY_PRESET hidden
             VISIBILITY_INLINES_HIDDEN YES)

generate_export_header(ra_cpp_library EXPORT_FILE_NAME ${PROJECT_BINARY_DIR}/include/RA_cpp/ra_cpp_library_export.hpp)

if(NOT BUILD_SHARED_LIBS)
  target_compile_definitions(ra_cpp_library PUBLIC SAMPLE_LIBRARY_STATIC_DEFINE)
endif()
