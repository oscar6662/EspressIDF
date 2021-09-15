#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define MSG_BUFFER_LENGTH 256

const TickType_t read_delay = 50 / portTICK_PERIOD_MS;

// serial_out(..) method assumes string is null-terminated but does not
//	have the specification-mandated newline terminator.  This method applies
//	the newline terminator and writes the string over the serial connection.
void serial_out(const char* string) {
	int end = strlen(string);
	if (end >= MSG_BUFFER_LENGTH) {
		// Error: Output too long.
		return;
	}

	// NOTE: Working spec requires max of 256 bytes containing a newline terminator.
	//	Thus the storage buffer needs 256+1 bytes since we also need a null terminator
	//	to form a valid string.
	char msg_buffer[MSG_BUFFER_LENGTH+1];
	memset(msg_buffer, 0, MSG_BUFFER_LENGTH+1);
	strcpy(msg_buffer, string);
	msg_buffer[end] = '\n';
	printf(msg_buffer);
	fflush(stdout);
}

void app_main(void)
{
	serial_out("firmware ready");

	char query[MSG_BUFFER_LENGTH];
	while (true) {
		int complete = 0;
		int at = 0;
		memset(query, 0, MSG_BUFFER_LENGTH);

		while (!complete) {
			if (at >= 256) {
				// Error: Input too long.
				break;
			}
			int result = fgetc(stdin);
			if (result == EOF) {
				vTaskDelay(read_delay);
				continue;
			}
			else if ((char)result == '\n') {
				complete = true;
			}
			else {
				query[at++] = (char)result;
			}
		}

		// Echo query back.
		serial_out(query);
		serial_out("");
	}
}
