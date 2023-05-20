set(CMAKE_FOLDER ${CMAKE_FOLDER}/Database)
# OpenSSL
find_package(OpenSSL REQUIRED)

add_library(database STATIC
        database.cpp
        database.h
        databaseAsset.cpp
        databaseAsset.h)
target_link_libraries(database PRIVATE
        sqlite3 OpenSSL::SSL)