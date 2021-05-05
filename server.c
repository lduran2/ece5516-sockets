/* ./server.c
 * Runs a socket server.
 * By        : Leomar Duran <https://github.com/lduran2/>
 * When      : 2021-05-04t23:43
 * For       : ECE 5516
 * Version   : 1.4.1
 *
 * Changelog :
 * 	v1.4.1 - 2021-05-03t21:08
 * 		common socket operations abstracted to sockcommon.c
 * 		refactored to use sockcommon.c
 *
 * 	v1.4 - 2021-05-03t21:08
 * 		server was not responding to client, now fixed
 * 		fixed port default switch from (argc > 2)
 *
 * 	v1.3 - 2021-05-03t20:04
 * 		now returning input to client as upper case
 *
 * 	v1.2 - 2021-05-03t18:22
 * 		now reading from client through the socket
 *
 * 	v1.1 - 2021-05-03t17:48
 * 		server listening and accepting clients
 * 		tested
 * 		[todo] reading from socket
 *
 * 	v1.0 - 2021-05-03t14:06
 * 		created and tested socket listen
 */

#include <stdlib.h>	/* for exit, EXIT_SUCCESS */
#include <stdio.h>	/* for fprintf, stderr, stdout,
			   fdopen, getline, fclose */
#include <errno.h>	/* for errno */
#include <string.h>	/* for strerror */
#include <unistd.h>	/* for close */
#include <ctype.h>	/* for toupper */
#include "sockcommon.h"	/* for common socket operations */

/** limit on outstanding connections in socket's listen queue */
enum { BACKLOG = 020 };

/** character buffer size */
enum { CBUF_SIZE = 0100 };

int main(int argc, char **argv) {
	/* character buffer representing line
	 * to populate while reading */
	char *line = NULL;
	/* length of the line character buffer */
	size_t len = 0;

	/* client properties */
	struct {
		/* properties of the address */
		struct {
			/* value and length */
			union {
				/* as pointer to storage */
				struct sockaddr_storage pstorage[1];
				/* as pointer to socket address */ 
				struct sockaddr psa[1];
			} val;
			socklen_t len;
		} addr;
		/* client name and port number */
		char name[CBUF_SIZE];
		char port[CBUF_SIZE];
	} client;

	/* the socket connected */
	int connectedfd;
	/* the socket to an accepted client */
	int acceptedfd;
	/* file descriptor to the accepted client socket */
	FILE *faccepted;
 	/* linked list of selected candidate addresses */
	struct addrinfo *results;
	/* port to listen to if given, otherwise default */
	string_t port;

	/* choose the port */
	port = get_port(argc, argv);
	/* select the addresses to that port */
	results = select_addresses(port, argv[0]);
	/* bind socket to the first available address */
	connectedfd = find_connection(results, bind, "bind", argv[0]);
	/* free linked list of results */
	freeaddrinfo(results);

	/* try listening through connected socket */
	if (listen(connectedfd, BACKLOG) < 0) {
		fprintf(stderr,
			"[%s] error listening\n",
			argv[0]
		);
		close(connectedfd);
		exit(EXIT_FAILURE);
	} /* if (listen(connectedfd, BACKLOG) < 0) */

	/* report the successful connection */
	fprintf(stderr,
		"[%s] listening to port %s . . .\n",
		argv[0], port
	);

	/* listen indefinitely */
	for (; ; ) {
		/* use all storage for client address */
		client.addr.len = sizeof *client.addr.val.pstorage;

		/* attempt to accept a client */
		if (-1==(acceptedfd = accept(connectedfd,
			client.addr.val.psa, &client.addr.len)))
		{
			fprintf(stderr,
				"[%s] error accepting a client: %s\n",
				argv[0], strerror(errno)
			);
		} /* if (-1==accept(connectedfd, ...)) */
		else {
			/* convert the socket descriptor to a file */
			faccepted = fdopen(acceptedfd, "r");

			/* announce accepted connection, and client if possible */
			switch (getnameinfo(
				client.addr.val.psa, client.addr.len,
				client.name, CBUF_SIZE,
				client.port, CBUF_SIZE, 0))
			{
				case 0:
					/* case named client */
					fprintf(stdout,
						"[%s] accepted connection from %s:%s\n",
						argv[0],
						client.name, client.port
					);
				break; /* case 0 */
				default:
					/* case unnamed client */
					fprintf(stdout,
						"[%s] accepted connection from unnamed client: %s\n",
						argv[0], strerror(errno)
					);
				break; /* default */
			} /* switch (getnameinfo(
				client.addr.val, client.addr.len,
				client.name, CBUF_SIZE,
				client.port, CBUF_SIZE, 0))
			   */

			/* read from the socket */
			for (ssize_t nread; (0 <= (nread =
				getline(&line, &len, faccepted)
			)); )
			{
				/* report the reading */
				fprintf(stdout,
					"[%s] read: %s",
					argv[0], line
				);
				/* uppercase each character in place */
				for (size_t k = 0; (line[k]); ++k) {
					line[k] = toupper(line[k]);
				} /* for (; (line[k]); ) */
				/* send uppercase to the socket */
				write(acceptedfd, line, nread);
			} /* for (; (0 <= getline(&line, &len, faccepted)); ) */

			/* close the connection */
			fprintf(stdout, "[%s] connection closed\n", argv[0]);
			fclose(faccepted);
			close(acceptedfd);
		} /* (-1==accept(connectedfd, ...)) else */
	} /* for (; ; ) */

	/* close the connected socket */
	close(connectedfd);

	fprintf(stderr, "Done.\n");
	exit(EXIT_SUCCESS);
}
