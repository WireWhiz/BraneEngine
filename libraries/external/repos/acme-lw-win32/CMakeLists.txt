file (GLOB_RECURSE source "*.cpp" "*.h")

add_library(acme_lw STATIC ${source})

install (TARGETS acme_lw DESTINATION lib)
install (FILES acme-exception.h acme-lw.h DESTINATION include)

target_include_directories(acme_lw PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(acme_lw PRIVATE ssl curl)

