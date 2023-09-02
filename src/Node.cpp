#include <suffixtree/Node.h>


size_t Node::get_text_end(size_t end_of_text) const{
	if(text_end == 0){
		return end_of_text;
	}
	return text_end;
}

