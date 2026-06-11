if(EMSCRIPTEN)
  set(TARGET_NAME minigin_web)
  set(TARGET_PARAM )
else()
  set(TARGET_NAME minigin_game)
  set(TARGET_PARAM WIN32)
endif()

add_executable(${TARGET_NAME} ${TARGET_PARAM}
  Main.cpp
  CubeTileState.cpp
  LevelData.cpp
  QbertPyramid.cpp
  CoilyState.cpp
  GameStateManager.cpp
  QbertPlayerComponent.cpp
  CoilyComponent.cpp
  UggWrongwayComponent.cpp
  SlickSamComponent.cpp
)

target_compile_features(${TARGET_NAME} PUBLIC cxx_std_20)

target_link_libraries(${TARGET_NAME} PRIVATE minigin)

target_include_directories(${TARGET_NAME} PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
)

if(MSVC)
  target_compile_options(${TARGET_NAME} PRIVATE /W4 /WX)
else()
  target_compile_options(${TARGET_NAME} PRIVATE -Wall -Wextra -Werror)
endif()

if(EMSCRIPTEN)
  set(EMSCRIPTEN_COMPILE_FLAGS "${EMSCRIPTEN_LINK_FLAGS} -g")
  set(EMSCRIPTEN_LINK_FLAGS "${EMSCRIPTEN_LINK_FLAGS} -sUSE_FREETYPE=1 -sUSE_HARFBUZZ=1 -g -sALLOW_MEMORY_GROWTH --preload-file \"${CMAKE_SOURCE_DIR}/Data@/\"")

  set_target_properties(${TARGET_NAME} PROPERTIES
    COMPILE_FLAGS "${EMSCRIPTEN_COMPILE_FLAGS}"
    LINK_FLAGS    "${EMSCRIPTEN_LINK_FLAGS}"
  )

  add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy_directory
      "${CMAKE_SOURCE_DIR}/web"
      "$<TARGET_FILE_DIR:${TARGET_NAME}>/"
  )

  set(CMAKE_EXECUTABLE_SUFFIX ".html")

elseif(WIN32)
  set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${TARGET_NAME})
  set_target_properties(${TARGET_NAME} PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:${TARGET_NAME}>"
  )

  # Copy SDL DLLs
  add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "$<TARGET_FILE:SDL3::SDL3>"
      "$<TARGET_FILE_DIR:${TARGET_NAME}>"
    VERBATIM
  )
  add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "$<TARGET_FILE:SDL3_ttf::SDL3_ttf>"
      "$<TARGET_FILE_DIR:${TARGET_NAME}>"
    VERBATIM
  )

  add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different 
      "$<TARGET_FILE:SDL3_mixer::SDL3_mixer>"
      "$<TARGET_FILE_DIR:${TARGET_NAME}>" VERBATIM)

  if(Steamworks_FOUND)
    target_link_libraries(${TARGET_NAME} PRIVATE Steamworks::Steamworks)
    target_compile_definitions(${TARGET_NAME} PRIVATE USE_STEAMWORKS)
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
      COMMAND "${CMAKE_COMMAND}" -E copy "${Steamworks_RUNTIME_DLL}" "$<TARGET_FILE_DIR:${TARGET_NAME}>")
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
      COMMAND "${CMAKE_COMMAND}" -E echo "480" > "$<TARGET_FILE_DIR:${TARGET_NAME}>/steam_appid.txt")
  endif()

  add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy_directory
      "${CMAKE_SOURCE_DIR}/Data"
      "$<TARGET_FILE_DIR:${TARGET_NAME}>/Data"
  )

elseif(APPLE)
  set_target_properties(${TARGET_NAME} PROPERTIES
    MACOSX_BUNDLE TRUE
    INSTALL_RPATH "@executable_path/../Frameworks"
    BUILD_WITH_INSTALL_RPATH TRUE
  )
  add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E make_directory "$<TARGET_BUNDLE_DIR:${TARGET_NAME}>/Contents/Frameworks"
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_FILE:SDL3::SDL3-shared>"         "$<TARGET_BUNDLE_DIR:${TARGET_NAME}>/Contents/Frameworks/"
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_FILE:SDL3_ttf::SDL3_ttf-shared>" "$<TARGET_BUNDLE_DIR:${TARGET_NAME}>/Contents/Frameworks/"
    COMMAND install_name_tool -change "@rpath/libSDL3_ttf.0.dylib" "@executable_path/../Frameworks/libSDL3_ttf.0.2.2.dylib" "$<TARGET_FILE:${TARGET_NAME}>"
    COMMAND install_name_tool -change "@rpath/libSDL3.0.dylib"     "@executable_path/../Frameworks/libSDL3.0.dylib"          "$<TARGET_FILE:${TARGET_NAME}>"
  )

  if(Steamworks_FOUND)
    target_link_libraries(${TARGET_NAME} PRIVATE Steamworks::Steamworks)
    target_compile_definitions(${TARGET_NAME} PRIVATE USE_STEAMWORKS)
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
      COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${Steamworks_LIBRARIES}" "$<TARGET_BUNDLE_DIR:${TARGET_NAME}>/Contents/Frameworks/")
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
      COMMAND "${CMAKE_COMMAND}" -E echo "480" > "$<TARGET_FILE_DIR:${TARGET_NAME}>/steam_appid.txt")
  endif()

  add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy_directory
      "${CMAKE_SOURCE_DIR}/Data"
      "$<TARGET_FILE_DIR:${TARGET_NAME}>/Data"
  )

else()
  # Linux / other
  if(Steamworks_FOUND)
    target_link_libraries(${TARGET_NAME} PRIVATE Steamworks::Steamworks)
    target_compile_definitions(${TARGET_NAME} PRIVATE USE_STEAMWORKS)
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
      COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${Steamworks_LIBRARIES}" "$<TARGET_FILE_DIR:${TARGET_NAME}>")
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
      COMMAND "${CMAKE_COMMAND}" -E echo "480" > "$<TARGET_FILE_DIR:${TARGET_NAME}>/steam_appid.txt")
  endif()

  add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy_directory
      "${CMAKE_SOURCE_DIR}/Data"
      "$<TARGET_FILE_DIR:${TARGET_NAME}>/Data"
  )
endif()