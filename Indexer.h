#ifndef Indexer_h
#define Indexer_h

#include <experimental/filesystem>
#include <string>
#include <vector>
#include <fstream>
#include <iterator>
#include "Index.h"
#include "Dictionary.h"

namespace fs = std::experimental::filesystem::v1;

/*
	contains functions that help index files
*/
namespace indexer {

	/*
		Writes data from a Dictionary object to a file in binary.
		@param 1 - Dictionary object that maps terms to a term id.
		@param 2 - File location of where data should be written to.
	*/
	void writeDictionary(const Dictionary&, const fs::path&);


	/*
		Writes data from an Index object to a file in binary.
		@param 1 - Dictionary object that maps terms to a term id.
		@param 2 - Path of where data should be written to.
	*/
	void writeIndex(const Index&, const fs::path&);


	/*
		Combines data from two index files into one.
		@param 1 - location of the first index.
		@param 2 - location of the second index.
		@param 3 - location of where the new index will be stored.
	*/
	void mergeIndexFiles(const fs::path&, const fs::path&, const fs::path&);

}
#endif
