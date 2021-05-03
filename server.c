/* ./server.c
 * Runs a socket server.
 * By        : Leomar Duran <https://github.com/lduran2/>
 * When      : 2021-05-03t18:22
 * For       : ECE 5516
 * Version   : 1.2
 *
 * Changelog :
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

#include <stdlib.h>	/* for exit, EXIT_SUCCESS, malloc */
#include <stdio.h>	/* for fprintf, stderr, stdout */
#include <errno.h>	/* for errno */
#include <string.h>	/* for strerror */
#include <netdb.h>	/* for getaddrinfo */
#include <sys/socket.h>	/* for addrinfo */
#include <unistd.h>	/* for close */

/** string type */
typedef const char *string_t;

/** port chosen if no port given */
string_t const DEFAULT_PORT = "8080";

/** limit on outstanding connections in socket's listen queue */
enum { BACKLOG = 020 };

/** character buffer size */
enum { CBUF_SIZE = 0100 };

int main(int argc, char **argv) {
	/* constants for initializing */
	enum { SOCK_TCP = SOCK_STREAM };
	enum { AF_IPv4 = AF_INET };

	/* socket ready for listening */
	int listenfd;
	/* the socket connected */
	int connectedfd;
	/* the socket connected as a file */
	FILE *connectedf;

	/* character buffer representing line
	 * to populate while reading */
	char *line;
	/* length of the line character buffer */
	size_t len;

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
	port = (argc > 2) ? argv[1] : DEFAULT_PORT;

	/* NULL out the citeria */
	memset(&hints, 0, sizeof hints);
	/* initialize to TCP and IPv4 */
	hints.ai_socktype = SOCK_TCP;
       	hints.ai_family = AF_IPv4;

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
		if ((listenfd = socket(domain,
			pcnd->ai_socktype, pcnd->ai_protocol)) < 0)
		{
			fprintf(stderr,
				"[%s] error creating socket at (%p): %s\n",
				argv[0], pcnd, strerror(errno)
			);
		} /* if (socket(domain, ...) < 0) */
		/* try assigning address to socket descriptor listenfd */
		else if (0==bind(listenfd, pcnd->ai_addr, pcnd->ai_addrlen))
		{
			paddr = pcnd;
		} /* (socket(domain, ...) < 0) else
		     if (0==bind(listenfd, ...))
		   */
	} /* for (; (pcnd && !paddr); */

	/* free linked list of results */
	freeaddrinfo(results);

	/* if no address found */
	if (!paddr) {
		fprintf(stderr,
			"[%s] failed to find address to bind to socket\n",
			argv[0]
		);
		exit(EXIT_FAILURE);
	} /* if (!paddr) */
	/* otherwise try listening */
	else if (listen(listenfd, BACKLOG) < 0) {
		fprintf(stderr,
			"[%s] error listening\n",
			argv[0]
		);
		close(listenfd);
		exit(EXIT_FAILURE);
	} /* else if (listen(listenfd, BACKLOG) < 0) */

	fprintf(stderr,
		"[%s] listening to port %s . . .\n",
		argv[0], port
	);

	/* listen indefinitely */
	for (; ; ) {
		/* use all storage for client address */
		client.addr.len = sizeof *client.addr.val.pstorage;

		/* attempt to connect */
		if (-1==(connectedfd = accept(listenfd,
			client.addr.val.psa, &client.addr.len)))
		{
			fprintf(stderr,
				"[%s] error connecting: %s\n",
				argv[0], strerror(errno)
			);
		} /* if (-1==accept(listenfd, ...)) */
		else {
			/* convert the descriptor to a file */
			connectedf = fdopen(connectedfd, "r");

			/* announce connection, and client if possible */
			switch (getnameinfo(client.addr.val.psa, client.addr.len,
				client.name, CBUF_SIZE, client.port, CBUF_SIZE, 0))
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
			} /* switch (getnameinfo(client.addr.val, client.addr.len,
				client.name, CBUF_SIZE, client.port, CBUF_SIZE, 0))
			   */

			/* read from the socket */
			for (ssize_t nread; (0 <= (nread = getline(&line, &len, connectedf))); )
			{
				fprintf(stdout, "[%s] read: %s", argv[0], line);
			}

			/* close the connection */
			fprintf(stdout, "[%s] connection closed\n", argv[0]);
			close(connectedfd);
		} /* (-1==accept(listenfd, ...)) else */
	} /* for (; ; ) */

	fprintf(stdout, "Done.");
	exit(EXIT_SUCCESS);
}
