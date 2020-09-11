#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>

int main(void){

	char ip[32] = {0,};
	char hostbuffer[256];
	char *IPbuffer;
	struct hostent *hostentry;
	char buf[32] = {0,};
/*
	FILE* ipFile;

	ipFile = fopen("/proc/net/arp", "r");

	fscanf(ipFile, "%*s %*s %*s %*s %*s %*s %*s %*s %*s %s", buf);
	fclose(ipFile);
		
	//(IP type,
	inet_ntop(AF_INET, buf, ip, si%*seof(ip));
	
	printf("IP: %s\n", buf);
*/
	gethostname(hostbuffer, sizeof(hostbuffer));
	hostentry = gethostbyname(hostbuffer);

	IPbuffer = inet_ntoa(*((struct in_addr*)hostentry->h_addr_list[0]));

	printf("hostname = %s\n", IPbuffer); //hostname = /proc/net
	return 0;
}
