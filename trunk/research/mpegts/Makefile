GCC = gcc

ifeq ($(HANDSHAKE),)
    ST_ALL      = help
else
    ST_ALL      = objs/ts_test_file \
                objs/ts_test_output objs/srs_ts_udp
endif

.PHONY: default clean help ssl nossl

default: $(ST_ALL)

help:
	@echo "Usage: make <help>|<clean>|<ssl>|<nossl>"
	@echo "     help                    display this help"
	@echo "     clean                   cleanup build"
	@echo "     ssl                     all tools link ssl"
	@echo "     nossl                   all tools never link ssl"
	@echo "ssl/nossl will build the following tools:"
	@echo "     ts_test_output          test TS file to audio and video ES file"
	@echo "     srs_ts_udp		    publish TS over UDP to SSR by srs-librtmp"
	@echo "Remark: about simple/complex handshake, see: http://blog.csdn.net/win_lin/article/details/13006803"
	@echo "Remark: srs Makefile will auto invoke this by --with/without-ssl, "
	@echo "     that is, if user specified ssl(by --with-ssl), srs will make this by 'make ssl'"
	@echo "     that is, if user not use ssl(by --without-ssl), use 'make nossl'"
	@echo "     see: https://github.com/winlinvip/simple-rtmp-server/wiki/v1_CN_v1_Build"
	@echo "Remark: before make this sample, user must make the srs, with/without ssl"
    
clean:
	@rm -rf objs

# srs library root
SRS_OBJS = ../../objs
# srs-librtmp for publish/play, built by srs.
SRS_LIBRTMP_I = $(SRS_OBJS)/include/srs_librtmp.h
SRS_LIBRTMP_L = $(SRS_OBJS)/lib/srs_librtmp.a
# openssl for complex handshake, built by srs.
SRS_LIBSSL_L =
# public depends, the Makefile or public headers.
SRS_RESEARCH_DEPS = Makefile

# for x86/x64 platform
ifeq ($(GCC), gcc)
    EXTRA_CXX_FLAG = -g -O0 -ldl -lstdc++
endif
# for arm.
ifeq ($(GCC), arm-linux-gnueabi-gcc)
    EXTRA_CXX_FLAG = -g -O0 -ldl -static -lstdc++
endif
# for mips, add -lgcc_eh, or stl compile failed.
ifeq ($(GCC), mipsel-openwrt-linux-gcc)
    EXTRA_CXX_FLAG = -g -O0 -ldl -lstdc++ -lgcc_eh
endif
# for ssl or nossl
ifeq ($(HANDSHAKE), SSL)
    SRS_LIBSSL_L = $(SRS_OBJS)/openssl/lib/libssl.a $(SRS_OBJS)/openssl/lib/libcrypto.a
endif

ssl:
	@mkdir -p objs/
	$(MAKE) HANDSHAKE="SSL"
nossl:
	@mkdir -p objs/
	$(MAKE) HANDSHAKE="NOSSL"


MPEG_TS_OBJ=objs/
$(MPEG_TS_OBJ)/srs_ts_udp.o:srs_ts_udp.c
	$(GCC) -o $@ -c $^
$(MPEG_TS_OBJ)/ts_packet.o:ts_packet.c 
	$(GCC) -o $@ -c $^
$(MPEG_TS_OBJ)/ts_socket.o:ts_socket.c 
	$(GCC) -o $@ -c $^
$(MPEG_TS_OBJ)/ts_test_output.o:ts_test_output.c 
	$(GCC) -o $@ -c $^
$(MPEG_TS_OBJ)/ts_test_file.o:ts_test_file.c 
	$(GCC) -o $@ -c $^

objs/srs_ts_udp: srs_ts_udp.c ts_packet.c ts_socket.c $(SRS_RESEARCH_DEPS) $(SRS_LIBRTMP_I) $(SRS_LIBRTMP_L) $(SRS_LIBSSL_L)
	$(GCC) srs_ts_udp.c ts_packet.c ts_socket.c  $(SRS_LIBRTMP_L) $(SRS_LIBSSL_L) $(EXTRA_CXX_FLAG) -o objs/srs_ts_udp

objs/ts_test_output:$(MPEG_TS_OBJ)/ts_packet.o  $(MPEG_TS_OBJ)/ts_test_output.o 
	$(GCC) $(MPEG_TS_OBJ)/ts_packet.o $(MPEG_TS_OBJ)/ts_test_output.o    -o objs/ts_test_output
objs/ts_test_file:$(MPEG_TS_OBJ)/ts_packet.o  $(MPEG_TS_OBJ)/ts_test_file.o 
	$(GCC) $(MPEG_TS_OBJ)/ts_packet.o $(MPEG_TS_OBJ)/ts_test_file.o    -o objs/ts_test_file

