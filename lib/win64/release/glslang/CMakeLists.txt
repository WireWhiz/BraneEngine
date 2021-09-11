#lib/win64/release/glslang

add_library(glslang INTERFACE)
set(DIR ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(glslang INTERFACE
	${DIR}/GenericCodeGen.lib
	${DIR}/glslang.lib
	${DIR}/glslang-default-resource-limits.lib
	${DIR}/HLSL.lib
	${DIR}/MachineIndependent.lib
	${DIR}/OGLCompiler.lib
	${DIR}/OSDependent.lib
	${DIR}/SPIRV.lib
	${DIR}/SPIRV-Tools.lib
	${DIR}/SPIRV-Tools-link.lib
	${DIR}/SPIRV-Tools-opt.lib 
	${DIR}/SPIRV-Tools-reduce.lib
	${DIR}/SPVRemapper.lib
	 )