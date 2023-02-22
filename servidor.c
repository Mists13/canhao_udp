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


void calculateMetrics(int numOutOfOrder, int numNotReceived, int lastMsgReceived, int totalMsgs){

	puts("\n");
	printf("[Servidor] Calculando erros...\n");
	printf("[Servidor] Total mensagens esperadas: %d\n", totalMsgs);

	int totalNotReceived = totalMsgs - lastMsgReceived;
	if (numNotReceived != 0 || totalNotReceived != 0){
		printf("[Servidor] Total de mensagens não recebidas: %d\n", numNotReceived);
		printf("[Servidor] Total de mensagens não recebidas após falha no servidor: %d\n", totalNotReceived);
		printf("[Servidor] Porcentagem de mensagens não recebidas: %.2f%%\n", 
				100*((double)(totalNotReceived+numNotReceived)/(double)totalMsgs));
	}
	if(numOutOfOrder != 0){
		printf("[Servidor] Metricas de mensagens fora de ordem: \n");
		printf("[Servidor] Total de mensagens fora de ordem: %d\n", numOutOfOrder);
		printf("[Servidor] Ultima mensagem recebida: %d\n", lastMsgReceived);
		printf("[Servidor] Porcentagem de mensagens fora de ordem: %.2f%%\n",
		 		100*((double)numOutOfOrder/(double)totalMsgs));
		
	} 
	if ( totalNotReceived == 0 && numOutOfOrder == 0 && numNotReceived == 0){
		printf("[Servidor] Todas as mensagens foram recebidas corretamente \n");
	}
}


void handleMessageOutOfOrder(int *numMessagesOutOfOrder, int *numMessagesNotReceivedYet,
							 int *sequenceNum, int msgReceived, int lastMsgReceived){
	
	if (msgReceived < *sequenceNum){												// Msg chegou atrasadinha												
		printf("[Servidor] Recebi a mensagem %d atrasadinha\n", msgReceived);
		*numMessagesNotReceivedYet--; 											// remove das mensagens nao recebidas ainda
		*numMessagesOutOfOrder++; 	 											// adiciona nas mensagens fora de ordem
																				// num sequencia se mantem o mesmo

	} else if (msgReceived - *sequenceNum > 1 )	{								// Seq se msgs perdidas
		printf("[Servidor] Recebi a mensagem----> %d\n", msgReceived);	
		printf("[Servidor] Esperava %d mensagens a partir da mensagem %d\n",
				msgReceived - *sequenceNum, *sequenceNum);
		*numMessagesNotReceivedYet+= (msgReceived - *sequenceNum);				// Anota seq de mensagens perdidas
		*sequenceNum = msgReceived + 1; 							// Num seq se torna o seguinte ao ja recebido
	}
	else {
		printf("[Servidor] Esperava a mensagem %d\n", 			*sequenceNum);
		printf("[Servidor] Recebi a mensagem %d\n",   			msgReceived);
		*numMessagesNotReceivedYet++;											// Msg ainda pode ser recebida			
		*sequenceNum=+2;															// num de sequencia aumenta em 2
	}

}

void receiveMessages(char buf[], int socket_s, struct sockaddr_in cli_addr, int cli_addr_len,
					 int messagesOutOfOrder[], int messagesNotReceivedYet[], int totalMsgs){
	int indexNotReceived = 0;
	int indexOutOfOrder = 0;
	int lastMsgReceived = 0;
	int numNotReceived = 0;
	int numOutOfOrder = 0;
	int sequenceNum = 2;

	// Recebe sequencia ate encontrar 0
	while(atoi(buf)!=0){
		memset(buf,0, BUFSIZ);
		if (recvfrom(socket_s, buf, BUFSIZ, 0, (struct sockaddr *) &cli_addr, &cli_addr_len) > 0){
			if(atoi(buf)!=0){
				if(atoi(buf)!=sequenceNum){				// Mensagem fora de ordem			
					handleMessageOutOfOrder( &numOutOfOrder, &numNotReceived,
							  				 &sequenceNum, atoi(buf), lastMsgReceived);
				} else {
					printf("[Servidor] Recebi a mensagem----> %d OK\n", atoi(buf));
					sequenceNum++;
				}
				lastMsgReceived = atoi(buf);
				// verificar sequencia de mensagens perdidas
			}
		}else{
			puts("[Servidor] Erro ao receber mensagens. Finalizando..");
			memset(buf,'\0', BUFSIZ);
		}
	}
	
	printf("[Servidor] Fim de recebimento...\n");
	calculateMetrics(numOutOfOrder, numNotReceived, lastMsgReceived, totalMsgs);
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
	int timeout = 20; 
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
		if (strlen(buf) == 0){  // Recebe primeira mensagem
			while (firstMessage == 1 && 
				   recvfrom(socket_s, buf, BUFSIZ, 0, (struct sockaddr *) &cli_addr, &cli_addr_len) > 0){
				totalMsgs = atoi(buf);
				printf("[Servidor] Salvando total de mensagens: %d\n", totalMsgs);
				memset(buf,'\0', BUFSIZ);
				firstMessage = 0;
				break;
			}	
		}
		memset(buf,'\0', BUFSIZ);
		if (recvfrom(socket_s, buf, BUFSIZ, 0, (struct sockaddr *) &cli_addr, &cli_addr_len) > 0 &&
			firstMessage == 0){
			printf("[Servidor] Recebi a mensagem----> %d\n", atoi(buf));
			if(atoi(buf) == 1){
				receiveMessages(buf, socket_s, cli_addr, cli_addr_len, messagesOutOfOrder, messagesMissing, totalMsgs);
				// calculateMetrics(buf, socket_s, cli_addr, cli_addr_len, messagesOutOfOrder, indexErrors, lastMsgReceived);
			}
		}else {
			printf("[Servidor] Escutando...");
		} 
		memset(buf,'\0', BUFSIZ);
	}
}
