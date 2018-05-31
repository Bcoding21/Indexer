#ifndef Index_h
#define Index_h

#include <unordered_map>
#include <vector>
#include <fstream>


class Index {

private:
	std::unordered_map<unsigned long, std::vector<unsigned long>> data;

public:

	void add(unsigned long, unsigned long);

	void write(const std::string&);
};

#endif