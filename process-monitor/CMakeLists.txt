CMAKE_MINIMUM_REQUIRED(VERSION 3.8)
PROJECT(hello LANGUAGES C)

IF (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  MESSAGE(FATAL_ERROR "
No build type selected. You need to pass --preset=<type> in order to configure CMake project.
")
ENDIF()

SET(CMAKE_C_EXTENSIONS OFF)
SET(CMAKE_C_FLAGS "-std=gnu99 ${CMAKE_C_FLAGS}")

OPTION(USE_SANITIZER "Enable to build with target sanitizer" "no sanitizer")

SET(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
SET(SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)

INCLUDE_DIRECTORIES(${INCLUDE_DIR})
ADD_LIBRARY(utils ${SOURCE_DIR}/utils.c)

FUNCTION(ADD_EXECUTABLE_WITH_SANITIZER TARGET_NAME FILE_NAME)
  # Create the executable.
  ADD_EXECUTABLE(${TARGET_NAME} ${FILE_NAME})

  TARGET_LINK_LIBRARIES(${TARGET_NAME} PRIVATE utils)
  # Add basic compile flags.
  TARGET_COMPILE_OPTIONS(${TARGET_NAME} PRIVATE -Wall -Wextra -Wpedantic)
  
  # Setup sanitizer(s).
  IF (USE_SANITIZER STREQUAL "no sanitizer")
    MESSAGE(STATUS "No sanitizer enabled")
  ELSE()
    IF (USE_SANITIZER STREQUAL "Many")
    MESSAGE(STATUS "Enabling AddressSanitizer, LeakSanitizer and UndefinedBehaviorSanitizer")
      TARGET_COMPILE_OPTIONS(${TARGET_NAME} PUBLIC -fsanitize=address,leak,undefined)
      TARGET_LINK_OPTIONS(${TARGET_NAME} PUBLIC -fsanitize=address,leak,undefined)
    ELSEIF(USE_SANITIZER STREQUAL "Thread")
    MESSAGE(STATUS "Enabling ThreadSanitizer")
      TARGET_COMPILE_OPTIONS(${TARGET_NAME} PUBLIC -fsanitize=thread)
      TARGET_LINK_OPTIONS(${TARGET_NAME} PUBLIC -fsanitize=thread)
    ENDIF()
  ENDIF()
ENDFUNCTION()

ADD_EXECUTABLE_WITH_SANITIZER("users_processes" "${SOURCE_DIR}/1_users_processes.c")
ADD_EXECUTABLE_WITH_SANITIZER("from_sbin" "${SOURCE_DIR}/2_from_sbin.c")
ADD_EXECUTABLE_WITH_SANITIZER("pid_of_last" "${SOURCE_DIR}/3_pid_of_last.c")
ADD_EXECUTABLE_WITH_SANITIZER("cpu_burst" "${SOURCE_DIR}/4_cpu_burst.c")
ADD_EXECUTABLE_WITH_SANITIZER("avg_run_child_or_parent" "${SOURCE_DIR}/5_avg_run_child_or_parent.c")
ADD_EXECUTABLE_WITH_SANITIZER("the_fattest_one" "${SOURCE_DIR}/6_the_fattest_one.c")
ADD_EXECUTABLE_WITH_SANITIZER("io" "${SOURCE_DIR}/7_io.c")
