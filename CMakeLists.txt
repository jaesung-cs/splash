cmake_minimum_required(VERSION 3.18)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(splash)

find_package(glad CONFIG REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

# splash executable
add_executable(splash
  src/main.cc
  src/splash/application.cc
)

target_link_libraries(splash PRIVATE
  glad::glad
  glfw
  glm::glm
)

target_include_directories(splash PRIVATE
  ./include
  ./src
)
