
cmake_minimum_required(VERSION 3.12)

project(person_detection_screen C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)

#添加编译子目录
add_subdirectory(lib/Config)
add_subdirectory(lib/LCD)
add_subdirectory(lib/arducam_s)

#添加头文件目录
include_directories(./lib/Config)
include_directories(./lib/LCD)
include_directories(lib/arducam_s)

add_executable(person_detection_screen_int8 "")

target_include_directories(person_detection_screen_int8
  PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}/.
  )

set_target_properties(
  person_detection_screen_int8
  PROPERTIES
  COMPILE_FLAGS -fno-rtti
  COMPILE_FLAGS -fno-exceptions
  COMPILE_FLAGS -fno-threadsafe-statics
  COMPILE_FLAGS -nostdlib
)

target_sources(person_detection_screen_int8
  PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}/detection_responder.cpp
  ${CMAKE_CURRENT_LIST_DIR}/image_provider.cpp
  ${CMAKE_CURRENT_LIST_DIR}/main.cpp
  ${CMAKE_CURRENT_LIST_DIR}/main_functions.cpp
  ${CMAKE_CURRENT_LIST_DIR}/model_settings.cpp
  ${CMAKE_CURRENT_LIST_DIR}/tensorflow/lite/micro/tools/make/downloads/person_model_int8/person_detect_model_data.cpp
  ${CMAKE_CURRENT_LIST_DIR}/detection_responder.h
  ${CMAKE_CURRENT_LIST_DIR}/image_provider.h
  ${CMAKE_CURRENT_LIST_DIR}/main_functions.h
  ${CMAKE_CURRENT_LIST_DIR}/model_settings.h
  ${CMAKE_CURRENT_LIST_DIR}/person_detect_model_data.h
  )

target_link_libraries(
  person_detection_screen_int8
  pico-tflmicro
  hardware_pwm
  hardware_pio
  pico_stdlib
  arducam_s
  Config
  LCD

)
# enable usb output, disable uart output
pico_enable_stdio_usb(person_detection_screen_int8 1)
pico_enable_stdio_uart(person_detection_screen_int8 0)
pico_add_extra_outputs(person_detection_screen_int8)
