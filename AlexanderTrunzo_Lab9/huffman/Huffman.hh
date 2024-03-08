//----------------------------------------------------------------------------
// huffman coding for file compression with tries
// copyright 2010 christopher rasmussen
// university of delaware
//----------------------------------------------------------------------------

#include <iostream>
#include <ctype.h>
#include <fstream>
#include <vector>
#include <stack>
#include <queue>
#include <map>
#include <cstdlib>
#include <string>
#include <math.h>
#include <string.h>

using namespace std;

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

#define NUM_ASCII                      128     // the actual characters, including non-printing
#define BITS_PER_BYTE                  8
#define BYTES_PER_CHUNK                1      
#define BITS_PER_CHUNK                 (BYTES_PER_CHUNK*BITS_PER_BYTE)
#define BITS_PER_ASCII_CHAR            8
#define ASCII_TAB                      9
#define ASCII_NEWLINE                  10
#define ASCII_FIRST_PRINTING           32
#define NO_CHAR                        '\0'

//----------------------------------------------------------------------------

class TrieNode
{
public:

  // default 0-argument constructor

  TrieNode() { parent = NULL; left = NULL; right = NULL; character = NO_CHAR; frequency = 0; }

  // note use of default arguments so this constructor can be called with just 2 arguments or the full 5
 
  TrieNode(char c, int freq, TrieNode *par = NULL, TrieNode *l = NULL, TrieNode *r = NULL)
  { 
    parent = par; 
    left = l;
    right = r;
    character = c; 
    frequency = freq; 
  }

  // member variables

  TrieNode *parent;        // parent in tree (NULL if root)
  TrieNode *left, *right;  // children in tree (NULL if leaf)
  int frequency;           // how many occurrences of character
  char character;          // what is the char
  string huffcode;         // the Huffman code for this char

};

//----------------------------------------------------------------------------

// have to define custom comparison operator on TrieNode pointers or else
// PQ will default to comparing addresses, which would be NOT GOOD

// this will make first element the one with the SMALLEST frequency

class TrieNodePtrCompare
{
public:
  bool operator() (TrieNode * & lhs, TrieNode * & rhs) const
  {
    return lhs->frequency > rhs->frequency;
  }
};

//----------------------------------------------------------------------------

// NOTE: use of "int" type here for counting effectively limits maximum file 
// size this will work with

class Huffman
{
public:

  Huffman();
  void compute_frequencies(ifstream &);
  void print_frequencies();
  void build_optimal_trie();
  void merge_two_least_frequent_subtries();
  void print_trie_roots();
  void compute_all_codes_from_trie(TrieNode *);
  int calculate_ascii_file_size();
  int calculate_custom_file_size();
  int calculate_huffman_file_size();
  void print_compression_map();
  void print_decompression_map(ostream &, bool = false);
  void read_decompression_map(ifstream &, bool = false);
  void compress(string, string, bool = false);
  void decompress(string, string, bool = false);
  int binary_2_int(string);
  string int_2_binary(int, int);
  int pad_bit_length(int);
  void write_binary_chunk(string &, ofstream &);
  bool is_bad_ascii_code(int i) 
  { return i < 0 || i >= NUM_ASCII || (i != ASCII_TAB && i != ASCII_NEWLINE && i < ASCII_FIRST_PRINTING); }

  // optional utility function declarations (not defined for this assignment)

  string compute_code_from_path(TrieNode *);
  int recursive_calculate_huffman_file_size(TrieNode *);

  // first pass: char frequency statistics for file to compress

  vector <int> char_counter;     // how many of each char are in the file
  int num_chars;                 // sum of every entry in char_counter
  int code_table_size;           // how many chars occur at least once

  // compression stats

  int ascii_bits, custom_bits, huffman_bits;
  int ascii_bytes, custom_bytes, huffman_bytes;
  int bad_bits_in_last_chunk;   // how many bits in last compressed chunk ARE padding

  // one TrieNode * is the root of a binary tree (aka "trie").
  // a priority queue is used to maintain an entire forest of tries
  // for the Huffman merging procedure

  priority_queue <TrieNode *, vector<TrieNode *>, TrieNodePtrCompare> trie;      

  // these are the mappings between chars and bit codes and vice versa

  map <char, string> compression_map;
  map <string, char> decompression_map;
};

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
