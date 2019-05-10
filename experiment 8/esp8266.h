#ifndef esp8266_h
#define esp8266_h

#define BUFFER_SIZE 512

extern char received[512];
extern const unsigned int timeout;

typedef struct Request
{
	char* id;
	char* length;
	char *data;
	char *type;
}Request;

int ATcommand(char*, int, char*);
int readOutput(char* buffer, char* searchFor, unsigned long tm);
int SearchIndexOf(char src[], char str[]);
Request parse_request(char* string_to_parse);
void free_request(Request request);
void respond(void);
int send_get_request(char*, int);

#endif
