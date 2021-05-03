/* ./server.c
 * Runs a socket server.
 * By        : Leomar Duran <https://github.com/lduran2/>
 * When      : 2021-05-03t14:06
 * For       : ECE 5516
 * Version   : 1.0
 *
 * Changelog :
 * 	v1.0 - 2021-05-03t14:06
 * 		created and tested socket listen
 */

#include <stdlib.h>	/* for exit, EXIT_SUCCESS */
#include <stdio.h>	/* for fprintf, stderr */
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

int main(int argc, char **argv) {
	/* constants for initializing */
	enum { SOCK_TCP = SOCK_STREAM };
	enum { AF_IPv4 = AF_INET };

	/* socket ready for listening */
	int listenfd;

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
	}

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
	if (paddr == NULL) {
		fprintf(stderr,
			"[%s] failed to find address to bind to socket\n",
			argv[0]
		);
		exit(EXIT_FAILURE);
	} /* if (paddr == NULL) */
	/* otherwise try listening */
	else if (listen(listenfd, BACKLOG) < 0) {
		fprintf(stderr,
			"[%s] error listening to socket %d\n",
			argv[0], listenfd);
		close(listenfd);
		exit(EXIT_FAILURE);
	} /* else if (listen(listenfd, BACKLOG) < 0) */

	return port[0];

	exit(EXIT_SUCCESS);
}
