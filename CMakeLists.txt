cmake_minimum_required(VERSION 3.22)

project(halx LANGUAGES CXX)

include(FetchContent)
FetchContent_Declare(stm32cubemx_helper
  GIT_REPOSITORY https://github.com/eyr1n/stm32cubemx_helper.git
)
FetchContent_MakeAvailable(stm32cubemx_helper)

# stm32rcos
add_library(${PROJECT_NAME} INTERFACE)
target_sources(${PROJECT_NAME} INTERFACE
  src/stdout.cpp
)
target_link_libraries(${PROJECT_NAME} INTERFACE
  stm32cubemx_helper
)
target_include_directories(${PROJECT_NAME} INTERFACE
  include
)
target_compile_features(${PROJECT_NAME} INTERFACE
  cxx_std_23
)

# halx::printf_float
add_library(${PROJECT_NAME}_printf_float INTERFACE)
add_library(${PROJECT_NAME}::printf_float ALIAS ${PROJECT_NAME}_printf_float)
target_link_libraries(${PROJECT_NAME}_printf_float INTERFACE
  stm32cubemx_helper::printf_float
)

# Doxygen
find_package(Doxygen)
if(DOXYGEN_FOUND)
  set(DOXYGEN_EXTRACT_ALL YES)
  set(DOXYGEN_OUTPUT_LANGUAGE Japanese)
  set(DOXYGEN_USE_MDFILE_AS_MAINPAGE README.md)
  set(DOXYGEN_DISABLE_INDEX NO)
  set(DOXYGEN_GENERATE_TREEVIEW NO)
  doxygen_add_docs(docs
    README.md
    ${CMAKE_CURRENT_SOURCE_DIR}/include
  )
endif()
