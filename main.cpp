#include <vector>
#include <algorithm>
#include <experimental/filesystem>
#include <iostream>
#include <queue>
#include <iterator>
#include "Dictionary.h"
#include "Index.h"


// mainpulates filesystem
namespace fs = std::experimental::filesystem::v1;

/*
Combines two term id indexes and writes the combined index to a file in binary.
Param 1 - path of file that stores term index (termDict1.bin)
Param 2 - path of file that stores term index (termDict2.bin)
Param 3 - desired path of the new index. (termDict1and2.bin)
*/
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


int main(char* argc, char* argv[]) {

	Dictionary termDict;
	Dictionary docDict;
	std::queue<std::string> outputPaths;

	std::string inputDir = "pa1-data";
	std::string outputDir = "C:\\Users\\Brandon\\source\\repos\\Query\\Query\\";

	for (const auto& subDir : fs::directory_iterator(inputDir)) {

		auto path = outputDir + getDocTitle(subDir);
		outputPaths.push(path);

		Index index;

		for (const auto& file : fs::directory_iterator(subDir)) {

			docDict.add(getDocTitle(file));

			auto filePath = file.path().string();

			for (const auto& term : getTermsFromFile(filePath)) {
				
				termDict.add(term);

				index.add(termDict.getCurrId() , docDict.getCurrId());
			}
		}
		index.write(path);
	}

	/*
	Creates one list by merging two.
	The new list is added to the queue.
	The two list used to merge are deleted.
	One list left in the end.
	*/
	while (true) {

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
	termDict.write("");
	docDict.write("");
}

std::pair<unsigned long, std::vector<unsigned long>> readOneIndexEntry(std::ifstream& stream) {

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
	std::ifstream instream2(path2, std::ifstream::binary | std::ifstream::ate);
	std::ofstream outStream(path3, std::ofstream::binary);

	if (inStream1.is_open() && instream2.is_open() && outStream.is_open()) {

		int indexOneSize = getIndexSize(inStream1);
		int indexTwoSize = getIndexSize(instream2);
		unsigned int newIndexSize = 0;

		auto entry1 = readOneIndexEntry(inStream1);
		unsigned int indexOneCount = 1;

		auto entry2 = readOneIndexEntry(instream2);
		unsigned int indexTwoCount = 1;

		/* similar to merging sorted list
		reading file increments file pointer
		*/

		while (indexOneCount < indexOneSize && indexTwoCount < indexTwoSize) {

			if (entry1.first < entry2.first) {
				writeOneIndexEntry(entry1, outStream);
				entry1 = readOneIndexEntry(inStream1);
				++indexOneCount;
			}

			else if (entry1.first > entry2.first) {
				writeOneIndexEntry(entry2, outStream);
				entry2 = readOneIndexEntry(instream2);
				++indexTwoCount;
			}

			else {
				writeOneIndexEntry(getCombinedEntry(entry1, entry2), outStream);
				entry1 = readOneIndexEntry(inStream1);
				entry2 = readOneIndexEntry(instream2);
				++indexOneCount;
				++indexTwoCount;
			}
			++newIndexSize;
		}

		while (indexOneCount++ < indexOneSize) {
			writeOneIndexEntry(entry1, outStream);
			entry1 = readOneIndexEntry(inStream1);
			newIndexSize++;
		}

		while (indexTwoCount++ < indexTwoSize) {
			writeOneIndexEntry(entry2, outStream);
			entry1 = readOneIndexEntry(instream2);
			newIndexSize++;
		}

		outStream.write(reinterpret_cast<char*>(&newIndexSize), sizeof(newIndexSize));
	}
}

void writeOneIndexEntry(const std::pair<unsigned long, std::vector<unsigned long>>& entry, std::ofstream& stream) {

	stream.write(reinterpret_cast<const char*>(&entry.first), sizeof(entry.first));

	unsigned int vecSize = entry.second.size();

	stream.write(reinterpret_cast<char*>(&vecSize), sizeof(vecSize));

	stream.write(reinterpret_cast<const char*>(&entry.second[0]), sizeof(entry.second[0]) * vecSize);
}

int getIndexSize(std::ifstream& stream) {
	int intSize = sizeof(int);
	stream.seekg(-intSize, std::ios::end);
	int size = 0;
	stream.read(reinterpret_cast<char*>(&size), sizeof(size));
	stream.clear(); // clear eof flag
	stream.seekg(0);
	return size;
}

std::pair<unsigned long, std::vector<unsigned long>> getCombinedEntry(
	const std::pair<unsigned long, std::vector<unsigned long>>& entry1,
	const std::pair<unsigned long, std::vector<unsigned long>>& entry2) {

	std::vector<unsigned long> combinedList;

	combinedList.reserve(
		std::max(entry1.second.size(), entry2.second.size())
	);

	std::set_union(
		entry1.second.begin(), entry1.second.end(),
		entry2.second.begin(), entry2.second.end(),
		std::back_inserter(combinedList)
	);

	return std::make_pair(entry1.first, combinedList);
}

std::vector<std::string> getTermsFromFile(const std::string& path) {
	std::ifstream stream(path);
	std::istream_iterator<std::string> start(stream), end;
	return std::vector<std::string>(start, end);
}


std::string getDocTitle(const fs::directory_entry& entry) {
	return entry.path().filename().string();
}
