//mettre d'abord tous les include
//puis la fonction err qui ecris "fatal error" en cas d'erreur
//puis la fonction qui envoie un message a tous sauf server et client qui envoie le message (via send())

#include <stdlib.h>//fonctions standardes comme exit()
#include <stdio.h>//fonctions d'entree et sortie standard comme sprintf()
#include <string.h>//fonctions de manipulation de chaine de caractere comme strlen() et bzero()
#include <sys/socket.h>//fonctions socket comme : socket(), send(), recv(), bind(), listen(), accept()
#include <sys/select.h>//fonction select() pour surveiller l'activite des fd/sockets
#include <netinet/in.h>//inclut des definitions pour la structure des addresses internets (sockaddr_in) et fonctions de conversion d'ordre des octets (htons, htonl)
#include <unistd.h>//fonction de l'api POSIX comme write() et close ()

void err() {
	write(2, "fatal error\n", 12);//sortie 2 = stderr
	exit(1);
}

void	send_to_all(char* msg, int server, int client, int max_fd){
	for (int i = 0; i <= max_fd; i++){
		if (i != server && i != client)
			send(i, msg, strlen(msg), 0);//0 = flag donc aucun ici
	}
}

//clients[5000]={0} est un tableau qui map les fd  a un id unique, initialise a 0
//tmp_fds = une copie temporaire de all_fds qui va etre modif par select pour indiquer les fds pret a etre lu
// servaddr contiendra les infos d'addresse du serveur
int main(int ac, char** av) {
	int server, client, max_fd, bytes_read, clients[5000] = {0}, id = 0;
	fd_set all_fds, tmp_fds;
	struct sockaddr_in servaddr;
	char msg_send[400000];
	char msg_recv[400000];

	if (ac =! 2) {
		write(2, "Wrong number of arguments\n", 26);
		exit(1);
	}

	//creation du socket server
	server = socket(AF_INET, SOCK_STREAM, 0);//AF_INET pour IPv4, SOCK_STREAM pour TCP, 0 pour pour protocole par defaut (TCP)
	if (server < 0)
		err();

	//initialisation des ensembles de fds et addr server
	FD_ZERO(&all_fds);
	FD_ZERO(&tmp_fds);
	bzero(&servaddr, sizeof(servaddr));

	//configuration de l'adresse du server
	servaddr.sin_family = AF_INET;//famille d'addresse IPv4
	servaddr.sin_addr.s_addr = htonl(2130706433);//definit l'addresse IP du server, c'est a dire localhost qui est 127.0.0.1 qui donne en decimal 2130706433 qui est convertit dans l'ordre d'octet du reseau
	servaddr.sin_port = htons(atoi(av[1]));//convertit l'argument de char* en entier puis le convertit en octet dans l'ordre d'octet du reseau

	//liaison et ecoute du socket
	if (bind(server, (const struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
		err();
	if (listen(server, 10) < 0)
		err();
	FD_SET(server, &all_fds);//ajoute le socket server a all_fds
	max_fd = server;

	while(1) {
		tmp_fds = all_fds;//initialise tmp_fds a all_fds
		if (select(max_fd + 1, &tmp_fds, 0, 0, 0) < 0)//nombre de fd a surveiller/ ens des fds a surveiller/ attendre indefiniment qu'un event se produise
			err();
		//traitement des fds prets
		for (int fd = 0; fd <= max_fd; fd++){
			if (!FD_ISSET(fd, &tmp_fds))//verifie que fd est dans tmp_fds
				continue;
			bzero(&msg_send, 400000);
			if (fd = server) {
				if ((client = accept(server, 0, 0)) < 0)
					err();
				if(client > max_fd)
					max_fd = client;
				clients[client] = id++;
				FD_SET(client, &all_fds);//ajoute le nouveau fd client a all_fds
				sprintf(msg_send, "server: client %d just arrived\n", clients[client]);//cree un message pour annoncer le nouveau client
				send_to_all(msg_send, server, client, max_fd);
			} else {
				bzero(&msg_recv, 400000);
				bytes_read = 1;
				while( bytes_read == 1 && (!msg_recv[0] || msg_recv[strlen(msg_recv) - 1] != '\n')) {//cette boucle lit le msg recu octet par octet
					bytes_read = recv(fd, &msg_recv[strlen(msg_recv)], 1, 0);// lit un octet et le met a la fin de msg_recv jusau'a ne plus rien lire ou un\n
				}
				if (bytes_read <= 0) {
					sprintf(msg_send, "server: client %d just left\n", clients[fd]);
					send_to_all(msg_send, server, fd, max_fd);
					FD_CLR(fd, &all_fds);
					close(fd);
				} else {
					sprintf(msg_send, "client %d: %s", clients[fd], msg_recv);
					send_to_all(msg_send, server, fd, max_fd);
				}
			}
		}
	}
	return(0);
}

// quelle est la difference entre printf et sprintf ? c'est quoi l'api POSIX ? explique moi en detail clients[5000] = {0} ? a quoi correspond fd_set ? pourquoi on ecrit struct sockaddr_in et pas juste sockaddr_in? FD_ZERO est une fonction ? commet retrouver le nombre 2130706433 pendant l'examen sans l'apprendre par coeur? si av[1] n'est pas un bon format, que se pass t'il avec htons ? explique moi cet argument de bind() : (const struct sockaddr*)&servaddr? pourquoi listen(10), comment ce chiffre est-il choisi? les fds sont attribues forcement dans l'ordre croissant ?
