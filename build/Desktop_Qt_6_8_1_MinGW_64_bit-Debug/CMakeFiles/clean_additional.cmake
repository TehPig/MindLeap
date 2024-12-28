# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "Anki_autogen"
  "CMakeFiles\\Anki_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\Anki_autogen.dir\\ParseCache.txt"
  )
endif()
