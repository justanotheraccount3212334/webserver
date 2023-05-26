#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int sockfd;
struct sockaddr_in host_addr;
int host_addrlen;
struct sockaddr_in client_addr;
int client_addrlen;

char	buffer	[BUFFER_SIZE];
char	response[BUFFER_SIZE];
int	headerLength;
char*	afterHeader;
char	method	[BUFFER_SIZE];
char	uri	[BUFFER_SIZE];
char	version	[BUFFER_SIZE];

int init();
void mainLogic(void (*responsesHandler)());

void setResponse(char* path, char* html) {
	if (strcasecmp(uri, path) == 0) {
		memset(afterHeader, 0, BUFFER_SIZE - headerLength - 1);
		strcat(response, html);
	}
}

void handleResponses() {
	setResponse("/",
"<html>"
"	<head>"
"		<title>Boostore</title>"
"	</head>"
"	<body>"
"		<h1>Boostore</h1>"
"		<em>Ukrainian bookstores network</em><p></p>"
"		<p><b>Select your town</b> (Kyiv is the only town currently available)<b>:</b></p>"
"		<a href=\"http://localhost:8080/kyiv\">Kyiv</a>"
"		<hr>"
"		Main page"
"	</body>"
"</html>"
);
	setResponse("/kyiv",
"<html>"
"	<head>"
"		<title>Boostore</title>"
"	</head>"
"	<body>"
"		<h1>Boostore</h1>"
"		<em>Ukrainian bookstores network</em><p></p>"
"		<p><b>Select a desired shop:</b></p>"
"		<ul>"
"			<li><a href=\"http://localhost:8080/kyiv/address-a\">Kyiv, address A</a></li>"
"			<li><a href=\"http://localhost:8080/kyiv/address-b\">Kyiv, address B</a></li>"
"		</ul>"
"		<hr>"
"		<a href=\"http://localhost:8080\">Main page</a>"
"	</body>"
"</html>"
);
	setResponse("/kyiv/address-a",
"<html>"
"	<head>"
"		<title>Boostore</title>"
"	</head>"
"	<body>"
"		<h1>Boostore</h1>"
"		<em>Ukrainian bookstores network</em><p></p>"
"		<p><b>Books currently available for purchase in Kyiv, address A:</b></p>"
"		<ul>"
"			<li>Book A</li>"
"			<li>Book B</li>"
"			<li>Book C</li>"
"			<li>Book D</li>"
"			<li>Book E</li>"
"		</ul>"
"		<hr>"
"		<a href=\"http://localhost:8080\">Main page</a>"
"	</body>"
"</html>"
);
	setResponse("/kyiv/address-b",
"<html>"
"	<head>"
"		<title>Boostore</title>"
"	</head>"
"	<body>"
"		<h1>Boostore</h1>"
"		<em>Ukrainian bookstores network</em><p></p>"
"		<p><b>Books currently available for purchase in Kyiv, address B:</b></p>"
"		<ul>"
"			<li>Book A</li>"
"			<li>Book B</li>"
"			<li>Book C</li>"
"		</ul>"
"		<hr>"
"		<a href=\"http://localhost:8080\">Main page</a>"
"	</body>"
"</html>"
);
}

int main() {
	init();
	mainLogic(&handleResponses);
	return 0;
}

int init() {
	strcpy(response, "HTTP/1.0 200 OK\r\n"
			 "Server: webserver-c\r\n"
			 "Content-type: text/html\r\n\r\n");
	headerLength = strlen(response);
	afterHeader = &(response[headerLength]);

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		perror("webserver (socket)");
		return 1;
	}
	printf("socket created successfully\n");

	host_addrlen = sizeof(host_addr);
	host_addr.sin_family = AF_INET;
	host_addr.sin_port = htons(PORT);
	host_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	client_addrlen = sizeof(client_addr);

	if (bind(sockfd, (struct sockaddr *)&host_addr, host_addrlen) != 0) {
		perror("webserver (bind)");
		return 1;
	}
	printf("socket successfully bound to address\n");

	if (listen(sockfd, SOMAXCONN) != 0) {
		perror("webserver (listen)");
		return 1;
	}
	printf("server listening for connections\n");
}

void mainLogic(void (*responsesHandler)()) {
	while (1) {
		int newsockfd = accept(sockfd, (struct sockaddr *)&host_addr,
					(socklen_t *)&host_addrlen);
		if (newsockfd < 0) {
			perror("webserver (accept)");
			continue;
		}
		printf("connection accepted\n");

		int sockn = getsockname(newsockfd,
					(struct sockaddr *)&client_addr,
					(socklen_t *)&client_addrlen);
		if (sockn < 0) {
			perror("webserver (getsockname)");
			continue;
		}

		int valread = read(newsockfd, buffer, BUFFER_SIZE);
		if (valread < 0) {
			perror("webserver (read)");
			continue;
		}

		sscanf(buffer, "%s %s %s", method, uri, version);

		responsesHandler();

		int valwrite = write(newsockfd, response, BUFFER_SIZE);
		if (valwrite < 0) {
			perror("webserver (write");
			continue;
		}

		close(newsockfd);
	}
}
