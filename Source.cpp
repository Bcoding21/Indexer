#include <fstream>
#include <string>
#include <vector>
#include <fstream>
#include <iterator>
#include <algorithm>
#include <forward_list>
#include <unordered_set>
#include <set>
#include <experimental/filesystem>
#include <iostream>
#include <unordered_map>
#include <map>
#include <queue>

namespace fs = std::experimental::filesystem::v1;


void storeStringLongMap(const std::unordered_map<std::string, unsigned long>&, std::string);

void storeIndex(const std::map<unsigned long, std::set<unsigned long>>&, std::string);

void merge(const std::string&, const std::string&, const std::string&);

std::pair<unsigned long, std::set<unsigned long>> readOneWordDoc(std::ifstream&);

void writeOneWordDoc(const std::pair<unsigned long, std::set<unsigned long>>&, std::ofstream&);


int main(char* argc, char* argv[]) {

	unsigned long wordID = 0; 
	unsigned long docID = 0;

	std::unordered_map <std::string, unsigned long> wordIndex; // maps word to number representing word (cat - 1)
	std::unordered_map <std::string, unsigned long> docIndex; // maps document to number represnenting doc (cat.txt - 1)
	
	std::queue<std::string> blockQueue;

	std::string inputDir = "pa1-data";
	std::string outputDir = "C:\\Users\\Brandon\\source\\repos\\Query\\Query\\";
	
	for (const auto& subDir : fs::directory_iterator(inputDir)) { // iterate through sub directories

		auto outputPath = outputDir  + fs::path(subDir).filename().string(); // create path to store index at later

		blockQueue.push(outputPath); 

		std::map<unsigned long, std::set<unsigned long>> index; // map wordID to DocumentIDs (1 - 1 2 3 4 5)

		for (const auto& file : fs::directory_iterator(subDir)) { // iterate through all files in sub directory

			auto docName = fs::path(file).string(); // get file name

			docIndex.emplace(docName, ++docID); // index file

			std::ifstream fileStream(file); // open file

			std::string word;

			while (fileStream >> word) { // read words until end of file

				auto insertionResult = wordIndex.emplace(word, ++wordID); // attempted to insert word and word ID

				unsigned long currWordID = wordID;

				bool containsKey = !insertionResult.second; // if insertion fails key is already inserted

				if (containsKey) { // if word already in map 

					auto keyValuePair = insertionResult.first; // get key/value pair

					currWordID = keyValuePair->second; // set value for id to currWordID

					--wordID; // take away 1 because a new word was not encountered

				}

				auto set = { docID }; // create set of 1 containing current doc id

				auto insertResult = index.emplace(currWordID, set); // attemp to insert

				containsKey = !insertResult.second;

				if (containsKey) {
					
					auto keyValuePair = insertResult.first; // get key/value pair

					keyValuePair->second.insert(docID); // get set of doc ids

				}

			}

		}

		storeIndex(index, outputPath);

	}

	while (true) {

		if (blockQueue.size() <= 1) {
			break;
		}

		auto filePath1 = blockQueue.front();
		blockQueue.pop();

		auto filePath2 = blockQueue.front();
		blockQueue.pop();

		auto fileName1 = fs::path(filePath1).filename().string();

		auto fileName2 = fs::path(filePath2).filename().string();

		auto mergedFilePath = outputDir + fileName1 + fileName2;

		merge(filePath1, filePath2, mergedFilePath);

		blockQueue.push(mergedFilePath);

		fs::remove(filePath1);
		fs::remove(filePath2);
	}

	std::string indexPath = blockQueue.front();

	storeStringLongMap(wordIndex, outputDir + "wordIndex.bin");
	storeStringLongMap(docIndex, outputDir + "docIndex.bin");
	

	std::cin.get();

}


void storeStringLongMap(const std::unordered_map<std::string, unsigned long>& docMap, std::string filePath) {

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

	}// adfsd
	else {
		std::cout << "Could not create docMap file";
		return; //
	}

}

void storeIndex(const std::map<unsigned long, std::set<unsigned long>>& wordDocMap, std::string filePath) {

	std::ofstream outFile(filePath, std::ofstream::binary);

	std::ostream_iterator<unsigned long> stream_iter(outFile);

	if (outFile) {
		
		for (const auto& pair : wordDocMap) { // loop through map

			outFile.write(reinterpret_cast<const char*>(&pair.first), sizeof(pair.first)); // write wordID

			int listSize = pair.second.size(); // get size of docIdList

			outFile.write(reinterpret_cast<char*>(&listSize), sizeof(listSize)); // store size

			for (const auto& id : pair.second) { // loop through docIdList

				outFile.write(reinterpret_cast<const char*>(&id), sizeof(id)); // write docId

			}
		}

		unsigned int mapSize = wordDocMap.size(); // get size

		outFile.write(reinterpret_cast<char*>(&mapSize), sizeof(int)); // store size
	}
}

std::pair<unsigned long, std::set<unsigned long>> readOneWordDoc(std::ifstream& inFile) {

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

		inFile1.read(reinterpret_cast<char*>(&firstSize), sizeof(firstSize));

		inFile1.clear();

		inFile1.seekg(0);

		inFile2.seekg(-sizeOfInt, std::ios::end);

		unsigned int secondSize = 0;

		inFile2.read(reinterpret_cast<char*>(&secondSize), sizeof(secondSize));

		inFile2.clear();

		inFile2.seekg(0);

		auto pair1 = readOneWordDoc(inFile1);
		unsigned int firstCount = 1;

		auto pair2 = readOneWordDoc(inFile2);
		unsigned int secondCount = 1;

		unsigned int outFileSize = 0;

		while (firstCount < firstSize && secondCount < secondSize) {

			++outFileSize;

			if (pair1.first < pair2.first) {

				writeOneWordDoc(pair1, outFile);
				pair1 = readOneWordDoc(inFile1);
				++firstCount;
			}

			else if (pair1.first > pair2.first) {
				writeOneWordDoc(pair2, outFile);
				pair1 = readOneWordDoc(inFile2);
				++secondCount;
			}

			else {
				std::set<unsigned long> combinedSet;

				std::set_union(
					pair1.second.begin(), pair1.second.end(),
					pair2.second.begin(), pair2.second.end(),
					std::inserter(combinedSet, combinedSet.end()));

				auto combinedPair = std::make_pair(pair1.first, combinedSet);

				writeOneWordDoc(combinedPair, outFile);
				pair1 = readOneWordDoc(inFile1);
				pair2 = readOneWordDoc(inFile2);
				++firstCount;
				++secondCount;
			}

		}

		while (firstCount++ < firstSize) {
			writeOneWordDoc(pair1, outFile);
			pair1 = readOneWordDoc(inFile1);
			outFileSize++;
		}

		while (secondCount++ < secondSize) {
			writeOneWordDoc(pair2, outFile);
			pair1 = readOneWordDoc(inFile2);
			outFileSize++;
		}

		outFile.write(reinterpret_cast<char*>(&outFileSize), sizeof(outFileSize));
	}
}

void writeOneWordDoc(const std::pair<unsigned long, std::set<unsigned long>>& wordDoc, std::ofstream& outFile) {

	outFile.write(reinterpret_cast<const char*>(&wordDoc.first), sizeof(wordDoc.first));

	unsigned int setSize = wordDoc.second.size();

	outFile.write(reinterpret_cast<char*>(&setSize), sizeof(setSize));

	for (const auto& id : wordDoc.second) {

		outFile.write(reinterpret_cast<const char*>(&id), sizeof(id));

	}
}
