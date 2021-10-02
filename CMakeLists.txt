cmake_minimum_required(VERSION 3.0)
project(ImageUpscalerQt)

#Enable C++17 for MSVC
if (MSVC)
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_FLAGS "/permissive-") #Disable weird errors
#And C++17 for GCC
elseif(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_STANDARD 17)
else()
	message(SEND_ERROR "Unknown compiler")
endif()

#Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
#Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
#Instruct CMake to create code from Qt designer ui files
set(CMAKE_AUTOUIC ON)

#Find source files
file(GLOB_RECURSE imageupscalerqt_SRC ${PROJECT_SOURCE_DIR}/src/*.cpp)
find_package(Qt5Widgets CONFIG REQUIRED) #Find the QtWidgets library
#Add Qt resources
qt5_add_resources(imageupscalerqt_SRC res/resources.qrc)

#Add executable (also for windows WIN32)
if(CMAKE_COMPILER_IS_GNUCXX)
	add_executable(imageupscalerqt ${imageupscalerqt_SRC})
elseif(MSVC)
	add_executable(imageupscalerqt WIN32 ${imageupscalerqt_SRC})
else()
	message(SEND_ERROR "Unknown compiler")
endif()

#OpenImageIO
find_package(OpenImageIO CONFIG REQUIRED)
target_link_libraries(imageupscalerqt OpenImageIO::OpenImageIO OpenImageIO::OpenImageIO_Util)

#LibTorch
find_package(Torch REQUIRED)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")
target_link_libraries(imageupscalerqt "${TORCH_LIBRARIES}")

#Qt
target_link_libraries(imageupscalerqt Qt5::Widgets) #Use the Widgets module from Qt 5.

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

#Install the executable
install(TARGETS imageupscalerqt DESTINATION bin)
