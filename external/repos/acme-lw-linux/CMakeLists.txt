file (GLOB_RECURSE source "*.cpp" "*.h" "../internal/*.h" "../internal/*.cpp")

add_library(acme_lw ${source})

find_package (CURL REQUIRED)
find_package (OpenSSL 1.1 REQUIRED)

install (TARGETS acme_lw DESTINATION lib)
install (FILES acme-exception.h acme-lw.h DESTINATION include)

include(CheckFunctionExists)
set(CMAKE_REQUIRED_LIBRARIES OpenSSL::Crypto)
check_function_exists(ASN1_TIME_to_tm HAS_OPENSSL_TO_TM)
check_function_exists(timegm HAS_TIMEGM)
if (OPENSSL_TO_TM AND HAS_TIMEGM)
add_definitions(-DOPENSSL_TO_TM)
endif ()

target_include_directories(acme_lw PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(acme_lw PRIVATE -lcurl -lssl)
