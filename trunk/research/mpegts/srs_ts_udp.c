/*
 * srs_ts_udp.c
 *
 *  Created on: 2015-1-25
 *      Author: shenwei
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <signal.h>
#include <string.h>

#include "ts_socket.h"
#include "ts_log.h"
#include "ts_packet.h"
#include "ts_epoll.h"

#include "../../objs/include/srs_librtmp.h"

struct ip_addr{
	char *output;
	char *ip;
	int32_t port;
	struct ts_socket sk;
	struct ts_packet *ts;
	srs_rtmp_t rtmp;
};

struct ip_list{
	int32_t num;
	struct ip_addr *addr;
	char *input;
};

int32_t ts_out_es_config(struct ts_packet *ts){
	int32_t j;
	char filename[64];
	FILE *fp = NULL;
	struct ts_pid *ts_pid;
	struct es_info_header *es_info, *tmp;
	for(j = 0; j < ts->program_num; j++){


		/*
		 * struct priv_data *priv_data = NULL;
		 * do something
		 * */

		es_info = ts->programs[j].next;


		struct ip_addr *ip_addr = (struct ip_addr *)ts->priv_data;
		char rtmp_url[512] ;
		snprintf(rtmp_url, sizeof(rtmp_url), "%s_%s:%d_%d",ip_addr->output, ip_addr->ip,ip_addr->port, ts->programs[j].service_id);
		srs_human_trace("%s\n", rtmp_url);
		   srs_rtmp_t rtmp = srs_rtmp_create(rtmp_url);
			ip_addr->rtmp = rtmp;
		    
		    if (srs_rtmp_handshake(rtmp) != 0) {
			srs_human_trace("simple handshake failed.");
			srs_rtmp_destroy(rtmp);
			return 0;
		    }
		    srs_human_trace("simple handshake success");
		    
		    if (srs_rtmp_connect_app(rtmp) != 0) {
			srs_human_trace("connect vhost/app failed.");
			srs_rtmp_destroy(rtmp);
			return 0;
		    }
		    srs_human_trace("connect vhost/app success");
		    
		    if (srs_rtmp_publish_stream(rtmp) != 0) {
			srs_human_trace("publish stream failed.");
			srs_rtmp_destroy(rtmp);
			return 0;
		    }
		    srs_human_trace("publish stream success");

		while(es_info){
			tmp = es_info;
			es_info = tmp->next;
			ts_pid = ts_get_pid(ts, tmp->pid);
			if(ts_pid && ts_pid->pass){

				/*
				 * ts_pid->priv_data = priv_data;
				 * */
				ts_pid->priv_data = rtmp;
			}
		}
	}
	return 0;
}


int32_t ts_exit_es_config(struct ts_packet *ts){
	int32_t j;
	FILE *fp = NULL;
	struct ts_pid *ts_pid;
	struct es_info_header *es_info, *tmp;
	for(j = 0; j < ts->program_num; j++){


		/*
		 * struct priv_data *priv_data = NULL;
		 * do something
		 * */

		es_info = ts->programs[j].next;
		struct ip_addr *ip_addr = (struct ip_addr *)ts->priv_data;
		srs_rtmp_destroy(ip_addr->rtmp);
		
	}
	return 0;
}

int32_t es_output(struct ts_pid *pid, char *buf, int32_t size){
	int n = 0;
	srs_rtmp_t rtmp = (FILE *)pid->priv_data;
	struct timeval now;
	gettimeofday(&now, NULL);
	int timestamp = now.tv_sec*1000 + now.tv_usec/1000;
	// timestamp = pid->pts/90;

	if(pid->type == TS_AUDIO_PID){
	  	char sound_format = 10;//aac
		char sound_rate = 3; //44.1kHZ
		// 1 = 16-bit samples
		char sound_size = 1;
		// 1 = Stereo sound
		char sound_type = 1;
	
		int ret = 0;
		if ((ret = srs_audio_write_raw_frame(rtmp, 
		    sound_format, sound_rate, sound_size, sound_type,
		    buf, size, timestamp)) != 0
		) {
		    srs_human_trace("send audio raw data failed. ret=%d", ret);
		    return 0;
		}
	}else{
		int pts, dts;
		pts = timestamp;
		dts = pts + 40; 
		int ret = srs_h264_write_raw_frames(rtmp, buf, size, dts, pts);
		if (ret != 0) {
		    if (srs_h264_is_dvbsp_error(ret)) {
		        srs_human_trace("ignore drop video error, code=%d", ret);
		    } else if (srs_h264_is_duplicated_sps_error(ret)) {
		        srs_human_trace("ignore duplicated sps, code=%d", ret);
		    } else if (srs_h264_is_duplicated_pps_error(ret)) {
		        srs_human_trace("ignore duplicated pps, code=%d", ret);
		    } else {
		        srs_human_trace("send h264 raw data failed. ret=%d", ret);
		       return 0;
		    }
		}
	}
	return n;
}

struct srs_optget{
	char *input;
	char *output;
};

int32_t srs_optget(int32_t argc, char **argv, struct srs_optget *o){
	int c;
	char *opt = "i:y:h";
    while((c = getopt (argc, argv, opt)) != -1) {
    	switch(c){
    	case 'i':
    		o->input = optarg;
		printf("-i %s \n", optarg);
    		break;
    	case 'y':
    		o->output = optarg;
		printf("-y %s \n", optarg);
    		break;
    	case 'h':
    		printf("1) %s -i udp://ip:port -y rtmp://127.0.0.1/live/livestream\n", argv[0]);
    		printf("2) %s -i udp://ip:port#ip:port#ip:port -y rtmp://127.0.0.1/live/livestream\n", argv[0]);
    		break;
    	default:
    		break;
    	}
    }
    return 0;
}


int32_t srs_input_para(struct srs_optget *o, struct ip_list *list){
	if(strncmp("udp://", o->input, strlen("udp://"))){
		ts_warn("no udp stream\n");
		return 0;
	}
	int32_t n = 0;
	char *p = o->input;
	while(*p){
		if(*p == '#'){
			n++;
		}
		p++;
	}
	list->addr =  malloc(sizeof(struct ip_addr)*(n + 1));
	memset(list->addr, 0, sizeof(struct ip_addr)*(n + 1));
	list->input = strdup(o->input + strlen("udp://"));
	p = list->input;
	int i = 0;
	while(*p){
		/* IP */
		list->addr[i].ip = p;
		list->addr[i].output = o->output;
		while(*p && *p != ':'){
			p++;
		}
		/*port*/
		if(*p && *p == ':'){
			*p = '\0';
			p++;
			sscanf(p, "%d", &list->addr[i].port);
		}
		while(*p && *p != '#'){
			p++;
		}
		p++;
		ts_info("add input IP:%s:%d\n", list->addr[i].ip, list->addr[i].port);
		i++;
	}
	list->num = i;
	return 1;
}

static int ts2es_is_run = 0;

void srs_ts2es_sigroutine(int sig){
	ts2es_is_run = 0;
}

void srs_ts2es_signal(){
	signal(SIGHUP, srs_ts2es_sigroutine);
	signal(SIGINT, srs_ts2es_sigroutine);
}

int32_t main(int32_t argc, char **argv){

	char buf[1316];
	int32_t n;
	int32_t size;
	int32_t result = 1;
	struct ip_list ip_list = {0};
	struct srs_optget opt;
	srs_ts2es_signal();
	srs_optget(argc, argv, &opt);
	srs_input_para(&opt, &ip_list);
	int epfd = ts_epoll_create(ip_list.num + 1);
	if(epfd < 0){
		ts_warn("create epoll failed\n");
		result = 0;
	}

	int i;
	for(i = 0; result && i < ip_list.num; i++){
		if(!ts_udp_open(&ip_list.addr[i].sk, ip_list.addr[i].ip, ip_list.addr[i].port, 1)){
			result = 0;
			break;
		}
		
		struct ts_packet *ts = ts_packet_init();
		ip_list.addr[i].ts = ts;
		ts->priv_data = &ip_list.addr[i];
		ts_register_output_callback(ts, es_output);
		ts_register_config_callback(ts, ts_out_es_config);
		ts_epoll_add(epfd, ip_list.addr[i].sk.fd, &ip_list_addr[i]);
	}

	ts2es_is_run = result;
	struct ts_epoll_event events[ip_list.num + 1];
	while(ts2es_is_run){
		n = ts_epoll_wait(epfd, ip_list.num + 1, events);
		for(i = 0; i < n; i++){
			struct ip_addr *ip_addr = (struct ip_addr *)events[i].data;
			struct ts_packet *ts = ip_addr->ts;
			struct ts_socket *sk = &ip_addr->sk;
			size = ts_udp_read(sk, buf, sizeof(buf));
			if(size >= 188){
				ts_packet(ts, buf, size, 188);
			}
		}
	}

	for(i = 0; i < ip_list.num; i++){
		ts_udp_close(&ip_list.addr[i].sk);
		struct ts_packet *ts = ip_list.addr[i].ts;
		if(ts){
			ts_unregister_output_callback(ts);
			ts_unregister_config_callback(ts);
			ts_exit_es_config(ts);
			ts_packet_exit(ts);
		}
	}
	ts_epoll_close(epfd);
	if(ip_list.num){
		free(ip_list.input);
		free(ip_list.addr);
	}
	return 0;
}
