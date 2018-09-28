#
# This file is part of GreatFET
#

enable_language(C CXX ASM)

include(${CMAKE_CURRENT_LIST_DIR}/../cmake/dfu-util.cmake)

SET(PATH_GREATFET ${CMAKE_CURRENT_LIST_DIR}/../..)
SET(PATH_GREATFET_FIRMWARE ${PATH_GREATFET}/firmware)
SET(PATH_GREATFET_FIRMWARE_COMMON ${PATH_GREATFET_FIRMWARE}/common)
SET(LIBOPENCM3 ${PATH_GREATFET_FIRMWARE}/libopencm3)

# FIXME: pull these out into libgreat
SET(PATH_LIBGREAT ${CMAKE_CURRENT_LIST_DIR}/../../libgreat)
SET(PATH_LIBGREAT_FIRMWARE ${PATH_LIBGREAT}/firmware)
SET(PATH_LIBGREAT_FIRMWARE_DRIVERS ${PATH_LIBGREAT_FIRMWARE}/drivers)

# FIXME: make this configurable
SET(LIBGREAT_PLATFORM lpc43xx)

execute_process(
	COMMAND git log -n 1 --format=%h
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
	RESULT_VARIABLE GIT_VERSION_FOUND
	ERROR_QUIET
	OUTPUT_VARIABLE GIT_VERSION
	OUTPUT_STRIP_TRAILING_WHITESPACE
)
if (GIT_VERSION_FOUND)
	set(VERSION "unknown")
else (GIT_VERSION_FOUND)
	set(VERSION ${GIT_VERSION})
endif (GIT_VERSION_FOUND)

if(NOT DEFINED BOARD)
	set(BOARD GREATFET_ONE)
endif()

if(BOARD STREQUAL "GREATFET_ONE")
	set(MCU_PARTNO LPC4330)
endif()

if(BOARD STREQUAL "NXP_XPLORER")
	set(MCU_PARTNO LPC4330)
endif()

if(BOARD STREQUAL "RAD1O_BADGE")
	set(MCU_PARTNO LPC4330)
	set(BUILD_OUTPUT_TYPE "L0ADABLE")
endif()

if(NOT DEFINED SRC_M0)
	set(SRC_M0 "${PATH_GREATFET_FIRMWARE_COMMON}/m0_sleep.c")
endif(NOT DEFINED SRC_M0)

SET(GREATFET_OPTS "-D${BOARD} -DLPC43XX -D${MCU_PARTNO} -D'VERSION_STRING=\"git-${VERSION}\"'")

if(BUILD_OUTPUT_TYPE STREQUAL "L0ADABLE")
	SET(LDSCRIPT_M4 "-T${PATH_GREATFET_FIRMWARE_COMMON}/${MCU_PARTNO}_M4_memory_l0adable.ld -T${PATH_GREATFET_FIRMWARE_COMMON}/LPC43xx_l0adable.ld")
else()
	SET(LDSCRIPT_M4 "-T${PATH_GREATFET_FIRMWARE_COMMON}/${MCU_PARTNO}_M4_memory.ld -Tlibopencm3_lpc43xx_rom_to_ram.ld -T${PATH_GREATFET_FIRMWARE_COMMON}/LPC43xx_M4_M0_image_from_text.ld")
endif()

SET(LDSCRIPT_M4_DFU "-T${PATH_GREATFET_FIRMWARE_COMMON}/${MCU_PARTNO}_M4_memory.ld -Tlibopencm3_lpc43xx.ld -T${PATH_GREATFET_FIRMWARE_COMMON}/LPC43xx_M4_M0_image_from_text.ld")

SET(LDSCRIPT_M0 "-T${PATH_GREATFET_FIRMWARE_COMMON}/LPC43xx_M0_memory.ld -Tlibopencm3_lpc43xx_m0.ld")

SET(CFLAGS_COMMON "-Os -g3 -Wall -Wextra ${GREATFET_OPTS} -fno-common -MD -fno-builtin-printf")
SET(LDFLAGS_COMMON "-nostartfiles -Wl,--gc-sections")

if(V STREQUAL "1")
	SET(LDFLAGS_COMMON "${LDFLAGS_COMMON} -Wl,--print-gc-sections")
endif()

SET(CPUFLAGS_M0 "-mthumb -mcpu=cortex-m0 -mfloat-abi=soft")
SET(CFLAGS_M0 "-std=gnu99 ${CFLAGS_COMMON} ${CPUFLAGS_M0} -DLPC43XX_M0")
SET(CXXFLAGS_M0 "-std=gnu++0x ${CFLAGS_COMMON} ${CPUFLAGS_M0} -DLPC43XX_M0")
SET(LDFLAGS_M0 "${LDFLAGS_COMMON} ${CPUFLAGS_M0} ${LDSCRIPT_M0} -Xlinker -Map=m0.map")

SET(CPUFLAGS_M4 "-mthumb -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16")
SET(CFLAGS_M4 "-std=gnu99 ${CFLAGS_COMMON} ${CPUFLAGS_M4} -DLPC43XX_M4")
SET(CXXFLAGS_M4 "-std=gnu++0x ${CFLAGS_COMMON} ${CPUFLAGS_M4} -DLPC43XX_M4")
SET(LDFLAGS_M4 "${LDFLAGS_COMMON} ${CPUFLAGS_M4} ${LDSCRIPT_M4} -Xlinker -Map=m4.map")
SET(LDFLAGS_M4_DFU "${LDFLAGS_COMMON} ${CPUFLAGS_M4} ${LDSCRIPT_M4_DFU} -Xlinker -Map=m4.map")

set(BUILD_SHARED_LIBS OFF)

include_directories("${LIBOPENCM3}/include/")
include_directories("${PATH_GREATFET_FIRMWARE_COMMON}")

# FIXME: pull out into libgreat, probably?
include_directories("${PATH_LIBGREAT_FIRMWARE}/include")


macro(DeclareTargets)
	SET(SRC_M4
		${SRC_M4}

		#fixme: pull into libgreat
		${PATH_LIBGREAT_FIRMWARE}/platform/lpc43xx/crt0.c

		${PATH_GREATFET_FIRMWARE_COMMON}/greatfet_core.c
		${PATH_GREATFET_FIRMWARE_COMMON}/spiflash_target.c
		${PATH_GREATFET_FIRMWARE_COMMON}/spiflash.c
		${PATH_GREATFET_FIRMWARE_COMMON}/spi_ssp.c
		${PATH_GREATFET_FIRMWARE_COMMON}/i2c_bus.c
		${PATH_GREATFET_FIRMWARE_COMMON}/i2c_lpc.c
		${PATH_GREATFET_FIRMWARE_COMMON}/gpio_lpc.c
		${PATH_GREATFET_FIRMWARE_COMMON}/gpio_int.c
		${PATH_GREATFET_FIRMWARE_COMMON}/sgpio.c
		${PATH_GREATFET_FIRMWARE_COMMON}/debug.c
		${PATH_GREATFET_FIRMWARE_COMMON}/time.c
	)

	configure_file(
		${PATH_GREATFET_FIRMWARE}/cmake/m0_bin.s.cmake
		m0_bin.s
	)

	link_directories(
		"${PATH_GREATFET_FIRMWARE_COMMON}"
		"${LIBOPENCM3}/lib"
		"${LIBOPENCM3}/lib/lpc43xx"
		"${CMAKE_INSTALL_PREFIX}/lib/armv7e-m/fpu"
	)

	add_executable(${PROJECT_NAME}_m0.elf ${SRC_M0})

	target_link_libraries(
		${PROJECT_NAME}_m0.elf
		c
		nosys
		opencm3_lpc43xx_m0
	)

	set_target_properties(${PROJECT_NAME}_m0.elf PROPERTIES COMPILE_FLAGS "${CFLAGS_M0}")
	set_target_properties(${PROJECT_NAME}_m0.elf PROPERTIES LINK_FLAGS "${LDFLAGS_M0}")

	add_custom_target(
		${PROJECT_NAME}_m0.bin
		DEPENDS ${PROJECT_NAME}_m0.elf
		COMMAND ${CMAKE_OBJCOPY} -Obinary ${PROJECT_NAME}_m0.elf ${PROJECT_NAME}_m0.bin
	)

	# Object files to be linked for both DFU and SPI flash versions
	add_library(OBJ_FILES_${PROJECT_NAME} OBJECT ${SRC_M4} m0_bin.s)
	set_target_properties(OBJ_FILES_${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "${CFLAGS_M4}")
	add_dependencies(OBJ_FILES_${PROJECT_NAME} ${PROJECT_NAME}_m0.bin)

	# SPI flash version
	add_executable(${PROJECT_NAME}.elf $<TARGET_OBJECTS:OBJ_FILES_${PROJECT_NAME}>)

	target_link_libraries(
		${PROJECT_NAME}.elf
		c
		nosys
		opencm3_lpc43xx
		m
	)

	#set_target_properties(${PROJECT_NAME}.elf PROPERTIES COMPILE_FLAGS "${CFLAGS_M4}")
	set_target_properties(${PROJECT_NAME}.elf PROPERTIES LINK_FLAGS "${LDFLAGS_M4}")

	add_custom_target(
		${PROJECT_NAME}.bin ALL
		DEPENDS ${PROJECT_NAME}.elf
		COMMAND ${CMAKE_OBJCOPY} -Obinary ${PROJECT_NAME}.elf ${PROJECT_NAME}.bin
	)

	# DFU - using a differnet LD script to run directly from RAM
	add_executable(${PROJECT_NAME}_dfu.elf $<TARGET_OBJECTS:OBJ_FILES_${PROJECT_NAME}>)

	target_link_libraries(
		${PROJECT_NAME}_dfu.elf
		c
		nosys
		opencm3_lpc43xx
		m
	)

	#set_target_properties(${PROJECT_NAME}_dfu.elf PROPERTIES COMPILE_FLAGS "${CFLAGS_M4}")
	set_target_properties(${PROJECT_NAME}_dfu.elf PROPERTIES LINK_FLAGS "${LDFLAGS_M4_DFU}")

	add_custom_target(
		${PROJECT_NAME}_dfu.bin
		DEPENDS ${PROJECT_NAME}_dfu.elf
		COMMAND ${CMAKE_OBJCOPY} -Obinary ${PROJECT_NAME}_dfu.elf ${PROJECT_NAME}_dfu.bin
	)

	add_custom_target(
		${PROJECT_NAME}.dfu ${DFU_ALL}
		DEPENDS ${PROJECT_NAME}_dfu.bin
		COMMAND rm -f _tmp.dfu _header.bin
		COMMAND cp ${PROJECT_NAME}_dfu.bin _tmp.dfu
		COMMAND ${DFU_COMMAND}
		COMMAND python ${PATH_GREATFET_FIRMWARE}/dfu.py ${PROJECT_NAME}
		COMMAND cat _header.bin _tmp.dfu >${PROJECT_NAME}.dfu
	)

	# Program / flash targets
	add_custom_target(
		${PROJECT_NAME}-flash
		DEPENDS ${PROJECT_NAME}.bin
		COMMAND greatfet_firmware -Rw ${PROJECT_NAME}.bin
	)

	add_custom_target(
		${PROJECT_NAME}-program
		DEPENDS ${PROJECT_NAME}.dfu
		COMMAND dfu-util --device 1fc9:000c --alt 0 --download ${PROJECT_NAME}.dfu
	)
endmacro()
