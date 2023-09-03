#include <suffixtree/SuffixTree.h>

#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <fstream>


static void test_suffixes(const SuffixTree &tree){
	std::cout << "> Testing suffixes..." << std::endl;
	const std::string text = tree.get_text();
	for(size_t i=0; i<text.length(); i++){
		const std::string suffix = text.substr(i, text.length()-i);
		if(!tree.ends_with(suffix)){
			throw std::runtime_error("suffix " + suffix + " not in tree");
		}
	}
}


static void test_substrings(const SuffixTree &tree){
	std::cout << "> Testing substrings..." << std::endl;
	const std::string text = tree.get_text();
	for(size_t i=0; i<text.length(); i++){
		for(size_t j=i+1; j<=text.length(); j++){
			const std::string substring = text.substr(i, j-i);
			if(!tree.contains(substring)){
				throw std::runtime_error("tree does not contain substring " + substring);
			}

			// compare with regular string matching
			auto occurrences = tree.find(substring);
			std::sort(occurrences.begin(), occurrences.end());
			auto find = text.find(substring);
			auto iter = occurrences.begin();
			while(find != std::string::npos && iter != occurrences.end() && find == *iter){
				find = text.find(substring, find+1);
				iter++;
			}
			if(find != std::string::npos){
				throw std::runtime_error("substring " + substring + " found at " + std::to_string(find) + " but not in tree");
			}else if(iter != occurrences.end()){
				throw std::runtime_error("substring " + substring + " in tree at " + std::to_string(*iter) + " but not in string");
			}
		}
	}
}


static void test_not_contained(const SuffixTree &tree){
	std::cout << "> Testing non contained substrings..." << std::endl;
	const std::string text = tree.get_text();
	const std::vector<std::string> substrings = {
		"zoeglfrex",
		"kraxlburg",
		"qvnts",
	};
	for(const auto &substring : substrings){
		const auto find = text.find(substring);
		const std::vector<size_t> occurrences = tree.find(substring);
		if(find == std::string::npos && !occurrences.empty()){
			throw std::runtime_error("string " + substring + " found in tree but not in string");
		}else if(find != std::string::npos && occurrences.empty()){
			throw std::runtime_error("string " + substring + " found in string but not in tree");
		}
	}
}


static void test_suffix_links(const SuffixTree &tree){
	std::cout << "> Testing suffix links..." << std::endl;
	tree.check_suffix_links();
}


static void test_tree(const SuffixTree &tree){
	test_suffixes(tree);
	test_substrings(tree);
	test_not_contained(tree);
	test_suffix_links(tree);
}


static void test_text(const std::string &text){
	if(text.length() <= 50){
		std::cout << "Testing text " << text << std::endl;
	}else{
		std::cout << "Testing text of size " << text.length() << std::endl;
	}
	std::cout << "> Building tree..." << std::endl;
	SuffixTree tree(text);
	//tree.print();
	test_tree(tree);
}


int main(int argc, char **argv){
	std::vector<std::string> texts;
	bool is_file = false;
	for(int i=1; i<argc; i++){
		const std::string arg = argv[i];
		if(arg == "-f"){
			is_file = true;
		}else if(is_file){
			std::ifstream stream(arg);
			if(!stream){
				throw std::runtime_error("Opening file " + arg + " failed");
			}
			const std::string text = std::string(std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>());
			texts.push_back(text);
			is_file = false;
		}else{
			texts.push_back(argv[i]);
		}
	}
	if(texts.empty()){
		texts.push_back("");
		texts.push_back("bbbbbbbbbbbbbbbbbbbbbbbbbbbbbb");
		texts.push_back("abracadabra");
		texts.push_back("bringst du opi opium bringt opium den opi um");
		texts.push_back("der inder in der inderin drin");
		texts.push_back("bismarck biss mark, bis mark bismarck biss");
	}

	for(const auto &text : texts){
		try{
			test_text(text);
		}catch(const std::runtime_error &err){
			if(text.length() <= 50){
				std::cerr << "Error with text " << text << ": " << err.what() << std::endl;
			}else{
				std::cerr << "Error with text of size " << text.length() << ": " << err.what() << std::endl;
			}
			return 1;
		}
	}

	std::cout << "All tests successful" << std::endl;
	return 0;
}
