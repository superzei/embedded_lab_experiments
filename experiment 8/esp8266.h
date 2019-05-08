#ifndef esp8266_h
#define esp8266_h

typedef struct Request
{
	int id;
	int length;
	char *data;
	char *type;
}Request;

extern char received[512];
extern const unsigned int timeout;

void ATcommand(char* command, int d);
void readOutput(char* buffer, char* searchFor, unsigned long tm);
int SearchIndexOf(char src[], char str[]);
Request parse_request(char* string_to_parse);
void free_request(Request request);
void respond(void);
void initialize_server(void);

#endif
