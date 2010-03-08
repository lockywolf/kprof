project(kprof)

cmake_minimum_required(VERSION 2.6)

set(KPROF_SRC
  diffmodel.cpp
  flatproxymodel.cpp
  gprofmodel.cpp
  hierarchicproxymodel.cpp
  objectproxymodel.cpp
  kprofwidget.cpp
  kproftoplevel.cpp
  main.cpp
  profilemodel.cpp
  profileproxyview.cpp
)

find_package(KDE4 REQUIRED)
include_directories(${KDE4_INCLUDES})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ..)

kde4_add_executable(kprof ${KPROF_SRC} ${MOC_SRC} ${UIC_SRC})

if(UNIX OR MINGW)
  set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -pg")
endif(UNIX OR MINGW)

if(NOT WIN32)
  set_target_properties(kprof PROPERTIES COMPILE_FLAGS
    "-ansi -pedantic -Wall -Wno-long-long")
endif(NOT WIN32)

target_link_libraries(kprof ${KDE4_KDEUI_LIBS} ${KDE4_KPARTS_LIBS})

install(TARGETS kprof DESTINATION ${BIN_INSTALL_DIR})
install(FILES kprofui.rc DESTINATION  ${DATA_INSTALL_DIR}/kprof)
