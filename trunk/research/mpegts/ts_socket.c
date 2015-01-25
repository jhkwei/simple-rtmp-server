/*
 * ts_socket.c
 *
 *  Created on: 2015-1-25
 *      Author: shenwei
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "ts_log.h"
#include "ts_socket.h"

int32_t ts_add_membership(struct ts_socket *s){
	struct ip_mreq mreq;

	mreq.imr_multiaddr.s_addr = inet_addr(s->ip);
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);

	if (setsockopt(s->fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(struct ip_mreq)) == -1) {
	    perror("setsockopt IP_ADD_MEMBERSHIP");
	   return 0;
	}
	return 1;
}

int32_t ts_drop_membership(struct ts_socket *s){
	struct ip_mreq mreq;

	mreq.imr_multiaddr.s_addr = inet_addr(s->ip);
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);

	if (setsockopt(s->fd, IPPROTO_IP, IP_DROP_MEMBERSHIP, &mreq, sizeof(struct ip_mreq)) == -1) {
	    perror("setsockopt IP_DROP_MEMBERSHIP");
	   return 0;
	}
	return 1;
}

int32_t ts_udp_open(struct ts_socket *s, char *ip, int32_t port, int32_t is_server){
	assert(s);
	socklen_t socklen;
	s->is_server = is_server;
	s->fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (s->fd < 0) {
		ts_warn("socket creating error/n");
		return 0;
	}
	socklen = sizeof(struct sockaddr_in);
	memset(&s->address, 0, socklen);
	s->address.sin_family = AF_INET;
	s->address.sin_port = htons(port);
	if (inet_pton(AF_INET, ip, &s->address.sin_addr) <= 0){
		ts_warn("IP:%s:%d inet_pton error\n", ip, port);
		return 0;
	}
	s->is_mulitcast = 0;
	if((0xE000000 & s->address.sin_addr.s_addr) == 0xE000000){
		s->is_mulitcast = 1;
	}

	if(is_server || (s->is_mulitcast && is_server)){
		if(bind(s->fd, (struct sockaddr *)&s->address, sizeof(struct sockaddr_in)) == -1){
			ts_warn("IP:%s:%d bind error/n", ip, port);
			close(s->fd);
			return 0;
		}
		if(s->is_mulitcast && is_server){
			if(!ts_add_membership(s)){
				close(s->fd);
				return 0;
			}
		}
	}
	return s->fd;
}

int32_t ts_udp_read(struct ts_socket *s, char *buf, int32_t size){
	assert(s);
	socklen_t len = sizeof(struct sockaddr);
	struct sockaddr_in address;
	int32_t n = recvfrom(s->fd, buf, size, 1, (struct sockaddr *)&address, &len);
	return n;
}

int32_t ts_udp_write(struct ts_socket *s, char *buf, int32_t size){
	assert(s);
	int32_t n = sendto(s->fd, buf, size, 0, (struct sockaddr *)&s->address,sizeof(s->address));
	return n;
}

void ts_udp_close(struct ts_socket *s){
	assert(s);
	close(s->fd);
	if(s->is_mulitcast && s->is_server){
		ts_drop_membership(s);
	}
}
