# ts_packet

MPEG-TS to ES

本程序只是简单的把 TS 提出 ES，
1) 现在已经可以从TS 文件中提出 ES， 
	运行 ./ts_test_output xxx.ts
2) TS over UDP 部分，没有 UDP TS 流 ， socket 部分没有经过测试
	支持多IP输入，IP 之间用“#”隔开
	运行 ./srs_ts_udp -i udp://192.168.1.223:1234 -y rtmp://127.0.0.8/live/livestream
	    ./srs_ts_udp -i udp://192.168.1.223:1234#239.100.100.100:1234 -y rtmp://127.0.0.8/live/livestream
3) 输出 
	-y rtmp://127.0.0.8/live/livestream 这后面会自动添加一些字符来区分流，
	现在的格式是:
	-y rtmp://127.0.0.8/live/livestream_[input IP:port]_[service_id]


最终的目的实现如下配置功能：

listen              1935;
max_connections     1000;
vhost __defaultVhost__ {
    ingest livestream {
        enabled      on;
        input {
            type    stream;
            url     udp://192.168.1.222:1234;
        }
        srs_ts_udp      ./research/mpegts/srs_ts_udp;
        engine {
            enabled         on;
            output          rtmp://127.0.0.1:[port]/live?vhost=[vhost]/livestream;
        }
    }
}
