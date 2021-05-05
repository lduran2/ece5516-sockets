/* ./sockcommon.c
 * Common operations on sockets.
 * By        : Leomar Duran <https://github.com/lduran2/>
 * When      : 2021-05-04t23:04
 * For       : ECE 5516
 * Version   : 1.0
 *
 * Changelog :
 * 	v1.0 - 2021-05-03t23:04
 * 		abstracted all common code from client and server
 *
 * 	v0.0 - 2021-05-03t21:08
 * 		template from ./client.c
 */

#include "sockcommon.h"	/* the header */
#include <stdlib.h>	/* for exit, EXIT_SUCCESS */
#include <stdio.h>	/* for fprintf, stderr */
#include <errno.h>	/* for errno */
#include <string.h>	/* for strerror */

/** port chosen if no port given */
string_t const DEFAULT_PORT = "8080";

/** returns the port given in arguments
 *  or the default port if none given */
string_t get_port(int argc, char **argv) {
	/* port to listen to */
	string_t port;
	/* set port if given, otherwise use default */
	port = (argc > 1) ? argv[1] : DEFAULT_PORT;
	return port;
} /* string_t get_port(int argc, char **argv) */

/** selects IPv4 addresses over TCP to given port */
struct addrinfo *select_addresses(string_t port, string_t logger_name) {
	/* constants for initializing hints */
	enum { SOCK_TCP = SOCK_STREAM };
	enum { AF_IPv4 = AF_INET };

	/* criteria for selecting socket addresses */
	struct addrinfo hints;
	/* resulting addresses */
	struct addrinfo *results;
	/* error status of finding addresses */
	int status;

	/* NULL out the citeria */
	memset(&hints, 0, sizeof hints);
	/* initialize to TCP and IPv4 */
	hints.ai_socktype = SOCK_TCP;
       	hints.ai_family = AF_IPv4;

	fprintf(stderr, "[%s] looking up to port %s . . .\n", logger_name, port);

	/* select the addresses */
	if ((status = getaddrinfo(NULL, port, &hints, &results))) {
		fprintf(stderr,
			"[%s] unable to find IPv4 address over TCP: %s: %s\n",
			logger_name, gai_strerror(status), strerror(errno)
		);
		exit(EXIT_FAILURE);
	} /* if (getaddrinfo(NULL, port, ... )) */

	return results;
} /* struct addrinfo *select_addresses(string_t port, string_t logger_name)
   */

/**
 * Finds a socket through which a connection can be made from the
 * selected addresses.
 * @param
 * 	results : struct addrinfo *
 * 		= linked list of selected candidate addresses
 * 	get_addr : int (*)(int sockfd, const struct sockaddr *
 * 		= function for translating the socket into an address
 * 			(&bind for server or &connect for client)
 * 	get_addr_fn_name : string_t = name of get_addr function
 * 	logger_name : string_t = name of the rror logger
 */
int find_connection(struct addrinfo *results,
	int (*get_addr)(int sockfd,
		const struct sockaddr *addr, socklen_t addrlen)
	, string_t get_addr_fn_name, string_t logger_name)
{
	/* the socket connected */
	int connectedfd;
	/* available address */
	struct addrinfo *paddr = NULL;
	/* communications domain to establish */
	int domain;

	/* search for the first available address */
	for (struct addrinfo *pcnd = results; /* candidate address */
		(pcnd && !paddr); pcnd = pcnd->ai_next)
	{
		/* communications domain to establish */
		domain = pcnd->ai_family;
		/* try creating the socket */
		if ((connectedfd = socket(domain,
			pcnd->ai_socktype, pcnd->ai_protocol)) < 0)
		{
			fprintf(stderr,
				"[%s] error creating socket at (%p): %s\n",
				logger_name, pcnd, strerror(errno)
			);
		} /* if (socket(domain, ...) < 0) */
		/* try assigning address to socket descriptor connectedfd */
		else if (0==get_addr(connectedfd, pcnd->ai_addr, pcnd->ai_addrlen))
		{
			/* if successful, set paddr */
			paddr = pcnd;
		} /* (socket(domain, ...) < 0) else
		     if (0==get_addr(connectedfd, ...))
		   */
	} /* for (; (pcnd && !paddr); */

	/* if no address found */
	if (!paddr) {
		fprintf(stderr,
			"[%s] failed to find address to %s to socket\n",
			logger_name, get_addr_fn_name
		);
		exit(EXIT_FAILURE);
	} /* if (!paddr) */

	return connectedfd;
} /* int find_connection(struct addrinfo *results,
	int (*get_addr)(int sockfd,
		const struct sockaddr *addr, socklen_t addrlen)
	, string_t get_addr_fn_name, string_t logger_name)
   */

