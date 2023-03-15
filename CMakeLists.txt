cmake_minimum_required(VERSION 3.6 FATAL_ERROR)

project(wx_cmake_template_core LANGUAGES CXX)

set(STAGED_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/wxWidgets-prefix/src/wxWidgets)

include(ExternalProject)
ExternalProject_Add(wxWidgets
	URL ${CMAKE_BINARY_DIR}/../wxwidgets.7z
	CONFIGURE_COMMAND ""
	BUILD_COMMAND ""
	INSTALL_COMMAND ""
)

set(wxWidgets_ROOT_DIR ${STAGED_INSTALL_PREFIX})
file(TO_NATIVE_PATH "${wxWidgets_ROOT_DIR}" wxWidgets_ROOT_DIR)
set(wxWidgets_ROOT_DIR ${wxWidgets_ROOT_DIR} CACHE INTERNAL "wxWidgets installation dir")

set(ENV_WX_CONFIG ${STAGED_INSTALL_PREFIX}/bin/wx-config)
file(TO_NATIVE_PATH "${ENV_WX_CONFIG}" ENV_WX_CONFIG)
set(ENV_WX_CONFIG ${ENV_WX_CONFIG} CACHE INTERNAL "wx-config dir")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(wxWidgets_USE_STATIC 1)

find_package(wxWidgets)

set(SRCS src/main.cpp)

include(${wxWidgets_USE_FILE})

add_executable(main WIN32 ${SRCS} src/main.exe.manifest)
add_dependencies(main wxWidgets)

target_link_libraries(main PRIVATE ${wxWidgets_LIBRARIES})