cmake_minimum_required(VERSION 3.0)
project(ImageUpscalerQt)

#Enable C++20
set(CMAKE_CXX_STANDARD 20)

#Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
#Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
#Instruct CMake to create code from Qt designer ui files
set(CMAKE_AUTOUIC ON)

#Find source files
file(GLOB_RECURSE imageupscalerqt_SRC ${PROJECT_SOURCE_DIR}/src/*.cpp)
add_executable(imageupscalerqt ${imageupscalerqt_SRC})

#OpenImageIO
find_library(OIIO "OpenImageIO" REQUIRED)
target_link_libraries(imageupscalerqt ${OIIO})
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

#Qt
find_package(Qt5Widgets CONFIG REQUIRED) #Find the QtWidgets library
target_link_libraries(imageupscalerqt Qt5::Widgets) #Use the Widgets module from Qt 5.

#Install the executable
install(TARGETS imageupscalerqt DESTINATION bin)

#Copy resources
file(COPY src/res/ DESTINATION .)
