//----------------------------------------------------------------------------
// huffman coding for file compression with tries
// copyright 2010 christopher rasmussen
// university of delaware
//----------------------------------------------------------------------------

#include "Huffman.hh"
#include <iostream>
#include <vector>
#include <algorithm>
//----------------------------------------------------------------------------

bool debug_flag = false;
bool ascii_flag = false;

vector<char> v;
//----------------------------------------------------------------------------

// ** FILL THIS FUNCTION IN ** 

// assumes that trie has AT LEAST 2 elements in it

// (1) remove trie in PQ with SMALLEST frequency, assign to first
// (2) remove trie in PQ with NEXT SMALLEST frequency, assign to second
// (3) make new trie node (new_root) and plug first and second tries in as children, 
//     set new_root's frequency to first->frequency + second->frequency, and 
//     insert in new_root in PQ

// hints:
// * look at TrieNode constructor functions
// * see STL priority_queue documentation at http://www.sgi.com/tech/stl/priority_queue.html
//   for more details 
// * see example_function() below for sample calls to trie PQ 

void Huffman::merge_two_least_frequent_subtries()
{
  TrieNode *new_root, *first, *second;
  int a, b, c; 
  first = trie.top();
  trie.pop();
  second = trie.top();
  trie.pop();
  a = first->frequency;
  b = second->frequency;
  c = a+b;
  new_root = new TrieNode(NULL, c, NULL, first, second);
  //cout<<first<<'\n';
  //new_root->left = first;//first;
  //new_root->right = second;
  first->parent = new_root;
  second->parent = new_root;
  trie.push(new_root);
  
 
  //.top - shows least frequent node
  //.pop - removes it

//   have to do assign top to first and then pop
//	assign top to second and then pop 

//	then make a new try node (create the parent of those two things, call constructor on trienode class new trienode(char, frequency, etc.)
//	pass no char, first frequency plus second frequency, null parent, child first, second

//	trie.push (above node) have to put the node into the priority queue
//	first parent = NODE CREATED ABOVE IN CONSTRUCTOR
//	second parent = NODE CREATED ABOVE IN CONSTRUCTOR

		
  // comment this line out when you start to code, obviously
  //cout << "merge_two_least_frequent_subtries(): NOT YET WRITTEN\n"; exit(1);

  // ???
}

//----------------------------------------------------------------------------

// ** FILL THIS FUNCTION IN **

// recursively traverse binary tree rooted at T
// to determine the huffcode string at every leaf

// if T is a leaf, make sure to execute the 
// compression_map and decompression_map assignments

//have to figure out the huffcode, so going down the tree if i go left its 0, right its 1, 
void Huffman::compute_all_codes_from_trie(TrieNode *T)
{

TrieNode *root;
root = T;
if(T->left != NULL){
  T = T->left;
  v.push_back(T->character);
  T->huffcode = '0';
  compression_map.insert(make_pair(T->character, T->huffcode));
  decompression_map.insert(make_pair(T->huffcode, T->character));
  compression_map[T->character] = T->huffcode;
  decompression_map[T->huffcode] = T->character;
}
else if(T->right != NULL){
  T=T->right;
  v.push_back(T->character);
  T->huffcode = '1';
  compression_map.insert(make_pair(T->character, T->huffcode));
  decompression_map.insert(make_pair(T->huffcode, T->character));
  compression_map[T->character] = T->huffcode;
  decompression_map[T->huffcode] = T->character;
}
while (T->parent != NULL){
	while (T->left != NULL && find(v.begin(), v.end(), T->left->character) == v.end()){
		T=T->left; 
  		v.push_back(T->character);
  		T->huffcode = T->huffcode + '0';
  		compression_map.insert(make_pair(T->character, T->huffcode));
  		decompression_map.insert(make_pair(T->huffcode, T->character));
  		compression_map[T->character] = T->huffcode;
  		decompression_map[T->huffcode] = T->character;
	}
	while (T->right == NULL && T->parent != NULL){
		T=T->parent;
  		v.push_back(T->character);
  		//T->huffcode = T->huffcode.pop_back();
		if(T->huffcode.size()>0){
  			//v.push_back(T->character);
			T->huffcode.resize(T->huffcode.size() - 1);
  			//compression_map.insert(make_pair(T->character, T->huffcode));
  			//decompression_map.insert(make_pair(T->huffcode, T->character));
  			compression_map[T->character] = T->huffcode;
  			decompression_map[T->huffcode] = T->character;
  		}else{ 
		}	
		
		
	}
	if (T->parent!=NULL && find(v.begin(), v.end(), T->right->character) == v.end()){
 	
		T=T->right;	
  		v.push_back(T->character);
  		T->huffcode = T->huffcode + '1';
  		compression_map.insert(make_pair(T->character, T->huffcode));
  		decompression_map.insert(make_pair(T->huffcode, T->character));
  		compression_map[T->character] = T->huffcode;
  		decompression_map[T->huffcode] = T->character;
}else if (T->parent!=NULL){
	T=T->parent;
  	v.push_back(T->character);
	if(T->huffcode.size()>0){
		T->huffcode.resize(T->huffcode.size()-1);
	}
}}
T=root;
if(T->right != NULL){
	T=T->right;
	v.push_back(T->character);
	T->huffcode = '1';
  	compression_map.insert(make_pair(T->character, T->huffcode));
  	decompression_map.insert(make_pair(T->huffcode, T->character));
	compression_map[T->character] = T->huffcode;
	decompression_map[T->huffcode] = T->character;		 
}	
while (T->parent != NULL){	 
	while (T->left != NULL && find(v.begin(), v.end(), T->left->character) == v.end()){
		T=T->left; 
  		v.push_back(T->character);
  		T->huffcode = T->huffcode + '0';
  		compression_map.insert(make_pair(T->character, T->huffcode));
  		decompression_map.insert(make_pair(T->huffcode, T->character));
  		compression_map[T->character] = T->huffcode;
  		decompression_map[T->huffcode] = T->character;
	}
	while (T->right == NULL && T->parent != NULL){
		T=T->parent;
  		v.push_back(T->character);
  		//T->huffcode = T->huffcode.pop_back();
  		T->huffcode.resize(T->huffcode.size() - 1);
  		compression_map.insert(make_pair(T->character, T->huffcode));
  		decompression_map.insert(make_pair(T->huffcode, T->character));
  		compression_map[T->character] = T->huffcode;
  		decompression_map[T->huffcode] = T->character;
	}
	if (T->parent!=NULL && find(v.begin(), v.end(), T->right->character) == v.end()){
		T=T->right;
 		v.push_back(T->character);
  		T->huffcode = T->huffcode + '1';
  		compression_map.insert(make_pair(T->character, T->huffcode));
  		decompression_map.insert(make_pair(T->huffcode, T->character));
  		compression_map[T->character] = T->huffcode;
  		decompression_map[T->huffcode] = T->character;
}else if (T->parent != NULL){
	T=T->parent;
 	v.push_back(T->character);
	if(T->huffcode.size()>0){
		T->huffcode.resize(T->huffcode.size()-1);
	}
}
}
}
  // comment this line out when you start to code, obviously
  //cout << "compute_all_codes_from_trie(): NOT YET WRITTEN\n"; exit(1);
 // int x= 0;
//while(x < T.size()){
//	T->huffcode = ???
//	compression_map[T->character] = T->huffcode;
//	decompression_map[T->huffcode] = T->character;
//}else{}
//	x = x + 1;
//}
//while trienode doesn't equal the letter,
//TrieNodePtrCompare if true then 0
//if false then 1

//----------------------------------------------------------------------------

// ** FILL THIS FUNCTION IN ** 

// return how many bits huffman code takes

// this is the sum of the frequency * huffcode length
// over every leaf

int Huffman::calculate_huffman_file_size()
{
  // comment this line out when you start to code, obviously
  //cout << "calculate_huffman_file_size(): NOT YET WRITTEN\n"; exit(1);
  int sum = 0;
  int mult;
  char letter; 
  int let;
  for(int i = 0; i < 128; i++){
	
	letter = (char) i;
	//cout<<letter;
	//cout<<char_counter[i];

	//cout<< compression_map.find(letter)<<'\n';
	mult = char_counter[i]; //* //compression_map[i].size() //huffcode length;
  	sum = sum + mult;
  
  // ???
}
  return sum;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

// example of a priority queue using the TrieNode class

void example_function()
{
  priority_queue <TrieNode *, vector<TrieNode *>, TrieNodePtrCompare> pq;      

  TrieNode *x;

  x = new TrieNode('a', 10);
  pq.push(x);
  x = new TrieNode('b', 8);
  pq.push(x);
  x = new TrieNode('c', 20);
  pq.push(x);
  x = new TrieNode('d', 30);
  pq.push(x);

  cout << "pq contains " << pq.size() << " elements.\n";

  while (!pq.empty()) {
    x = pq.top();
    cout << x->character << " " << x->frequency << endl;  // print highest-priority TrieNode object in the PQ
    pq.pop();                                             // remove the highest priority element from PQ
    delete(x);
  }
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

int main(int argc, char **argv)
{
  if (argc < 2) {
    cout << "huffman [-debug | -ascii | -example] <filename>\n";
    exit(1);
  }

  // flags?

  for (int i = 1; i < argc; i++) {
    if (!strcmp("-debug", argv[i]))		
      debug_flag = true;
    else if (!strcmp("-ascii", argv[i]))		
      ascii_flag = true;
    else if (!strcmp("-example", argv[i])) {
      example_function();
      exit(1);
    }
  }

  string input_filename(argv[argc - 1]);
  string output_filename(argv[argc - 1]);
  Huffman H;

  // DECOMPRESS!!! output will end in .HUF

  if (input_filename.substr(input_filename.length() - 4, 4) == ".huf") {
    output_filename.replace(output_filename.length() - 4, 4, ".HUF");
    H.decompress(input_filename, output_filename, !ascii_flag);
  }

  // COMPRESS!!! output will end in .huf

  else {
    if (input_filename.substr(input_filename.length() - 4, 4) == ".HUF")
      output_filename.replace(output_filename.length() - 4, 4, ".huf");
    else
      output_filename += ".huf";
    H.compress(input_filename, output_filename, !ascii_flag);
  }

  return 1;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
