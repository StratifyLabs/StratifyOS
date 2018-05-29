cmake_minimum_required (VERSION 3.6)

#Usage:
#cmake -DPATH=/Users/tgil/git/StratifyOS-libc/newlib/libm/math -P /Applications/StratifyLabs-SDK/Tools/gcc/arm-none-eabi/cmake/generate_sources.cmake

file(GLOB FILELIST
	${PATH}/*.c
	${PATH}/*.h
	${PATH}/*.cpp
	${PATH}/*.hpp
	${PATH}/*.S
	${PATH}/*.s
	${PATH}/*.cc
	${PATH}/*.json
	${PATH}/*.md)

message(STATUS "Checking Path for sources: " ${PATH})

set(OUTPUT ${PATH}/CMakeLists.txt)

file(WRITE ${OUTPUT} "set(SOURCES\n")

foreach(FILE ${FILELIST})
	message(STATUS "OUTPUT FILE: " ${FILE})
	get_filename_component(FILENAME ${FILE} NAME)
	set(ENTRY "\${SOURCES_PREFIX}/${FILENAME}")
	message(STATUS "Adding Entry: " ${ENTRY})
	file(APPEND ${OUTPUT} "\t${ENTRY}")
	file(APPEND ${OUTPUT} "\n")
endforeach()

file(APPEND ${OUTPUT} "\tPARENT_SCOPE)")
