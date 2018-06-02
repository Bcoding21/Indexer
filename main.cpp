#include "Indexer.h"
#include <queue>

int main(char* argc, char* argv[]) {

	fs::path inputDir("Data/");
	fs::path outputDir("Ouput/");

	std::queue<fs::path> indexPathsQueue;

	indexer::Dictionary termDict;
	indexer::Dictionary docDict;

	for (const auto& subDir : fs::directory_iterator(inputDir)) {

		indexer::Index index;

		for (const auto& file : fs::directory_iterator(subDir)) {

			docDict.add(file.path().filename().string());

			std::ifstream stream(file.path());

			std::string term;

			while ( stream >> term ) {

				termDict.add(term);

				index.add(termDict.getCurrId() , docDict.getCurrId());
			}
		}

		indexer::writeIndex(index, subDir.path());
		indexPathsQueue.push(subDir.path());
	}

	indexer::writeDictionary(termDict, "");
	indexer::writeDictionary(docDict, "");

	/*
	Creates one list by merging two.
	The new list is added to the queue.
	The two list used to merge are deleted.
	One list left in the end.
	*/
	while (true) {

		if (indexPathsQueue.size() <= 1) {
			break;
		}

		auto path1 = indexPathsQueue.front();
		indexPathsQueue.pop();

		auto path2 = indexPathsQueue.front();
		indexPathsQueue.pop();

		fs::path newPath(outputDir / (path1.filename().string() + path2.filename().string()));

		indexer::mergeIndexFiles(path1, path2, newPath);

		indexPathsQueue.push(newPath);

		fs::remove(path1);
		fs::remove(path2);
	}

}

