find_package(OpenSSL REQUIRED)
find_package(SQLite3 REQUIRED)

set(CMAKE_FOLDER ${CMAKE_FOLDER}/Database)
add_library(database STATIC database.cpp database.h databaseAsset.cpp databaseAsset.h)
target_link_libraries(database PRIVATE SQLite::SQLite3 OpenSSL::SSL)