/*
 * Copyright 2012 Jared Boone <jared@sharebrained.com>
 * Copyright 2013 Benjamin Vernoux <titanmkd@gmail.com>
 * Copyright 2013 Michael Ossmann <mike@ossmann.com>
 *
 * This file is part of GreatFET.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#include <greatfet.h>

#include <stdio.h>
#include <stdlib.h>

int main(void)
{
	int result = GREATFET_SUCCESS;
	uint8_t board_id = BOARD_ID_INVALID;
	char version[255 + 1];
	read_partid_serialno_t read_partid_serialno;
	greatfet_device_list_t *list;
	greatfet_device* device;
	int i;

	result = greatfet_init();
	if (result != GREATFET_SUCCESS) {
		fprintf(stderr, "greatfet_init() failed: %s (%d)\n",
				greatfet_error_name(result), result);
		return EXIT_FAILURE;
	}
	
	list = greatfet_device_list();
	
	if (list->devicecount < 1 ) {
		printf("No GreatFET boards found.\n");
		return EXIT_FAILURE;
	}
	
	for (i = 0; i < list->devicecount; i++) {
		if (i > 0)
			printf("\n");
			
		printf("Found GreatFET board %d:\n", i);
		
		if (list->serial_numbers[i])
			printf("USB descriptor string: %s\n", list->serial_numbers[i]);

		device = NULL;
		result = greatfet_device_list_open(list, i, &device);
		if (result != GREATFET_SUCCESS) {
			fprintf(stderr, "greatfet_open() failed: %s (%d)\n",
					greatfet_error_name(result), result);
			return EXIT_FAILURE;
		}

		result = greatfet_board_id_read(device, &board_id);
		if (result != GREATFET_SUCCESS) {
			fprintf(stderr, "greatfet_board_id_read() failed: %s (%d)\n",
					greatfet_error_name(result), result);
			return EXIT_FAILURE;
		}
		printf("Board ID Number: %d (%s)\n", board_id,
				greatfet_board_id_name(board_id));

		result = greatfet_version_string_read(device, &version[0], 255);
		if (result != GREATFET_SUCCESS) {
			fprintf(stderr, "greatfet_version_string_read() failed: %s (%d)\n",
					greatfet_error_name(result), result);
			return EXIT_FAILURE;
		}
		printf("Firmware Version: %s\n", version);

		result = greatfet_board_partid_serialno_read(device, &read_partid_serialno);	
		if (result != GREATFET_SUCCESS) {
			fprintf(stderr, "greatfet_board_partid_serialno_read() failed: %s (%d)\n",
					greatfet_error_name(result), result);
			return EXIT_FAILURE;
		}
		printf("Part ID Number: 0x%08x 0x%08x\n", 
					read_partid_serialno.part_id[0],
					read_partid_serialno.part_id[1]);
		printf("Serial Number: 0x%08x 0x%08x 0x%08x 0x%08x\n", 
					read_partid_serialno.serial_no[0],
					read_partid_serialno.serial_no[1],
					read_partid_serialno.serial_no[2],
					read_partid_serialno.serial_no[3]);
		
		result = greatfet_close(device);
		if (result != GREATFET_SUCCESS) {
			fprintf(stderr, "greatfet_close() failed: %s (%d)\n",
					greatfet_error_name(result), result);
		}
	}
	
	greatfet_device_list_free(list);
	greatfet_exit();

	return EXIT_SUCCESS;
}
