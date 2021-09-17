#include <stdio.h>
#include <string.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "limits.h"
#include "Stack.h"
#include "serial_out.h"
#include "global.h"

#define MAX_STORED_VARIABLES 32

const TickType_t read_delay = 50 / portTICK_PERIOD_MS;

char error[256] = "success"; //variable used to log errors.
struct Stack* stack;

/**
 *  VARIOUS FUNCTIONS FOR DIFFERENT COMMANDS 
 **/

//mac
void print_mac() {
  unsigned char mac[6] = {0};
  char readytoprint[18];
  esp_efuse_mac_get_default(mac);
  sprintf(readytoprint,"%02X:%02X:%02X:%02X:%02X:%02X", mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
	serial_out(readytoprint);
}
//ping-pong
void print_pong () {
  serial_out("pong");
}
//id
void print_id () {
  serial_out("s107");
}
//version
void print_version () {
  serial_out("1.0.0");
}
//error
void print_error () {
  serial_out(error);
}

typedef struct variable {
  char name[17];
  int value;
} variable;

variable stored_variable_array[MAX_STORED_VARIABLES];
int stored_count = 0;

bool get_variable_by_name (const char * name, int * returnvalue) {
  if(name == NULL) return false;
  for (int i = 0; i < MAX_STORED_VARIABLES; i++) {
    if(strcasecmp(stored_variable_array[i].name,name) == 0) {
      *returnvalue = stored_variable_array[i].value;
      return true;
    }
  }
  return false;
}

void set_variable_by_name (const char * name, int value) {
  for (int i = 0; i < MAX_STORED_VARIABLES; i++) {
    if(strcasecmp(stored_variable_array[i].name,name) == 0) {
      stored_variable_array[i].value = value;
    }
  }
}

void store_variable(const char* name, int value){
  if (stored_count >= MAX_STORED_VARIABLES){
      sprintf(error, "Stored array is full");
      return;
    }
  for (int i = 0; i < strlen(name); i++) {
    if((int)name[i] < 65 || (int)name[i] > 122) {
      sprintf(error, "Variable \"%s\" not possible. Use only Letters or _",name);
      return;
    }
  }
  if (abs(value) >= INT_MAX) {
    sprintf(error, "Variable value to big/small");
    return;
  }
  int foundvalue = 0;
  if (get_variable_by_name(name, &foundvalue)) {
    set_variable_by_name(name,value);
  }
    variable var = {};
    strcpy(var.name, name);
    var.value = value;
    
    stored_variable_array[stored_count] = var;
    stored_count++;
}

void print_variable(const char * variable) {
  int foundvalue = 0;
  if (get_variable_by_name(variable, &foundvalue)){
    char out[31];
    itoa(foundvalue, out, 10);
    serial_out(out);
  } else{
    sprintf(error, "Variable \"%s\" not found",variable);
  }
}

void add (const char * first, const char * second) {
  int value_one = 0;
  if (!get_variable_by_name(first, &value_one)) {
    if(first == NULL){
      serial_out("insert a variable name");
      sprintf(error, "No variable inserted!");
      return;
    }
    serial_out("variable does not exist");
    sprintf(error, "Variable \"%s\" not found",first);
    return;
  }
  int value_two = 0;
  if (!get_variable_by_name(second, &value_two)) {
    value_two = peek(stack);
  }
  char out[31];
  itoa(value_one+value_two, out, 10);
  serial_out(out);
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
    int second_argument = atoi(strtok(NULL," "));
    store_variable(first_argument,second_argument);
  } else if (strcasecmp(string_with_arguments,"query") == 0) {
    char * first_argument = strtok(NULL," ");
    print_variable(first_argument);
  } else if (strcasecmp(string_with_arguments,"push") == 0) {
    push(stack,atoi(strtok(NULL," ")));
  } else if (strcasecmp(string,"pop") == 0) {
    pop(stack);
  } else if (strcasecmp(string,"add") == 0) {
    char * first_argument = strtok(NULL," ");
    char * second_argument = strtok(NULL," ");
    add(first_argument,second_argument);
  } else {
    sprintf(error, "Command \"%s\" not recognized",string);
  }
	serial_out("");
}

void app_main(void)
{
  stack = createStack(32);
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
