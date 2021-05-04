/* ./client.c
 * Runs a socket clienT.
 * By        : Leomar Duran <https://github.com/lduran2/>
 * When      : 2021-05-03t23:51
 * For       : ECE 5516
 * Version   : 1.0
 *
 * Changelog :
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
#include <netdb.h>	/* for getaddrinfo */
#include <sys/socket.h>	/* for addrinfo */
#include <unistd.h>	/* for close */

/** string type */
typedef const char *string_t;

/** port chosen if no port given */
string_t const DEFAULT_PORT = "8080";

/** character buffer size */
enum { CBUF_SIZE = 0100 };

int main(int argc, char **argv) {
	/* constants for initializing */
	enum { SOCK_TCP = SOCK_STREAM };
	enum { AF_IPv4 = AF_INET };

	/* the socket connected */
	int connectedfd;
	/* the socket connected as a file */
	FILE *fconnected;

	/* character buffer representing line
	 * to populate while reading */
	char *line;
	/* length of the line character buffer */
	size_t len;

	/* criteria for selecting socket addresses */
	struct addrinfo hints;
	/* resulting addresses */
	struct addrinfo *results;
	/* error status of finding addresses */
	int status;
	/* available address */
	struct addrinfo *paddr = NULL;

	/* port to listen to */
	string_t port;

	/* set port if given, otherwise use default */
	port = (argc > 1) ? argv[1] : DEFAULT_PORT;

	/* NULL out the citeria */
	memset(&hints, 0, sizeof hints);
	/* initialize to TCP and IPv4 */
	hints.ai_socktype = SOCK_TCP;
       	hints.ai_family = AF_IPv4;

	fprintf(stderr, "[%s] looking up to port %s . . .\n", argv[0], port);

	/* select the addresses */
	if ((status = getaddrinfo(NULL, port, &hints, &results))) {
		fprintf(stderr,
			"[%s] unable to find TCP address to IPv4: %s: %s\n",
			argv[0], gai_strerror(status), strerror(errno)
		);
		return EXIT_FAILURE;
	} /* if (getaddrinfo(NULL, port, ... )) */

	/* search for the first available address */
	for (struct addrinfo *pcnd = results; /* candidate address */
		(pcnd && !paddr); pcnd = pcnd->ai_next)
	{
		/* communications domain to establish */
		int domain = pcnd->ai_family;
		/* try creating the socket */
		if ((connectedfd = socket(domain,
			pcnd->ai_socktype, pcnd->ai_protocol)) < 0)
		{
			fprintf(stderr,
				"[%s] error creating socket at (%p): %s\n",
				argv[0], pcnd, strerror(errno)
			);
		} /* if (socket(domain, ...) < 0) */
		/* try assigning address to socket descriptor connectedfd */
		else if (0==connect(connectedfd, pcnd->ai_addr, pcnd->ai_addrlen))
		{
			paddr = pcnd;
		} /* (socket(domain, ...) < 0) else
		     if (0==bind(connectedfd, ...))
		   */
	} /* for (; (pcnd && !paddr); */

	/* free linked list of results */
	freeaddrinfo(results);

	/* if no address found */
	if (!paddr) {
		fprintf(stderr,
			"[%s] failed to find address to connect to socket\n",
			argv[0]
		);
		exit(EXIT_FAILURE);
	} /* if (!paddr) */

	fprintf(stdout,
		"[%s] connected to port %s . . .\n",
		argv[0], port
	);

	/* convert the socket descriptor to a file */
	fconnected = fdopen(connectedfd, "r");

	/* explain to user */
	fprintf(stderr, "Escape character is 'C-d'.\n>>> ");
	fflush(stderr);

	/* read from standard input and loop until no input */
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
