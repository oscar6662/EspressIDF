#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define MSG_BUFFER_LENGTH 256

const TickType_t read_delay = 50 / portTICK_PERIOD_MS;

void serial_out(const char* string) {
	int end = strlen(string);
	if (end >= MSG_BUFFER_LENGTH) {
		// Error: Output too long.
		return;
	}
	char msg_buffer[MSG_BUFFER_LENGTH+1];
	memset(msg_buffer, 0, MSG_BUFFER_LENGTH+1);
	strcpy(msg_buffer, string);
	msg_buffer[end] = '\n';
	printf(msg_buffer);
	fflush(stdout);
}
void print_mac(const unsigned char *mac) {
	printf("%02X:%02X:%02X:%02X:%02X:%02X", mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
}

void check_what_came_in (const char * string) {
 	unsigned char mac_base[6] = {0};
    esp_efuse_mac_get_default(mac_base);
    printf("MAC Address: ");
    print_mac(mac_base);
	char ping[4] = "ping";
	char pong[4] = "pong";
	char mac[3] = "mac"; 
	for (int i = 0; i < 3; i ++) {
		if(string[i] != ping[i]) {
			return;
		} 
	}

	serial_out(pong);
	serial_out("");
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
		check_what_came_in(query);
	}
}
