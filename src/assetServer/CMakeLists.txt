#src/assetServer

#add_subdirectory(bin)
add_subdirectory(database)
add_subdirectory(gltf)

add_executable(AssetServer main.cpp assetProcessing/asssetBuilder.cpp assetProcessing/asssetBuilder.h assetServer.cpp assetServer.h)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(AssetServer PRIVATE runtime config networking ecs utility fileManager database gltf)
target_compile_definitions(AssetServer PRIVATE ${COMMON_DEFS} ${ASSET_SERVER_DEFS} ASIO_STANDALONE)

set(INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/assetServer)

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
install(DIRECTORY assets
        DESTINATION ${INSTALL_DIR}
        COMPONENT AssetServer)

add_custom_command(TARGET AssetServer POST_BUILD
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND cmake --install . --component AssetServer --prefix ${CMAKE_BINARY_DIR}
        COMMENT "Installing AssetServer...")