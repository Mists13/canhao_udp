#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#define TAMFILA      5
#define MAXHOSTNAME 30


void receiveMessages(char buf[], int socket_s, struct sockaddr_in cli_addr, int cli_addr_len,
					 int messagesOutOfOrder[], int messagesNotReceivedYet[], int *intexErrors){

	int i = 0;
	int sequenceNum = 2;
	int index = 0;

	// Recebe sequencia ate encontrar 0
	while(atoi(buf)!=0){
		memset(buf,0, BUFSIZ);
		if (recvfrom(socket_s, buf, BUFSIZ, 0, (struct sockaddr *) &cli_addr, &cli_addr_len) > 0){
			if(atoi(buf)!=0){
				printf("Sou o servidor, recebi a mensagem----> %d\n", atoi(buf));
				if(atoi(buf)!=sequenceNum){
					// saveMessagesNotSent(messagesNotSentYet, i, buf); // armazenar array
					messagesNotReceivedYet[index] = sequenceNum;		// msg deveria ser enviada
					messagesOutOfOrder[index] = atoi(buf);				// mensagem enviada na ordem errada
					index++;
					sequenceNum=+2;
				} else {
					sequenceNum++;
				}
			}	
		}else{
			messagesOutOfOrder[index] = sequenceNum;
			memset(buf,'\0', BUFSIZ);
		}
	}
	printf("fim de recebimento...\n");
	memset(buf,'\0', BUFSIZ);
}

void calculateMetrics(char buf[], int socket_s, struct sockaddr_in cli_addr, int cli_addr_len,
					 int messagesOutOfOrder[], int indexErrors){

	// Receber numero de total de mensagens
	recvfrom(socket_s, buf, BUFSIZ, 0, (struct sockaddr *) &cli_addr, &cli_addr_len);
	int total=atoi(buf);
	printf("calculando erros...\n");
	printf("total mensagens esperadas: %d\n", total);

	if(indexErrors != 0){
		printf("mensagens erradas: \n");
		for(int i=0; i<indexErrors; i++)
			printf("%d ", messagesOutOfOrder[i]);
		printf("\n");
		printf("total de mensagens erradas: %d\n", indexErrors);
		printf("porcentagem de mensagens erradas: %.2f%%\n", 100*((double)indexErrors/(double)total));
	}
	else{
		printf("nao tem mensagens erradas\n");
	}
	memset(messagesOutOfOrder,'\0', BUFSIZ);
	memset(buf,'\0', BUFSIZ);
}

int main ( int argc, char *argv[] ){

	int socket_s, messagesOutOfOrder[10000], messagesMissing[10000];
	unsigned int cli_addr_len;
    char buf [BUFSIZ + 1];
	struct sockaddr_in serv_addr, cli_addr;
	struct hostent *hp;
	char localhost [MAXHOSTNAME];
	int indexErrors = 0;
	int sequenceNum;

	if (argc != 2) {
		puts("Uso correto: servidor <porta>");
		exit(1);
	}
	
	// Obter nome e IP do host
	gethostname(localhost, MAXHOSTNAME);
	if ((hp = gethostbyname( localhost)) == NULL){
		puts ("Nao consegui meu proprio IP");
		exit (1);
	}	

	// Preparar estrutura de socket do servidor com porta, ip e tipo de IP(IPv4 ou IPv6)
	serv_addr.sin_port = htons(atoi(argv[1]));
	bcopy((char *)hp->h_addr_list[0], (char *)&serv_addr.sin_addr, hp->h_length);
	serv_addr.sin_family = hp->h_addrtype;		

	// Abrir socket
	if ((socket_s = socket(hp->h_addrtype,SOCK_DGRAM,0)) < 0){
           puts ( "Nao consegui abrir o socket" );
		exit ( 1 );
	}	

	// tempo limite de 10 segundos para evitar 
	// que o servidor fique esperando por muito tempo
	int timeout = 20; 
	struct timeval tv;
	tv.tv_sec = timeout;
	tv.tv_usec = 0;

	setsockopt(socket_s, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

	// Vincular socket com endereco ip local e porta
	if (bind(socket_s, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0){
		puts ( "Nao consegui fazer o bind" );
		exit ( 1 );
	}
	
	printf("servidor pronto\n");
	while(1){
		cli_addr_len = sizeof(cli_addr);
		if (recvfrom(socket_s, buf, BUFSIZ, 0, (struct sockaddr *) &cli_addr, &cli_addr_len) > 0)
			printf("Sou o servidor, recebi a mensagem----> %s\n", buf);
		else printf("Servidor escutando...\n");
			
		if(atoi(buf) == 1){
			receiveMessages(buf, socket_s, cli_addr, cli_addr_len, messagesOutOfOrder, messagesMissing, &indexErrors);
			calculateMetrics(buf, socket_s, cli_addr, cli_addr_len, messagesOutOfOrder, indexErrors);
		}
	}
}
