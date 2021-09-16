#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define MSG_BUFFER_LENGTH 256

const TickType_t read_delay = 50 / portTICK_PERIOD_MS;

char error[256] = "success";

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

void print_mac() {
  unsigned char mac[6] = {0};
  char readytoprint[18];
  esp_efuse_mac_get_default(mac);
  sprintf(readytoprint,"%02X:%02X:%02X:%02X:%02X:%02X", mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
	serial_out(readytoprint);
}

void print_pong () {
  serial_out("pong");
}

void print_id () {
  serial_out("s107");
}

void print_version () {
  serial_out("1.0.0");
}

void print_error () {
  serial_out(error);
}

struct variable {
  char name[17];
  int value;
}

void store_variable (char * first, int second) {
  struct variable fist;
  first.name = first;
  first.value = second;
}

void check_what_came_in (char * string) {
  printf("%s\n",string);
  char * string_with_arguments = strtok(string," ");
  if (strcasecmp(string,"ping") == 0) {
    print_pong();
  } else if (strcasecmp(string,"mac") == 0) {
    print_mac();
  } else if (strcasecmp(string,"id") == 0) {
    print_id();
  } else if (strcasecmp(string,"version") == 0) {
    print_version();
  } else if (strcasecmp(string,"error") == 0) {
    print_error();
  } else if (strcasecmp(string_with_arguments,"store") == 0) {
    char * first_argument = strtok(NULL," ");
    char * second_argument = strtok(NULL," ");
    store_variable(first_argument,second_argument);
  }else {
    sprintf(error, "Command \"%s\" not recognized",string);
  }
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

		check_what_came_in(query);
	}
}
