/* ./sockcommon.h
 * Declares common operations on sockets.
 * By        : Leomar Duran <https://github.com/lduran2/>
 * When      : 2021-05-04t23:25
 * For       : ECE 5516
 * Version   : 1.0
 *
 * Changelog :
 * 	v1.0 - 2021-05-03t23:25
 * 		create the header file for sockcommon.c
 */

#ifndef	__SOCKCOMMON_H__
	#define	__SOCKCOMMON_H__

	#include <netdb.h>	/* for addrinfo */
	#include <sys/socket.h>	/* for sockaddr, sockaddr_store,
	                       	   socklen_t */

	/** string type */
	typedef const char *string_t;

	/** port chosen if no port given */
	extern string_t const DEFAULT_PORT;

	/** returns the port given in arguments
	 *  or the default port if none given */
	string_t get_port(int argc, char **argv);

	/** selects IPv4 addresses over TCP to given port */
	struct addrinfo *select_addresses(string_t port, string_t logger_name);

	/** finds a socket through which a connection can be made from the
	 *  selected addresses. */
	int find_connection(struct addrinfo *results,
		int (*get_addr)(int sockfd,
			const struct sockaddr *addr, socklen_t addrlen)
		, string_t get_addr_fn_name, string_t logger_name
	);
#endif /* ndef	__SOCKCOMMON_H__ */

