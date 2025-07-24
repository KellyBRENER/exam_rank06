// #include <errno.h>
#include <string.h>
#include <unistd.h>
// #include <netdb.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>

void	err() {
	write(2, "Fatal error\n", 12);
	exit(1);
}

void	send_to_all(char* msg, int server, int client, int max_fd) {
	for (int i = 0; i <= max_fd; i++) {
		if (i != server && i != client)
			send(i, msg, strlen(msg), 0);
	}
}

int	main(int argc, char** argv) {
	int	server, max_fd, bytes_read, clients[65565] = {0}, client, id = 0;
	fd_set	all_fds, tmp_fds;
	struct sockaddr_in servaddr;
	char msg_send[400000], msg_rcv[400000];
	if (argc != 2) {
		write(2, "Wrong number of arguments\n", 26);
		exit(1);
	}
	server = socket(AF_INET, SOCK_STREAM, 0);
	if (server < 0)
		err();
	FD_ZERO(&all_fds);
	FD_ZERO(&tmp_fds);
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(2130706433);
	servaddr.sin_port = htons(atoi(argv[1]));
	if (bind(server, (const struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
		err();
	if (listen(server, 10) < 0)
		err();
	FD_SET(server, &all_fds);
	max_fd = server;
	while (1) {
		tmp_fds = all_fds;
		if (select(max_fd + 1, &tmp_fds, 0, 0, 0) < 0)
			err();
		for (int fd = 0; fd <= max_fd; fd++) {
			if (!FD_ISSET(fd, &tmp_fds))
				continue;
			bzero(&msg_send, sizeof(msg_send));
			if (fd == server) {
				if (client = accept(server, 0, 0) < 0)
					err();
				clients[client] = id++;
				if (client > max_fd)
					max_fd = client;
				FD_SET(client, &all_fds);
				sprintf(msg_send, "server: client %d just arrived\n", clients[client]);
				send_to_all(msg_send, server, client, max_fd);
			} else {
				bzero(&msg_rcv, sizeof(msg_rcv));
				bytes_read = 1;
				while (bytes_read == 1 && (!msg_rcv[0] || msg_rcv[strlen(msg_rcv) - 1] != '\n')) {
					bytes_read = recv(fd, msg_rcv[strlen(msg_rcv)], 1, 0);
				}
				if (bytes_read <= 0) {
					sprintf(msg_send, "server: client %d just left\n", clients[fd]);
					send_to_all(msg_send, server, fd, max_fd);
					FD_CLR(fd, &all_fds);
					close(fd);
				} else {
					sprintf(msg_send, "client %d: %s", clients[fd], msg_rcv);
					send_to_all(msg_send, server, fd, max_fd);
				}
			}
		}
		return(0);
	}
}
