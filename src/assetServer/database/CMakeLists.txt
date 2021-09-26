# src/assetServer/database

add_library(database STATIC 
database.cpp
)

target_link_libraries(database PUBLIC mysql config ssl)
target_compile_definitions(database PRIVATE ${COMMON_DEFS} ${ASSET_SERVER_DEFS})