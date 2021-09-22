#lib/win64/debug/mysql
add_library(mysql INTERFACE)

target_link_libraries(mysql INTERFACE 
${CMAKE_CURRENT_SOURCE_DIR}/mysqlcppconn8-static.lib
ssl
)
