
set(DOWNLOAD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/repos)
set(LIB_DIR ${CMAKE_SOURCE_DIR}/external/libs)
set(BUILD_DIR ${CMAKE_BINARY_DIR}/external/)

#ACME client for getting ssl cert
include(ExternalProject)
ExternalProject_Add(acme-lw-proj
        GIT_REPOSITORY    https://github.com/jmccl/acme-lw.git
        GIT_TAG           d64b7827a50cb2a82369ea082392bdc97736d478
        SOURCE_DIR        "${DOWNLOAD_DIR}/acme-lw"
        BINARY_DIR        "${BUILD_DIR}/acme-lw"
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${LIB_DIR} -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        )

add_library(acmelw INTERFACE)
target_link_libraries(acmelw INTERFACE ${LIB_DIR}/lib/libacme_lw.a -lcurl -lcrypto)
target_include_directories(acmelw INTERFACE libs/include)