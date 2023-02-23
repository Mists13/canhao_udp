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

void handleErrors(int messages[], int totalMsgs, int receivedMsgs){
	puts("\n");
        printf("[Servidor] Calculando erros...\n");
        printf("[Servidor] Total mensagens esperadas: %d\n", totalMsgs);
	printf("[Servidor] Total mensagens recebidas: %d\n", receivedMsgs+2);
	int checkErrors[totalMsgs];
	memset(checkErrors, 0, totalMsgs*sizeof(checkErrors[0]));
	for(int i=0;i<=receivedMsgs;i++){
		for(int j=2;j<=totalMsgs;j++){
			if(messages[i]==j)
				checkErrors[i]=1;
		}
	}
	int index=0, index2=0;
	for(int i=0;i<=totalMsgs-2;i++){
		if(checkErrors[i]==0){
                        index++;
                }
        }
	int maior=0;
	for(int i=0;i<=totalMsgs-2;i++){
		if(checkErrors[i]!=0){
			if(messages[i]<maior)
				index2++;
			if(messages[i]>maior){
				maior=messages[i];
			}	
		}
	}
	if(index2!=0){
		puts("\n");
		printf("[Servidor] Metricas de mensagens fora de ordem: \n");
		printf("[Servidor] Total de mensagens recebidas fora de ordem: %d\n", index2+1);
		printf("[Servidor] Porcentagem de mensagens fora de ordem: %.2f%%\n", 100*((double)(index2+1)/(double)totalMsgs));
	}
	if(index!=0){
		puts("\n");
		printf("[Servidor] Metricas de mensagens nao recebidas: \n");
		printf("[Servidor] Total de mensagens nao recebidas: %d\n", index);
		printf("[Servidor] Porcentagem de mensagens não recebidas: %.2f%%\n", 100*((double)(index)/(double)totalMsgs));
		puts("\n");
	}
	if(index2==0 && index==0)
		printf("[Servidor] Todas as mensagens chegaram");
}	
void receiveMessages(char buf[], int socket_s, struct sockaddr_in cli_addr, int cli_addr_len,
					 int messagesOutOfOrder[], int messagesNotReceivedYet[], int totalMsgs){
	int indexNotReceived = 0;
	int indexOutOfOrder = 0;
	int lastMsgReceived = 0;
	int numNotReceived = 0;
	int numOutOfOrder = 0;
	int sequenceNum = 2;
	int messages[totalMsgs+2], i=0;
	// Recebe sequencia ate encontrar 0
	while(atoi(buf)!=0){
		memset(buf,0, BUFSIZ);
		if (recvfrom(socket_s, buf, BUFSIZ, 0, (struct sockaddr *) &cli_addr, &cli_addr_len) > 0){
			if(atoi(buf)!=0){
				printf("[Servidor] Recebi a mensagem----> %d OK\n", atoi(buf));
				messages[i]=atoi(buf);
				i++;
			}
		}else{
			puts("[Servidor] Erro ao receber mensagens. Finalizando..\n");
			memset(buf,'\0', BUFSIZ);
		}
	}
	
	printf("[Servidor] Fim de recebimento...\n");
	handleErrors(messages, totalMsgs, i-1);
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
	int sequenceNum = 0;
	int lastMsgReceived = 0;

	if (argc != 2) {
		puts("Uso correto: servidor <porta>");
		exit(1);
	}
	
	// Obter nome e IP do host
	gethostname(localhost, MAXHOSTNAME);
	if ((hp = gethostbyname( localhost)) == NULL){
		puts ("[Servidor] Nao consegui meu proprio IP");
		exit (1);
	}	

	// Preparar estrutura de socket do servidor com porta, ip e tipo de IP(IPv4 ou IPv6)
	serv_addr.sin_port = htons(atoi(argv[1]));
	bcopy((char *)hp->h_addr_list[0], (char *)&serv_addr.sin_addr, hp->h_length);
	serv_addr.sin_family = hp->h_addrtype;		

	// Abrir socket
	if ((socket_s = socket(hp->h_addrtype,SOCK_DGRAM,0)) < 0){
           puts ( "[Servidor] Nao consegui abrir o socket" );
		exit ( 1 );
	}	

	// tempo limite de 10 segundos para evitar 
	// que o servidor fique esperando por muito tempo
	// no bloqueio para recebimento
	int timeout = 10; 
	struct timeval tv;
	tv.tv_sec = timeout;
	tv.tv_usec = 0;

	setsockopt(socket_s, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

	// Vincular socket com endereco ip local e porta
	if (bind(socket_s, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0){
		puts ( "[Servidor] Nao consegui fazer o bind" );
		exit ( 1 );
	}

	int totalMsgs = 0;
	printf("[Servidor] Estou pronto para receber mensagens\n");

	int firstMessage = 1;
	while(1){
		cli_addr_len = sizeof(cli_addr);
		// Recebe primeira mensagem
		if(strlen(buf) == 0){  
			while(firstMessage == 1){
				if (recvfrom(socket_s, buf, BUFSIZ, 0, (struct sockaddr *) &cli_addr, &cli_addr_len) > 0){
					totalMsgs = atoi(buf);
					printf("[Servidor] Iniciando o recebimento de mensagens\n");
					printf("[Servidor] Salvando total de mensagens: %d\n", totalMsgs);
					memset(buf,'\0', BUFSIZ);
					firstMessage = 0;
					break;
				}
			}	
		}
		memset(buf,'\0', BUFSIZ);
		if (recvfrom(socket_s, buf, BUFSIZ, 0, (struct sockaddr *) &cli_addr, &cli_addr_len) > 0 &&
			firstMessage == 0){
			printf("[Servidor] Recebi a mensagem----> %d\n", atoi(buf));
			if(atoi(buf) == 1){
				receiveMessages(buf, socket_s, cli_addr, cli_addr_len, messagesOutOfOrder, messagesMissing, totalMsgs);
			}
			totalMsgs = 0;
			firstMessage = 1;
		}else {
			printf("[Servidor] Escutando...\n");
		} 
		memset(buf,'\0', BUFSIZ);
	}
}

