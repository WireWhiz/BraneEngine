add_library(database STATIC Database.cpp Database.h)
target_link_libraries(database PRIVATE sqlite)