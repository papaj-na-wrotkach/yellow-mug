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
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
set(CMAKE_CXX_MODULE_STD ON)
set(CMAKE_EXPERIMENTAL_CXX_MODULE_DYNDEP ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_compile_options(
	"$<$<CONFIG:Debug>:$<IF:$<CXX_COMPILER_ID:MSVC>,/Zi,-g>>"
	"$<$<CONFIG:Release>:$<IF:$<CXX_COMPILER_ID:MSVC>,/O2,-O3>>"
	"$<$<CONFIG:Release>:$<IF:$<CXX_COMPILER_ID:MSVC>,/fp:fast,-ffast-math>>"
)

# dependencies
include(FetchContent)

set(GLFW_DIR ${CMAKE_CURRENT_LIST_DIR}/vendor/glfw)
set(IMGUI_DIR ${CMAKE_CURRENT_LIST_DIR}/vendor/imgui)
set(IMNODEFLOW_DIR ${CMAKE_CURRENT_LIST_DIR}/vendor/ImNodeFlow)
set(IMGUI_ZOOMABLE_IMAGE_DIR ${CMAKE_CURRENT_LIST_DIR}/vendor/imgui_zoomable_image)
set(STB_IMAGE_DIR ${CMAKE_CURRENT_LIST_DIR}/vendor/stb_image)

FetchContent_Declare(glfw
	GIT_REPOSITORY "https://github.com/glfw/glfw.git"
	GIT_TAG "3.4"
	SOURCE_DIR ${GLFW_DIR}
	GIT_SHALLOW TRUE
)

FetchContent_Declare(ImGui
	 GIT_REPOSITORY "https://github.com/ocornut/imgui.git"
	 GIT_TAG "v1.92.8-docking"
	 SOURCE_DIR ${IMGUI_DIR}
	 GIT_SHALLOW TRUE
)

FetchContent_Declare(ImNodeFlow
	 GIT_REPOSITORY "https://github.com/Fattorino/ImNodeFlow.git"
	 GIT_TAG "e4cb098"
	 SOURCE_DIR ${IMNODEFLOW_DIR}
	 GIT_SHALLOW TRUE
)

FetchContent_Declare(imgui_zoomable_image
	 GIT_REPOSITORY "https://github.com/danielm5/imgui_zoomable_image.git"
	 GIT_TAG "v0.1.0"
	 SOURCE_DIR ${IMGUI_ZOOMABLE_IMAGE_DIR}
	 GIT_SUBMODULES ""
	 SOURCE_SUBDIR  "" # do not use CMakeLists.txt from the dependency
	 GIT_SHALLOW TRUE
)

FetchContent_Declare(stb_image
	 GIT_REPOSITORY "https://github.com/nothings/stb.git"
	 GIT_TAG "31c1ad3"
	 SOURCE_DIR ${STB_IMAGE_DIR}
	 GIT_SHALLOW TRUE
)

set(NAME yellow-mug)

project(${NAME} LANGUAGES C CXX)

# For Doxygen
set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
set(CMAKE_C_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES})

# GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

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
	PRIVATE
		src/stb_image.cpp
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
		src/nodes/crop.ixx
		src/nodes/grayscale.ixx
		src/nodes/mask_blend.ixx
		src/processors/blur.ixx
		src/processors/crop.ixx
		src/processors/grayscale.ixx
		src/processors/invert.ixx
		src/processors/mask_blend.ixx
		src/processors/processor.ixx
		src/processors/source.ixx
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

find_package(Doxygen)
if(DOXYGEN_FOUND)
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

	FetchContent_Declare(doxygen-awesome-css
		GIT_REPOSITORY "https://github.com/jothepro/doxygen-awesome-css.git"
		GIT_TAG        "v2.4.2"
		GIT_SHALLOW    TRUE
	)
	FetchContent_MakeAvailable(doxygen-awesome-css)
	FetchContent_GetProperties(doxygen-awesome-css SOURCE_DIR AWESOME_CSS_DIR)

	set(DOXYGEN_PROJECT_NAME "Yellow Mug")
	set(DOXYGEN_PROJECT_BRIEF "A visual node-editor-based image processing pipeline")
	set(DOXYGEN_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/docs")

	set(DOXYGEN_EXTRACT_ALL YES)
	set(DOXYGEN_EXTRACT_PRIVATE YES)
	set(DOXYGEN_EXTRACT_PRIV_VIRTUAL YES)
	set(DOXYGEN_EXTRACT_PACKAGE YES)
	set(DOXYGEN_EXTRACT_STATIC YES)
	set(DOXYGEN_SORT_MEMBERS_CTORS_1ST YES)
	set(DOXYGEN_RECURSIVE YES)
	set(DOXYGEN_CLANG_ASSISTED_PARSING YES)
	set(DOXYGEN_HAVE_DOT YES)
	set(DOXYGEN_UML_LOOK YES)
	set(DOXYGEN_UML_LIMIT_NUM_FIELDS 20)
	set(DOXYGEN_TEMPLATE_RELATIONS YES)
	set(DOXYGEN_CALL_GRAPH YES)
	set(DOXYGEN_CALLER_GRAPH YES)
	set(DOXYGEN_DOT_IMAGE_FORMAT svg)
	set(DOXYGEN_INTERACTIVE_SVG YES)
	set(DOXYGEN_DOT_MULTI_TARGETS YES)
	set(DOXYGEN_TAGFILES "${TAG_FILE}=https://cppreference.com/w/")
	set(DOXYGEN_EXCLUDE_SYMBOLS "std::*")
	set(DOXYGEN_GENERATE_LATEX YES)
	set(DOXYGEN_LATEX_CMD_NAME pdflatex)
	set(DOXYGEN_GENERATE_TREEVIEW YES)
	set(DOXYGEN_DISABLE_INDEX NO)
	set(DOXYGEN_FULL_SIDEBAR NO)
	set(DOXYGEN_HTML_COLORSTYLE LIGHT)
	set(DOXYGEN_HTML_EXTRA_STYLESHEET
		"${AWESOME_CSS_DIR}/doxygen-awesome.css"
		"${AWESOME_CSS_DIR}/doxygen-awesome-sidebar-only.css")
	set(DOXYGEN_USE_MDFILE_AS_MAINPAGE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")

	doxygen_add_docs(docs
		"${CMAKE_CURRENT_SOURCE_DIR}/src"
		"${CMAKE_CURRENT_SOURCE_DIR}/README.md"
		COMMENT "Generating API documentation with Doxygen"
	)

	find_package(LATEX)
	if(PDFLATEX_COMPILER)
		add_custom_target(docs-pdf
			COMMAND ${CMAKE_COMMAND} -E make_directory "${DOXYGEN_OUTPUT_DIRECTORY}/pdf"
			COMMAND ${PDFLATEX_COMPILER} -interaction=nonstopmode -output-directory "${DOXYGEN_OUTPUT_DIRECTORY}/pdf" refman.tex
			COMMAND ${PDFLATEX_COMPILER} -interaction=nonstopmode -output-directory "${DOXYGEN_OUTPUT_DIRECTORY}/pdf" refman.tex
			WORKING_DIRECTORY "${DOXYGEN_OUTPUT_DIRECTORY}/latex"
			DEPENDS docs
			COMMENT "Compiling LaTeX docs into PDF with pdflatex"
			VERBATIM
		)

	else()
		set(DOXYGEN_GENERATE_LATEX NO)
		message(STATUS "PDF LaTeX compiler not found — skipping docs-pdf target")
	endif()
else()
	message(STATUS "Doxygen not found — skipping docs target")
endif()
