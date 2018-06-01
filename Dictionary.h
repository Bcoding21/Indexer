#ifndef Dictionary_h
#define Dictionary_h

#include <unordered_map>
#include <string>
#include <fstream>

class Dictionary {

private:
	std::unordered_map<std::string, unsigned long> data;
	unsigned long counter;
	unsigned long currId;

public:
	Dictionary();

	void add(const std::string&);

	unsigned long getCurrId() const;

};

#endif