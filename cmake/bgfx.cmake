# ============================================================================
#  bgfx / bx / bimg  –  CMake integration
#
#  This file defines CMake targets for:
#    bx       – base utility library
#    bimg     – image library
#    bgfx     – rendering library (amalgamated build)
#    shaderc  – bgfx shader compiler tool
#
#  It also provides a function  bgfx_compile_shaders()  that adds custom
#  commands to compile .sc shader files during the build.
# ============================================================================

set(BX_DIR       ${CMAKE_SOURCE_DIR}/third_party/bx)
set(BIMG_DIR     ${CMAKE_SOURCE_DIR}/third_party/bimg)
set(BGFX_DIR     ${CMAKE_SOURCE_DIR}/third_party/bgfx)
set(FCPP_DIR     ${BGFX_DIR}/3rdparty/fcpp)
set(GLSLANG_DIR  ${BGFX_DIR}/3rdparty/glslang)
set(SPIRV_CROSS_DIR  ${BGFX_DIR}/3rdparty/spirv-cross)
set(SPIRV_TOOLS_DIR  ${BGFX_DIR}/3rdparty/spirv-tools)
set(SPIRV_HEADERS_DIR ${BGFX_DIR}/3rdparty/spirv-headers)
set(GLSL_OPTIMIZER_DIR ${BGFX_DIR}/3rdparty/glsl-optimizer)
set(TINT_DIR     ${BGFX_DIR}/3rdparty/dawn)

# ── bx ───────────────────────────────────────────────────────────────────────

add_library(bx STATIC ${BX_DIR}/src/amalgamated.cpp)

target_include_directories(bx
  PUBLIC
    ${BX_DIR}/include
    ${BX_DIR}/include/compat/msvc     # Windows MSVC compat (dirent.h)
  PRIVATE
    ${BX_DIR}/3rdparty
)

target_compile_definitions(bx
  PUBLIC
    BX_CONFIG_DEBUG=$<IF:$<CONFIG:Debug>,1,0>
    __STDC_FORMAT_MACROS
    __STDC_LIMIT_MACROS
    __STDC_CONSTANT_MACROS
)

if(MSVC)
  target_compile_options(bx PRIVATE /W0)
else()
  target_compile_options(bx PRIVATE -w)
endif()

if(WIN32)
  target_link_libraries(bx PUBLIC psapi)
endif()

# ── bimg ─────────────────────────────────────────────────────────────────────

set(BIMG_SOURCES
  ${BIMG_DIR}/src/image.cpp
  ${BIMG_DIR}/src/image_cubemap_filter.cpp
  ${BIMG_DIR}/src/image_decode.cpp
  ${BIMG_DIR}/src/image_encode.cpp
  ${BIMG_DIR}/src/image_gnf.cpp
)

file(GLOB ASTCENC_SOURCES ${BIMG_DIR}/3rdparty/astc-encoder/source/*.cpp)

add_library(bimg STATIC ${BIMG_SOURCES} ${ASTCENC_SOURCES})

target_include_directories(bimg
  PUBLIC
    ${BIMG_DIR}/include
  PRIVATE
    ${BIMG_DIR}/src
    ${BIMG_DIR}/3rdparty
    ${BIMG_DIR}/3rdparty/astc-encoder
    ${BIMG_DIR}/3rdparty/astc-encoder/include
    ${BIMG_DIR}/3rdparty/iqa/include
    ${BIMG_DIR}/3rdparty/tinyexr/deps/miniz
)

target_link_libraries(bimg PUBLIC bx)

if(MSVC)
  target_compile_options(bimg PRIVATE /W0)
else()
  target_compile_options(bimg PRIVATE -w)
endif()

# ── bgfx ─────────────────────────────────────────────────────────────────────

add_library(bgfx STATIC ${BGFX_DIR}/src/amalgamated.cpp)

target_include_directories(bgfx
  PUBLIC
    ${BGFX_DIR}/include
  PRIVATE
    ${BGFX_DIR}/src
    ${BGFX_DIR}/3rdparty
    ${BGFX_DIR}/3rdparty/khronos
)

target_compile_definitions(bgfx PRIVATE
  BGFX_CONFIG_DEBUG_PIX=0
  BGFX_CONFIG_DEBUG_ANNOTATION=0
)

target_link_libraries(bgfx PUBLIC bx bimg)

if(MSVC)
  target_compile_options(bgfx PRIVATE /W0 /bigobj)
else()
  target_compile_options(bgfx PRIVATE -w)
endif()

if(WIN32)
  target_link_libraries(bgfx PRIVATE gdi32 user32)
endif()

# ── fcpp (C preprocessor for shaderc) ────────────────────────────────────────

add_library(fcpp STATIC
  ${FCPP_DIR}/cpp1.c
  ${FCPP_DIR}/cpp2.c
  ${FCPP_DIR}/cpp3.c
  ${FCPP_DIR}/cpp4.c
  ${FCPP_DIR}/cpp5.c
  ${FCPP_DIR}/cpp6.c
)

target_include_directories(fcpp PUBLIC ${FCPP_DIR})

target_compile_definitions(fcpp PRIVATE
  NINCLUDE=64
  NWORK=65536
  NBUFF=65536
  OLD_PREPROCESSOR=0
)

if(MSVC)
  target_compile_options(fcpp PRIVATE /W0)
else()
  target_compile_options(fcpp PRIVATE -w)
endif()

# ── spirv-opt (SPIRV-Tools) ─────────────────────────────────────────────────

file(GLOB_RECURSE SPIRV_OPT_SOURCES
  ${SPIRV_TOOLS_DIR}/source/opt/*.cpp
  ${SPIRV_TOOLS_DIR}/source/reduce/*.cpp
  ${SPIRV_TOOLS_DIR}/source/val/*.cpp
  ${SPIRV_TOOLS_DIR}/source/util/bit_vector.cpp
  ${SPIRV_TOOLS_DIR}/source/util/parse_number.cpp
  ${SPIRV_TOOLS_DIR}/source/util/string_utils.cpp
)

set(SPIRV_TOOLS_CORE_SOURCES
  ${SPIRV_TOOLS_DIR}/source/assembly_grammar.cpp
  ${SPIRV_TOOLS_DIR}/source/binary.cpp
  ${SPIRV_TOOLS_DIR}/source/diagnostic.cpp
  ${SPIRV_TOOLS_DIR}/source/disassemble.cpp
  ${SPIRV_TOOLS_DIR}/source/ext_inst.cpp
  ${SPIRV_TOOLS_DIR}/source/extensions.cpp
  ${SPIRV_TOOLS_DIR}/source/libspirv.cpp
  ${SPIRV_TOOLS_DIR}/source/name_mapper.cpp
  ${SPIRV_TOOLS_DIR}/source/opcode.cpp
  ${SPIRV_TOOLS_DIR}/source/operand.cpp
  ${SPIRV_TOOLS_DIR}/source/parsed_operand.cpp
  ${SPIRV_TOOLS_DIR}/source/print.cpp
  ${SPIRV_TOOLS_DIR}/source/software_version.cpp
  ${SPIRV_TOOLS_DIR}/source/spirv_endian.cpp
  ${SPIRV_TOOLS_DIR}/source/spirv_optimizer_options.cpp
  ${SPIRV_TOOLS_DIR}/source/spirv_reducer_options.cpp
  ${SPIRV_TOOLS_DIR}/source/spirv_target_env.cpp
  ${SPIRV_TOOLS_DIR}/source/spirv_validator_options.cpp
  ${SPIRV_TOOLS_DIR}/source/table.cpp
  ${SPIRV_TOOLS_DIR}/source/table2.cpp
  ${SPIRV_TOOLS_DIR}/source/text.cpp
  ${SPIRV_TOOLS_DIR}/source/text_handler.cpp
  ${SPIRV_TOOLS_DIR}/source/to_string.cpp
)

add_library(spirv-opt STATIC ${SPIRV_OPT_SOURCES} ${SPIRV_TOOLS_CORE_SOURCES})

target_include_directories(spirv-opt
  PUBLIC
    ${SPIRV_TOOLS_DIR}
    ${SPIRV_TOOLS_DIR}/include
    ${SPIRV_TOOLS_DIR}/include/generated
    ${SPIRV_TOOLS_DIR}/source
    ${SPIRV_HEADERS_DIR}/include
)

if(MSVC)
  target_compile_options(spirv-opt PRIVATE /W0)
else()
  target_compile_options(spirv-opt PRIVATE -w)
endif()

# ── spirv-cross ──────────────────────────────────────────────────────────────

add_library(spirv-cross STATIC
  ${SPIRV_CROSS_DIR}/spirv_cfg.cpp
  ${SPIRV_CROSS_DIR}/spirv_cpp.cpp
  ${SPIRV_CROSS_DIR}/spirv_cross.cpp
  ${SPIRV_CROSS_DIR}/spirv_cross_parsed_ir.cpp
  ${SPIRV_CROSS_DIR}/spirv_cross_util.cpp
  ${SPIRV_CROSS_DIR}/spirv_glsl.cpp
  ${SPIRV_CROSS_DIR}/spirv_hlsl.cpp
  ${SPIRV_CROSS_DIR}/spirv_msl.cpp
  ${SPIRV_CROSS_DIR}/spirv_parser.cpp
  ${SPIRV_CROSS_DIR}/spirv_reflect.cpp
)

target_include_directories(spirv-cross
  PUBLIC
    ${SPIRV_CROSS_DIR}
    ${SPIRV_CROSS_DIR}/include
)

target_compile_definitions(spirv-cross PRIVATE
  SPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS
)

if(MSVC)
  target_compile_options(spirv-cross PRIVATE /W0)
else()
  target_compile_options(spirv-cross PRIVATE -w)
endif()

# ── glslang ──────────────────────────────────────────────────────────────────

file(GLOB_RECURSE GLSLANG_SOURCES
  ${GLSLANG_DIR}/glslang/*.cpp
  ${GLSLANG_DIR}/SPIRV/*.cpp
)

# Remove Unix OS-dependent sources on Windows
if(WIN32)
  list(FILTER GLSLANG_SOURCES EXCLUDE REGEX "OSDependent/Unix/")
else()
  list(FILTER GLSLANG_SOURCES EXCLUDE REGEX "OSDependent/Windows/")
endif()

add_library(glslang STATIC ${GLSLANG_SOURCES})

target_include_directories(glslang
  PUBLIC
    ${GLSLANG_DIR}
    ${GLSLANG_DIR}/glslang/Public
    ${GLSLANG_DIR}/glslang/Include
  PRIVATE
    ${BGFX_DIR}/3rdparty         # for "glslang/build_info.h"
    ${SPIRV_TOOLS_DIR}/include
    ${SPIRV_TOOLS_DIR}/source
)

target_compile_definitions(glslang PRIVATE
  ENABLE_OPT=1
  ENABLE_HLSL=1
)

target_link_libraries(glslang PRIVATE spirv-opt)

if(MSVC)
  target_compile_options(glslang PRIVATE /W0)
else()
  target_compile_options(glslang PRIVATE -w -fno-strict-aliasing)
endif()

# ── glsl-optimizer ───────────────────────────────────────────────────────────

file(GLOB GLSL_OPTIMIZER_GLSL_SOURCES
  ${GLSL_OPTIMIZER_DIR}/src/glsl/*.cpp
  ${GLSL_OPTIMIZER_DIR}/src/glsl/*.c
)

# Remove files that should not be compiled
list(FILTER GLSL_OPTIMIZER_GLSL_SOURCES EXCLUDE REGEX "main\\.cpp$")
list(FILTER GLSL_OPTIMIZER_GLSL_SOURCES EXCLUDE REGEX "builtin_stubs\\.cpp$")
list(FILTER GLSL_OPTIMIZER_GLSL_SOURCES EXCLUDE REGEX "ir_set_program_inouts\\.cpp$")

set(GLSL_OPTIMIZER_SOURCES
  ${GLSL_OPTIMIZER_GLSL_SOURCES}
  ${GLSL_OPTIMIZER_DIR}/src/glsl/glcpp/glcpp-lex.c
  ${GLSL_OPTIMIZER_DIR}/src/glsl/glcpp/glcpp-parse.c
  ${GLSL_OPTIMIZER_DIR}/src/glsl/glcpp/pp.c
  ${GLSL_OPTIMIZER_DIR}/src/mesa/main/imports.c
  ${GLSL_OPTIMIZER_DIR}/src/mesa/program/prog_hash_table.c
  ${GLSL_OPTIMIZER_DIR}/src/mesa/program/symbol_table.c
  ${GLSL_OPTIMIZER_DIR}/src/util/hash_table.c
  ${GLSL_OPTIMIZER_DIR}/src/util/ralloc.c
)

add_library(glsl-optimizer STATIC ${GLSL_OPTIMIZER_SOURCES})

target_include_directories(glsl-optimizer
  PUBLIC
    ${GLSL_OPTIMIZER_DIR}/include
    ${GLSL_OPTIMIZER_DIR}/src/glsl
  PRIVATE
    ${GLSL_OPTIMIZER_DIR}/src
    ${GLSL_OPTIMIZER_DIR}/src/mesa
    ${GLSL_OPTIMIZER_DIR}/src/mapi
)

if(MSVC)
  target_include_directories(glsl-optimizer PRIVATE
    ${GLSL_OPTIMIZER_DIR}/src/glsl/msvc
    ${GLSL_OPTIMIZER_DIR}/include/c99
  )
  target_compile_definitions(glsl-optimizer PRIVATE
    __STDC__
    __STDC_VERSION__=199901L
    "strdup=_strdup"
    "alloca=_alloca"
    "isascii=__isascii"
  )
  target_compile_options(glsl-optimizer PRIVATE /W0)
else()
  target_compile_options(glsl-optimizer PRIVATE -w -fno-strict-aliasing)
endif()

# ── tint (Dawn/WGSL support for shaderc) ─────────────────────────────────────

file(GLOB_RECURSE TINT_CORE_SOURCES
  ${TINT_DIR}/src/tint/utils/*.cc
  ${TINT_DIR}/src/tint/lang/core/*.cc
  ${TINT_DIR}/src/tint/lang/null/*.cc
)

file(GLOB_RECURSE TINT_LANG_SOURCES
  ${TINT_DIR}/src/tint/lang/spirv/*.cc
  ${TINT_DIR}/src/tint/lang/wgsl/*.cc
)

file(GLOB_RECURSE TINT_API_SOURCES
  ${TINT_DIR}/src/tint/api/*.cc
)

# Filter test files
list(FILTER TINT_CORE_SOURCES EXCLUDE REGEX "_test\\.cc$|_bench\\.cc$|_test_helper")
list(FILTER TINT_LANG_SOURCES EXCLUDE REGEX "_test\\.cc$|_bench\\.cc$|_test_helper")
list(FILTER TINT_API_SOURCES  EXCLUDE REGEX "_test\\.cc$|_bench\\.cc$|_test_helper")

set(TINT_ALL_SOURCES ${TINT_CORE_SOURCES} ${TINT_LANG_SOURCES} ${TINT_API_SOURCES})

if(TINT_ALL_SOURCES)
  add_library(tint STATIC ${TINT_ALL_SOURCES})

  target_include_directories(tint
    PUBLIC
      ${TINT_DIR}
      ${TINT_DIR}/src/tint
    PRIVATE
      ${TINT_DIR}/third_party/protobuf/src
      ${TINT_DIR}/third_party/abseil-cpp
      ${SPIRV_TOOLS_DIR}
      ${SPIRV_TOOLS_DIR}/include
      ${SPIRV_TOOLS_DIR}/include/generated
      ${SPIRV_HEADERS_DIR}/include
  )

  target_compile_definitions(tint PRIVATE
    TINT_BUILD_GLSL_WRITER=0
    TINT_BUILD_HLSL_WRITER=0
    TINT_BUILD_MSL_WRITER=0
    TINT_BUILD_NULL_WRITER=0
    TINT_BUILD_SPV_READER=1
    TINT_BUILD_SPV_WRITER=0
    TINT_BUILD_WGSL_READER=0
    TINT_BUILD_WGSL_WRITER=1
    TINT_BUILD_IS_LINUX=0
    TINT_BUILD_IS_MAC=0
    TINT_BUILD_IS_WIN=$<IF:$<PLATFORM_ID:Windows>,1,0>
    TINT_ENABLE_IR_VALIDATION=0
  )

  if(MSVC)
    target_compile_options(tint PRIVATE /W0 /bigobj)
  else()
    target_compile_options(tint PRIVATE -w)
  endif()

  set(TINT_TARGET tint)
else()
  set(TINT_TARGET "")
endif()

# ── shaderc ──────────────────────────────────────────────────────────────────

file(GLOB SHADERC_SOURCES ${BGFX_DIR}/tools/shaderc/*.cpp)

# vertexlayout + shader sources from bgfx/src (needed by shaderc)
set(SHADERC_BGFX_SOURCES
  ${BGFX_DIR}/src/vertexlayout.cpp
  ${BGFX_DIR}/src/vertexlayout.h
  ${BGFX_DIR}/src/shader.cpp
  ${BGFX_DIR}/src/shader.h
  ${BGFX_DIR}/src/shader_dxbc.cpp
  ${BGFX_DIR}/src/shader_dxbc.h
  ${BGFX_DIR}/src/shader_spirv.cpp
  ${BGFX_DIR}/src/shader_spirv.h
)

add_executable(shaderc ${SHADERC_SOURCES} ${SHADERC_BGFX_SOURCES})

target_include_directories(shaderc PRIVATE
  ${BIMG_DIR}/include
  ${BGFX_DIR}/include
  ${BGFX_DIR}/src
  ${BGFX_DIR}/3rdparty/directx-headers/include/directx
  ${FCPP_DIR}
  ${GLSLANG_DIR}/glslang/Public
  ${GLSLANG_DIR}/glslang/Include
  ${GLSLANG_DIR}
  ${GLSL_OPTIMIZER_DIR}/include
  ${GLSL_OPTIMIZER_DIR}/src/glsl
  ${SPIRV_CROSS_DIR}
  ${SPIRV_TOOLS_DIR}/include
  ${TINT_DIR}
  ${TINT_DIR}/src
)

set(SHADERC_LIBS bx fcpp glslang glsl-optimizer spirv-opt spirv-cross)
if(TINT_TARGET)
  list(APPEND SHADERC_LIBS ${TINT_TARGET})
endif()

target_link_libraries(shaderc PRIVATE ${SHADERC_LIBS})

if(MSVC)
  target_include_directories(shaderc PRIVATE ${GLSL_OPTIMIZER_DIR}/include/c99)
  target_compile_options(shaderc PRIVATE /W0)
  target_link_libraries(shaderc PRIVATE psapi)
else()
  target_compile_options(shaderc PRIVATE -w)
endif()

# ── Shader compilation function ──────────────────────────────────────────────

function(bgfx_compile_shader)
  cmake_parse_arguments(ARG "" "INPUT;OUTPUT;TYPE;PROFILE;VARYINGDEF;ARRAY_NAME" "" ${ARGN})

  # Derive a C array name from the output filename if not given
  if(NOT ARG_ARRAY_NAME)
    get_filename_component(_outname ${ARG_OUTPUT} NAME)
    string(REPLACE "." "_" ARG_ARRAY_NAME "${_outname}")
  endif()

  add_custom_command(
    OUTPUT ${ARG_OUTPUT}
    COMMAND $<TARGET_FILE:shaderc>
      -f ${ARG_INPUT}
      -o ${ARG_OUTPUT}
      --type ${ARG_TYPE}
      --platform windows
      -p ${ARG_PROFILE}
      -i ${BGFX_DIR}/src
      --varyingdef ${ARG_VARYINGDEF}
      --bin2c ${ARG_ARRAY_NAME}
    DEPENDS ${ARG_INPUT} ${ARG_VARYINGDEF} shaderc
    COMMENT "Compiling shader: ${ARG_INPUT} -> ${ARG_OUTPUT}"
    VERBATIM
  )
endfunction()
