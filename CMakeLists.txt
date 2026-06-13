cmake_minimum_required(VERSION 3.30.0)

# set the correct experimental gate value depending on CMake version
if(CMAKE_VERSION VERSION_GREATER_EQUAL "4.3.0")
	set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "451f2fe2-a8a2-47c3-bc32-94786d8fc91b")
elseif(CMAKE_VERSION VERSION_GREATER_EQUAL "4.1.0")
	set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "d0edc3af-4c50-42ea-a356-e2862fe7a444")
elseif(CMAKE_VERSION VERSION_GREATER_EQUAL "4.0.0")
	set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "a9e1cf81-9932-4810-974b-6eccaf14e457")
elseif(CMAKE_VERSION VERSION_GREATER_EQUAL "3.30.0")
	set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "0e5b6991-d74f-4b3d-a41c-cf096e0b2508")
endif()
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
# set(CMAKE_CXX_SCAN_FOR_MODULES ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
set(CMAKE_CXX_MODULE_STD ON)
set(CMAKE_EXPERIMENTAL_CXX_MODULE_DYNDEP ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -ffast-math")

# dependencies
include(FetchContent)

set(IMGUI_DIR ${CMAKE_CURRENT_LIST_DIR}/vendor/imgui)
set(IMNODEFLOW_DIR ${CMAKE_CURRENT_LIST_DIR}/vendor/ImNodeFlow)

FetchContent_Declare(imgui
	 GIT_REPOSITORY "https://github.com/ocornut/imgui.git"
	 GIT_TAG "docking"
	 SOURCE_DIR ${IMGUI_DIR}
)

FetchContent_Declare(ImNodeFlow
	 GIT_REPOSITORY "https://github.com/Fattorino/ImNodeFlow.git"
	 GIT_TAG "master"
	 SOURCE_DIR ${IMNODEFLOW_DIR}
)

set(NAME yellow-mug)

project(${NAME} LANGUAGES C CXX)

# ImGui
FetchContent_MakeAvailable(imgui)

# ImGui::Core
add_library(imgui_core STATIC
	${IMGUI_DIR}/imgui.cpp
	${IMGUI_DIR}/imgui_draw.cpp
	${IMGUI_DIR}/imgui_tables.cpp
	${IMGUI_DIR}/imgui_widgets.cpp
	${IMGUI_DIR}/misc/cpp/imgui_stdlib.cpp
)
target_include_directories(imgui_core PUBLIC ${IMGUI_DIR})
add_library(ImGui::Core ALIAS imgui_core)

# ImGui::GLFW
find_package(glfw3 REQUIRED)
add_library(imgui_glfw STATIC ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp)
target_include_directories(imgui_glfw PUBLIC ${IMGUI_DIR}/backends)
target_link_libraries(imgui_glfw PUBLIC ImGui::Core glfw)
add_library(ImGui::GLFW ALIAS imgui_glfw)

# ImGui::OpenGL3
find_package(OpenGL REQUIRED)
add_library(imgui_opengl3 STATIC ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp)
target_include_directories(imgui_opengl3 PUBLIC ${IMGUI_DIR}/backends)
target_link_libraries(imgui_opengl3 PUBLIC ImGui::Core OpenGL::GL)
add_library(ImGui::OpenGL3 ALIAS imgui_opengl3)

# ImNodeFlow
FetchContent_MakeAvailable(ImNodeFlow)
add_library(imnodeflow STATIC ${IMNODEFLOW_DIR}/src/ImNodeFlow.cpp)
target_compile_definitions(imnodeflow PUBLIC IMGUI_DEFINE_MATH_OPERATORS)
target_include_directories(imnodeflow PUBLIC
	${IMNODEFLOW_DIR}/include
	${IMNODEFLOW_DIR}/src
)
target_link_libraries(imnodeflow PUBLIC ImGui::Core)
add_library(ImNodeFlow::ImNodeFlow ALIAS imnodeflow)

# main executable
add_executable(${NAME} src/main.cpp)
target_sources(${NAME}
    PUBLIC
    FILE_SET CXX_MODULES FILES
    src/app.ixx
)
target_link_libraries(${NAME} PRIVATE
	ImGui::Core
	ImGui::GLFW
	ImGui::OpenGL3
	ImNodeFlow::ImNodeFlow
)
