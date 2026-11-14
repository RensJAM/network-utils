#include <iostream>
#include <thread>
#include <vector>
#include <mutex>

#include "icmplib.h"

#include "ping.h"

int main(int argc, char* argv[])
{
    std::string pWorking_ips[255];

    std::string range;
    std::cout << "Please input your check range for example: '192.168.1.': ";
    std::cin >> range;

    int MAX = 255;
    int y = 0;

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

    std::cout << "\n";

    std::cout << "Please select a device: ";
    int selectedHost;
    int programID;
    int amount;
    std::cin >> selectedHost;
    while (std::cin.fail())
    {
        std::cin.clear();
        std::cin.ignore();
        std::cout << "Please enter a valid id/number. ";
        std::cin >> selectedHost;
    }
    std::cout << "Selected: " << pWorking_ips[selectedHost] << std::endl;
    std::cout << "\n";

    std::cout << "What would you like to do with the selected host?" << std::endl;
    std::cout << "1. Ping the selected host." << std::endl;
    std::cout << "\n";

    std::cout << "Enter the number here: ";
    std::cin >> programID;
    while (std::cin.fail())
    {
        std::cin.clear();
        std::cin.ignore();
        std::cout << "Please enter a valid id/number. ";
        std::cin >> programID;
    }
    std::cout << "Selected option: " << programID << std::endl;
    std::cout << "\n";
    if (programID == 1)
    {
        std::cout << "How many times would you want to ping this device? ";
        std::cin >> amount;
        while (std::cin.fail())
        {
            std::cin.clear();
            std::cin.ignore();
            std::cout << "Please enter a valid amount." << std::endl;
            std::cin >> amount;
        }
        for (int i = 0; i < amount; i++) 
            threads.emplace_back(pingSelected, pWorking_ips[selectedHost], argc, argv);
    
        for (auto& t : threads)
            t.join();
    }

    return 0;
    
}