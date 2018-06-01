#include "Dictionary.h"
#include "Index.h"
#include "IndexUtility.h"
#include <queue>

int main(char* argc, char* argv[]) {

	Dictionary termDict;
	Dictionary docDict;
	std::queue<std::string> outputPaths;

	std::string inputDir = "Data/";
	std::string outputDir = "Ouput/";

	for (const auto& subDir : fs::directory_iterator(inputDir)) {

		auto path = outputDir + iu::getDocTitle(subDir);
		outputPaths.push(path);

		Index index;

		for (const auto& file : fs::directory_iterator(subDir)) {

			docDict.add(iu::getDocTitle(file));

			std::ifstream stream(file);

			std::string term;

			while (stream >> term ) {

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

		iu::merge(path1, path2, newPath);
		outputPaths.push(newPath);

		fs::remove(path1);
		fs::remove(path2);
	}
	termDict.write("");
	docDict.write("");
}

