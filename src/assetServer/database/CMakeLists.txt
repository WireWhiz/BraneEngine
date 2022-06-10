add_library(database STATIC Database.cpp Database.h databaseAsset.cpp databaseAsset.h)
target_link_libraries(database PRIVATE sqlite ssl include_dir)