cmake_minimum_required(VERSION 3.18)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(splash)

find_package(glad CONFIG REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)

# splash executable
add_executable(splash
  src/main.cc
  src/splash/application.cc
  src/splash/fluid/neighbor_search.cc
  src/splash/fluid/neighbor_search_naive.cc
  src/splash/fluid/neighbor_search_spatial_hashing.cc
  src/splash/geom/particles.cc
  src/splash/geom/particles_bvh.cc
  src/splash/gl/boxes_geometry.cc
  src/splash/gl/geometry.cc
  src/splash/gl/particles_geometry.cc
  src/splash/gl/shader.cc
  src/splash/gl/shaders.cc
  src/splash/gl/texture.cc
  src/splash/model/camera.cc
  src/splash/model/image.cc
  src/splash/scene/resources.cc
  src/splash/scene/scene.cc
  src/splash/scene/scene_animation.cc
  src/splash/scene/scene_fluid.cc
  src/splash/scene/scene_particles.cc
  include/splash/application.h
  include/splash/fluid/neighbor.h
  include/splash/fluid/neighbor_search.h
  include/splash/fluid/neighbor_search_naive.h
  include/splash/fluid/neighbor_search_spatial_hashing.h
  include/splash/fluid/sph_kernel.h
  include/splash/geom/particle.h
  include/splash/geom/particles.h
  include/splash/geom/particles_bvh.h
  include/splash/gl/boxes_geometry.h
  include/splash/gl/geometry.h
  include/splash/gl/particles_geometry.h
  include/splash/gl/shader.h
  include/splash/gl/shaders.h
  include/splash/gl/texture.h
  include/splash/model/box.h
  include/splash/model/camera.h
  include/splash/model/image.h
  include/splash/model/light.h
  include/splash/scene/resources.h
  include/splash/scene/scene.h
  include/splash/scene/scene_animation.h
  include/splash/scene/scene_fluid.h
  include/splash/scene/scene_particles.h
)

target_link_libraries(splash PRIVATE
  glad::glad
  glfw
  glm::glm
  imgui::imgui
)

target_include_directories(splash PRIVATE
  ./include
  ./src
)
