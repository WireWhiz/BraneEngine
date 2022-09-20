#src/assetServer

set(CMAKE_FOLDER ${CMAKE_FOLDER}/AssetServer)
add_subdirectory(database)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
add_executable(AssetServer main.cpp assetServer.cpp assetServer.h)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(AssetServer PRIVATE runtime config networking ecs utility fileManager database assets_server)
target_compile_definitions(AssetServer PRIVATE ${COMMON_DEFS} ${ASSET_SERVER_DEFS} ASIO_STANDALONE)

set(INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/assetServer)
configure_file(config.json ${CMAKE_CURRENT_BINARY_DIR}/config.json COPYONLY)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/database/data.db DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/database)

install(TARGETS AssetServer
        RUNTIME
        DESTINATION ${INSTALL_DIR}
        COMPONENT AssetServer)
install(FILES config.json
        DESTINATION ${INSTALL_DIR}
        COMPONENT AssetServer)
install(FILES database/data.db
        DESTINATION ${INSTALL_DIR}/database
        COMPONENT AssetServer)