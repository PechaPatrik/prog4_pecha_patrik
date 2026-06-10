add_library(minigin STATIC
  Font.cpp
  GameObject.cpp
  InputManager.cpp
  Minigin.cpp
  Renderer.cpp
  ResourceManager.cpp
  Scene.cpp
  SceneManager.cpp
  TextComponent.cpp
  Texture2D.cpp
  Transform.cpp
  Component.cpp
  FPSComponent.cpp
  ImageComponent.cpp
  RotatorComponent.cpp
  ThrashTheCache.cpp
  Controller.cpp
  ServiceLocator.cpp
  SDLSoundSystem.cpp
)

target_compile_features(minigin PUBLIC cxx_std_20)

target_include_directories(minigin PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${imgui_SOURCE_DIR}
  ${VLD_INCLUDE_DIR}
)

target_link_libraries(minigin PUBLIC
  SDL3::SDL3
  SDL3_ttf::SDL3_ttf
  SDL3_mixer::SDL3_mixer
  glm::glm
  imgui
  nlohmann_json::nlohmann_json
  ${VLD_LIBRARIES}
)

if(MSVC)
  target_compile_options(minigin PRIVATE /W4 /WX)
else()
  target_compile_options(minigin PRIVATE -Wall -Wextra -Werror)
endif()