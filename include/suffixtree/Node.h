#pragma once

#include <memory>
#include <unordered_map>


class Node{
public:
	size_t get_text_end(size_t end_of_text) const;

	std::unordered_map<char, std::shared_ptr<Node>> children;
	std::weak_ptr<Node> parent;
	std::weak_ptr<Node> suffix_link;
	// edge from parent to this has label [text_begin, ..., text_end-1]. If text_end == 0, it represents the tree's current end_of_text.
	size_t text_begin = 0;
	size_t text_end = 0;
	size_t suffix_start = 0;
};
