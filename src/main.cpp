#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <vector>
#include <mutex>
#include <cstdlib>

#include "icmplib.h"

int pingAll(std::string* pWorking_ips, int i, int& y, std::string range, std::mutex& ips_mutex, int argc, char *argv[]) {
    std::string id = std::to_string(i);

    std::string address = range + id, resolved;
    uint16_t packet_size = ICMPLIB_PING_DATA_SIZE;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-l" && i + 1 < argc) {
            try {
            int size = std::stoi(argv[++i]);
            if (size < 0 || size > ICMPLIB_MAX_PING_DATA_SIZE) {
                std::cerr << "Packet size must be between 0 and " << ICMPLIB_MAX_PING_DATA_SIZE << "." << std::endl;
                return EXIT_FAILURE;
            }
            packet_size = static_cast<uint16_t>(size);
            } catch (...) {
                std::cerr << "Invalid packet size value." << std::endl;
                return EXIT_FAILURE;
            }
        } else {
            address = arg;
        }
    }

    try {
        if (!icmplib::IPAddress::IsCorrect(address, icmplib::IPAddress::Type::Unknown)) {
            resolved = address; address = icmplib::IPAddress(address);
        }
    } catch (...) {
        std::cerr << "Ping request could not find host " << address << ". Please check the name and try again." << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "Pinging " << (resolved.empty() ? address : resolved + " [" + address + "]") << " with " << packet_size << " bytes of data:" << std::endl;
    auto result = icmplib::Ping(address, ICMPLIB_TIMEOUT_1S, 1, 255, packet_size);
    switch (result.response) {
    case icmplib::PingResponseType::Failure:
        std::cerr << "Network error." << std::endl;
        break;
    case icmplib::PingResponseType::Timeout:
        std::cout << "Request timed out." << std::endl;
        break;
    default:
        std::cout << "Reply from " << static_cast<std::string>(result.address) << ": ";
        switch (result.response) {
        case icmplib::PingResponseType::Success:
            std::cout << "time=" << result.delay;
            if (result.address.GetType() != icmplib::IPAddress::Type::IPv6) {
                std::cout << " TTL=" << static_cast<unsigned>(result.ttl);
            }
            {
                std::lock_guard<std::mutex> lock(ips_mutex);
                pWorking_ips[y] = address;
                y++;
            }
            break;
        case icmplib::PingResponseType::Unreachable:
            std::cout << "Destination unreachable.";
            break;
        case icmplib::PingResponseType::TimeExceeded:
            std::cout << "Time exceeded.";
            break;
        default:
            std::cout << "Response not supported.";
        }
        std::cout << std::endl;
    }
    return EXIT_SUCCESS;
}

int main(int argc, char* argv[])
{
    std::string pWorking_ips[255];

    std::string range;
    std::cout << "Please input your check range for example: '192.168.1.': ";
    std::cin >> range;

    int MAX = 255;
    int y = 0;
    int e;

    std::vector<std::thread> threads;
    std::mutex ips_mutex;

    for (int i = 0; i < MAX; i++) 
        threads.emplace_back(pingAll, pWorking_ips, i, std::ref(y), range, std::ref(ips_mutex), argc, argv);
    
    for (auto& t : threads)
        t.join();

    std::cout << "Available IP's: " << std::endl;
    for (int v = 0; v < y; v++) {
        
        struct addrinfo hints;
        struct addrinfo *res=0;
        int status;

        WSADATA wsadata;
        int statuswsadata;
        if ((statuswsadata=WSAStartup(MAKEWORD(2, 2), &wsadata))!=0) {
            printf("WSAStartup failed: %d\n", statuswsadata);
        }

        hints.ai_family = AF_INET;

        const char *cstr = pWorking_ips[v].c_str();
        status = getaddrinfo(cstr, 0, 0, &res);

        {
            char host[512], port[128];
            status = getnameinfo(res->ai_addr, res->ai_addrlen, host, 512, 0, 0, 0);

            std::cout << v << ". ";
            printf("Host: %s", host);
            std::cout << " - " << pWorking_ips[v] << std::endl;

            freeaddrinfo(res);
        }
    }
    std::cin >> e;
    return 0;
    
}