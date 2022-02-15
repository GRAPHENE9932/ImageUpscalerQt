cmake_minimum_required(VERSION 3.0)
project(ImageUpscalerQt)

# Enable C++20
set(CMAKE_CXX_STANDARD 20)

if (MSVC)
    set(CMAKE_CXX_FLAGS "/permissive-") # Disable the weird errors.
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    # vcpkg integration.
    add_custom_target(run
        COMMAND ${PROJECT_NAME}
        DEPENDS ${PROJECT_NAME}
        WORKING_DIRECTORY ${CMAKE_PROJECT_DIR}
    )
    # Copy DLLs in the output directory.
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

    message(STATUS "vcpkg and Windows set up.")
endif()

# Find includes in corresponding build directories.
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
# Instruct CMake to create code from Qt designer ui files.
set(CMAKE_AUTOUIC ON)

# Find source files
file(GLOB_RECURSE imageupscalerqt_SRC ${PROJECT_SOURCE_DIR}/src/*.cpp)
find_package(Qt5Widgets CONFIG REQUIRED) # Find the QtWidgets library.
# Add Qt resources
qt5_add_resources(imageupscalerqt_SRC res/resources.qrc)

# Add executable (also for Windows).
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    add_executable(imageupscalerqt WIN32 ${imageupscalerqt_SRC})
else()
    add_executable(imageupscalerqt ${imageupscalerqt_SRC})
endif()

# OpenImageIO.
find_package(OpenImageIO CONFIG REQUIRED)
target_link_libraries(imageupscalerqt OpenImageIO::OpenImageIO)
if (DEFINED OPENIMAGEIO_INCLUDE_DIR)
    message(STATUS "Using the OPENIMAGEIO_INCLUDE_DIR argument")
    target_include_directories(imageupscalerqt PUBLIC ${OPENIMAGEIO_INCLUDE_DIR})
endif()

# oneDNN (oneAPI, DNNL).
find_package(dnnl REQUIRED)
target_link_libraries(imageupscalerqt DNNL::dnnl)
if (DEFINED DNNL_INCLUDE_DIR)
    message(STATUS "Using the DNNL_INCLUDE_DIR argument")
    target_include_directories(imageupscalerqt PUBLIC ${DNNL_INCLUDE_DIR})
endif()

# pthread.
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(STATUS "Using pthread.")
    target_link_libraries(imageupscalerqt pthread)
endif()

# Qt.
target_link_libraries(imageupscalerqt Qt5::Widgets) # Use the Widgets module from Qt 5.
if (DEFINED QT5_INCLUDE_DIR)
    target_include_directories(imageupscalerqt PUBLIC ${QT5_INCLUDE_DIR})
endif()

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Install the executable.
install(TARGETS imageupscalerqt DESTINATION bin)
install(FILES com.graphene9932.ImageUpscalerQt.desktop DESTINATION share/applications/com.graphene9932.ImageUpscalerQt.desktop)
install(FILES res/icon.png DESTINATION share/icons/hicolor/scalable/apps/com.graphene9932.ImageUpscalerQt.png)
