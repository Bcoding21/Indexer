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
#include <thread>
#include <mutex>

namespace fs = std::experimental::filesystem::v1;


struct Indexer {
	unsigned long termID = 0;
	unsigned long linkID = 0;
	std::string inputDir = "pa1-data";
	const std::string outputDir = "C:\\Users\\Brandon\\source\\repos\\Query\\Query\\";
	std::unordered_map <std::string, unsigned long> wordIndexer;
	std::unordered_map <std::string, unsigned long> linkIndexer;
	std::queue<std::string> blockQueue;
};




void storeStringLongMap(const std::unordered_map<std::string, unsigned long>&, std::string);

void storeIndex(const std::map<unsigned long, std::set<unsigned long>>&, std::string);

void merge(const std::string&, const std::string&, const std::string&);

std::pair<unsigned long, std::set<unsigned long>> readOneWordDoc(std::ifstream&);

void writeOneWordDoc(const std::pair<unsigned long, std::set<unsigned long>>&, std::ofstream&);

std::vector<std::vector<std::string>> getSections(const fs::directory_entry&);

void updateIndex(Indexer&, const std::vector<std::string>&, std::map<unsigned long, std::set<unsigned long>>);

void setSections(std::vector<std::vector<std::string>>&,std::vector<std::string>&&, int, int);



int main(char* argc, char* argv[]) {

	Indexer index;


	for (const auto& subDir : fs::directory_iterator(index.inputDir)) { // iterate through sub directories

		auto outputPath = index.outputDir + fs::path(subDir).filename().string(); // create path to store index at later

		auto sections = getSections(subDir);

		index.blockQueue.push(outputPath);

		std::map<unsigned long, std::set<unsigned long>> index; // map wordID to DocumentIDs (1 - 1 2 3 4 5)

		for (const auto& file : fs::directory_iterator(subDir)) { // iterate through all files in sub directory

			auto docName = fs::path(file).filename().string(); // get file name

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


		Creates one list by merging two.
			The new list is added to the queue.
			The two list used to merge are deleted.
			One list left in the end.


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

	if (outFile) {
		
		for (const auto& pair : wordDocMap) { // loop through map

			outFile.write(reinterpret_cast<const char*>(&pair.first), sizeof(pair.first)); // write wordID

			int listSize = pair.second.size(); // get size of docIdList

			outFile.write(reinterpret_cast<char*>(&listSize), sizeof(listSize)); // store size

			std::ostream_iterator<char> out_iter(outFile); // char data type to write in binary

			std::copy(pair.second.begin(), pair.second.end(), out_iter); // write to file

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


	std::ifstream inFile1(filePath1, std::ifstream::binary | std::ifstream::ate); // open in binary. pointer to end of file
	std::ifstream inFile2(filePath2, std::ifstream::binary | std::ifstream::ate);
	std::ofstream outFile(filePath3, std::ofstream::binary);

	if ((inFile1 && inFile2) && outFile) { // check if open

		int sizeOfInt = sizeof(int); // get size in bytes

		inFile1.seekg(-sizeOfInt, std::ios::end); // travel back 4 bytes

		unsigned int firstSize = 0;

		inFile1.read(reinterpret_cast<char*>(&firstSize), sizeof(firstSize)); // read 4 bytes. Size of index stored at last 4.

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

			else { // same word but different list of documents. Merge list and store.
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

std::vector<std::vector<std::string>> getSections(const fs::directory_entry& dir) {

	std::vector<std::string> fileNamesList;

	std::transform(
		fs::directory_iterator(dir), fs::directory_iterator(),
		std::back_inserter(fileNamesList), [](const auto& filePath) {
		return fs::path(filePath).filename().string(); }
	);

		int maxThreads = std::thread::hardware_concurrency();

		std::vector<std::vector<std::string>> sections;

		setSections(sections, std::move(fileNamesList),0, maxThreads);

		
		return sections;

}

void setSections(std::vector<std::vector<std::string>>& sections, std::vector<std::string>&& fileNamesList, int iteration,
	int maxSections) {

	if (iteration == log(maxSections) / log(2)) {
		std::vector<std::string> section(std::make_move_iterator(fileNamesList.begin()),
			std::make_move_iterator(fileNamesList.end()));
		sections.push_back(std::move(section));
		return;
	}

	std::vector<std::string> leftHalf(std::make_move_iterator(fileNamesList.begin()),
		std::make_move_iterator(fileNamesList.begin() + fileNamesList.size() / 2));
	std::vector<std::string> rightHalf(std::make_move_iterator(fileNamesList.begin() + fileNamesList.size() / 2),
		std::make_move_iterator(fileNamesList.end()));

	setSections(sections, std::move(leftHalf), iteration + 1, maxSections);
	setSections(sections, std::move(rightHalf), iteration + 1, maxSections);

}


void updateIndex(Indexer& indexer, const std::vector<std::string>& filePaths,
	std::map<unsigned long, std::set<unsigned long>> wordDocIndex) {


}