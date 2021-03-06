#include "Indexer.h"


namespace {
	/*
		Reads one index entry from index file.
		@param 1 - ifstream object that reads from file.
		@return - pair that groups term id to vector of doc ids
	*/
	std::pair<unsigned long, std::vector<unsigned long>> readIndexEntry(std::ifstream& stream) {

		long termId = 0;

		stream.read(reinterpret_cast<char*>(&termId), sizeof(termId));

		int listSize = 0;

		stream.read(reinterpret_cast<char*>(&listSize), sizeof(listSize));

		std::vector<unsigned long> docIdSet(listSize);

		stream.read(reinterpret_cast<char*>(&docIdSet[0]), sizeof(docIdSet[0]) * listSize);

		return std::make_pair(termId, docIdSet);
	}
}

namespace {
	/*
		Reads index size from index file.
		@param - 1 ifstream object that reads from file.
	*/
	int readIndexSize(std::ifstream& stream) {
		int intSize = sizeof(int);
		stream.seekg(-intSize, std::ios::end);
		int size = 0;
		stream.read(reinterpret_cast<char*>(&size), sizeof(size));
		stream.clear(); // clear eof flag
		stream.seekg(0);
		return size;
	}
}

namespace {
	/*
		Writes index entry to file in binary.
		@param 1 - pair consisting of term id and vector of document ids
		@param 2 - ofstream object that writes to file.
	*/
	void writeIndexEntry(const std::pair<unsigned long, std::vector<unsigned long>>& entry, std::ofstream& stream) {

		stream.write(reinterpret_cast<const char*>(&entry.first), sizeof(entry.first));

		unsigned int vecSize = entry.second.size();

		stream.write(reinterpret_cast<char*>(&vecSize), sizeof(vecSize));

		stream.write(reinterpret_cast<const char*>(&entry.second[0]), sizeof(entry.second[0]) * vecSize);
	}
}


namespace {
	/*
		Combines two index entries to form one entry.
		@param 1 - pair that represents first index entry.
		@param 2 - pair that represents second index entry.
		@return - pair that represents the union of both entries.
	*/
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

}


namespace indexer {

	void mergeIndexFiles(const fs::path& path1, const fs::path& path2, const fs::path& path3) {


		std::ifstream inStream1(path1, std::ifstream::binary | std::ifstream::ate);
		std::ifstream instream2(path2, std::ifstream::binary | std::ifstream::ate);
		std::ofstream outStream(path3, std::ofstream::binary);

		if (inStream1.is_open() && instream2.is_open() && outStream.is_open()) {

			int indexOneSize = readIndexSize(inStream1);
			int indexTwoSize = readIndexSize(instream2);
			unsigned int newIndexSize = 0;

			auto entry1 = readIndexEntry(inStream1);
			unsigned int indexOneCount = 1;

			auto entry2 = readIndexEntry(instream2);
			unsigned int indexTwoCount = 1;

			/* similar to merging sorted list
			reading file increments file pointer
			*/

			while (indexOneCount < indexOneSize && indexTwoCount < indexTwoSize) {

				if (entry1.first < entry2.first) {
					writeIndexEntry(entry1, outStream);
					entry1 = readIndexEntry(inStream1);
					++indexOneCount;
				}

				else if (entry1.first > entry2.first) {
					writeIndexEntry(entry2, outStream);
					entry2 = readIndexEntry(instream2);
					++indexTwoCount;
				}

				else {
					writeIndexEntry(getCombinedEntry(entry1, entry2), outStream);
					entry1 = readIndexEntry(inStream1);
					entry2 = readIndexEntry(instream2);
					++indexOneCount;
					++indexTwoCount;
				}
				++newIndexSize;
			}

			while (indexOneCount++ < indexOneSize) {
				writeIndexEntry(entry1, outStream);
				entry1 = readIndexEntry(inStream1);
				newIndexSize++;
			}

			while (indexTwoCount++ < indexTwoSize) {
				writeIndexEntry(entry2, outStream);
				entry1 = readIndexEntry(instream2);
				newIndexSize++;
			}

			outStream.write(reinterpret_cast<char*>(&newIndexSize), sizeof(newIndexSize));
		}
	}
	
	void writeDictionary(const Dictionary& dict, const fs::path& path){}

	void writeIndex(const Index& index, const fs::path& path) {}
}