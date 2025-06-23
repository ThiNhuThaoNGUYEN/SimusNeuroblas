# file cmake_generate_header.cmake.in
# generate header files 

cmake_minimum_required(VERSION 3.18)
# Probably OK with lower versions but needs testing

# Locate "--" in argument list
set(i_double_dash 0)
while(i_double_dash LESS ${CMAKE_ARGC})
  if(${CMAKE_ARGV${i_double_dash}} STREQUAL "--")
    break()
  endif()
  math(EXPR i_double_dash "${i_double_dash} + 1")
endwhile()
# Exit if "--" not found
if(${i_double_dash} EQUAL ${CMAKE_ARGC})
  message(FATAL_ERROR "no `--` found in argument list")
endif()
# Exit if the number of remaining arguments is not correct
math(EXPR nb_remaining_args "${CMAKE_ARGC} - ${i_double_dash} - 1")
if(NOT ${nb_remaining_args} EQUAL 3)
  message(FATAL_ERROR "script requires 3 arguments, ${nb_remaining_args} provided")
endif()

# Get arg values
math(EXPR i_arg "${i_double_dash} + 1")
set(HEADER_INPUT_DIR ${CMAKE_ARGV${i_arg}})
math(EXPR i_arg "${i_arg} + 1")
set(VAL_INPUT_DIR ${CMAKE_ARGV${i_arg}})
math(EXPR i_arg "${i_arg} + 1")
set(FILE_CANONICAL_NAME ${CMAKE_ARGV${i_arg}})

# Read input values from provided value file
file(STRINGS "${VAL_INPUT_DIR}/${FILE_CANONICAL_NAME}.values.in" lines)
foreach(line ${lines})
  string(STRIP "${line}" val)
  # check that line is a valid identifier
  if (NOT val MATCHES "^[a-zA-Z_]+[a-zA-Z_0-9]*")
    message(SEND_ERROR "In ${VAL_INPUT_DIR}/${FILE_CANONICAL_NAME}.values.in, name ${val} is not a valid identifier")
  else()
    list(APPEND values ${val})
  endif()
endforeach()

# Copy the content of .h.in file with injection on #VALUES and #MAP_VALUES tags
file(WRITE "${FILE_CANONICAL_NAME}.h" "")
file(STRINGS "${HEADER_INPUT_DIR}/${FILE_CANONICAL_NAME}.h.in" header_lines)
foreach(header_line ${header_lines})
  string(STRIP "${header_line}" stripped_line)
  if ("${stripped_line}" STREQUAL "#VALUES")
    foreach(value ${values})
      file(APPEND "${FILE_CANONICAL_NAME}.h"
        "  " ${value} ",\n")
    endforeach()
  elseif ("${stripped_line}" STREQUAL "#MAP_VALUES")
    foreach(value ${values})
      file(APPEND "${FILE_CANONICAL_NAME}.h"
        "  { ${FILE_CANONICAL_NAME}::${value}, \"${value}\" },\n" )
    endforeach()
  else()
    file(APPEND "${FILE_CANONICAL_NAME}.h" "${header_line}\n")
  endif()
endforeach()
