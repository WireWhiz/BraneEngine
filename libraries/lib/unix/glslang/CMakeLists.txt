#lib/unix/glslang

add_library(glslang INTERFACE)
set(DIR ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(glslang INTERFACE
	${DIR}/libMachineIndependent.a
	${DIR}/libeffcee.a
	${DIR}/libGenericCodeGen.a
	${DIR}/libglslang.a
	${DIR}/libglslang-default-resource-limits.a
	${DIR}/libHLSL.a
	${DIR}/libSPIRV.a
	${DIR}/libOGLCompiler.a
	${DIR}/libOSDependent.a
	${DIR}/libSPIRV-Tools-opt.a 
	${DIR}/libSPIRV-Tools.a
	${DIR}/libSPIRV-Tools-link.a
	${DIR}/libSPIRV-Tools-comp.a
	${DIR}/libSPIRV-Tools-reduce.a
	${DIR}/libSPVRemapper.a
	 )