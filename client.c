/* ./client.c
 * Runs a socket clienT.
 * By        : Leomar Duran <https://github.com/lduran2/>
 * When      : 2021-05-05t05:31
 * For       : ECE 5516
 * Version   : 1.0.2
 *
 * Changelog :
 * 	v1.0.1 - 2021-05-05t05:31
 * 		abstracted requesting and loop logic from main
 *
 * 	v1.0.1 - 2021-05-05t00:15
 * 		common socket operations abstracted to sockcommon.c
 * 		refactored to use sockcommon.c
 *
 * 	v1.0 - 2021-05-03t23:51
 * 		adapted to be client
 *
 * 	v0.0 - 2021-05-03t21:08
 * 		template from ./server.c
 */

#include <stdlib.h>	/* for exit, EXIT_SUCCESS */
#include <stdio.h>	/* for fprintf, stderr, stdout,
			   fdopen, getline, fclose */
#include <stdbool.h>	/* for bool, true, false */
#include <errno.h>	/* for errno */
#include <string.h>	/* for strerror */
#include <unistd.h>	/* for close */
#include "sockcommon.h"	/* for common socket operations */

/** sends a request to the server through socket connectedfd from input
 *  file fin, and sends each response to output file fout */
void request_from(int connectedfd,
	FILE *fin, FILE *fout, string_t client_name
);
/** sends a request to the server, and prints out the response,
 *  stopping if either the read of the request or reply from the server
 *  is invalid */
bool requesting_until_closed(int connectedfd, FILE *fconnected,
	char **pline, size_t *plen, FILE *fin, FILE *fout,
	string_t client_name
);

int main(int argc, char **argv) {
	/* the socket connected */
	int connectedfd;
 	/* linked list of selected candidate addresses */
	struct addrinfo *results;
	/* port to listen to if given, otherwise default */
	string_t port;

	/* choose the port */
	port = get_port(argc, argv);
	/* select the addresses to that port */
	results = select_addresses(port, argv[0]);
	/* connect socket to the first available address */
	connectedfd = find_connection(results, connect, "connect", argv[0]);
	/* free linked list of results */
	freeaddrinfo(results);

	/* report the successful connection */
	fprintf(stdout,
		"[%s] connected to port %s . . .\n",
		argv[0], port
	);

	/* request from the server */
	request_from(connectedfd, stdin, stdout, argv[0]);

	/* close the connected socket */
	close(connectedfd);

	fprintf(stderr, "Done.\n");
	exit(EXIT_SUCCESS);
}

/**
 * Sends a request to the server through socket connectedfd from input
 * file fin, and sends each response to output file fout.
 * @params
 * 	connectedfd : int = socket to the server
 * 	fin : FILE * = input file from which to read requests
 * 	fout : FILE * = output file to which to write responses
 * 	client_name : string_t = name of the client for logging
 */
void request_from(int connectedfd,
	FILE *fin, FILE *fout, string_t client_name)
{
	/* character buffer representing line
	 * to populate while reading */
	char *line = NULL;
	/* length of the line character buffer */
	size_t len = 0;

	/* file descriptor to the connected socket */
	FILE *fconnected;

	/* convert the socket descriptor to a file */
	fconnected = fdopen(connectedfd, "r");

	/* explain to user */
	fprintf(stderr, "Escape character is 'C-d'.\n");

	/* read from standard input until no input */
	while (requesting_until_closed(connectedfd, fconnected,
		&line, &len, fin, fout, client_name))
	{ /* no op */ }

	/* close the connection */
	fclose(fconnected);
} /* void request_from(int connectedfd,
	FILE *fin, FILE *fout, string_t client_name)
   */

/**
 * Sends a request to the server, and prints out the response,
 * stopping if either the read of the request or reply from the server
 * is invalid.
 * @params
 * 	connectedfd : int = socket to the server
 * 	fconnected : FILE * = file descriptor of the socket
 * 	pline : char ** = pointer character line buffer
 *	plen : size_t * = pointer to line buffer length
 * 	fin : FILE * = input file from which to read requests
 * 	fout : FILE * = output file to which to write responses
 * 	client_name : string_t = name of the client for logging
 * @return true if not closed; false if closed
 */
bool requesting_until_closed(int connectedfd, FILE *fconnected,
	char **pline, size_t *plen, FILE *fin, FILE *fout,
	string_t client_name)
{
	/* number of characters read from file input to write to server */
	ssize_t nread;

	/* prompt */
	fprintf(stderr, ">>> ");
	fflush(stderr);

	/* accept input from the input file */
	if ((nread = getline(pline, plen, fin)) < 0) {
		fprintf(stderr, "\n");
		/* announce that the connection was closed by the client */
		fprintf(fout,
			"[%s] connection closed locally\n",
			client_name);
		return false;
	} /* if (getline(*pline, *plen, fin) < 0) */

	/* write request to the server */
	write(connectedfd, *pline, nread);

	/* read and print response from the server */
	if (getline(pline, plen, fconnected) < 0) {
		/* announce that the connection was closed by the client */
		fprintf(fout,
			"[%s] connection closed by foreign host\n",
			client_name);
		return false;
	} /* if (getline(*pline, *plen, fconnected) < 0) */
	fprintf(fout, "%s", *pline);

	return true;
} /* int requesting_until_closed(int connectedfd, FILE *fconnected,
	char **pline, size_t *plen, FILE *fin, FILE *fout,
	string_t client_name)
   */
