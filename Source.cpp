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

namespace fs = std::experimental::filesystem::v1;

/*
	Writes contents of a term Index to a file binary.
	Param 1 - map that maps terms to a unique term id.
	Param 2 - path to write data to.
*/
void writeTermIndex(const std::unordered_map<std::string, unsigned long>&, std::string);

/*
	Writes contents of term Id index to a file in binary
	Param 1 - map that maps unique term ids to a list of unique document
	ids the term occurs in.
	Param 2 - path to write data to
*/
void writeTermIdIndex(std::map<unsigned long, std::vector<unsigned long>>&, std::string);


/*
	Combines two term id indexes and writes the combined index to a file in binary.
	Param 1 - path of file where term id Index is stored.
	Param 2 - path of file where term id Index is stored.
	Param 3 - desired path of the new index.
*/
void merge(const std::string&, const std::string&, const std::string&);

/*
	Reads one term id and coressponding list of document ids from a binary file.
	Param 1 - filestream object used to read a file.
	Returns - Pair consisting of a term id and a list of document ids.
*/
std::pair<unsigned long, std::set<unsigned long>> readTermIdEntry(std::ifstream&);

/*
	Writes one entry to a file in binary.
	Param 1 - Pair that holds a unique term id and a list of unique document ids
	Param 2 - ofstream variable used to write to an output.
*/
void writeTermIdEntry(const std::pair<unsigned long, std::set<unsigned long>>&, std::ofstream&);


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

		std::map<unsigned long, std::vector<unsigned long>> termIdIndex; // maps unique term id to list of unique document ids the term occured in

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
					it.first->second.push_back(currDocId); 
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

		auto filePath1 = outputPaths.front();
		outputPaths.pop();

		auto filePath2 = outputPaths.front();
		outputPaths.pop();

		auto fileName1 = fs::path(filePath1).filename().string();

		auto fileName2 = fs::path(filePath2).filename().string();

		auto mergedFilePath = outputDir + fileName1 + fileName2;

		merge(filePath1, filePath2, mergedFilePath);

		outputPaths.push(mergedFilePath);

		fs::remove(filePath1);
		fs::remove(filePath2);
	}

	std::string indexPath = outputPaths.front();

	writeTermIndex(termIndex, outputDir + "wordIndex.bin");
	writeTermIndex(docIndex, outputDir + "docIndex.bin");
	

	std::cin.get();

}


void writeTermIndex(const std::unordered_map<std::string, unsigned long>& docMap, std::string filePath) {

	std::ofstream outFile(filePath, std::ofstream::binary);

	if (outFile) {

		unsigned int mapSize = docMap.size();

		outFile.write(reinterpret_cast<char*>(&mapSize), sizeof(mapSize));

		std::for_each(docMap.begin(), docMap.end(), [&outFile](const auto& keyValuePair) {

			outFile.write(reinterpret_cast<const char*>(&keyValuePair.second), sizeof(keyValuePair.second));

			short stringSize = keyValuePair.first.size();

			outFile.write(reinterpret_cast<char*> (&stringSize), sizeof(stringSize));

			outFile.write(keyValuePair.first.c_str(), stringSize);

		});

	}
	else {
		std::cout << "Could not create docMap file";
		return;
	}

}

void writeTermIdIndex(std::map<unsigned long, std::vector<unsigned long>>& wordDocMap, std::string filePath) {

	std::ofstream outFile(filePath, std::ofstream::binary);

	if (outFile) {
		
		for (auto& pair : wordDocMap) { 

			std::sort(std::execution::par_unseq, pair.second.begin(), pair.second.end());

			outFile.write(reinterpret_cast<const char*>(&pair.first), sizeof(pair.first));

			int listSize = pair.second.size(); 

			outFile.write(reinterpret_cast<char*>(&listSize), sizeof(listSize));

			std::ostream_iterator<char> out_iter(outFile);

			std::copy(pair.second.begin(), pair.second.end(), out_iter); 

		}

		unsigned int mapSize = wordDocMap.size(); 

		outFile.write(reinterpret_cast<char*>(&mapSize), sizeof(int)); 
	}
}

std::pair<unsigned long, std::set<unsigned long>> readTermIdEntry(std::ifstream& inFile) {

	long wordID = 0;

	inFile.read(reinterpret_cast<char*>(&wordID), sizeof(wordID));

	int listSize = 0;

	inFile.read(reinterpret_cast<char*>(&listSize), sizeof(listSize));

	std::set<unsigned long> docIDs;

	for (int j = 0; j < listSize; j++) {

		unsigned long docID = 0;

		inFile.read(reinterpret_cast<char*>(&docID), sizeof(docID));

		docIDs.insert(docID);

	}

	return std::make_pair(wordID, docIDs);
}

void merge(const std::string& filePath1, const std::string& filePath2, const std::string& filePath3) {


	std::ifstream inFile1(filePath1, std::ifstream::binary | std::ifstream::ate); 
	std::ifstream inFile2(filePath2, std::ifstream::binary | std::ifstream::ate);
	std::ofstream outFile(filePath3, std::ofstream::binary);

	if ((inFile1 && inFile2) && outFile) { 

		int sizeOfInt = sizeof(int);

		inFile1.seekg(-sizeOfInt, std::ios::end); 

		unsigned int firstSize = 0;

		inFile1.read(reinterpret_cast<char*>(&firstSize), sizeof(firstSize)); // read last for bytes to get size

		inFile1.clear();

		inFile1.seekg(0);

		inFile2.seekg(-sizeOfInt, std::ios::end); 

		unsigned int secondSize = 0;

		inFile2.read(reinterpret_cast<char*>(&secondSize), sizeof(secondSize));

		inFile2.clear();

		inFile2.seekg(0);

		auto pair1 = readTermIdEntry(inFile1);
		unsigned int firstCount = 1;

		auto pair2 = readTermIdEntry(inFile2);
		unsigned int secondCount = 1;

		unsigned int outFileSize = 0;

		/* similar to merging sorted list
		reading file increments file pointer
		to the start of element
		*/

		while (firstCount < firstSize && secondCount < secondSize) { 

			++outFileSize;

			if (pair1.first < pair2.first) {

				writeTermIdEntry(pair1, outFile);
				pair1 = readTermIdEntry(inFile1);
				++firstCount;
			}

			else if (pair1.first > pair2.first) {
				writeTermIdEntry(pair2, outFile);
				pair1 = readTermIdEntry(inFile2);
				++secondCount;
			}

			else { 
				std::set<unsigned long> combinedSet;

				std::set_union( 
					pair1.second.begin(), pair1.second.end(),
					pair2.second.begin(), pair2.second.end(),
					std::inserter(combinedSet, combinedSet.end()));

				auto combinedPair = std::make_pair(pair1.first, combinedSet);

				writeTermIdEntry(combinedPair, outFile);
				pair1 = readTermIdEntry(inFile1);
				pair2 = readTermIdEntry(inFile2);
				++firstCount;
				++secondCount;
			}

		}

		while (firstCount++ < firstSize) {
			writeTermIdEntry(pair1, outFile);
			pair1 = readTermIdEntry(inFile1);
			outFileSize++;
		}

		while (secondCount++ < secondSize) {
			writeTermIdEntry(pair2, outFile);
			pair1 = readTermIdEntry(inFile2);
			outFileSize++;
		}

		outFile.write(reinterpret_cast<char*>(&outFileSize), sizeof(outFileSize));
	}
}

void writeTermIdEntry(const std::pair<unsigned long, std::set<unsigned long>>& wordDoc, std::ofstream& outFile) {

	outFile.write(reinterpret_cast<const char*>(&wordDoc.first), sizeof(wordDoc.first));

	unsigned int setSize = wordDoc.second.size();

	outFile.write(reinterpret_cast<char*>(&setSize), sizeof(setSize));

	for (const auto& id : wordDoc.second) {

		outFile.write(reinterpret_cast<const char*>(&id), sizeof(id));

	}
}
