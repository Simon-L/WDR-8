# DISTRHO Plugin Framework (DPF)
# Copyright (C) 2021 Jean Pierre Cimalando <jp-dev@inbox.ru>
# Copyright (C) 2021-2022 Filipe Coelho <falktx@falktx.com>
# SPDX-License-Identifier: ISC

cmake_minimum_required(VERSION 3.7)

set(CMAKE_CXX_STANDARD 17)

set(NAME eightohat)
project(${NAME})

add_subdirectory(dpf)
add_subdirectory(sst-basic-blocks)

dpf_add_plugin(${NAME}
  TARGETS vst3 clap lv2 jack
  MONOLITHIC
  FILES_DSP
      src/PluginDSP.cpp
      src/PluginDSP_metadata.cpp
      src/PluginDSP_midi.cpp
      src/PluginDSP_parameters.cpp
  FILES_UI
      src/PluginUI.cpp
      implot/implot.cpp
      implot/implot_demo.cpp
      implot/implot_items.cpp
      imgui-knobs/imgui-knobs.cpp
      dpf-widgets/opengl/DearImGui.cpp)

target_link_libraries(${NAME} PUBLIC sst-basic-blocks)

target_include_directories(${NAME} PUBLIC src)
target_include_directories(${NAME} PUBLIC include)
target_include_directories(${NAME} PUBLIC dpf-widgets/generic)
target_include_directories(${NAME} PUBLIC dpf-widgets/opengl)
target_include_directories(${NAME} PUBLIC dpf-widgets/opengl/DearImGui)
target_include_directories(${NAME} PUBLIC implot)
target_include_directories(${NAME} PUBLIC imgui-knobs)
target_include_directories(${NAME} PUBLIC lunasvg)
target_include_directories(${NAME} PUBLIC chowdsp_utils/modules/dsp)
target_include_directories(${NAME} PUBLIC chowdsp_wdf/include)
target_include_directories(${NAME} PUBLIC sst-filters/include)
target_include_directories(${NAME} PUBLIC chowdsp_utils/modules/common)

target_compile_definitions(${NAME} PUBLIC IMGUI_USER_CONFIG="eightohat_imconfig.h")

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_subdirectory(chowdsp_utils)

# create static library based on these modules
setup_chowdsp_lib(chowdsp_lib               # Name of the static library
    MODULES chowdsp_sources chowdsp_dsp_utils chowdsp_filters # DSP modules that should be included in the library...
)

# link the static library to your project
target_link_libraries(${NAME} PRIVATE chowdsp_lib)

add_subdirectory(sst-filters)
target_link_libraries(${NAME} PUBLIC sst-filters)

add_subdirectory(lunasvg)
target_link_libraries(${NAME} PUBLIC lunasvg)

set(THISISJUSTSOTOFORCECMAKETOCOPYFILES ON)

add_custom_command(TARGET ${NAME} POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:${NAME}>/bin/${NAME}.lv2/resources
                   COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:${NAME}>/bin/${NAME}.vst3/Contents/Resources
                   COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:${NAME}>/bin/eightohat_resources
                   COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/res/ $<TARGET_FILE_DIR:${NAME}>/bin/${NAME}.lv2/resources
                   COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/res/ $<TARGET_FILE_DIR:${NAME}>/bin/${NAME}.vst3/Contents/Resources
                   COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/res/ $<TARGET_FILE_DIR:${NAME}>/bin/eightohat_resources)