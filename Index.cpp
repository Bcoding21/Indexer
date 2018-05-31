#include "Index.h"

void Index::add(unsigned long termId, unsigned long docId) {

	auto it = data.find(termId);
	if (it == data.end()) {
		auto docList = { docId };
		data.emplace(termId, docList);
	}
	else {
		it->second.emplace(docId);
	}
}

void Index::write(const std::string& path) {

	// TODO

}