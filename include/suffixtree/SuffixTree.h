#pragma once

#include <suffixtree/Node.h>

#include <string>
#include <vector>
#include <memory>



class SuffixTree{
public:
	SuffixTree(char end_marker = 0x03);
	SuffixTree(const std::string &text, char end_marker = 0x03);
	std::string get_text() const;
	void set_text(const std::string &text);

	std::vector<size_t> find(const std::string &needle) const;
	bool ends_with(const std::string &suffix) const;
	bool contains(const std::string &needle) const;

	void check_suffix_links() const;
	void print() const;

private:
	char end_marker;
	std::string text;
	std::shared_ptr<Node> root;
	size_t end_of_text = 0;                  // points to the first unprocessed character

	void rebuild();
	void relabel_text_end();
	std::shared_ptr<Node> traverse_node(std::shared_ptr<Node> node, const std::string &find_text, size_t &position, size_t end) const;
	std::shared_ptr<const Node> traverse_tree(const std::string &find_text, size_t &position, size_t end) const;
	std::string get_edge_label(std::shared_ptr<const Node> node) const;
	std::string get_path_label(std::shared_ptr<const Node> node) const;
	void print_node(std::shared_ptr<const Node> node, size_t indent) const;
};
