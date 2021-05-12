# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles/imageupscalerqt_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/imageupscalerqt_autogen.dir/ParseCache.txt"
  "imageupscalerqt_autogen"
  )
endif()
