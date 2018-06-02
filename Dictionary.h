#ifndef Dictionary_h
#define Dictionary_h

#include <unordered_map>
#include <string>
#include <fstream>

namespace indexer {

	/*
		This class represents a dictionary
		that keeps a record of terms and 
		ids that are mapped to a term.
	*/
	class Dictionary {

	private:
		std::unordered_map<std::string, unsigned long> data;
		unsigned long counter;
		unsigned long lastQueried;

	public:
		Dictionary();

		void add(const std::string&);

		unsigned long getLastQueriedId() const;

	};
}
#endif