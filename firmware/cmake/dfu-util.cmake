#
# This file is part of GreatFET
#

execute_process(
	COMMAND dfu-suffix -V
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
	RESULT_VARIABLE DFU_NOT_FOUND
	ERROR_QUIET
	OUTPUT_VARIABLE DFU_VERSION_STRING
	OUTPUT_STRIP_TRAILING_WHITESPACE
)

set(DFU_ALL "")
if(NOT DFU_NOT_FOUND)
	string(REGEX REPLACE ".*([0-9]+)\\.[0-9]+.*" "\\1" DFU_VERSION_MAJOR "${DFU_VERSION_STRING}")
	string(REGEX REPLACE ".*[0-9]+\\.([0-9])+.*" "\\1" DFU_VERSION_MINOR "${DFU_VERSION_STRING}")
	MESSAGE( STATUS "DFU utils version: " ${DFU_VERSION_MAJOR} "." ${DFU_VERSION_MINOR})
	execute_process(
	        COMMAND dfu-prefix -V
		WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
		RESULT_VARIABLE DFU_PREFIX_NOT_FOUND
		ERROR_QUIET
		OUTPUT_VARIABLE DFU_PREFIX_VERSION_STRING
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)
	if(DFU_PREFIX_NOT_FOUND)
		set(DFU_COMMAND dfu-suffix --vid=0x1fc9 --pid=0x000c --did=0x0 -s 0 -a _tmp.dfu)
	else(DFU_PREFIX_NOT_FOUND)
		set(DFU_COMMAND dfu-suffix --vid=0x1fc9 --pid=0x000c --did=0x0 -a _tmp.dfu && dfu-prefix -s 0 -a _tmp.dfu)
	endif(DFU_PREFIX_NOT_FOUND)
    set(DFU_ALL "ALL")
else(NOT DFU_NOT_FOUND)
	MESSAGE(STATUS "dfu-suffix not found: not building DFU file")
endif(NOT DFU_NOT_FOUND)



    