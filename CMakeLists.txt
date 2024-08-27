# LigidAPI - The only CMakeLists.txt file

cmake_minimum_required(VERSION 3.9)

# Define the project data
project(LigidAPI VERSION 1.2.0) #version

# Glob all source files in the LigidAPI directory
file(GLOB_RECURSE LIGIDAPI_SOURCES
   ${PROJECT_SOURCE_DIR}/Main/*.c
)

# Set C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Define CMAKE_HAS_X
add_definitions(-DCMAKE_HAS_X)

if(WIN32)
    # Add Windows-specific configurations
    add_executable(  
                    LigidAPI #WIN32
                    ${LIGIDAPI_SOURCES}
                 )
   
    target_compile_definitions(LigidAPI PRIVATE CRT)

else()
# Add non-Windows configurations
   add_executable(
                    LigidAPI
                    ${LIGIDAPI_SOURCES}
                )
endif(WIN32)

# Set the character set to Unicode
target_compile_definitions(LigidAPI PRIVATE UNICODE _UNICODE)

# Set the project version
set(PROJECT_VERSION_MAJOR 1) #version
set(PROJECT_VERSION_MINOR 2)
set(PROJECT_VERSION_PATCH 0)

target_link_options(LigidAPI PRIVATE "/STACK:3000000")

#find_package(OpenGL REQUIRED)

target_include_directories(LigidAPI PRIVATE ${PROJECT_SOURCE_DIR}/LigidAPI/Main/src)

# Add link directories for LigidAPI target
target_link_directories(LigidAPI PRIVATE 
    ${PROJECT_SOURCE_DIR}/Main/
)

# Link libraries for LigidAPI target
# target_link_libraries(LigidAPI PRIVATE OpenGL)