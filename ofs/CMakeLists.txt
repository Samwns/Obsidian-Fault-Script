cmake_minimum_required(VERSION 3.20)
project(ofs CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ── Find LLVM ─────────────────────────────────────────────────────────────
find_package(LLVM REQUIRED CONFIG)
message(STATUS "LLVM version: ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake from: ${LLVM_DIR}")

if (LLVM_PACKAGE_VERSION VERSION_LESS "17.0.0")
    message(FATAL_ERROR "OFS requires LLVM >= 17. Found: ${LLVM_PACKAGE_VERSION}")
endif()

include_directories(${LLVM_INCLUDE_DIRS})
if(LLVM_DEFINITIONS)
    separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND "${LLVM_DEFINITIONS}")
    add_compile_options(${LLVM_DEFINITIONS_LIST})
endif()

# ── Detect native architecture for correct LLVM backend ───────────────────
if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64|ARM64")
    set(OFS_ARCH_COMPONENTS AArch64CodeGen AArch64AsmParser AArch64Desc AArch64Info)
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64|i386|i686")
    set(OFS_ARCH_COMPONENTS X86CodeGen X86AsmParser X86Desc X86Info)
else()
    # Fallback: let LLVM figure it out via nativecodegen
    set(OFS_ARCH_COMPONENTS)
endif()

# ── LLVM components needed ─────────────────────────────────────────────────
llvm_map_components_to_libnames(LLVM_LIBS
    Core
    Support
    IRReader
    Target
    MC
    ${OFS_ARCH_COMPONENTS}
    passes
    Analysis
    TransformUtils
    ScalarOpts
    InstCombine
    nativecodegen
)

# ── OFS compiler sources ───────────────────────────────────────────────────
add_executable(ofs
    src/main.cpp
    src/lexer/lexer.cpp
    src/parser/parser.cpp
    src/semantic/semantic.cpp
    src/semantic/type_system.cpp
    src/codegen/codegen.cpp
)

target_include_directories(ofs PRIVATE src)
target_link_libraries(ofs PRIVATE ${LLVM_LIBS})

# ── Runtime (compiled as C, linked into every OFS program) ────────────────
add_library(ofs_runtime STATIC src/runtime/ofs_runtime.c)

# ── Tests ─────────────────────────────────────────────────────────────────
enable_testing()

add_executable(test_lexer    tests/test_lexer.cpp    src/lexer/lexer.cpp)
add_executable(test_parser   tests/test_parser.cpp   src/lexer/lexer.cpp src/parser/parser.cpp)
add_executable(test_semantic tests/test_semantic.cpp  src/lexer/lexer.cpp src/parser/parser.cpp
                                                      src/semantic/semantic.cpp
                                                      src/semantic/type_system.cpp)

foreach(t test_lexer test_parser test_semantic)
    target_include_directories(${t} PRIVATE src)
    add_test(NAME ${t} COMMAND ${t})
endforeach()
