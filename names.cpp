#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <set>
#include <sstream>
#include <fstream>

using namespace std;
using psi = pair<string, int>;


// The problem is solved using prefix tree. This program presents an API for
// adding name-score pairs to the tree and querying it for a query string.

struct classcomp {
  bool operator() (const psi& lhs, const psi& rhs) const
  {return lhs.second >= rhs.second;}
};


// The Prefix tree node contains char `c` as id, hashmap `children` to save
// pointers to it's children, a vector of name-score pairs that contain the
// names ending at `c` and a set of name-score pairs with top scores among
// pairs present at node and in its subtree.
class Node {
public:
	char c;
	unordered_map<char, Node*> children;
	vector<psi> names;
	set<psi, classcomp> topten;

	Node(char c) : c(c), names(vector<psi>()) {}
	Node() = default;
};


void update_top_ten(Node*, psi&);
vector<psi> query(Node*, string, int);
vector<string> split(string);
void insert(Node*, string&, psi&, int);
void insert(Node* root, vector<psi>&);
void insert(Node* root, psi&);


// Inserts the string `str` from index `k` of pair `input` at tree node `root`.
void insert(Node* root, string& str, psi& input, int k)
{
	if (k == (int)str.size()){
		// If end of name is reached, insert the pair at current node and update
    // top 10 name-score set.
		root->names.push_back(input);
		update_top_ten(root, input);
		return;
	}

  char curr = str[k];
	if (root->children.find(curr) == root->children.end()){
		// Make new node and insert it.
		Node* newNode = new Node(curr);
		root->children[curr] = newNode;
	}

	update_top_ten(root, input);
	insert(root->children[curr], str, input, k+1);
}


// Inserts a list of name-score pair `input` in the tree at `root`.
void insert(Node* root, vector<psi>& input)
{
  for (psi& p : input){
    insert(root, p);
  }
}


// Inserts name-score pair `input` in the tree.
void insert(Node* root, psi& input)
{
	vector<string> words = split(input.first);
	for (string word : words){
		if(!word.empty())
			insert(root, word, input, 0);
	}
}


// Updates the set of top 10 score pairs at node `root` while insertion of new
// pair `input`.
void update_top_ten(Node* root, psi& input)
{
  // If size of set is less than 10, insert the pair.
	if ((int)(root->topten).size() < 10){
		root->topten.insert(input);
	}
	else {
    // If score of `input` pair is greater than the minimum score in set,
    // replace that pair with `input`.
		if ((*(root->topten.rbegin())).second < input.second){
			root->topten.erase(--root->topten.end());
			root->topten.insert(input);
		}
	}
}


// Takes the query string `str` and root node of the tree, returns a list of
// matching name-score pairs with top scores.
vector<psi> query(Node* root, string str, int k)
{
  // If index `k` has reached the end of the string then return the `topten`
  // pairs at the current node.
	if (k == (int)str.size()){
		vector<psi> ans;
		ans.assign(root->topten.begin(), root->topten.end());
		return ans;
	}

	char curr = str[k];

  // If char is not present among the children return empty list else traverse
  // down the tree from node `root`.
	if (root->children.find(curr) == root->children.end())
		return vector<psi>();

	return query(root->children[curr], str, k+1);
}


void display(vector<psi>& v)
{
	for (psi& p : v)
		cout << p.first << " " << p.second << endl;
}


void display(vector<string>& v)
{
	for (string& p : v)
		cout << p << endl;
}


// Splits a string `s` into a list of substrings with delimiter '_'.
vector<string> split(string s)
{
    std::string delim = "_";
    vector<string> ans;

    auto start = 0U;
    auto end = s.find(delim);
    while (end != std::string::npos)
    {
        ans.push_back(s.substr(start, end - start));
        start = end + delim.length();
        end = s.find(delim, start);
    }
    if (start != end)
    	ans.push_back(s.substr(start, end - start));

    if ((int)ans.size() > 1)
    	ans.push_back(s);

    return ans;
}


// Builds up the prefix tree after reading the name-sccore pairs from a file
// and returns it's root.
Node* load()
{
  Node *root = new Node();
	vector<psi> vec;

	string line, name;
	int score;
	stringstream ss;
	ifstream myfile ("example.txt");
	if (myfile.is_open())
	{
	    while ( getline (myfile,line) )
	    {
	    	stringstream ss(line);
	        ss >> name >> score;
	        vec.push_back(make_pair(name, score));
	    }
	    myfile.close();
	}
  insert(root, vec);
  return root;
}
