/*---------------------------------------------------------------------------------------
--	SOURCE FILE:	portForwarder.c - A port forwarder server that reads from a configuration file
--
--	PROGRAM:		portForwarder
--
--	DATE:			March 26, 2012
--
--	REVISIONS:		(Date and Description)
--
--					March 26, 2012
--						* Added multiple connections
--						* Added read from configuration file
--
--	DESIGNERS:		Unknown
--
--	PROGRAMMERS:	Unknown
--
--	MODIFIED BY:	Wayne Cheng
--					Wesley Yue
--					Kelley Zhang
--
--	SOURCE:			http://linux.die.net/man/2/select_tut
--
--	NOTES:
-- 	The program will read from a configuration file.
--	The program will accept only TCP connections.
--	The program will support OOB traffic.
--	The program will support multiple inbound connection requests.
-- 	The program will read data from the client socket and forward it to another IP address.
---------------------------------------------------------------------------------------*/
#include "portForwarder.h"

int main (void){
  int pid,i;
  int max=0;
  max=maxChild();

  for(i = 0; i< max; i++){    
    switch((pid=fork())){
      case -1:        		// Call error message function
		abortSystem("The fork has fail \n");
      case 0:        		// Call error message function
		child(i);			// fork for create more listen socket for different ports
		exit(0);
      default:    
		continue; // wait till the child process exit
    }
  }

  return 0;
}


/************************************************************************************************/
/*																								*/		
/*	Child funaction  																			*/
/*	The Child process will all run this funaction.												*/ 
/* 	This will run create listen socket and await for client 									*/
/*																								*/ 
/*																								*/ 
/************************************************************************************************/
void child(int currChild){
  int	i = 0;
	int	sd, new_sd;
	socklen_t client_len;
	struct sockaddr_in server, client;

	connInfo connMsg;
	forwardInfo fi;
    
	// Read the listen and forward port, and IP address from the configuration file
	fi = readConfig(currChild);

	printf("Listen Port:\t%d \nForward IP:\t%s \nForward Port:\t%d\n",fi.listen_port, fi.forward_ip, fi.forward_port);

	// Create a stream socket
	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
	   abortSystem("Can't create the stream socket!");
	}

	// Bind an address to the socket
	bzero((char *)&server, sizeof(struct sockaddr_in));

	server.sin_family = AF_INET;
	server.sin_port = htons(fi.listen_port);	// Grab the listen port from the configuration file
	server.sin_addr.s_addr = htonl(INADDR_ANY); // Accept connections from any client

	if (bind(sd, (struct sockaddr *)&server, sizeof(server)) == -1)
       abortSystem("Can't bind name to socket");

	signal (SIGPIPE, SIG_IGN);

	// Listen up to 5 connection requests at a time (MAXLISTERNERS = 5)
	listen(sd, MAXLISTENERS);

	while (TRUE){        
		pthread_t ta[i];
		client_len= sizeof(client);

		// Server is receiving a connection request
		if ((new_sd = accept (sd, (struct sockaddr *)&client, &client_len)) == -1)
		   abortSystem("Can't accept client\n");

		// Set the socket, and client's information to the connMsg structure 
		connMsg.clientAddr = inet_ntoa(client.sin_addr);
		connMsg.port = client.sin_port;
		connMsg.curr_sd = new_sd;
		connMsg.forward_port = fi.forward_port; 
		connMsg.forward_ip = fi.forward_ip;

		// Create threads to handle multiple connections to forward sockets
		pthread_create (&ta[i], NULL, outputMsg, (void*) &connMsg);
		i++;
	}
	close(sd);

}

/************************************************************************************************/
/*																								*/		
/*	outputMsg funaction  																		*/
/*	Send all of the accept socket's data to a new socket										*/ 
/* 																								*/
/*																								*/ 
/*																								*/ 
/************************************************************************************************/
void* outputMsg (void* msg)
{
	connInfo new_sd2;

	char buf1[BUF_SIZE], buf2[BUF_SIZE];
	int buf1_avail=0, buf1_written=0;
	int buf2_avail=0, buf2_written=0;

	int fd1 = -1, fd2 = -1;
	
	new_sd2 = *(connInfo*) msg;

	// Set the accept socket to fd1
	fd1 = new_sd2.curr_sd;

	// Set the connect socket to fd2
	fd2 = connect_socket(new_sd2.forward_port, new_sd2.forward_ip);

	// Check if the fd2 socket is created 
	if (fd2 < 0) {
		SHUT_FD1;
	}
	else{
		printf("Connect from %s\n", new_sd2.clientAddr);
	}
	
	while(TRUE){
		int r, nfds = 0;
		fd_set rd, wr, er;

		FD_ZERO (&rd);
		FD_ZERO (&wr);
		FD_ZERO (&er);

		if (fd1 > 0 && buf1_avail < BUF_SIZE) {
			FD_SET (fd1, &rd);
			nfds = max (nfds, fd1);
		}
		if (fd2 > 0 && buf2_avail < BUF_SIZE) {
			FD_SET (fd2, &rd);
			nfds = max (nfds, fd2);
		}
		if (fd1 > 0 && buf2_avail - buf2_written > 0) {
			FD_SET (fd1, &wr);
			nfds = max (nfds, fd1);
		}
		if (fd2 > 0 && buf1_avail - buf1_written > 0) {
			FD_SET (fd2, &wr);
			nfds = max (nfds, fd2);
		}
		if (fd1 > 0) {
			FD_SET (fd1, &er);
			nfds = max (nfds, fd1);
		}
		if (fd2 > 0) {
			FD_SET (fd2, &er);
			nfds = max (nfds, fd2);
		}

		r = select (nfds + 1, &rd, &wr, &er, NULL);

		if (r == -1 && errno == EINTR)
			continue;
		if (r < 0)
			abortSystem("select()");

		// Read data if fd1 or fd2 is greater than 0
		if (fd1 > 0){
			if (FD_ISSET (fd1, &er)) {
				char c;
				errno = 0;
				r = recv (fd1, &c, 1, MSG_OOB);
				if (r < 1) {
					SHUT_FD1;
				} 
				else
					send (fd2, &c, 1, MSG_OOB);
			}
			if (FD_ISSET (fd1, &rd)) {
			r = read (fd1, buf1 + buf1_avail, BUF_SIZE - buf1_avail);
				if (r < 1) {
					SHUT_FD1;
				} 
				else
					buf1_avail += r;
			}
		}
		if (fd2 > 0) {
			if (FD_ISSET (fd2, &er)) {
				char c;
				errno = 0;
				r = recv (fd2, &c, 1, MSG_OOB);
				if (r < 1) {
					SHUT_FD1;
				} 
				else
					send (fd1, &c, 1, MSG_OOB);
			}
			if (FD_ISSET (fd2, &rd)) {
			  r = read (fd2, buf2 + buf2_avail, BUF_SIZE - buf2_avail);
			  if (r < 1) {
				  SHUT_FD2;
			  }
			  else
				  buf2_avail += r;
			}
		}
		if (fd1 > 0) {
			if (FD_ISSET (fd1, &wr)) {
				r = write (fd1, buf2 + buf2_written, buf2_avail - buf2_written);
				if (r < 1) {
				  SHUT_FD1;
				}
				else
				  buf2_written += r;
			}
		}
		if (fd2 > 0) {
			if (FD_ISSET (fd2, &wr)) {
				r = write (fd2, buf1 + buf1_written, buf1_avail - buf1_written);
				if (r < 1) {
					SHUT_FD2;
				}
				else
					buf1_written += r;
			}
		}
		// Check if write data has caught read data
		if (buf1_written == buf1_avail)
			buf1_written = buf1_avail = 0;

		if (buf2_written == buf2_avail)
			buf2_written = buf2_avail = 0;

		/* One of the side has closed the connection, so
			keep writing to the other side until empty */
		if (fd1 < 0 && buf1_avail - buf1_written == 0) {
			SHUT_FD2;
		}
		if (fd2 < 0 && buf2_avail - buf2_written == 0) {
			SHUT_FD1;
		}
	}
	return NULL;
}
/************************************************************************************************/
/*																								*/		
/*	connect_socket funaction  																	*/
/*	Connect the accept socket to a new socket													*/ 
/* 																								*/
/*																								*/ 
/************************************************************************************************/
static int connect_socket(int connect_port, char *address)
{
	struct sockaddr_in a;
	int sockFD;

	// Create the forward socket to the forward IP address
	if ((sockFD = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("Can't create the connect socket!");
		close(sockFD);
		return -1;
	}
	
	// Bind an address to the socket
	bzero((char *)&a, sizeof(a));

	a.sin_port = htons(connect_port);	// Grab the forward port from the configuration file
	a.sin_family = AF_INET;

	// Check if the IP address is valid
	if (!inet_aton(address, (struct in_addr *) &a.sin_addr.s_addr)) {
		perror("Bad IP address format!");
		close(sockFD);
		return -1;
	}

	// Forward the accept socket to the another socket
	if (connect(sockFD, (struct sockaddr *) &a, sizeof(a)) == -1) {
		perror("Can't connect the accept socket to the connect socket!");
		shutdown(sockFD, SHUT_RDWR);
		close(sockFD);
		return -1;
	}
    return sockFD;
}
/************************************************************************************************/
/*																								*/		
/*	connect_socket funaction  																	*/
/*	Prints the error stored in errno and aborts the program										*/ 
/* 																								*/
/*																								*/ 
/************************************************************************************************/
static void abortSystem(const char* message)
{
    perror (message);
    exit (EXIT_FAILURE);
}
/************************************************************************************************/
/*																								*/		
/*	readConfig funaction  																		*/
/*	Read the values from the portForwarder.conf file											*/ 
/* 																								*/
/*																								*/ 
/************************************************************************************************/
forwardInfo readConfig(int currChild)
{
	char bufLen[200];
	int i=0,npos=1,end=0;
	int currInfoSave=0,spaceBetwnInfo=4; // count used to determine forward info to be save in this child process
	FILE *file;
	forwardInfo fi;
	currInfoSave = (currChild*spaceBetwnInfo);
	file = fopen("portForwarder.conf", "r");

	// Get all of the texts from file
	while (fgets(bufLen, sizeof(bufLen), file) != NULL) { 
		npos += strcspn(bufLen, "=");	// Set the npos to the position it found "=" 
		
		if(i == (0+currInfoSave)){
			// Set the forwardInfo.listen_port to the variable grabbed from the file
			fi.listen_port = atoi(bufLen+npos); 
		}
		if(i == (1+currInfoSave)){
			end=strcspn(bufLen, "\n");

			// Set the forwardInfo.forward_ip to the variable grabbed from the file
			fi.forward_ip = substring(bufLen,npos,(end-npos));
		}
		if(i == (2+currInfoSave)){
			// Set the forwardInfo.forward_port to the variable grabbed from the file
			fi.forward_port = atoi(bufLen+npos);
			return fi;
		}

		if (bufLen[0] == '\0')
			continue;
		i++;
		npos=1;
	}
	fclose(file);
	return fi;
}
/************************************************************************************************/
/*																								*/		
/*	maxChild funaction  																		*/
/*	Count the number of child need to be create													*/ 
/* 																								*/
/*																								*/ 
/************************************************************************************************/
int maxChild()
{
	char bufLen[200];
	int i=0;
	int totalChild=0;
	int spaceBetwnInfo=4; // count used to determine forward info to be save in this child process
	FILE *file;
	
	file = fopen("portForwarder.conf", "r");
	// Get all of the texts from file
	while (fgets(bufLen, sizeof(bufLen), file) != NULL) { 
		i++;
	}
	fclose(file);
	totalChild = i/spaceBetwnInfo;
	return totalChild;
}
/************************************************************************************************/
/*																								*/		
/*	substring funaction  																		*/
/*	Takes a source string from a certain position to an end position							*/ 
/* 																								*/
/*																								*/ 
/************************************************************************************************/
char* substring(const char* str, size_t begin, size_t len)
{
	if (str == 0 || strlen(str) == 0 || strlen(str) < begin || strlen(str) < (begin+len))
		return 0;

	return strndup(str + begin, len);
} 
