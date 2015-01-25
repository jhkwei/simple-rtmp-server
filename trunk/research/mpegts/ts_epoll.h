/*
 * ts_epoll.h
 *
 *  Created on: 2015-1-25
 *      Author: shenwei
 */

#ifndef TS_EPOLL_H_
#define TS_EPOLL_H_

#include <sys/epoll.h>
#include <assert.h>
#include <unistd.h>

struct ts_epoll_event{
	int32_t type;
	void *data;
};

int32_t ts_epoll_create(int32_t size){
	assert(size > 0);
	return epoll_create(size);
}

int32_t ts_epoll_add(int32_t epfd, int32_t fd, void *data){
	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.ptr = data;
	ev.data.fd = fd;

	return epoll_ctl( epfd, EPOLL_CTL_ADD,  fd, &ev);
}

int32_t ts_epoll_del(int32_t epfd, int fd){
	struct epoll_event ev;
	ev.events = EPOLLIN;
	return epoll_ctl( epfd, EPOLL_CTL_DEL,  fd, &ev);
}

int32_t ts_epoll_close(int32_t epfd){
	return close(epfd);
}

int32_t ts_epoll_wait(int32_t epfd, int32_t size, struct ts_epoll_event *events){
	int32_t n, i, j;
	struct epoll_event evs[size];

	j = 0;
	n = epoll_wait (epfd, evs, size, 1000);
	for (i = 0; i < n; i++)  {
		if (evs[i].events & EPOLLIN){
			events[j].data = evs[i].data.ptr;
			events[j].type = EPOLLIN;
			j++;
		}
	}
	return j;
}

#endif /* TS_EPOLL_H_ */
