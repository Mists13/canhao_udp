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
int contarerrados(int a[], int n){
	int count=0;
	for(int i=1;i<=n;i++){
		if(a[i]!=i)
			count++;
	}
	return count;
}
int main ( int argc, char *argv[] )
  {
	int s, t, receivedNumbers[5000];
	unsigned int i;
        char buf [BUFSIZ + 1];
	struct sockaddr_in sa, isa;  /* sa: servidor, isa: cliente */
	struct hostent *hp;
	char localhost [MAXHOSTNAME];

        if (argc != 2) {
           puts("Uso correto: servidor <porta>");
           exit(1);
        }

	gethostname (localhost, MAXHOSTNAME);

	if ((hp = gethostbyname( localhost)) == NULL){
		puts ("Nao consegui meu proprio IP");
		exit (1);
	}	
	
	sa.sin_port = htons(atoi(argv[1]));

	bcopy ((char *) hp->h_addr, (char *) &sa.sin_addr, hp->h_length);

	sa.sin_family = hp->h_addrtype;		


	if ((s = socket(hp->h_addrtype,SOCK_DGRAM,0)) < 0){
           puts ( "Nao consegui abrir o socket" );
		exit ( 1 );
	}	

	if (bind(s, (struct sockaddr *) &sa,sizeof(sa)) < 0){
		puts ( "Nao consegui fazer o bind" );
		exit ( 1 );
	}
	int index=0;
	printf("servidor pronto\n");
	while(1){
		i = sizeof(isa);
		recvfrom(s, buf, BUFSIZ, 0, (struct sockaddr *) &isa, &i);
		printf("recebendo sequencia...\n");
		printf("Sou o servidor, recebi a mensagem----> %s\n", buf);
		if(atoi(buf) == 1){
			int ordem=2;
			index=0;
			while(atoi(buf)!=0){
				memset(buf,0, BUFSIZ);
				recvfrom(s, buf, BUFSIZ, 0, (struct sockaddr *) &isa, &i);
				if(atoi(buf)!=0){
					printf("Sou o servidor, recebi a mensagem----> %d\n", atoi(buf));
					if(atoi(buf)!=ordem){
						receivedNumbers[index]=atoi(buf);
						index++;
					}
					ordem++;
				}	
			}
			printf("fim de recebimento...\n");
			memset(buf,'\0', BUFSIZ);
                        recvfrom(s, buf, BUFSIZ, 0, (struct sockaddr *) &isa, &i);
			int total=atoi(buf);
			printf("calculando erros...\n");
			printf("total mensagens esperadas: %d\n", total);
			if(index != 0){
				printf("mensagens erradas: \n");
				for(int i=0;i<index;i++)
					printf("%d ", receivedNumbers[i]);
				printf("\n");
				printf("total de mensagens erradas: %d\n", index);
				printf("porcentagem de mensagens erradas: %.2f%%\n", 100*((double)index/(double)total));
			}
			else{
				printf("nao tem mensagens erradas\n");
			}
			memset(receivedNumbers,'\0', BUFSIZ);
			memset(buf,'\0', BUFSIZ);
		}
	}
}
