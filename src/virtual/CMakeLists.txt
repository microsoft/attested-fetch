find_package(OpenSSL REQUIRED)
find_package(CURL REQUIRED)

add_executable(afetch.virtual virtual.cpp)
target_include_directories(afetch.virtual PRIVATE ../common)
target_include_directories(afetch.virtual PRIVATE ${CMAKE_SOURCE_DIR}/../3rdparty/nlohmann-json/single_include)
target_link_libraries(afetch.virtual PRIVATE OpenSSL::Crypto CURL::libcurl)

set_target_properties(afetch.virtual PROPERTIES OUTPUT_NAME "afetch")
install(TARGETS afetch.virtual RUNTIME DESTINATION .)
