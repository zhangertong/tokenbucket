#include <pcap.h>
#include<stdlib.h>
#include <stdio.h>
#include<sys/socket.h>
#include<netinet/ip.h>
#include<netinet/ip_icmp.h>
#include<net/ethernet.h>
#include<arpa/inet.h>
#include<string.h>
#include <math.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include "dbscan.h"
#define BUFLEN 1450
#define PORT 9932
#define SIZE_ETHERNET 14
struct UDP_hdr {
u_short uh_sport;       /* source port */
u_short uh_dport;       /* destination port */
u_short uh_ulen;        /* datagram length */
u_short uh_sum;         /* datagram checksum */
};
double rates[5000];
int times[5000];
static int n=0,count=0;
struct timeval oldtime;
int pok, ok=0;
double record[100][2];
void err(char *s)
{
    perror(s);
    exit(1);
}
void send_msg(char *host, double rate)
{
  struct sockaddr_in serv_addr;
  int sockfd, i, slen=sizeof(serv_addr);
  char buf[BUFLEN];
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
        err("socket");

    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    if (inet_aton(host, &serv_addr.sin_addr)==0)
    {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }
    sprintf(buf, "%lf",rate);
    if (sendto(sockfd, buf, BUFLEN, 0, (struct sockaddr*)&serv_addr, slen)==-1)
          err("sendto()");
    close(sockfd);
    return;
}
int  update_rate(double rates[], int times[], double rate, double count, int n)
{
	if(rate==0) return n;
	if(n==0) {rates[0]=rate; times[0]=1; n++; return n;}
	int j, i=0;
	double result=1;
	for(j=0;j<n;j++)
	{
	double temp=rates[j]-rate;
	if(temp<0)temp=-temp;
	if(temp/rate<result)
		{result=temp/rate;i=j;}
	}
	if(result<0.05)
	{
	times[i]+=count;
	}
	else
	{
//        char host[]="192.168.137.14";
	times[n]=count;
	rates[n]=rate;
 //       send_msg(host,rates[n]);
	n++;
	}
	return n;
}	
void sort(int  array[], double r[],  int length)
{
int i,j;
int temp;
double t;
for(i=0;i<length;i++)
for(j=i+1;j<length;j++)
{
	if(array[j]>array[i])
	{temp=array[i];
         array[i]=array[j];
          array[j]=temp;
         t=r[i];
         r[i]=r[j];
         r[j]=t;
       }
}
}
void sort1 (double   array[],  int length)
{
int i,j;
double temp; 
for(i=0;i<length;i++)
for(j=i+1;j<length;j++)
{
	if(array[j]<array[i])
	{temp=array[i];array[i]=array[j];array[j]=temp;}
}
}
int isIn(double array[][2], double probingrate, int len, double nearestmode)
{
//nearestmode is the lowerbound, i.e. record[*][0], of the nearest mode, (the higher one)
int i=0, n=0;
for(;i<len;i++) 
  {
	if (array[i][1]<probingrate) continue;
	else if(array[i][1]<nearestmode) {n=1; break;}
	if (array[i][1]>nearestmode) break;
  }
return n;
}
void dump_UDP_packet(u_char *arg, const struct pcap_pkthdr* pkthdr, const u_char * packet)
{
struct ip *ip;
struct UDP_hdr *udp;
unsigned int IP_header_length;
int capture_len=pkthdr->caplen;
if(count!=0){
    double tempdelay=(pkthdr->ts.tv_sec-oldtime.tv_sec)*1000000+(pkthdr->ts.tv_usec-oldtime.tv_usec);
    double r=pkthdr->caplen*8.0/(tempdelay);
    if(r>0)
    rates[count]=r;
    else 
    count--;
}  
 else
    {
        // first_rec=pkthdr->ts;
	//oldtime[1]=pkthdr->ts;
    } 
    count++;
    oldtime=pkthdr->ts;
    int i;
    if(pkthdr->caplen<100)
    { 
	
        char host[]="192.168.137.14";
	sort1(rates,count);
	double temprecord[100][2];
        
	
	if(ok==0)
	{
		ok=DBSCAN(rates,count, 2, 39,record);
		for (i=0;i<ok;i++) printf("record %lf~%lf\n",record[i][0],record[i][1]);
		while(record[ok-1][0]-record[ok-2][1]<4 && ok>1)
		{
			ok--;
 			record[ok-1][1]=record[ok][1];
		}
                double next_rate=(record[ok-1][0]+record[ok-2][1])/2;
	//printf("next rate=%lf\n",next_rate);
		send_msg(host, next_rate);
                pok=ok-1;
		ok--;
		while(record[ok-1][0]-record[ok-2][1]<4 && ok>1)
		{
			ok--;
 			record[ok-1][1]=record[ok][1];
		}
                count=0;
	}
	else
	{
		int modes=DBSCAN(rates,count, 2, 39,temprecord);
		for(i=0;i<modes;i++) printf("temprecord %lf~%lf\n",temprecord[i][0],temprecord[i][1]);
		//search if any new mode between the probing rate and the nearest recorded mode
		//if exists, consider this probing rate is able to travel through the measured path
		//that is to say, capacity>probing rate
		if(ok>1 && !isIn(temprecord,(record[pok][0]+record[pok-1][1])/2, modes, record[pok][0])){
		send_msg(host, (record[ok-1][0]+record[ok-2][1])/2);
		pok=ok-1;
		ok--;
		while(record[ok-1][0]-record[ok-2][1]<4 && ok>1)
		{
			ok--;
 			record[ok-1][1]=record[ok][1];
		}
		}
		else
		{
			printf("fianl rate is %lf~%lf\n",record[pok][0], record[pok][1]);
			send_msg(host,1); 
                        ok=0;
		}
               count=0;
	}
        n=0;
    }
return;
}
	 int main(int argc, char *argv[])
	 {
		pcap_t *handle;			/* Session handle */
		char *dev;			/* The device to sniff on */
		char errbuf[PCAP_ERRBUF_SIZE];	/* Error string */
		struct bpf_program fp;		/* The compiled filter */
		char filter_exp[] = "udp and host 173.0.0.1";	/* The filter expression */
		bpf_u_int32 mask;		/* Our netmask */
		bpf_u_int32 net;		/* Our IP */
		struct pcap_pkthdr header;	/* The header that pcap gives us */
		const u_char *packet;		/* The actual packet */
		int count = 0;
		dev = "eth1";
		if (pcap_lookupnet(dev, &net, &mask, errbuf) == -1) {
			fprintf(stderr, "Couldn't get netmask for device %s: %s\n", dev, errbuf);
			net = 0;
			mask = 0;
		}
		/* Open the session in promiscuous mode */
		handle = pcap_open_live(dev, BUFSIZ, 1, 1000, errbuf);
		if (handle == NULL) {
			fprintf(stderr, "Couldn't open device %s: %s\n", dev, errbuf);
			return(2);
		}
		/* Compile and apply the filter */
		if (pcap_compile(handle, &fp, filter_exp, 0, net) == -1) {
			fprintf(stderr, "Couldn't parse filter %s: %s\n", filter_exp, pcap_geterr(handle));
			return(2);
		}
		if (pcap_setfilter(handle, &fp) == -1) {
			fprintf(stderr, "Couldn't install filter %s: %s\n", filter_exp, pcap_geterr(handle));
		return(2);
		}
		 if (pcap_set_tstamp_type(handle, PCAP_TSTAMP_ADAPTER_UNSYNCED)==-1)
 		{
  	                fprintf(stderr, "ERROR: %s\n", errbuf);
                        return(2);
 		}  
		/* Grab a packet */
		if ( pcap_loop(handle, -1, dump_UDP_packet, (u_char *)&count) == -1){
   		 fprintf(stderr, "ERROR: %s\n", pcap_geterr(handle) );
    		 return -1;
 		}
		pcap_close(handle);
		return(0);
	 }
