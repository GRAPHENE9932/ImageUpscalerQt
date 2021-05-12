cmake_minimum_required(VERSION 3.0)
project(ImageUpscalerQt)

#Enable c++20
set(CMAKE_CXX_STANDARD 20)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
# Instruct CMake to create code from Qt designer ui files
set(CMAKE_AUTOUIC ON)

# Find the QtWidgets library
find_package(Qt5Widgets CONFIG REQUIRED)

#Find source files
file(GLOB_RECURSE imageupscalerqt_SRC ${PROJECT_SOURCE_DIR}/src/*.cpp)
#set(imageupscalerqt_SRC
#  src/main.cpp
#  src/imageupscalerqt.cpp
#)

# Tell CMake to create the helloworld executable
add_executable(imageupscalerqt ${imageupscalerqt_SRC})

# Use the Widgets module from Qt 5.
target_link_libraries(imageupscalerqt Qt5::Widgets)

# Install the executable
install(TARGETS imageupscalerqt DESTINATION bin)
