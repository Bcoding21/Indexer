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


void storeStringLongMap(const std::unordered_map<std::string, unsigned long>&, std::string);

void storeIndex(std::map<unsigned long, std::vector<unsigned long>>&, std::string);

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
	
	for (const auto& subDir : fs::directory_iterator(inputDir)) { 

		auto outputPath = outputDir + fs::path(subDir).filename().string();

		blockQueue.push(outputPath);  // store location of file

		std::map<unsigned long, std::vector<unsigned long>> wordDocIndex; // map wordID to DocumentIDs (1 - 1 2 3 4 5)

		for (const auto& file : fs::directory_iterator(subDir)) {

			auto docName = fs::path(file).filename().string(); 

			auto success = docIndex.emplace(docName, docID);
			// returns pair pair.first is key/value
			// pair.second is bool val denoting insertion sucess

			docID += success.second; 

			unsigned long currDocID = success.first->second;

			std::ifstream fileStream(file); 
			std::istream_iterator<std::string> start(fileStream), end;
			std::vector<std::string> words(start, end);

			for (const auto& word: words) { 

				auto ok = wordIndex.emplace(word, wordID); 
				wordID += ok.second; 

				unsigned long currWordID = ok.first->second;

				auto set = { docID }; 
				auto good = wordDocIndex.emplace(currWordID, set); 

				if (!good.second) {
					good.first->second.push_back(currDocID); 
				}
			}
		}
		storeIndex(wordDocIndex, outputPath);
	}

	while (true) {

		/*
		Creates one list by merging two.
		The new list is added to the queue.
		The two list used to merge are deleted.
		One list left in the end.
		*/

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

	}
	else {
		std::cout << "Could not create docMap file";
		return;
	}

}

void storeIndex(std::map<unsigned long, std::vector<unsigned long>>& wordDocMap, std::string filePath) {

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

		inFile1.read(reinterpret_cast<char*>(&firstSize), sizeof(firstSize)); // read last for bytes to get size

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

		/* similar to merging sorted list
		reading file increments file pointer
		to the start of element
		*/

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
