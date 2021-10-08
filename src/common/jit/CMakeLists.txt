# src/common/jit

add_library(jit STATIC 
	"virtualSystemCompiler.cpp"
)

target_link_libraries(jit PUBLIC asmjit)