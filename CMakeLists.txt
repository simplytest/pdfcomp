cmake_minimum_required(VERSION 3.21)
project(pdfcomp LANGUAGES CXX VERSION 2.1.0)

# --------------------------------------------------------------------------------------------------------
# Setup Executable
# --------------------------------------------------------------------------------------------------------

add_executable(${PROJECT_NAME} "src/main.cpp" "src/pdf.cpp")

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 23 CXX_EXTENSIONS OFF CXX_STANDARD_REQUIRED ON)
  
if (PROJECT_IS_TOP_LEVEL)
  if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX /external:anglebrackets /external:W0)
  else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror -pedantic -pedantic-errors -Wfatal-errors)
  endif()
endif()

# --------------------------------------------------------------------------------------------------------
# Include directories
# --------------------------------------------------------------------------------------------------------

target_include_directories(${PROJECT_NAME} PUBLIC "include")

# --------------------------------------------------------------------------------------------------------
# Setup Dependencies
# --------------------------------------------------------------------------------------------------------

include("cmake/cpm.cmake")
include("cmake/magick.cmake")

CPMFindPackage(
  NAME           clap
  VERSION        0.4.3
  GIT_REPOSITORY "https://github.com/karnkaul/clap"
)

CPMFindPackage(
  NAME           expected
  VERSION        1.1.0
  GIT_REPOSITORY "https://github.com/TartanLlama/expected"
  OPTIONS        "EXPECTED_BUILD_TESTS OFF"
)

target_link_libraries(${PROJECT_NAME} PUBLIC clap::clap tl::expected ImageMagick::MagickCore ImageMagick::Magick++)
