#include "./getIP.h"

void getIP(char* ipstr){
        struct ifreq ifr;
        //char ipstr[40];
        int s;

        s = socket(AF_INET, SOCK_DGRAM, 0);
        strncpy(ifr.ifr_name, "enp0s3", IFNAMSIZ);

        if(ioctl(s, SIOCGIFADDR, &ifr) < 0){
                printf("Error");
        } else{
                inet_ntop(AF_INET, ifr.ifr_addr.sa_data+2, ipstr, sizeof(struct sockaddr));
//              printf("My IP : %s\n", ipstr);
        }
}

