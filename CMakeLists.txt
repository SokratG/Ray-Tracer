cmake_minimum_required (VERSION 3.18)

if (WITH_CUDA)
    project(RayTracer LANGUAGES C CXX CUDA)
else()
    project(RayTracer LANGUAGES C CXX)
endif()

set(CMAKE_CXX_STANDARD 17)


# GLM
#find_package(glm REQUIRED)
set(GLM_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include/glm")

# CUDA
if (WITH_CUDA)
    # enable_language(CUDA)
    if(NOT DEFINED ${CMAKE_CUDA_ARCHITECTURES})
        set(CMAKE_CUDA_ARCHITECTURES 61 75)
    endif()
    find_package(CUDA QUIET REQUIRED)
    #[[
    set( CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS} -g -gencode -use_fast_math arch=compute_75, code=sm_75 )
    ]]
    set(CUDA_LINK_LIBRARIES_KEYWORD PUBLIC) 
    if(NOT DEFINED CMAKE_CUDA_STANDARD)
        set(CMAKE_CUDA_STANDARD 14)
        set(CMAKE_CUDA_STANDARD_REQUIRED ON)
    endif()
    file(GLOB GPU_SRCRT "src/cuda/*.cu")
    include_directories(${CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES})
    add_definitions(-D_USE_CUDA)
    #cuda_compile(${PROJECT_NAME} ${GPU_SRCRT})
endif()


file(GLOB SRCRT "src/*.cpp")


include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories(${GLM_INCLUDE_DIR})
include_directories("${PROJECT_BINARY_DIR}")

add_definitions(-D_USE_MATH_DEFINES)
#add_definitions(-D_USE_THREAD)


if(WITH_CUDA)
    add_executable(${PROJECT_NAME} ${SRCRT} ${GPU_SRCRT})
    target_link_libraries(${PROJECT_NAME} ${CUDA_LIBRARIES})
    set_target_properties( ${PROJECT_NAME}
                           PROPERTIES CUDA_SEPARABLE_COMPILATION ON)
else()
    add_executable(${PROJECT_NAME} ${SRCRT})
endif()

#target_link_libraries(${PROJECT_NAME} ${OPENGL_LIBRARIES})