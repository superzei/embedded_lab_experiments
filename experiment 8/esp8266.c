#include "esp8266.h"
#include "UART.h"
#include "SysTick.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "tm4c123ge6pm.h" 

const unsigned int timeout = 10000; // timeout for read operations
char received[512];

/*
	Serial handler 
*/
void UART1_Handler(void){
	if(UART1_RIS_R & UART_RIS_RXRIS){   // rx fifo >= 1/8 full
		UART1_ICR_R = UART_ICR_RXIC;      // acknowledge interrupt
		readOutput(received, "OK", timeout);				// read output, write to a buffer
		if (SearchIndexOf(received, "+IPD") != -1)	// starts with +IPD, it is a TCP request
		{
			respond();
		}
	}
	if(UART1_RIS_R & UART_RIS_RTRIS){   // receiver timed out
		UART1_ICR_R = UART_ICR_RTIC;      // acknowledge receiver time
	}
}

/*
	Send an AT command via UART
	Idea shamelessly stolen from online example
*/
void ATcommand(char* command, int d)
{
	// memset(received, 0, sizeof(received));	// clear the buffer
	while (*command != '\0')	// We do not want to end string with 0, we want to end it with newline
	{
		UART_OutChar(*command++);	// Write a char, increment the pointer
	}
	UART_OutChar('\r'); // Carriage return
	UART_OutChar('\n'); // Run command
}

/*
	Read from serial connection
		SearchFor: string to search for in text that will stop the search
		tm: timeout in milliseconds 
*/
void readOutput(char* buffer, char* searchFor, unsigned long tm)
{
	int i = 0;
	char a;
	memset(buffer, 0, sizeof(&buffer));	// clear the buffer
	unsigned long current = millis();
	while (millis() - current < tm)	// keep reading, until timeout
	{
		while(!(UART1_FR_R&UART_FR_RXFE))	// while we have chars to read
			{
				a = UART_InChar();	// read a char from UART1
				if(a == '\0') continue;		
				received[i]= a;
				i++;
			}
		if (SearchIndexOf(received, searchFor) != -1)	//When OK reply sent, do not wait for timeout, get out
		{
			break;
		}
		
	}
}

/*
	Substring search
	Shamelessly stolen
*/
int SearchIndexOf(char src[], char str[])
{
   int i, j, firstOcc;
   i = 0, j = 0;

   while (src[i] != '\0')
   {

      while (src[i] != str[0] && src[i] != '\0')
         i++;

      if (src[i] == '\0')
         return (-1); // nope, string ended

      firstOcc = i;

      while (src[i] == str[j] && src[i] != '\0' && str[j] != '\0')
      {
         i++;
         j++;
      }

      if (str[j] == '\0')
         return (firstOcc);	// got it
      if (src[i] == '\0')
         return (-1); // string ended, sadly

      i = firstOcc + 1;
      j = 0;
   }

   return (-1);	// couldn't found it
}

Request parse_request(char* string_to_parse)
{
	int index = SearchIndexOf(received, "+IPD") + 5;	// starting point of search
	char *token = string_to_parse+index;
	
	/*char *t1 = malloc(strlen(token));
	strcpy(t1, token);
	*/
	
	token = strtok(token, ",");
	int id = strtol(token, NULL, 10);
	
	token = strtok(NULL, ",");
	token = strtok(token, ":");
	int length = strtol(token, NULL, 10);
	
	token = strtok(NULL, ":");
	token = strtok(token, " ");
	char *type = malloc(strlen(token));
	strcpy(type, token);

	char *data = malloc(strlen(token));
	data = "";
	
	while(token!=NULL)
	{
		if (SearchIndexOf(token, "new_data") != -1)
		{
			token = token + 9;
			strcpy(data, token);
			break;
		}
		token = strtok(NULL, " ");
	}
	
	Request request = {.id=id, .length=length, .type=type, .data=data};
	
	return request;
}

void free_request(Request request)
{
	free(request.data);
	free(request.type);
}


/*
	Responds to requests based on type (get, post)
*/
void respond(void)
{
	Request request = parse_request(received);
	char command[30];
	char response[1024];	// MAX 2048 bytes can be send
	
	if (strcmp(request.type, "GET") == 0)
	{

		sprintf(response, "<html><head> <link rel=\"stylesheet\" href=\"https://stackpath.bootstrapcdn.com/bootstrap/4.3.1/css/bootstrap.min.css\"> </link></head><body> <div class=\"container\"> <h1>Hello WORLD</h1> <h5>%d</h5> <h5>%d</h5> <h5>%s</h5> <h5 id=\"r\"></h5> <button type=\"button\" id=\"t\" class=\"btn btn-primary\">POST</button> <script src=\"https://code.jquery.com/jquery-3.4.1.min.js\"> </script> <script>$(\"#t\").click(function (e){$.post(\"192.168.1.105:80\",{new_data: 5}, function(data){$(\"#h5\").html(data);})})</script> </div></body></html>",
		request.id, request.length, request.type);

		snprintf(command, 30, "AT+CIPSEND=0,%d", strlen(response) * sizeof(char));	// we are sending data, catch
		ATcommand(command, 100);
		if (SearchIndexOf(received, "OK") != -1)	// received signal
		{
			ATcommand(response, 100);	// send the data
		}
		
		ATcommand("AT+CIPCLOSE=0", 100);	// close connection
	}
	else if (strcmp(request.type, "POST") == 0)
	{
		sprintf(response, "OK");
		snprintf(command, 30, "AT+CIPSEND=0,%d", strlen(response) * sizeof(char));	// we are sending data, catch
		ATcommand(command, 100);
		if (SearchIndexOf(received, "OK") != -1)	// received signal
		{
			ATcommand(response, 100);	// send the data
		}
	}
	
	free_request(request);
}

void initialize_server(void)
{
		
	ATcommand("AT+RST", 500); // reset
	ATcommand("AT+CWQAP", 500); // disconnect
	ATcommand("AT+CWMODE=3", 500);	// set mode to station + soft ap
	ATcommand("AT+CIPSTATUS", 500);	// status report??
	ATcommand("AT+CWJAP=\"TurkTelekom_TA65B\",\"NavaxmGc\"", 250);	// connect
	ATcommand("AT+CIPMUX=1", 300);	// set multi connection mode
	ATcommand("AT+CIPSERVER=1,80", 300);	// start webserver at port 80
}
