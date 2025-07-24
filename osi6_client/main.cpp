#include <iostream>
#include <sstream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <optional>

enum {
	SUCCESS,
	ERR,
	EXIT
};

struct CLIENT {
	int socket;
	sockaddr_in socket_addr;
};

CLIENT client;

int make_client(const std::string& server_ip, const int port) {
	client.socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (client.socket < 0) {
		std::cout << "Socket creation failed\n\n";
		perror("socket");
		return ERR;
	}

	client.socket_addr.sin_family = AF_INET;
	client.socket_addr.sin_port = htons(port);
	inet_pton(AF_INET, server_ip.data(), &client.socket_addr.sin_addr);

	return SUCCESS;

}

int connect_server() {
	int ret_val;

	ret_val = connect(client.socket, (sockaddr*)&client.socket_addr, sizeof(client.socket_addr));
	if (ret_val < 0) {
		std::cout << "Connecting failed \n\n";
		perror("connect");
		close(client.socket);
		return ERR;
	}

	return SUCCESS;
}

std::optional<std::string> sum(const std::string& summands) {
	int ret_val;

	ret_val = send(client.socket, summands.data(), summands.size(), 0);

	if (ret_val < 0) {
		std::cout << "Sending failed\n\n";
		perror("send");
		close(client.socket);
		return std::nullopt;
	}

	if (summands.compare("exit") == 0) {
		close(client.socket);
		return std::nullopt;
	}

	std::string res;
	res.resize(1024);

	ret_val = recv(client.socket, (char*)res.data(), 1024, 0);

	if (ret_val < 0) {
		std::cout << "Receiving failed\n\n";
		perror("recv");
		close(client.socket);
		return std::nullopt;
	}

	res.resize(ret_val);

	return res;

}

int menu() {
	int ret_val = -1;
	while (true) {
		std::cout << "Input server IP and port space-separated or \"exit\" for exit" << std::endl;

		std::string server_ip;
		std::cin >> server_ip;
		if (server_ip.compare("exit") == 0)
			return EXIT;

		int port;
		std::cin >> port;

		std::cin.ignore();

		ret_val = make_client(server_ip, port);

		if (ret_val == ERR)
			continue;

		ret_val = connect_server();

		if (ret_val == ERR)
			continue;

		std::string summands;
		while (true) {
			std::cout << "Input 2 summands space separated or \"stop\" for exit\n";

			std::getline(std::cin, summands);
			if (summands == "exit") {
				close(client.socket);
				return EXIT;
			}

			auto res = sum(summands);
			
			if (res.has_value())
				std::cout << "Sum: " << *res << "\n\n";
			else
				break;
		}
	}
}

int main() {
	setlocale(LC_ALL, "rus");

	menu();
	
	return SUCCESS;
}