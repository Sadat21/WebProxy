/*
 * Proxy.c
 *  This program is a Basic HTTP proxy which receives the HTTP Requests from the web browser
 *  and forward it to the desired web server. After receiving the response from the web server
 *  it forward it to the web browser of the client.
 *  The proxy listens on port 8001
 *  Created on: Jan 23, 2018

 *	This program was intitally given to me by my TA in CPSC 441 Winter 2018,
 *	but has been modified by Sadat Islam.
 */

/* Standard libraries */
#include <stdio.h>
#include <stdlib.h>

/* Libraries for socket programming */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

/* Library for parsing strings */
#include <string.h>
#include <strings.h>

/* h_addr?! */
#include <netdb.h>

/* Clean exit! */
#include <signal.h>

int lstn_sock;

/* The function will run after catching Ctrl+c in terminal */
void catcher(int sig) {
	close(lstn_sock);
	printf("catcher with signal  %d\n", sig);
	exit(0);

}

int main() {

	/* For catching Crtl+c in terminal */
	signal(SIGINT, catcher);
	int lstn_port = 8001;

	/* Initializing the Address */
	struct sockaddr_in addr_proxy;
	addr_proxy.sin_family = AF_INET;
	addr_proxy.sin_addr.s_addr = htonl(INADDR_ANY);
	addr_proxy.sin_port = htons(lstn_port);
	printf("Address Initialization: done.\n");

	/* Creating the listening socket for proxy */
	lstn_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (lstn_sock < 0) {
		printf("Error in socket() call.\n");
		exit(-1);
	} else {
		printf("Listening Socket creation: done.\n");
	}

	/* Binding the socket to address and port */
	int bind_status;
	bind_status = bind(lstn_sock, (struct sockaddr *) &addr_proxy,
			sizeof(struct sockaddr_in));
	if (bind_status < 0) {
		printf("Error in bind() call.\n");
		exit(-1);
	} else {
		printf("Binding: done.\n");
	}

	/* Listening on binded port number */
	int lstn_status;
	lstn_status = listen(lstn_sock, 10);
	if (lstn_status < 0) {
		printf("Error in listen() call.\n");
		exit(-1);
	}
	/* Infinite while loop for listening accepting connection requests */
	while (1) {
		/* Accepting connection requests */
		/* Creates a new socket for the Client */
		int data_sock;
		data_sock = accept(lstn_sock, NULL, NULL);
		if (data_sock < 0) {
			printf("Error in accept() call");
			exit(-1);
		} else {
			printf("Accepting connection request: done.\n");
			printf("\n\n\n\n\n");
		}

		/* Receiving HTTP message from the client */
		char c_message_in[1024];
		char s_message_out[1024];
		int c_recv_status;
		c_recv_status = recv(data_sock, c_message_in, sizeof(c_message_in), 0);
		if (c_recv_status < 0) {
			printf("Error in recv() call for client recv message\n");
			exit(-1);
		} else {
			printf("## HTTP message received from the client. ##\n ");
		}

		/* Preserving the HTTP request for sending it to the web server later */
		strcpy(s_message_out, c_message_in);

		/* Parsing the HTTP message to extract the HOST name of the desired web server */
		char host[1024];
		char URL[1024];
		char PATH[1024];
		char *extension;
		int i;
		int isGET;

		// find and parse the GET request, isolating the URL for later use
		char *pathname = strtok(c_message_in, "\r\n");
		printf("Found HTTP request: %s\n", pathname);
		if (sscanf(pathname, "GET http://%s", URL) == 1){
			printf("URL = %s\n", URL);
			isGET = 1;
		}
		else{
			printf("This is not a GET Request\n");
			isGET = 0;
		}

		if(isGET == 0){
			close(data_sock);
			printf("data socket is closed.\n");
		}
		else{
			// seperate the hostname from the pathname
			for (i = 0; i < strlen(URL); i++) {
				if (URL[i] == '/') {
					strncpy(host, URL, i); //copy out the hostname
					host[i] = '\0';
					break;
				}
			}

			bzero(PATH, 500); //to clear junk at the beginning of this buffer. don't know why its there
			for (; i < strlen(URL); i++) {
				strcat(PATH, &URL[i]); //copy out the path
				break;
			}

			//Find out the file extension
			char * temp;

			temp = strtok(URL,".");
			extension = temp;
			while(temp != NULL){
				extension = temp;

				temp = strtok(NULL,".");
			}

			printf("******************************\n");
			printf("First Half(host): %s\n", host); //firstHalf is the hostname
			printf("Second Half(path): %s\n", PATH); //secondHalf is the path
			printf("File Extension: %s\n", extension); //secondHalf is the path
			printf("******************************\n");


			/* Creating the TCP socket for connecting to the desired web server */
			// Address initialization
			struct sockaddr_in addr_server;
			struct hostent *server;

			// Getting web server's Address by its host name
			server = gethostbyname(host);
			if (server == NULL)
				printf("Error in gethostbyname() call.\n");
			else
				printf("Web server = %s\n", server->h_name);

			// Initialize socket structure
			bzero((char *) &addr_server, sizeof(addr_server));
			addr_server.sin_family = AF_INET;
			bcopy((char *) server->h_addr, (char *) &addr_server.sin_addr.s_addr, server->h_length);
			addr_server.sin_port = htons(80);

			// Creating the socket
			int web_sock = socket(AF_INET, SOCK_STREAM, 0);
			if (web_sock < 0) {
				printf(
						"Error in socket() call for creating --proxy-WebServer-- socket. #################\n");
			} else {
				printf("## --proxy-WebServer-- socket creation: done ##\n");
			}

			// Connecting to the web server's socket
			int connect_status;
			connect_status = connect(web_sock, (struct sockaddr *) &addr_server,
					sizeof(addr_server));
			if (connect_status < 0) {
				printf(
						"Error in connect() call for connecting to the web server's socket.\n");
				exit(-1);
			} else {
				printf("Web server's socket connection establishment: done\n ");
			}

			//Not an html page
			if(strcmp("html", extension) != 0){
				printf("## Non HTML File ##\n");
				/* Sending the HTTP request of the client to the web server */
				int web_send_status;
				web_send_status = send(web_sock, s_message_out, sizeof(c_message_in),
						0);
				if (web_send_status < 0) {
					printf(
							" Error in send() call for sending HTTP request to the web server.\n ");
					exit(-1);
				} else {
					printf("## Sending HTTP request to the server: done ##\n");
				}

				/* Receiving the HTTP response from the web server */
				char w_message_in[10000];
				int web_recv_status;
				web_recv_status = recv(web_sock, w_message_in, sizeof(w_message_in), 0);
				if (web_recv_status < 0) {
					printf(
							" Error in recv() call for receiving web server's HTTP response.\n ");
					exit(-1);
				} else {
					printf("## Receiving web server's HTTP response: done ##\n");
				}
				/* Closing the socket connection with the web server */
				close(web_sock);
				/* Sending the HTTP response to the client */
				int c_send_status;
				c_send_status = send(data_sock, w_message_in, sizeof(w_message_in), 0);
				if (c_send_status < 0) {
					printf(
							"Error in send() call for sending HTTP response to the client.\n");
					exit(-1);
				} else {
					printf("## Sending HTTP response to the client: done ##\n");
				}
			}
			//html page, so slow it down
			else{
				printf("## This is an HTML Page ##\n");
				/* Sending the HTTP request of the client to the web server */
				int web_send_status;
				web_send_status = send(web_sock, s_message_out, sizeof(c_message_in),
						0);
				if (web_send_status < 0) {
					printf(
							" Error in send() call for sending HTTP request to the web server.\n ");
					exit(-1);
				} else {
					printf("## Sending HTTP request to the server: done ##\n");
				}

				/* Receiving the HTTP response from the web server */
				char w_message_in[10000];
				int web_recv_status;
				web_recv_status = recv(web_sock, w_message_in, sizeof(w_message_in), 0);
				if (web_recv_status < 0) {
					printf(
							" Error in recv() call for receiving web server's HTTP response.\n ");
					exit(-1);
				} else {
					printf("## Receiving web server's HTTP response: done ##\n");
				}
				//Read the bytes
				char maxBytes[1024] = "-1";

				char w_message_in_copy[10000];
				strcpy(w_message_in_copy, w_message_in);

				// find and parse the Response, isolating the bytes for later use
				temp = strtok(w_message_in, "\r\n");
				while(temp != NULL){
					if (sscanf(temp, "Content-Length: %s", maxBytes) == 1){
						printf("Max Bytes = %s\n", maxBytes);
						break;
					}
					else{
						//printf("Temp contains: %s\n", temp);
						temp = strtok(NULL, "\r\n");
					}
				}

				//If there is no maxBytes value in the request
				if (strcmp(maxBytes, "-1") == 0) {
					printf("There is no reference to the number of bytes, hence I will loop and then deliver the message.\n");
					int x;
					for(x = 0; x < 100000; x++){}
					//Send response

					/* Sending the HTTP response to the client */
					int c_send_status;
					c_send_status = send(data_sock, w_message_in_copy, sizeof(w_message_in_copy), 0);
					if (c_send_status < 0) {
						printf(
								"Error in send() call for sending HTTP response to the client.\n");
						exit(-1);
					} else {
						printf("## Sending HTTP response to the client: done ##\n");
					}
				}
				//If you know the max bytes, then just ask for some at a time
				else{
					printf("To be finished\n");
					printf("%s\n", maxBytes );


				}

				/* Closing the socket connection with the web server */
				close(web_sock);
				exit(1);
			}



			/* Closing the socket connection with the client */
			close(data_sock);
			printf("data socket is closed.\n");
		}
	}
	close(lstn_sock);
	printf("lstn_sock is closed.\n");
	return 0;

}
