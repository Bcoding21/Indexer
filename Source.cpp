#include <fstream>
#include <string>
#include <vector>
#include <iterator>
#include <algorithm>
#include <unordered_set>
#include <set>
#include <experimental/filesystem>
#include <iostream>
#include <unordered_map>
#include <map>
#include <queue>
#include <deque>
#include <execution>

namespace fs = std::experimental::filesystem::v1; // mainpulates filesystem

/*
	Writes contents of a term Index to a file in binary.
	Param 1 - map that maps terms to a unique term id. (cat - 1), (dog - 2), (fish - 3)
	Param 2 - path to write data to. 
*/
void writeTermIndex(const std::unordered_map<std::string, unsigned long>&, std::string);

/*
	Writes contents of term Id index to a file in binary
	Param 1 - map that maps unique term ids to a list of unique document 
	ids the term occurs in.  (1 - 1,2,3,4,5) (2, 3, 34, 44, 5) (3, 4, 5, 6 ,6)
	Param 2 - path to write data to
*/
void writeTermIdIndex(std::map<unsigned long, std::set<unsigned long>>&, std::string);


/*
	Combines two term id indexes and writes the combined index to a file in binary.
	Param 1 - path of file that stores term index (termIndex1.bin)
	Param 2 - path of file that stores term index (termIndex2.bin)
	Param 3 - desired path of the new index. (termIndex1and2.bin)
*/
void merge(const std::string&, const std::string&, const std::string&);

/*
	Reads one term id and coressponding list of document ids from a binary file.
	Param 1 - filestream object used to read a file.
	Returns - Pair consisting of a term id and a list of document ids.
*/
std::pair<unsigned long, std::vector<unsigned long>> readTermIdEntry(std::ifstream&);

/*
	Writes one entry to a file in binary.
	Param 1 - Pair that holds a unique term id and a list of unique document ids
	Param 2 - ofstream variable used to write to an output.
*/
void writeTermIdEntry(const std::pair<unsigned long, std::vector<unsigned long>>&, std::ofstream&);


int main(char* argc, char* argv[]) {

	unsigned long termId = 0; 
	unsigned long docId = 0;

	std::unordered_map <std::string, unsigned long> termIndex; // maps terms to unique term id
	std::unordered_map <std::string, unsigned long> docIndex; // maps document to unique document id
	
	std::queue<std::string> outputPaths; 

	std::string baseDir = "pa1-data";
	std::string outputDir = "C:\\Users\\Brandon\\source\\repos\\Query\\Query\\";
	
	for (const auto& subDir : fs::directory_iterator(baseDir)) { 

		auto outputPath = outputDir + fs::path(subDir).filename().string();

		outputPaths.push(outputPath); 

		std::map<unsigned long, std::set<unsigned long>> termIdIndex; // maps unique term id to list of unique document ids the term occured in

		for (const auto& file : fs::directory_iterator(subDir)) {

			auto docTitle = fs::path(file).filename().string(); 

			auto iterator = docIndex.emplace(docTitle, docId); 

			docId += iterator.second; // iterator.second is bool denoting if emplacement pass/failed.

			unsigned long currDocId = iterator.first->second; //iterator.first is key/value pair.

			std::ifstream stream(file); 
			std::istream_iterator<std::string> start(stream), end;
			std::vector<std::string> terms(start, end);

			for (const auto& term : terms) { 

				auto iterator = termIndex.emplace(term, termId); 
				termId += iterator.second; 

				unsigned long currTermId = iterator.first->second; 

				auto newDocIdList = { docId }; 
				auto it = termIdIndex.emplace(currTermId, newDocIdList); 
				bool containsTermId = !it.second;

				if (containsTermId) {
					it.first->second.emplace(currDocId); 
				}
			}
		}
		writeTermIdIndex(termIdIndex, outputPath);
	}

	while (true) {

		/*
		Creates one list by merging two.
		The new list is added to the queue.
		The two list used to merge are deleted.
		One list left in the end.
		*/

		if (outputPaths.size() <= 1) {
			break;
		}

		auto path1 = outputPaths.front();
		outputPaths.pop();

		auto path2 = outputPaths.front();
		outputPaths.pop();

		auto docTitle1 = fs::path(path1).filename().string();

		auto docTitle2 = fs::path(path2).filename().string();

		auto newPath = outputDir + docTitle1 + docTitle2;

		merge(path1, path2, newPath);

		outputPaths.push(newPath);

		fs::remove(path1);
		fs::remove(path2);
	}

	std::string indexPath = outputPaths.front();
	writeTermIndex(termIndex, outputDir + "wordIndex.bin");
	writeTermIndex(docIndex, outputDir + "docIndex.bin"); // same method used for writing doc index so same function used
	std::cin.get();
}


void writeTermIndex(const std::unordered_map<std::string, unsigned long>& index, std::string path) {

	std::ofstream stream(path, std::ofstream::binary);

	if (stream.is_open()) {

		unsigned int indexSize = index.size();

		stream.write(reinterpret_cast<char*>(&indexSize), sizeof(indexSize));

		for (const auto& entry : index) {

			stream.write(reinterpret_cast<const char*>(&entry.second), sizeof(entry.second));

			short stringSize = entry.first.size();

			stream.write(reinterpret_cast<char*> (&stringSize), sizeof(stringSize));

			stream.write(entry.first.c_str(), stringSize);

		}

	}
	else {
		std::cout << "Could not create docMap file";
		return;
	}
}

void writeTermIdIndex(std::map<unsigned long, std::set<unsigned long>>& termIdIndex, std::string path) {

	std::ofstream stream(path, std::ofstream::binary);

	if (stream.is_open()) {
		
		for (auto& entry : termIdIndex) { 

			stream.write(reinterpret_cast<const char*>(&entry.first), sizeof(entry.first));

			int listSize = entry.second.size(); 

			stream.write(reinterpret_cast<char*>(&listSize), sizeof(listSize));

			std::ostream_iterator<char> streamIter(stream);

			std::copy(entry.second.begin(), entry.second.end(), streamIter); 

		}

		unsigned int indexSize = termIdIndex.size(); 

		stream.write(reinterpret_cast<char*>(&indexSize), sizeof(indexSize)); 
	}
}

std::pair<unsigned long, std::vector<unsigned long>> readTermIdEntry(std::ifstream& stream) {

	long termId = 0;

	stream.read(reinterpret_cast<char*>(&termId), sizeof(termId));

	int listSize = 0;

	stream.read(reinterpret_cast<char*>(&listSize), sizeof(listSize));

	std::vector<unsigned long> docIdSet(listSize);

	stream.read(reinterpret_cast<char*>(&docIdSet[0]), sizeof(docIdSet[0]) * listSize);

	return std::make_pair(termId, docIdSet);
}

void merge(const std::string& path1, const std::string& path2, const std::string& path3) {


	std::ifstream inStream1(path1, std::ifstream::binary | std::ifstream::ate); 
	std::ifstream inStream2(path2, std::ifstream::binary | std::ifstream::ate);
	std::ofstream outStream(path3, std::ofstream::binary);

	if (inStream1.is_open() && inStream2.is_open() && outStream.is_open()) { 

		int sizeOfInt = sizeof(int);

		inStream1.seekg(-sizeOfInt, std::ios::end); 

		unsigned int indexOneSize = 0;

		inStream1.read(reinterpret_cast<char*>(&indexOneSize), sizeof(indexOneSize)); 

		inStream1.clear(); // clear eof flag

		inStream1.seekg(0);

		inStream2.seekg(-sizeOfInt, std::ios::end); 

		unsigned int indexTwoSize = 0;

		inStream2.read(reinterpret_cast<char*>(&indexTwoSize), sizeof(indexTwoSize));

		inStream2.clear(); // clear eof flag

		inStream2.seekg(0);

		auto entry1 = readTermIdEntry(inStream1);
		unsigned int indexOneCount = 1;

		auto entry2 = readTermIdEntry(inStream2);
		unsigned int indexTwoCount = 1;

		unsigned int newIndexSize = 0;

		/* similar to merging sorted list
		reading file increments file pointer
		*/

		while (indexOneCount < indexOneSize && indexTwoCount < indexTwoSize) { 

			++newIndexSize;

			if (entry1.first < entry2.first) {
				writeTermIdEntry(entry1, outStream);
				entry1 = readTermIdEntry(inStream1);
				++indexOneCount;
			}

			else if (entry1.first > entry2.first) {
				writeTermIdEntry(entry2, outStream);
				entry2 = readTermIdEntry(inStream2);
				++indexTwoCount;
			}

			else { 
				std::vector<unsigned long> combinedList;

				combinedList.reserve(
					std::max(entry1.second.size(), entry2.second.size())
				);

				std::set_union(
					entry1.second.begin(), entry1.second.end(),
					entry2.second.begin(), entry2.second.end(),
					std::back_inserter(combinedList)
				);

				auto newEntry = std::make_pair(entry1.first, combinedList);
				writeTermIdEntry(newEntry, outStream);

				entry1 = readTermIdEntry(inStream1);
				entry2 = readTermIdEntry(inStream2);
				++indexOneCount;
				++indexTwoCount;
			}

		}

		while (indexOneCount++ < indexOneSize) {
			writeTermIdEntry(entry1, outStream);
			entry1 = readTermIdEntry(inStream1);
			newIndexSize++;
		}

		while (indexTwoCount++ < indexTwoSize) {
			writeTermIdEntry(entry2, outStream);
			entry1 = readTermIdEntry(inStream2);
			newIndexSize++;
		}

		outStream.write(reinterpret_cast<char*>(&newIndexSize), sizeof(newIndexSize));
	}
}

void writeTermIdEntry(const std::pair<unsigned long, std::vector<unsigned long>>& entry, std::ofstream& stream) {

	stream.write(reinterpret_cast<const char*>(&entry.first), sizeof(entry.first));

	unsigned int vecSize = entry.second.size();

	stream.write(reinterpret_cast<char*>(&vecSize), sizeof(vecSize));

	stream.write(reinterpret_cast<const char*>(&entry.second[0]), sizeof(entry.second[0]) * vecSize);
}
