# include_directories(
#   ${PROJECT_SOURCE_DIR}/src
#   ${PROJECT_SOURCE_DIR}/include
# )

# set(SRC_FILES
#   src/main.cpp
# )

set(WAMR_BUILD_PLATFORM "linux")
set(WAMR_BUILD_INTERP 0)
set(WAMR_BUILD_FAST_INTERP 0)
set(WAMR_BUILD_AOT 0)
set(WAMR_BUILD_JIT 0)
set(WAMR_BUILD_FAST_JIT 1)
set(WAMR_BUILD_LAZY_JIT 0)
set(WAMR_BUILD_LIBC_BUILTIN 0)
set(WAMR_BUILD_LIBC_WASI 0)
set(WAMR_BUILD_SIMD 1)
set(WAMR_BUILD_REF_TYPES 0)
set(WAMR_BUILD_GC 0)
set(WAMR_BUILD_TAIL_CALL 1)
set(WAMR_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR}/wasm-micro-runtime)

include(${WAMR_ROOT_DIR}/build-scripts/runtime_lib.cmake)
add_library(vmlib ${WAMR_RUNTIME_LIB_SOURCE})

target_link_libraries(modules PRIVATE vmlib)
