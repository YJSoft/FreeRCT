# This file is part of FreeRCT.
# FreeRCT is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
# FreeRCT is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with FreeRCT. If not, see <http://www.gnu.org/licenses/>.
#

# Sanity check
IF(NOT FREERCT_DONE_TOP_LEVEL_CMAKE)
	message(FATAL_ERROR "Please run cmake on the top-level directory, not this one.")
ENDIF()

project(rcd_files)

# Make declaring the list shorter/nicer
set(FP "${CMAKE_SOURCE_DIR}/graphics/rcd")

#    Output file       Source file
list(APPEND RCDFILES
     "freerct.rcd"              "${FP}/freerct.txt"
     "path_8bpp64.rcd"          "${FP}/paths.txt"
     "entrance_exit.rcd"        "${FP}/entrance_exit.txt"
     "shops.rcd"                "${FP}/shops.txt"
     "gentle_thrill_rides.rcd"  "${FP}/gentle_thrill_rides.txt"
     "gui.rcd"                  "${FP}/gui.txt"
     "scenery.rcd"              "${FP}/scenery.txt"
     "tracks.rcd"               "${FP}/tracks.txt"
)

# Get lang files
file(GLOB LANGFILES "${FP}/lang/*.txt")

list(LENGTH RCDFILES RCDFILES_LENGTH)
math(EXPR RCDFILES_LENGTH_DIV_2 "${RCDFILES_LENGTH} / 2 - 1")

FOREACH(RCDFILE RANGE ${RCDFILES_LENGTH_DIV_2})
	math(EXPR INDEX_1 "${RCDFILE} * 2")
	math(EXPR INDEX_2 "${INDEX_1} + 1")
	list(GET RCDFILES ${INDEX_1} OUTFILE)
	list(GET RCDFILES ${INDEX_2} SRCFILE)

	set(OUT_DIR "${FRCT_BINARY_DIR}/rcd")
	add_custom_command(OUTPUT "${OUT_DIR}/${OUTFILE}" "${FP}/${OUTFILE}"
	                   COMMAND rcdgen ${LANGFILES} ${SRCFILE}
	                   COMMAND ${CMAKE_COMMAND} -E copy ${FP}/${OUTFILE} ${OUT_DIR}/${OUTFILE}
	                   COMMENT "Generating rcd files from ${SRCFILE}"
	                   DEPENDS ${SRCFILE} ${LANGFILES} rcdgen
	                   WORKING_DIRECTORY "${FP}"
	)

	list(APPEND OUTFILES "${OUT_DIR}/${OUTFILE}")
	set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${OUT_DIR}/${OUTFILE}")
ENDFOREACH(RCDFILE)

add_custom_target(rcd ALL DEPENDS ${OUTFILES})

install(DIRECTORY DESTINATION "${PACKAGING_DATA_DIR}/rcd")
install(FILES  ${OUTFILES} DESTINATION "${PACKAGING_DATA_DIR}/rcd")
