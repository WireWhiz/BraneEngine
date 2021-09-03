# file: include

file(GLOB_RECURSE LSOURCES *.cpp *.h *.hpp)

set(SOURCES ${LSOURCES} PARENT_SCOPE)
set(INCS ${includes} ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)