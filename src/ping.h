#pragma once

int pingAll(std::string* pWorking_ips, int i, int& y, std::string range, std::mutex& ips_mutex, int argc, char *argv[]);
int pingSelected(std::string selAddress, int argc, char *argv[]);
