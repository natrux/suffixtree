#include <suffixtree/SuffixTree.h>

#include <queue>
#include <stdexcept>


SuffixTree::SuffixTree(char end_marker_):
	end_marker(end_marker_)
{
}


SuffixTree::SuffixTree(const std::string &text_, char end_marker_):
	end_marker(end_marker_)
{
	set_text(text_);
}


std::string SuffixTree::get_text() const{
	auto result = text;
	if(!result.empty()){
		// remove end marker
		result.pop_back();
	}
	return result;
}


void SuffixTree::set_text(const std::string &text_){
	if(text_.find(end_marker) != std::string::npos){
		throw std::logic_error("text contains end marker");
	}
	text = text_;
	text.push_back(end_marker);
	rebuild();
}


std::vector<size_t> SuffixTree::find(const std::string &needle) const{
	size_t pos = 0;
	const size_t end = needle.size();
	auto stop = traverse_tree(needle, pos, end);
	if(pos < end){
		// not in text
		return {};
	}

	// visit all leaves below stop
	std::vector<size_t> result;
	std::queue<std::shared_ptr<const Node>> todo;
	todo.push(stop);
	bool cant_be_root = false;
	while(!todo.empty()){
		auto current_node = todo.front();
		todo.pop();
		if(current_node->children.empty() && (cant_be_root || stop->parent.lock())){
			result.push_back(current_node->suffix_start);
		}
		for(const auto &entry : current_node->children){
			todo.push(entry.second);
		}
		cant_be_root = true;
	}
	return result;
}


bool SuffixTree::ends_with(const std::string &suffix) const{
	auto suffix_ = suffix;
	suffix_.push_back(end_marker);
	size_t pos = 0;
	const size_t end = suffix_.size();
	auto stop = traverse_tree(suffix_, pos, end);
	return (pos >= end && stop->children.empty() && stop->parent.lock());
}


bool SuffixTree::contains(const std::string &needle) const{
	size_t pos = 0;
	const size_t end = needle.size();
	auto stop = traverse_tree(needle, pos, end);
	return (pos >= end);
}


void SuffixTree::check_suffix_links() const{
	std::queue<std::shared_ptr<const Node>> todo;
	todo.push(root);
	while(!todo.empty()){
		auto current_node = todo.front();
		todo.pop();

		if(current_node != root && !current_node->children.empty()){
			auto linked_node = current_node->suffix_link.lock();
			if(!linked_node){
				throw std::runtime_error("Inner node does not have a suffix link");
			}
			if(linked_node == root && (current_node->parent.lock() != root || current_node->text_begin+1 != current_node->text_end)){
				throw std::runtime_error("Wrong suffix link to root");
			}else{
				const auto current_label = get_path_label(current_node);
				const auto linked_label = get_path_label(linked_node);
				if(current_label.size() != linked_label.size()+1 || current_label.substr(1) != linked_label){
					throw std::runtime_error("Wrong suffix link");
				}
			}
		}
		for(const auto &entry : current_node->children){
			todo.push(entry.second);
		}
	}
}


/*
void SuffixTree::print() const{
	print_node(root, 0);
}
// */


void SuffixTree::rebuild(){
	end_of_text = 0;
	root = std::make_shared<Node>();
	size_t next_k = 0;
	std::shared_ptr<Node> link_wanted;
	size_t link_wanted_position = 0;
	for(size_t i=0; i<text.length(); i++){
		//std::cout << "Phase " << i << " (" << text.substr(0, i+1) << ")" << std::endl;
		std::shared_ptr<Node> link_follow;
		size_t link_follow_position = 0;
		const size_t k = next_k;
		//std::cout << "Doing " << k << " step" << (k == 1 ? "" : "s") << " implicitly" << std::endl;
		end_of_text++;
		next_k = i+1;

		for(size_t j=k; j<=i; j++){
			//std::cout << "[" << j << ", " << i << "] (";
			//for(size_t c=j; c<=i; c++){
			//	std::cout << text[c];
			//}
			//std::cout << ") -> ";
			std::shared_ptr<Node> current_node, next_node;
			size_t current_position, next_position;
			if(link_follow){
				if(auto link = link_follow->suffix_link.lock()){
					//std::cout << "[suffix link from " << link_follow.get() << " to " << link.get() << "] ";
					current_node = link;
					current_position = link_follow_position;
					link_follow = nullptr;
				}else{
					throw std::runtime_error("A suffix link we wanted to follow does not exist");
				}
			}else{
				current_node = root;
				current_position = j;
			}

			next_node = current_node;
			next_position = current_position;
			do{
				if(link_wanted && link_wanted_position == next_position){
					link_wanted->suffix_link = next_node;
					link_wanted = nullptr;
				}
				if(next_node->suffix_link.lock()){
					link_follow = next_node;
					link_follow_position = next_position;
				}
				current_node = next_node;
				current_position = next_position;
				next_node = traverse_node(current_node, text, next_position, i+1);
			}while(next_node && next_position <= i && current_node != next_node && next_node->get_text_end(end_of_text) - next_node->text_begin == next_position - current_position);

			if(!next_node){
				// Rule 1
				//std::cout << "Rule 1" << std::endl;
			}else if(next_position > i){
				// Rule 3
				//std::cout << "Rule 3" << std::endl;
				next_k = j;
				break;
			}else{
				// Rule 2
				//std::cout << "Rule 2 ";
				if(current_node == next_node){
					// stuck at the node
					//std::cout << "at node " << current_node.get() << std::endl;
					auto new_node = std::make_shared<Node>();
					new_node->parent = current_node;
					new_node->text_begin = current_position;
					new_node->suffix_start = j;
					current_node->children[text[current_position]] = new_node;
				}else{
					// stuck on the edge
					//std::cout << "on the edge from " << current_node.get() << " to " << next_node.get() << std::endl;
					/*
					Create new internal node mid_node with 2 children:
					 - next_node
					 - match_leaf which stores the rest of the current match
					*/
					const size_t offset = next_position - current_position;
					auto mid_node = std::make_shared<Node>();
					mid_node->parent = next_node->parent;
					mid_node->text_begin = next_node->text_begin;
					mid_node->text_end = next_node->text_begin + offset;
					if(mid_node->text_begin == mid_node->text_end && mid_node->text_end != 0){
						throw std::runtime_error("mid_node text_begin == text_end == " + std::to_string(mid_node->text_begin));
					}
					if(auto parent = next_node->parent.lock()){
						// replaces next_node as parent's child
						parent->children[text[next_node->text_begin]] = mid_node;
					}else{
						throw std::runtime_error("Node without a parent");
					}

					auto match_leaf = std::make_shared<Node>();
					match_leaf->parent = mid_node;
					match_leaf->text_begin = next_position;
					match_leaf->suffix_start = j;

					next_node->parent = mid_node;
					next_node->text_begin += offset;
					if(next_node->text_begin == next_node->text_end && next_node->text_end != 0){
						throw std::runtime_error("next_node text_begin == text_end == " + std::to_string(next_node->text_begin));
					}

					mid_node->children[text[next_node->text_begin]] = next_node;
					mid_node->children[text[match_leaf->text_begin]] = match_leaf;

					if(link_wanted){
						link_wanted->suffix_link = mid_node;
						link_wanted = nullptr;
					}
					if(mid_node->parent.lock() == root && mid_node->get_text_end(end_of_text) == mid_node->text_begin+1){
						mid_node->suffix_link = root;
					}else{
						link_wanted = mid_node;
						link_wanted_position = next_position;
					}
				}
			}
		}
	}
	relabel_text_end();
}


void SuffixTree::relabel_text_end(){
	std::queue<std::shared_ptr<Node>> todo;
	todo.push(root);
	while(!todo.empty()){
		auto next_node = todo.front();
		todo.pop();
		if(next_node->text_end == 0){
			next_node->text_end = end_of_text;
		}
		for(const auto &entry : next_node->children){
			todo.push(entry.second);
		}
	}
}


/*
 * PARAMETERS
 *   node: the node to traverse, i.e. the root of the subtree
 *   find_text: string along which to traverse
 *   position: first character of the text
 *   end: one past the last character of the text
 * RETURNS
 *   nullptr if node has no children,
 *   node if node has children, but there is no edge that fits the first character of the text
 *   the node at the end of the edge that fits the first character
 * POINTERS
 *   position is changed to denote the position of the first mismatch. If there was no mismatch it contains the next character to verify if we went all along the edge. If we ran out of characters during the edge, position == end.
 */
std::shared_ptr<Node> SuffixTree::traverse_node(std::shared_ptr<Node> node, const std::string &find_text, size_t &position, size_t end) const{
	if(node->children.empty() && node != root){
		// leaf
		return nullptr;
	}

	std::shared_ptr<Node> next_step;
	{
		const auto find = node->children.find(find_text[position]);
		if(find != node->children.end()){
			next_step = find->second;
		}
	}
	if(!next_step){
		// children present, but none suitable
		return node;
	}

	size_t offset = 0;
	while(next_step->text_begin+offset < next_step->get_text_end(end_of_text) && position+offset < end && text[next_step->text_begin+offset] == find_text[position+offset]){
		offset++;
	}
	// position+offset is now either the next character to verify or end or the position of the first mismatch

	position += offset;
	return next_step;
}


/*
 * PARAMETERS
 *   find_text: string along which to traverse
 *   position: first character of the text
 *   end: one past the last character of the text
 * RETURNS
 *   the last node on the way that could be successfully reached
 * POINTERS
 *   position is changed to denote the position of the first mismatch. If there was no mismatch (i.e. all charachters were verified), position == end..
 */
std::shared_ptr<const Node> SuffixTree::traverse_tree(const std::string &find_text, size_t &position, size_t end) const{
	std::shared_ptr<Node> current_node;
	size_t current_position;
	std::shared_ptr<Node> next_node = root;
	do{
		current_node = next_node;
		current_position = position;
		next_node = traverse_node(current_node, find_text, position, end);
	}while(next_node && position < end && current_node != next_node && next_node->text_end - next_node->text_begin == position - current_position);

	if(position >= end && next_node && position - current_position <= next_node->text_end - next_node->text_begin){
		// all characters verified
		return next_node;
	}else{
		// something went wrong
		return current_node;
	}
}


std::string SuffixTree::get_edge_label(std::shared_ptr<const Node> node) const{
	std::string result;
	const size_t text_end = node->get_text_end(end_of_text);
	result.reserve(text_end - node->text_begin);
	for(size_t i=node->text_begin; i<text_end; i++){
		result.push_back(text[i]);
	}
	return result;
}


std::string SuffixTree::get_path_label(std::shared_ptr<const Node> node) const{
	std::string result;
	auto current_node = node;
	while(current_node && current_node != root){
		result = get_edge_label(current_node) + result;
		current_node = current_node->parent.lock();
	}
	return result;
}


/*
void SuffixTree::print_node(std::shared_ptr<const Node> node, size_t indent) const{
	std::string indent_string(indent, '\t');

	std::cout << indent_string << "---------- Dumping node " << node.get() << " ----------" << std::endl;
	if(auto parent = node->parent.lock()){
		std::cout << indent_string << "has parent " << parent.get() << std::endl;
		std::cout << indent_string << "has edge label " << get_edge_label(node) << std::endl;
		std::cout << indent_string << "has path label " << get_path_label(node) << std::endl;
	}else{
		std::cout << indent_string << "is root node" << std::endl;
	}

	if(node->next_sibling){
		std::cout << indent_string << "has sibling " << node->next_sibling.get() << std::endl;
	}
	if(auto previous_sibling = node->previous_sibling.lock()){
		std::cout << indent_string << "has previous sibling " << previous_sibling.get() << std::endl;
	}

	if(auto suffix_link = node->suffix_link.lock()){
		std::cout << indent_string << "has suffix link to " << suffix_link.get() << std::endl;
	}

	if(node->parent.lock() && !node->first_child){
		std::cout << indent_string << "is leaf, has leaf label " << node->suffix_start << std::endl;
	}else{
		auto current_node = node->first_child;
		while(current_node){
			std::cout << indent_string << "has child " << current_node.get() << std::endl;
			print_node(current_node, indent+1);
			current_node = current_node->next_sibling;
		}
	}
	std::cout << indent_string << "---------- End of node " << node.get() << " dump ----------" << std::endl;
}
// */
