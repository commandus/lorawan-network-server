
/**
 * Usage
 * ./test-udp-listen
 * echo 123 | nc -u -6 ::1 9000 -w 0
 * echo 123 | nc -u -4 localhost 9001 -w 0
 * 
 * @see https://stackoverflow.com/questions/15260879/c-simple-ipv6-udp-server-using-select-to-listen-on-multiple-ports-receiving-m
 */

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <stdio.h>
#include <string.h>
#include <iostream>

#include "udp-socket.h"
#include "errlist.h"

int t2()
{
	// UDPSocket s6("::1:9000", MODE_OPEN_SOCKET_LISTEN, MODE_FAMILY_HINT_IPV6);
	UDPSocket s6(":9000", MODE_OPEN_SOCKET_LISTEN, MODE_FAMILY_HINT_IPV6);
	if (s6.errcode) {
		return s6.errcode;
	}
	// UDPSocket s4("127.0.0.1:9001", MODE_OPEN_SOCKET_LISTEN, MODE_FAMILY_HINT_IPV4);
	UDPSocket s4(":9001", MODE_OPEN_SOCKET_LISTEN, MODE_FAMILY_HINT_IPV4);
	if (s4.errcode) {
		return s4.errcode;
	}

    while (1)
    {
        fd_set read_handles;
	    FD_ZERO(&read_handles);
        FD_SET(s6.sock, &read_handles);
		FD_SET(s4.sock, &read_handles);

		struct timeval timeout_interval;
        timeout_interval.tv_sec = 2;
        timeout_interval.tv_usec = 0;
        int rs = select(s4.sock + 1, &read_handles, NULL, NULL, &timeout_interval);
        switch (rs) {
			case -1: 
				printf("Select error\n");
				break;
			case 0:
            	// printf("timeout\n");
				break;
        	default:
			{
				struct sockaddr_in client_address4;
				struct sockaddr_in6 client_address6;
				int bytes_received;
				char buffer[1000];
				if (FD_ISSET(s6.sock, &read_handles)) {
					if ((bytes_received = s6.recv(buffer, sizeof(buffer), &client_address6)) < 0) {
						perror("Error in recvfrom.");
						break;
					}
					printf("%.*s", bytes_received, buffer);
				}

				if (FD_ISSET(s4.sock, &read_handles)) {
					if ((bytes_received = s4.recv(buffer, sizeof(buffer), &client_address4)) < 0) {
						perror("Error in recvfrom.");
						break;
					}
					printf("%.*s", bytes_received, buffer);
				}
			}
        }
    }
}

int main(int argc, char **argv) {
	int r = t2();
	std::cerr << r << ": " << strerror_client(r) << std::endl;
}
