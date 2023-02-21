#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define TAMFILA      5
#define MAXHOSTNAME 30


void receiveMessages(char buf[], int socket_s, struct sockaddr_in cli_addr, int cli_addr_len,
					 int messagesWithError[], int index, int sequenceNum){

	// Recebe sequencia ate encontrar 0
	while(atoi(buf)!=0){
		memset(buf,0, BUFSIZ);
		recvfrom(socket_s, buf, BUFSIZ, 0, (struct sockaddr *) &cli_addr, &cli_addr_len);
		if(atoi(buf)!=0){
			printf("Sou o servidor, recebi a mensagem----> %d\n", atoi(buf));
			if(atoi(buf)!=sequenceNum){
				messagesWithError[index]=atoi(buf);
				index++;
			}
			sequenceNum++;
		}	
	}
	printf("fim de recebimento...\n");
	memset(buf,'\0', BUFSIZ);
}

void logMetrics(char buf[], int socket_s, struct sockaddr_in cli_addr, int cli_addr_len,
					 int messagesWithError[], int indexErrors){

	// Receber numero de total de mensagens
	recvfrom(socket_s, buf, BUFSIZ, 0, (struct sockaddr *) &cli_addr, &cli_addr_len);
	int total=atoi(buf);
	printf("calculando erros...\n");
	printf("total mensagens esperadas: %d\n", total);

	if(indexErrors != 0){
		printf("mensagens erradas: \n");
		for(int i=0; i<indexErrors; i++)
			printf("%d ", messagesWithError[i]);
		printf("\n");
		printf("total de mensagens erradas: %d\n", indexErrors);
		printf("porcentagem de mensagens erradas: %.2f%%\n", 100*((double)indexErrors/(double)total));
	}
	else{
		printf("nao tem mensagens erradas\n");
	}
	memset(messagesWithError,'\0', BUFSIZ);
	memset(buf,'\0', BUFSIZ);
}

int main ( int argc, char *argv[] ){

	int socket_s, messagesWithError[10000];
	unsigned int cli_addr_len;
    char buf [BUFSIZ + 1];
	struct sockaddr_in serv_addr, cli_addr;
	struct hostent *hp;
	char localhost [MAXHOSTNAME];
	int index = 0;
	int sequenceNum;
	
	if (argc != 2) {
		puts("Uso correto: servidor <porta>");
		exit(1);
	}
	
	// Preparar endereco do servidor
	gethostname(localhost, MAXHOSTNAME);
	if ((hp = gethostbyname( localhost)) == NULL){
		puts ("Nao consegui meu proprio IP");
		exit (1);
	}	
	serv_addr.sin_port = htons(atoi(argv[1]));
	bcopy ((char *) hp->h_addr, (char *) &serv_addr.sin_addr, hp->h_length);
	serv_addr.sin_family = hp->h_addrtype;		

	// Abrir socket
	if ((socket_s = socket(hp->h_addrtype,SOCK_DGRAM,0)) < 0){
           puts ( "Nao consegui abrir o socket" );
		exit ( 1 );
	}	

	// Vincular socket com endereco do host
	if (bind(socket_s, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0){
		puts ( "Nao consegui fazer o bind" );
		exit ( 1 );
	}
	
	printf("servidor pronto\n");
	while(1){
		cli_addr_len = sizeof(cli_addr);
		recvfrom(socket_s, buf, BUFSIZ, 0, (struct sockaddr *) &cli_addr, &cli_addr_len);
		printf("recebendo sequencia...\n");
		printf("Sou o servidor, recebi a mensagem----> %s\n", buf);
		if(atoi(buf) == 1){
			sequenceNum=2;
			index=0;
			receiveMessages(buf, socket_s, cli_addr, cli_addr_len, messagesWithError, index, sequenceNum);
			logMetrics(buf, socket_s, cli_addr, cli_addr_len, messagesWithError, index);
		}
	}
}
