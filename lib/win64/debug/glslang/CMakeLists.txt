#lib/win64/debug/glslang

add_library(glslang INTERFACE)
set(DIR ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(glslang INTERFACE
	${DIR}/MachineIndependentd.lib
	${DIR}/GenericCodeGend.lib
	${DIR}/glslangd.lib
	${DIR}/glslang-default-resource-limitsd.lib
	${DIR}/HLSLd.lib
	${DIR}/SPIRVd.lib
	${DIR}/OGLCompilerd.lib
	${DIR}/OSDependentd.lib
	${DIR}/SPIRV-Tools-optd.lib 
	${DIR}/SPIRV-Toolsd.lib
	${DIR}/SPIRV-Tools-linkd.lib
	${DIR}/SPIRV-Tools-reduced.lib
	${DIR}/SPVRemapperd.lib
	 )