# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles/CalFluxNeutron_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/CalFluxNeutron_autogen.dir/ParseCache.txt"
  "CalFluxNeutron_autogen"
  )
endif()
