/* ./client.c
 * Runs a socket clienT.
 * By        : Leomar Duran <https://github.com/lduran2/>
 * When      : 2021-05-05t00:15
 * For       : ECE 5516
 * Version   : 1.0.1
 *
 * Changelog :
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
#include <errno.h>	/* for errno */
#include <string.h>	/* for strerror */
#include <unistd.h>	/* for close */
#include "sockcommon.h"	/* for common socket operations */

int main(int argc, char **argv) {
	/* character buffer representing line
	 * to populate while reading */
	char *line = NULL;
	/* length of the line character buffer */
	size_t len = 0;

	/* the socket connected */
	int connectedfd;
	/* file descriptor to the connected socket */
	FILE *fconnected;
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

	/* convert the socket descriptor to a file */
	fconnected = fdopen(connectedfd, "r");

	/* explain to user */
	fprintf(stderr, "Escape character is 'C-d'.\n>>> ");
	fflush(stderr);

	/* read from standard input until no input */
	for (ssize_t nread; (0 <= (nread = getline(&line, &len, stdin))); )
	{
		/* write request to the server */
		write(connectedfd, line, nread);
		/* read and print response from the server */
		nread = getline(&line, &len, fconnected);
		fprintf(stdout, "%s", line);
		/* print the prompt */
		fprintf(stderr, ">>> ");
		fflush(stderr);
	}

	fprintf(stderr, "\n");
	/* announce that the connection was closed by the client */
	fprintf(stdout, "[%s] connection closed locally\n", argv[0]);

	/* close the connection */
	fclose(fconnected);
	close(connectedfd);

	fprintf(stderr, "Done.\n");
	exit(EXIT_SUCCESS);
}
