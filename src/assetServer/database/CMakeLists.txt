add_library(database STATIC database.cpp database.h databaseAsset.cpp databaseAsset.h)
target_link_libraries(database PRIVATE sqlite ssl include_dir)