#ifndef IndexUtility_h
#define IndexUtility_h

#include <experimental/filesystem>
#include <string>
#include <vector>
#include <fstream>
#include <iterator>

namespace fs = std::experimental::filesystem::v1;

namespace iu {

	void merge(const std::string&, const std::string&, const std::string&);

	std::pair<unsigned long, std::vector<unsigned long>> readOneIndexEntry(std::ifstream&);

	void writeOneIndexEntry(const std::pair<unsigned long,
		std::vector<unsigned long>>&, std::ofstream&);

	int getIndexSize(std::ifstream&);

	std::pair<unsigned long, std::vector<unsigned long>> getCombinedEntry(
		const std::pair<unsigned long, std::vector<unsigned long>>&,
		const std::pair<unsigned long, std::vector<unsigned long>>&);

	std::vector<std::string> getTermsFromFile(const std::string&);

	std::string getDocTitle(const fs::directory_entry&);
}

#endif
