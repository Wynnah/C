struct sendPacket
{
	struct iphdr ip;
	struct tcphdr tcp;
} sendPacket;

struct recvPacket
{
	struct iphdr ip;
	struct tcphdr tcp;
	char buffer[10000];
} recvPacket;

struct pseudo_header
   {
      unsigned int source_address;
      unsigned int dest_address;
      unsigned char placeholder;
      unsigned char protocol;
      unsigned short tcp_length;
      struct tcphdr tcp;
   } pseudo_header;

#define SOURCE_PORT			80		// Default source port
#define DEST_PORT			80		// Default dest port
#define READ				1	    // Read the argv
#define UNREAD				0	    // Unread the argv
#define TRUE				1
#define FALSE               0

// Function Prototypes
void forgePacket(unsigned int, unsigned short, unsigned 
                 short,char *,int); 
unsigned short inCksum(unsigned short *, int);
unsigned int hostConvert(char *);
void usage(char *);
int restartIncrement(int);
static void abortMsg(const char* );
