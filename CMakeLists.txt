cmake_minimum_required(VERSION 3.13)

set(CMAKE_CXX_STANDARD 14)

project(gear)

add_definitions(-DPROJECT_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

file(GLOB_RECURSE SOURCE_FILES "src/*.h" "src/*.cpp")
add_executable(gear ${SOURCE_FILES})
target_include_directories(gear PUBLIC "src")

# cgltf
add_library(cgltf INTERFACE)
target_include_directories(cgltf INTERFACE extern/cgltf)
target_link_libraries(gear PUBLIC cgltf)

# spark
add_subdirectory(extern/spark EXCLUDE_FROM_ALL spark.out)
target_link_libraries(gear PUBLIC spark)

# stb
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE extern/stb)
target_link_libraries(gear PUBLIC stb)

# astcenc
add_subdirectory(extern/astcenc EXCLUDE_FROM_ALL astcenc.out)
target_link_libraries(gear PUBLIC astcenc)