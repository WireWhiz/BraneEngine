
# sqlite3 library
add_library(sqlite3 STATIC
        include/sqlite3/sqlite3.h
        include/sqlite3/sqlite3.c
        include/sqlite3/sqlite3ext.h)
target_include_directories(sqlite3 PUBLIC
        include)