#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {

  int sockdescr;
  int numbytesrecv;
  struct sockaddr_in sa;
  struct hostent *hp;
  char buf[BUFSIZ+1];
  char *host;
  char *dados;

  unsigned int i;

  if(argc != 4) {
    puts("[Cliente] Uso correto: <cliente> <nome-servidor> <porta> <dados>");
    exit(1);
  }

  host = argv[1];
  int numMaximo = atoi(argv[3]);

  if((hp = gethostbyname(host)) == NULL){
    puts("[Cliente] Nao consegui obter endereco IP do servidor.");
    exit(1);
  }

  bcopy((char *)hp->h_addr_list[0], (char *)&sa.sin_addr, hp->h_length);

  sa.sin_family = hp->h_addrtype;
  sa.sin_port = htons(atoi(argv[2]));

  if((sockdescr=socket(hp->h_addrtype, SOCK_DGRAM, 0)) < 0) {
    puts("[Cliente] Nao consegui abrir o socket.");
    exit(1);
  }

  // Enviando numero total de mensagens
  printf("[Cliente] Enviando o total %d de mensagens\n", numMaximo);
  sprintf(buf, "%i", numMaximo);
  if(sendto(sockdescr, buf, strlen(buf), 0, (struct sockaddr *) &sa, sizeof sa) != strlen(buf)){
              puts("[Cliente] Nao consegui mandar os dados");
              exit(1);
  }
  for(int i=1;i<=numMaximo;i++){
    char buf[10];
    sprintf(buf, "%i", i);
      if(sendto(sockdescr, buf, strlen(buf), 0, (struct sockaddr *) &sa, sizeof sa) != strlen(buf)){
        puts("[Cliente] Nao consegui mandar os dados"); 
        exit(1);
    }
    printf("[Cliente] Enviando: %d\n", atoi(buf));
  }
  puts("[Cliente] Enviando '0' para finalizar a sequencia\n");
  sprintf(buf, "%i", 0);
  if(sendto(sockdescr, buf, strlen(buf), 0, (struct sockaddr *) &sa, sizeof sa) != strlen(buf)){
    puts("[Cliente] Nao consegui mandar os dados");
    exit(1);
  }
  
  /*for(int i=0;i<30;i++){
    recvfrom(sockdescr, buf, BUFSIZ, 0, (struct sockaddr *) &sa, &i);

    printf("Sou o cliente, recebi: %s\n", buf);
  }*/
  close(sockdescr);
  exit(0);
}
