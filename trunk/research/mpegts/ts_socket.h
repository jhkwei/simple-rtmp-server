/*
 * ts_socket.h
 *
 *  Created on: 2015-1-25
 *      Author: shenwei
 */

#ifndef TS_SOCKET_H_
#define TS_SOCKET_H_

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

struct ts_socket{
	int32_t type;
	int32_t version;
	int32_t fd;
	int32_t family;
	char *ip;
	int32_t port;
	int32_t is_server;
	int32_t is_mulitcast;
	struct sockaddr_in address;
};

int32_t ts_udp_open(struct ts_socket *s, char *ip, int32_t port, int32_t is_server);

int32_t ts_udp_read(struct ts_socket *s, char *buf, int32_t size);

int32_t ts_udp_write(struct ts_socket *s, char *buf, int32_t size);

void ts_udp_close(struct ts_socket *s);


#endif /* TS_SOCKET_H_ */
