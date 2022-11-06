file(GLOB SRC src/*.cpp src/*.c src/*.h)
add_library(OpenMfx_Sdk_Cpp_Common ${SRC})

target_include_directories(
  OpenMfx_Sdk_Cpp_Common
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(
  OpenMfx_Sdk_Cpp_Common
  PUBLIC
    OpenMfx::Core
)

set_property(TARGET OpenMfx_Sdk_Cpp_Common PROPERTY FOLDER "OpenMfx/Sdk/Cpp")
add_library(OpenMfx::Sdk::Cpp::Common ALIAS OpenMfx_Sdk_Cpp_Common)
