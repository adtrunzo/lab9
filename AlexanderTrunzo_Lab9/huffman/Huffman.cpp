//----------------------------------------------------------------------------
// huffman coding for file compression with tries
// copyright 2010 christopher rasmussen
// university of delaware
// v. 1.01 -- fix for problem with tellg() on strauss
//----------------------------------------------------------------------------

#include "Huffman.hh"

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

extern bool debug_flag;

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

// compressed files have .huf suffix added -- assume for now uncompressed

Huffman::Huffman()
{
  ifstream inStream;
  char c;
  int i;

  // initialize lookup table where we will store how many of each char are in the file

  char_counter.resize(NUM_ASCII);
  for (i = 0; i < char_counter.size(); i++)
    char_counter[i] = 0;

  code_table_size = 0;
  num_chars = 0;
}

//----------------------------------------------------------------------------

// read file character by character and keep track of how many times
// each character occurs

// debug with print_frequencies()

void Huffman::compute_frequencies(ifstream & inStream)
{
  char c;
  int i;

  // read chars

  while (!inStream.eof()) {

    inStream.get(c);

    if (!inStream.eof()) {

      i = (int) c;

      // check if out-of-range or non-printing

      if (is_bad_ascii_code(i))
	continue;

      if (char_counter[i] == 0)
	code_table_size++;

      char_counter[i]++;   // increment character counter
      num_chars++;

      // echo every character that is read
      //      cout << c << endl;
    }
  }
}

//----------------------------------------------------------------------------

// print char counter

void Huffman::print_frequencies()
{
  int i;

  cout << "code table size = " << code_table_size << endl;
  
  for (i = 0; i < char_counter.size(); i++) 
    if (!is_bad_ascii_code(i))
      cout << i << " (" << (char) i << "): " << char_counter[i] << endl;
}

//----------------------------------------------------------------------------

// Huffman's algorithm for building trie (aka code table) with minimal
// cost based on character frequencies

// debug with print_trie_roots(), before and/or after merging

void Huffman::build_optimal_trie()
{
  int i;

  // fill trie PQ "forest"

  for (i = 0; i < char_counter.size(); i++) 
    if (char_counter[i] > 0) 
      trie.push(new TrieNode((char) i, char_counter[i]));

  // merge least-frequent subtries together until there's only one

  while (trie.size() > 1)
    merge_two_least_frequent_subtries();

  // populate code table

  compute_all_codes_from_trie(trie.top());

  // how long would the compressed file be in various forms?

  ascii_bits = calculate_ascii_file_size();
  custom_bits = calculate_custom_file_size();
  huffman_bits = calculate_huffman_file_size();

  ascii_bytes = ascii_bits / BITS_PER_BYTE;
  custom_bytes = custom_bits / BITS_PER_BYTE;
  huffman_bytes = huffman_bits / BITS_PER_BYTE;

  // report results...

  if (debug_flag) {
    cout << "ASCII: " << ascii_bits << " bits (" << ascii_bytes << " bytes)\n";
    cout << "Custom: " << custom_bits << " bits (" << custom_bytes << " bytes)\n";
    cout << "Huffman: " << huffman_bits << " bits (" << huffman_bytes << " bytes)\n";
  }

  // this is important for binary compression/decompression

  bad_bits_in_last_chunk = BITS_PER_BYTE - (huffman_bits % BITS_PER_CHUNK);
  if (bad_bits_in_last_chunk == BITS_PER_BYTE)
    bad_bits_in_last_chunk = 0;
}

//----------------------------------------------------------------------------

// print char and frequency info for root node of every trie in PQ "forest".
// does NOT do a full traversal of underlying binary tree

void Huffman::print_trie_roots()
{
  cout<<"trie PQ contains " << trie.size() << " elements.\n";

  TrieNode *x;
  vector <TrieNode *> V;

  V.clear();

  // take each element out of PQ one by one, print it, and store in temporary vector

  while (!trie.empty()) {
    x = trie.top();
    cout << x->character << " " << x->frequency << endl;  
    trie.pop(); 
    V.push_back(x);
  }

  // re-create PQ from temp vector

  while (!V.empty()) {
    x = V.back();
    V.pop_back();
    trie.push(x);
  }
}

//----------------------------------------------------------------------------

// convert string of 0's and 1's to integer equivalent

int Huffman::binary_2_int(string s)
{
  int i, value;

  value = 0;

  for (i = s.length() - 1; i >= 0; i--) {
    if (s[i] == '1')
      value += pow(2, s.length() - i - 1);
  }

  return value;
}

//----------------------------------------------------------------------------

// convert integer to equivalent string of 0's and 1's.  if string length
// is < n, pad with 0's on left

string Huffman::int_2_binary(int value, int n)
{
  int i, quotient, remainder, orig_value;
  string s;

  orig_value = value;

  do {
    quotient = value / 2;
    remainder = value % 2;

    if (remainder == 0)
      s.insert(0, "0");
    else
      s.insert(0, "1");

    value = quotient;

  } while (quotient != 0);

  while (s.length() < n)
    s.insert(0, "0");

  return s;
}

//----------------------------------------------------------------------------

// iterate through mapping between chars and bitcodes, printing pairs

void Huffman::print_compression_map()
{
  map <char, string>::iterator cur;
  char c;

  for (cur = compression_map.begin(); cur != compression_map.end(); cur++) {
    c = (*cur).first;
    cout << c << ": " << (*cur).second << endl;
  }
}

//----------------------------------------------------------------------------

int Huffman::pad_bit_length(int num_bits)
{
  return BITS_PER_BYTE * (int) ceil((float) num_bits / (float) BITS_PER_BYTE);
}

//----------------------------------------------------------------------------

// iterate through mapping between bitcodes and chars, printing pairs

void Huffman::print_decompression_map(ostream & outStream, bool do_binary)
{
  map <string, char>::iterator cur;
  char c, cx;
  unsigned char ucx;
  string s, t;
  int pad_length, len, i;

  // binary version

  if (do_binary) {

    char *buffer = new char[BYTES_PER_CHUNK];

    ucx = decompression_map.size();   // assuming this can't be bigger than 256 
    outStream.write((char *) &ucx, 1);

    for (cur = decompression_map.begin(); cur != decompression_map.end(); cur++) {

      // get bit string from map

      s = (*cur).first;

      // write bit string length

      ucx = s.length();   // again, assuming no code is longer than 256 bits
      outStream.write((char *) &ucx, 1);

      // write bit string, 0-padded on left to nearest byte

      len = s.length();
      pad_length = pad_bit_length(len);
      for (i = 0; i < pad_length - len; i++)
	s.insert(0, "0");
      for (i = 0; i < pad_length; i += BITS_PER_BYTE) {
	t = s.substr(i, BITS_PER_BYTE);
	ucx = binary_2_int(t);   // assuming return value <= 256
	outStream.write((char *) &ucx, 1);
      }

      // write corresponding char from map

      c = (*cur).second;
      outStream.write(&c, 1);
    }

  }

  // non-binary version

  else {
    
    outStream << decompression_map.size() << endl;
    
    for (cur = decompression_map.begin(); cur != decompression_map.end(); cur++) {
      outStream << (*cur).first << " ";
      c = (*cur).second;
      outStream << c << endl;
    }
  }
}

//----------------------------------------------------------------------------

// iterate through mapping between bitcodes and chars, printing pairs

void Huffman::read_decompression_map(ifstream & inStream, bool do_binary)
{
  char c;
  unsigned char ucx;
  int num_elements, i, j, code_length, pad_length;
  string s;

  decompression_map.clear();

  // binary version

  if (do_binary) {

    //  read code table size (must be <= 256)

    inStream.read((char *) &ucx, 1);   
    num_elements = ucx;

    for (i = 0; i < num_elements; i++) {

      s.clear();

      // read bitstring length (must be <= 256)

      inStream.read((char *) &ucx, 1);    
      code_length = ucx;

      // read bitstring itself, 0-padded on left to nearest byte

      pad_length = pad_bit_length(code_length);
      for (j = 0; j < pad_length; j += BITS_PER_BYTE) {
	inStream.read((char *) &ucx, 1);
	s += int_2_binary(ucx, BITS_PER_BYTE);
      }

      s.erase(0, pad_length - code_length);

      // read char that bitstring translates to

      inStream.read(&c, 1);

      // put in map

      decompression_map[s] = c;
    }
  }

  // non-binary version

  else {

    inStream >> num_elements;
    
    for (i = 0; i < num_elements; i++) {
      inStream >> s;
      inStream.get(c);  // space
      inStream.get(c);  // THE CHAR
      decompression_map[s] = c;
      inStream.get(c);
    }
  }
}

//----------------------------------------------------------------------------

// fill char buffer from bitstring s and write to file in binary

void Huffman::write_binary_chunk(string & s, ofstream & outStream)
{
  int i;
  char buffer[BYTES_PER_CHUNK];

  for (i = 0; i < BYTES_PER_CHUNK; i++) 
    buffer[i] = binary_2_int(s.substr(i * BITS_PER_BYTE, BITS_PER_BYTE));

  outStream.write(buffer, BYTES_PER_CHUNK);
}

//----------------------------------------------------------------------------

// read file character by character and keep track of how many times
// each character occurs

void Huffman::compress(string in_filename, string out_filename, bool do_binary)
{
  ifstream inStream;
  ofstream outStream;
  char c;
  unsigned char ucx;
  int i;
  string char_queue, s;

  cout << "COMPRESSING to " << out_filename << endl;

  // INITIALIZE -- open file to compress

  inStream.open(in_filename.c_str());

  if (inStream.fail()) {
    cout << "Failed to open input file " << in_filename << endl;
    exit(1);
  }

  // FIRST PASS -- compute character statistics, build trie

  compute_frequencies(inStream);
  if (debug_flag)
    print_frequencies();
  build_optimal_trie();

  // SECOND PASS -- rewind to beginning of input, encode to output file

  inStream.clear();
  inStream.seekg(0);

  if (do_binary)
    outStream.open(out_filename.c_str(), ios::binary);
  else
    outStream.open(out_filename.c_str());

  // WRITE code table as header

  print_decompression_map(outStream, do_binary);
  if (debug_flag)
    print_decompression_map(cout);

  // BINARY ONLY: WRITE excess bit information so decompressor knows when to stop

  if (do_binary) {
    ucx = bad_bits_in_last_chunk;
    outStream.write((char *) &ucx, 1);
  }

  // ENCODE body of file

  while (!inStream.eof()) {

    inStream.get(c);

    if (!inStream.eof()) {

      i = (int) c;

      // if this char is out-of-range or non-printing, skip it

      if (is_bad_ascii_code(i)) 
	continue;

      // binary version

      if (do_binary) {

	char_queue += compression_map[c];

	// if we have enough code bits queued up to write a chunk, turn bit string
	// into chars and emit them

	if (char_queue.length() >= BITS_PER_CHUNK) {
	  s = char_queue.substr(0, BITS_PER_CHUNK);
	  write_binary_chunk(s, outStream);
	  char_queue.erase(0, BITS_PER_CHUNK);
	}
      }

      // non-binary version

      else
	outStream << compression_map[c];

    }
  }

  // binary only: write remaining data after RIGHT-padding with 0's to fill chunk

  if (do_binary && char_queue.length() > 0) {
    while (char_queue.length() < BITS_PER_CHUNK)
      char_queue += "0";
    write_binary_chunk(char_queue, outStream);
  }

  // clean up

  inStream.close();
  outStream.close();
}

//----------------------------------------------------------------------------

// read file character by character and keep track of how many times
// each character occurs

void Huffman::decompress(string in_filename, string out_filename, bool do_binary)
{
  ifstream inStream;
  ofstream outStream;
  char c;
  unsigned char ucx;
  int i, j, file_length, file_pos;

  cout << "DECOMPRESSING to " << out_filename << endl;

  // INITIALIZE 

  if (do_binary) 
    inStream.open(in_filename.c_str(), ios::binary);
  else
    inStream.open(in_filename.c_str());

  if (inStream.fail()) {
    cout << "Failed to open input file\n";
    exit(1);
  }
  
  // need length of file for binary

  if (do_binary) {
    inStream.seekg (0, ios::end);
    file_length = inStream.tellg();
    inStream.seekg (0, ios::beg);
  }

  outStream.open(out_filename.c_str());

  // READ code table from header
  
  read_decompression_map(inStream, do_binary);
  if (debug_flag)
    print_decompression_map(cout);

  // BINARY ONLY: READ excess bit information so decompressor knows when to stop
  
  if (do_binary) {
    inStream.read((char *) &ucx, 1);
    bad_bits_in_last_chunk = ucx;
  }

  // DECODE body of file

  string char_queue, s;
  map <string, char>::iterator cur;
  char *buffer = new char[BYTES_PER_CHUNK];
  int bits_read = 0;
  int value, code_len;
  bool got_a_match;

  if (do_binary) {

    while (1) {

      inStream.read(buffer, BYTES_PER_CHUNK);

      // hit end of file

      if (!inStream) {
	if (inStream.gcount() > 0 || char_queue.size() > 0) 
 	  cout << "binary decompression error: reached end of file with " << inStream.gcount() << " bytes unread and/or non-empty char queue " << char_queue << endl;
	break;
      }

      // in the middle of file

      else {
	bits_read += BITS_PER_BYTE * BYTES_PER_CHUNK;

	// grab a chunk's worth of bytes, turn into binary, and add to character queue
	// (aka "bit stream")

	for (j = 0; j < BYTES_PER_CHUNK; j++) {
	  value = (int) buffer[j];
	  if (value < 0)
	    value = 256 + value;	
	  char_queue += int_2_binary(value, BITS_PER_BYTE);
	}

	// if we just got the last bits in the file, snip off padding bits from last byte

	file_pos = inStream.tellg();
	if (file_pos == file_length) {
	  char_queue.erase(char_queue.length() - bad_bits_in_last_chunk, bad_bits_in_last_chunk);
	  bits_read -= bad_bits_in_last_chunk;
	}

	// can we decode anything in the bit stream we've queued up so far?
	// if we can decode one thing, try again with what's left, and so on until
	// we fail...then we need to read more in before trying again.

	do {

	  got_a_match = false;

	  // try taking substrings from character queue of increasing length
	  // and seeing if they're legal

	  for (code_len = 1; code_len <= char_queue.length(); code_len++) {
	    s = char_queue.substr(0, code_len);
	    cur = decompression_map.find(s); 
	    if (cur != decompression_map.end()) {
	      outStream << (*cur).second;
	      char_queue.erase(0, code_len);
	      got_a_match = true;
	      break;
	    }
	  }
	} while (got_a_match);
      }
    }

    // how much did we read and what's left?

    if (debug_flag)
      cout << "read " << bits_read << " bits\n";
  }

  // the non-binary way--simply take one "bit" (a 0 or 1) at a time and
  // add to character queue until we have a legal code, then emit and repeat.

  else {
    while (!inStream.eof()) {

      inStream.get(c);
      
      if (!inStream.eof()) {
	
	if (c == '0')
	  s += '0';
	else
	  s += '1';
	
	cur = decompression_map.find(s); 
	if (cur != decompression_map.end()) {
	  outStream << (*cur).second;
	  s.clear();
	}
      }
    } 
  }

  // clean up

  delete buffer;

  inStream.close();
  outStream.close();
}

//----------------------------------------------------------------------------

// how many bits does entire file take to store using ascii code?

// assumes compute_frequencies() has been called

int Huffman::calculate_ascii_file_size()
{
  return BITS_PER_ASCII_CHAR * num_chars;
}

//----------------------------------------------------------------------------

// how many bits does entire file take to store using "custom" code (i.e., based
// on which chars are used)?

int Huffman::calculate_custom_file_size()
{
  return ceil(log2(code_table_size)) * num_chars;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
