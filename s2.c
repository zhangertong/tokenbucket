#include <arpa/inet.h>
#include <netinet/in.h>
#include <linux/net_tstamp.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#define BUFLEN 1542
#define PORT 9933
#define PORT1 9932
 
void err(char *str)
{
    perror(str);
    exit(1);
}

double receive( )
{
    struct sockaddr_in my_addr, cli_addr;
    int sockfd, i;
    double r;
    socklen_t slen;
    
    char buf[BUFLEN]; 
    slen=sizeof(cli_addr);
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
      err("socket");
    else
      printf("Server : Socket() successful\n");
 
    bzero(&my_addr, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(PORT1);
    my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
     
    if (bind(sockfd, (struct sockaddr* ) &my_addr, sizeof(my_addr))==-1)
      err("bind");
    printf("Server : bind() successful\n");
    i=recvfrom(sockfd, buf, BUFLEN, 0, (struct sockaddr*)&cli_addr, &slen);
    if(i<0)
            err("recvfrom()");
	sscanf(buf, "hi%lf", &r);
     printf("%lf\n",r);
    close(sockfd);
    return r;
}
 
int main(int argc, char* argv[])
{
     
    struct sockaddr_in serv_addr;
    int sockfd, i, slen=sizeof(serv_addr);
    char buf[BUFLEN];
    struct timeval t1, t2;
    int t=1000;
    if(argc<2)
    {
      printf("./client ip\n");
      return -1;
    }
    if(argc>2) t=atoi(argv[2]);
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
        err("socket");
    int timestamp_flags=SOF_TIMESTAMPING_TX_SOFTWARE;
    if(setsockopt(sockfd, SOL_SOCKET,SO_TIMESTAMPING,&timestamp_flags,sizeof(timestamp_flags))<0) {
    err("timestamp error");
    } 
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    if (inet_aton(argv[1], &serv_addr.sin_addr)==0)
    {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }
   double delay=0; 
   int id=0;
   int jj;
double r=60;
int buffersize=1400;
while(r>1) {
for (jj=3;jj<5;jj+=1){
printf("jj=%d******************\n",jj);
	buffersize=500;
	delay=buffersize*8/r;
	for( i=0;i<t;i++) 
	{
	    gettimeofday(&t1,NULL);
	    sprintf(buf, "%d,%ld,%ld\n",id, t1.tv_sec,t1.tv_usec);
	    id++;
	    if (sendto(sockfd, buf, buffersize, 0, (struct sockaddr*)&serv_addr, slen)==-1)
		err("sendto()");
	    long dif=0;
	    double temp=5000;
	    if(i%2!=0) temp=delay;
	    while(dif<temp)
	    {
		gettimeofday(&t2,NULL);
		dif=(t2.tv_sec-t1.tv_sec)*1000000+(t2.tv_usec-t1.tv_usec);
	    } 
	}
}
	if(strcpy(buf,"exit\0") == 0) return -1;
	if (sendto(sockfd, buf, 20, 0, (struct sockaddr*)&serv_addr, slen)==-1)
		err("sendto()");
	r=receive();
}
    return 0;
}
