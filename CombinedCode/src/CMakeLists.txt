# This file was automatically generated for projects
# without default 'CMakeLists.txt' file.

#FILE(GLOB_RECURSE app_sources ${CMAKE_SOURCE_DIR}/src/*.*)

#idf_component_register(SRCS ${app_sources})

# This file was automatically generated for projects
# without default 'CMakeLists.txt' file.

# Include the main source file and all files in the src directory
set(MAIN_SRCS ${CMAKE_SOURCE_DIR}/src/main.c)

# Glob all source files from the src directory
file(GLOB_RECURSE SRC_FILES 
    ${CMAKE_SOURCE_DIR}/src/*.c
    ${CMAKE_SOURCE_DIR}/src/*.h
)

# Include specific subdirectories
file(GLOB_RECURSE COMMON_UTILS_FILES 
    ${CMAKE_SOURCE_DIR}/src/common_utils/*.c
    ${CMAKE_SOURCE_DIR}/src/common_utils/*.h
)

file(GLOB_RECURSE RECEPTION_FILES 
    ${CMAKE_SOURCE_DIR}/src/reception/*.c
    ${CMAKE_SOURCE_DIR}/src/reception/*.h
)

file(GLOB_RECURSE TRANSMISSION_FILES 
    ${CMAKE_SOURCE_DIR}/src/transmission/*.c
    ${CMAKE_SOURCE_DIR}/src/transmission/*.h
)

# Combine all source files
set(ALL_SRCS ${SRC_FILES} ${COMMON_UTILS_FILES} ${RECEPTION_FILES} ${TRANSMISSION_FILES})

# Register the component with all source files
idf_component_register(SRCS ${ALL_SRCS})
