/*
 *  Lab problem set for INP course
 *  by Chun-Ying Huang <chuang@cs.nctu.edu.tw>
 *  License: GPLv2
 */
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <assert.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <algorithm>
#include <ctime>
#include <vector>

#define NIPQUAD(m) ((unsigned char *)&(m))[0], ((unsigned char *)&(m))[1], ((unsigned char *)&(m))[2], ((unsigned char *)&(m))[3]
#define errquit(m) \
	{              \
		perror(m); \
		exit(-1);  \
	}

#define MYADDR 0x0a0000fe
#define ADDRBASE 0x0a00000a
#define NETMASK 0xffffff00
#define MTU 1400

int tun_alloc(char *dev) {
	struct ifreq ifr;
	int fd, err;
	if ((fd = open("/dev/net/tun", O_RDWR)) < 0)
		return -1;
	memset(&ifr, 0, sizeof(ifr));
	ifr.ifr_flags = IFF_TUN | IFF_NO_PI; /* IFF_TUN (L3), IFF_TAP (L2), IFF_NO_PI (w/ header) */
	if (dev && dev[0] != '\0')
		strncpy(ifr.ifr_name, dev, IFNAMSIZ);
	if ((err = ioctl(fd, TUNSETIFF, (void *)&ifr)) < 0) {
		close(fd);
		return err;
	}
	if (dev)
		strcpy(dev, ifr.ifr_name);
	return fd;
}

int ifreq_set_mtu(int fd, const char *dev, int mtu) {
	struct ifreq ifr;
	memset(&ifr, 0, sizeof(ifr));
	ifr.ifr_mtu = mtu;
	if (dev)
		strncpy(ifr.ifr_name, dev, IFNAMSIZ);
	return ioctl(fd, SIOCSIFMTU, &ifr);
}

int ifreq_get_flag(int fd, const char *dev, short *flag) {
	int err;
	struct ifreq ifr;
	memset(&ifr, 0, sizeof(ifr));
	if (dev)
		strncpy(ifr.ifr_name, dev, IFNAMSIZ);
	err = ioctl(fd, SIOCGIFFLAGS, &ifr);
	if (err == 0) {
		*flag = ifr.ifr_flags;
	}
	return err;
}

int ifreq_set_flag(int fd, const char *dev, short flag) {
	struct ifreq ifr;
	memset(&ifr, 0, sizeof(ifr));
	if (dev)
		strncpy(ifr.ifr_name, dev, IFNAMSIZ);
	ifr.ifr_flags = flag;
	return ioctl(fd, SIOCSIFFLAGS, &ifr);
}

int ifreq_set_sockaddr(int fd, const char *dev, int cmd, unsigned int addr) {
	struct ifreq ifr;
	struct sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = addr;
	memset(&ifr, 0, sizeof(ifr));
	memcpy(&ifr.ifr_addr, &sin, sizeof(struct sockaddr));
	if (dev)
		strncpy(ifr.ifr_name, dev, IFNAMSIZ);
	return ioctl(fd, cmd, &ifr);
}

int ifreq_set_addr(int fd, const char *dev, unsigned int addr) {
	return ifreq_set_sockaddr(fd, dev, SIOCSIFADDR, addr);
}

int ifreq_set_netmask(int fd, const char *dev, unsigned int addr) {
	return ifreq_set_sockaddr(fd, dev, SIOCSIFNETMASK, addr);
}

int ifreq_set_broadcast(int fd, const char *dev, unsigned int addr) {
	return ifreq_set_sockaddr(fd, dev, SIOCSIFBRDADDR, addr);
}

int udp_bind(struct sockaddr *addr, socklen_t* addrlen, bool is_client, const char *server_host, const char *bind_host, int port) {
  struct addrinfo hints;
  struct addrinfo *result;
  int sock, flags;

  memset(&hints, 0, sizeof(hints));
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_protocol = IPPROTO_UDP;

  if (is_client) {
  	if (0 != getaddrinfo(server_host, NULL, &hints, &result)) {
		perror("getaddrinfo error");
		return -1;
  	}
  } else {
  	if (0 != getaddrinfo(bind_host, NULL, &hints, &result)) {
		perror("getaddrinfo error");
		return -1;
  	}
  }

  if (result->ai_family == AF_INET)
    ((struct sockaddr_in *)result->ai_addr)->sin_port = htons(port);
  else if (result->ai_family == AF_INET6)
    ((struct sockaddr_in6 *)result->ai_addr)->sin6_port = htons(port);
  else {
    fprintf(stderr, "unknown ai_family %d", result->ai_family);
    freeaddrinfo(result);
    return -1;
  }
  memcpy(addr, result->ai_addr, result->ai_addrlen);
  *addrlen = result->ai_addrlen;

  if (-1 == (sock = socket(result->ai_family, SOCK_DGRAM, IPPROTO_UDP))) {
    perror("Cannot create socket");
    freeaddrinfo(result);
    return -1;
  }

  if (!is_client) {
	if (0 != bind(sock, result->ai_addr, result->ai_addrlen)) {
	  perror("Cannot bind");
	  close(sock);
	  freeaddrinfo(result);
	  return -1;
	}
  }

  freeaddrinfo(result);

  flags = fcntl(sock, F_GETFL, 0);
  if (flags != -1) {
    if (-1 != fcntl(sock, F_SETFL, flags | O_NONBLOCK))
      return sock;
  }
  perror("fcntl error");

  close(sock);
  return -1;
}

void encrypt(char *plaintext, char *ciphertext, int len) {
	// XXX: implement your encryption here ...
	memcpy(ciphertext, plaintext, len);
}

void decrypt(char *ciphertext, char *plaintext, int len) {
	// XXX: implement your decryption here ...
	memcpy(plaintext, ciphertext, len);
}

int tunvpn_common(bool is_client, const char *server_host, int port, unsigned int my_addr) {
	char tun_name[IFNAMSIZ];
	int tun_fd;
	if ((tun_fd = tun_alloc(tun_name)) < 0) {
		errquit("tun_alloc()");
	}

	int udp_ifr_fd;
	if ((udp_ifr_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		errquit("socket()");
	}

	if (ifreq_set_mtu(udp_ifr_fd, tun_name, 1400) < 0) {
		errquit("ifreq_set_mtu()");
	}

	if (ifreq_set_addr(udp_ifr_fd, tun_name, my_addr) < 0) {
		errquit("ifreq_set_addr()");
	}

	unsigned int netmask = htonl(NETMASK);
	if (ifreq_set_netmask(udp_ifr_fd, tun_name, netmask) < 0) {
		errquit("ifreq_set_netmask()");
	}

	unsigned int broadcast = htonl(ADDRBASE | ~NETMASK);
	if (ifreq_set_broadcast(udp_ifr_fd, tun_name, broadcast) < 0) {
		errquit("ifreq_set_broadcast()");
	}
	
	short flag;
	if (ifreq_get_flag(udp_ifr_fd, tun_name, &flag) < 0) {
		errquit("ifreq_get_flag()");
	}
	
	if (ifreq_set_flag(udp_ifr_fd, tun_name, flag | IFF_UP) < 0) {
		errquit("ifreq_set_flag()");
	}

	int udp_fd;
	struct sockaddr_storage client_addr;
	socklen_t client_addrlen = sizeof(client_addr);

	char bind_host[] = "0.0.0.0";

	if ((udp_fd = udp_bind((struct sockaddr *)&client_addr, &client_addrlen, is_client, server_host, bind_host, port)) < 0) {
		errquit("udp_bind()");
	}

	// server need to provide tunning for other clients
	// it needs to know where to send the corrsponding packet
	// by maintaining a table of client_addr
	std::vector<struct sockaddr_storage> client_addrs;

	char tun_buf[MTU], udp_buf[MTU];
	bzero(tun_buf, MTU);
	bzero(udp_buf, MTU);

	while (1) {
		fd_set readset;
		FD_ZERO(&readset);
		FD_SET(tun_fd, &readset);
		FD_SET(udp_fd, &readset);
		int max_fd = std::max(tun_fd, udp_fd) + 1;

		if (-1 == select(max_fd, &readset, NULL, NULL, NULL)) {
			perror("select error");
			break;
		}

		int r;
		if (FD_ISSET(tun_fd, &readset)) {
			r = read(tun_fd, tun_buf, MTU);
			if (r < 0) {
				perror("read from tun_fd error");
				break;
			}

			encrypt(tun_buf, udp_buf, r);
			
			if (is_client) {
				r = sendto(udp_fd, udp_buf, r, 0, (const struct sockaddr *)&client_addr, client_addrlen);
				if (r < 0) {
					perror("sendto udp_fd error");
					break;
				}
				continue;
			} else {
				for (auto it = client_addrs.begin(); it != client_addrs.end(); ++it) {
					r = sendto(udp_fd, udp_buf, r, 0, (const struct sockaddr *)&(*it), client_addrlen);
					if (r < 0) {
						perror("sendto udp_fd error");
						break;
					}
				}
			}
		}

		if (FD_ISSET(udp_fd, &readset)) {
			r = recvfrom(udp_fd, udp_buf, MTU, 0, (struct sockaddr *)&client_addr, &client_addrlen);
			if (r < 0) {
				perror("recvfrom udp_fd error");
				break;
			}

			decrypt(udp_buf, tun_buf, r);
			
			if (!is_client) {
				bool is_new_client = true;
				for (auto it = client_addrs.begin(); it != client_addrs.end(); ++it) {
					if (memcmp(&client_addr, &(*it), sizeof(client_addr)) == 0) {
						is_new_client = false;
						break;
					}
				}
				if (is_new_client) {
					client_addrs.emplace_back(client_addr);
				}
			}

			r = write(tun_fd, tun_buf, r);
			if (r < 0) {
				perror("write to tun_fd error");
				break;
			}
		}
	}

	close(tun_fd);
	close(udp_fd);

	return 0;
}

int tunvpn_server(int port) {
	// XXX: implement your server codes here ...
	fprintf(stderr, "## [server] starts ...\n");

	/*
	Your implementation can configure tun0 appropriately on the server. Your program has to configure tun0 appropriately. The involved configurations include (1) setting the MTU to 1400, (2) setting the address to 10.0.0.254/24, and (3) bringing up the tun0 interface. 
	*/

	unsigned int my_addr = htonl(MYADDR);
	tunvpn_common(false, NULL, port, my_addr);

	return 0;
}

int tunvpn_client(const char *server, int port) {
	// XXX: implement your client codes here ...
	fprintf(stderr, "## [client] starts ...\n");

	/*
	The client1 can connect to the server, obtain an assigned network configuration from the server (10.0.0.x/24), create the tun0 interface, and set up the tun0 interface appropriately.
	*/

	srand(time(NULL));

	unsigned int my_addr = htonl(ADDRBASE | (rand() & 0xaa));
	tunvpn_common(true, server, port, my_addr);

	return 0;
}

int usage(const char *progname) {
	fprintf(stderr, "usage: %s {server|client} {options ...}\n"
					"# server mode:\n"
					"	%s server port\n"
					"# client mode:\n"
					"	%s client servername serverport\n",
			progname, progname, progname);
	return -1;
}

int main(int argc, char *argv[]) {
	if (argc < 3) {
		return usage(argv[0]);
	}
	if (strcmp(argv[1], "server") == 0) {
		if (argc < 3)
			return usage(argv[0]);
		return tunvpn_server(strtol(argv[2], NULL, 0));
	} else if (strcmp(argv[1], "client") == 0) {
		if (argc < 4)
			return usage(argv[0]);
		return tunvpn_client(argv[2], strtol(argv[3], NULL, 0));
	} else {
		fprintf(stderr, "## unknown mode %s\n", argv[1]);
	}
	return 0;
}
