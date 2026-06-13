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
set(IMGUI_ZOOMABLE_IMAGE_DIR ${CMAKE_CURRENT_LIST_DIR}/vendor/imgui_zoomable_image)
set(STB_IMAGE_DIR ${CMAKE_CURRENT_LIST_DIR}/vendor/stb_image)

FetchContent_Declare(ImGui
	 GIT_REPOSITORY "https://github.com/ocornut/imgui.git"
	 GIT_TAG "docking"
	 SOURCE_DIR ${IMGUI_DIR}
	 GIT_SHALLOW TRUE
)

FetchContent_Declare(ImNodeFlow
	 GIT_REPOSITORY "https://github.com/Fattorino/ImNodeFlow.git"
	 GIT_TAG "master"
	 SOURCE_DIR ${IMNODEFLOW_DIR}
	 GIT_SHALLOW TRUE
)

FetchContent_Declare(imgui_zoomable_image
	 GIT_REPOSITORY "https://github.com/danielm5/imgui_zoomable_image.git"
	 GIT_TAG "main"
	 SOURCE_DIR ${IMGUI_ZOOMABLE_IMAGE_DIR}
	 GIT_SUBMODULES ""
	 SOURCE_SUBDIR  "" # do not use CMakeLists.txt from the dependency
	 GIT_SHALLOW TRUE
)

FetchContent_Declare(stb_image
	 GIT_REPOSITORY "https://github.com/nothings/stb.git"
	 GIT_TAG "master"
	 SOURCE_DIR ${STB_IMAGE_DIR}
	 GIT_SHALLOW TRUE
)

set(NAME yellow-mug)

project(${NAME} LANGUAGES C CXX)

# For Doxygen
set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
set(CMAKE_C_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES})

# ImGui
FetchContent_MakeAvailable(ImGui)

# ImGui::Core
add_library(imgui_core STATIC
	${IMGUI_DIR}/imgui.cpp
	${IMGUI_DIR}/imgui_draw.cpp
	${IMGUI_DIR}/imgui_tables.cpp
	${IMGUI_DIR}/imgui_widgets.cpp
	${IMGUI_DIR}/misc/cpp/imgui_stdlib.cpp
)
target_include_directories(imgui_core PUBLIC ${IMGUI_DIR} ${IMGUI_DIR}/misc/cpp/)
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

# imgui_zoomable_image
FetchContent_MakeAvailable(imgui_zoomable_image)
add_library(imgui_zoomable_image INTERFACE)
target_include_directories(imgui_zoomable_image INTERFACE ${IMGUI_ZOOMABLE_IMAGE_DIR})
target_link_libraries(imgui_zoomable_image INTERFACE ImGui::Core)
add_library(ImGuiImage::Zoomable ALIAS imgui_zoomable_image)

# main executable
FetchContent_MakeAvailable(stb_image)
add_executable(${NAME} src/main.cpp)
target_sources(${NAME}
	PUBLIC
	FILE_SET CXX_MODULES FILES
		src/app.ixx
		src/core.ixx
		src/nodes.ixx
		src/processors.ixx
		src/core/frame.ixx
		src/nodes/invert.ixx
		src/nodes/processor.ixx
		src/nodes/source.ixx
		src/nodes/target.ixx
		src/nodes/blur.ixx
		src/nodes/mask_blend.ixx
		src/processors/blur.ixx
		src/processors/invert.ixx
		src/processors/mask_blend.ixx
		src/processors/processor.ixx
		src/processors/source.ixx
		src/utility/stb_image.ixx
)
target_include_directories(${NAME} PRIVATE ${STB_IMAGE_DIR})
target_link_libraries(${NAME}
	PRIVATE
		ImGui::Core
		ImGui::GLFW
		ImGui::OpenGL3
		ImNodeFlow::ImNodeFlow
		ImGuiImage::Zoomable
)

# Doxygen documentation
set(TAG_URL "https://upload.cppreference.com/images/f/f8/cppreference-doxygen-web.tag.xml")
set(TAG_FILE "${CMAKE_BINARY_DIR}/cppreference-doxygen-web.tag.xml")

if(NOT EXISTS "${TAG_FILE}")
    message(STATUS "Downloading cppreference Doxygen tag file…")
    file(DOWNLOAD "${TAG_URL}" "${TAG_FILE}" STATUS download_status)
    list(GET download_status 0 status_code)
    if(NOT status_code EQUAL 0)
        message(WARNING "Failed to download tag file: ${download_status}")
    endif()
endif()
set(DOXYGEN_TAGFILES "${TAG_FILE}=https://cppreference.com/w/")

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in"
    "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile"
    @ONLY
)

find_package(Doxygen REQUIRED)
add_custom_target(docs
    COMMAND ${DOXYGEN_EXECUTABLE} "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    COMMENT "Generating API documentation with Doxygen"
    VERBATIM
)
