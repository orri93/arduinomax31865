add_library(libarduinomax31865 STATIC src/gosmax31865.cpp src/gosmax31865.h)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_PLATFORM_INDEPENDENT_CODE ON)

include_directories(
  ${sublibraries_include}
  ${CMAKE_SOURCE_DIR}/sublibraries/arduinosensor/src)
