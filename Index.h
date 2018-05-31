#ifndef Index_h
#define Index_h

#include <map>
#include <set>
#include <fstream>


class Index {

private:
	std::map<unsigned long, std::set<unsigned long>> data;

public:

	void add(unsigned long, unsigned long);

	void write(const std::string&);
};

#endif