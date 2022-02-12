add_library(database STATIC Database.cpp Database.h databaseAsset.cpp databaseAsset.h assetType.h assetType.cpp)
target_link_libraries(database PRIVATE sqlite)