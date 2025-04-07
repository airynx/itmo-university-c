CMAKE_MINIMUM_REQUIRED(VERSION 3.8)
PROJECT(process_control LANGUAGES C)

IF (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  MESSAGE(FATAL_ERROR "
No build type selected. You need to pass --preset=<type> in order to configure CMake project.
")
ENDIF()

SET(CMAKE_C_EXTENSIONS OFF)
SET(CMAKE_C_FLAGS "-std=gnu99 ${CMAKE_C_FLAGS}")

OPTION(USE_SANITIZER "Enable to build with most available sanitizers" OFF)
OPTION(USE_THREAD_SANITIZER "Enable to build with ThreadSanitizer" OFF)

SET(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
SET(SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)

add_library(utils STATIC utils.c)
target_include_directories(utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
FUNCTION(ADD_EXECUTABLE_WITH_SANITIZER TARGET_NAME FILE_NAME)
  # Create the executable.
  ADD_EXECUTABLE(${TARGET_NAME} ${FILE_NAME})

  # Add basic compile flags.
  TARGET_COMPILE_OPTIONS(${TARGET_NAME} PRIVATE -Wall -Wextra -Wpedantic)

  # Setup sanitizer(s).
  if (USE_SANITIZER)
    TARGET_COMPILE_OPTIONS(${TARGET_NAME} PUBLIC -fsanitize=address,leak,undefined -fno-sanitize-recover=all)
    TARGET_LINK_OPTIONS(${TARGET_NAME} PUBLIC -fsanitize=address,leak,undefined)
  ENDIF()

  IF (USE_THREAD_SANITIZER)
    TARGET_COMPILE_OPTIONS(${TARGET_NAME} PUBLIC -fsanitize=thread -fno-sanitize-recover=all)
    TARGET_LINK_OPTIONS(${TARGET_NAME} PUBLIC -fsanitize=thread)
  ENDIF()
  target_link_libraries(${TARGET_NAME} PRIVATE utils)
ENDFUNCTION()

# Tasks 1-4.
ADD_EXECUTABLE_WITH_SANITIZER("1_datetime" "${SOURCE_DIR}/1_datetime.c")
ADD_EXECUTABLE_WITH_SANITIZER("2_repeat_datetime" "${SOURCE_DIR}/2_repeat_datetime.c")
ADD_EXECUTABLE_WITH_SANITIZER("3_total_repeat_datetime" "${SOURCE_DIR}/3_total_repeat_datetime.c")
ADD_EXECUTABLE_WITH_SANITIZER("4_limitations" "${SOURCE_DIR}/4_limitations.c")

# Task 5.
ADD_EXECUTABLE_WITH_SANITIZER("5_producer" "${SOURCE_DIR}/5_producer.c")
ADD_EXECUTABLE_WITH_SANITIZER("5_handler" "${SOURCE_DIR}/5_handler.c")
ADD_EXECUTABLE_WITH_SANITIZER("5_launcher" "${SOURCE_DIR}/5_launcher.c")

# Task 6.
ADD_EXECUTABLE_WITH_SANITIZER("6_producer" "${SOURCE_DIR}/6_producer.c")
ADD_EXECUTABLE_WITH_SANITIZER("6_handler" "${SOURCE_DIR}/6_handler.c")
ADD_EXECUTABLE_WITH_SANITIZER("6_launcher" "${SOURCE_DIR}/6_launcher.c")