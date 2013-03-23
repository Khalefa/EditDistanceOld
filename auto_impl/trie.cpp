#include "trie.h"
/*
 * Algorithm: Edit distance using a trie-tree (Dynamic Programming)
 * Author: Murilo Adriano Vasconcelos <muriloufg@gmail.com>
 */
 
using namespace std;
 

 
// The minimum cost of a given word to be changed to a word of the dictionary
//extern int min_cost;
 
//
extern "C"  void search_impl(trie* tree, char ch, vector<int> last_row, const string& word, int &min_cost, int limit, int insert_del)
{
	
	int sz = last_row.size();
 
    vector<int> current_row(sz);
    current_row[0] = last_row[0] + 1;
 
    // Calculate the min cost of insertion, deletion, match or substution
    int insert_or_del, replace;
    for (int i = 1; i < sz; ++i) {
        if(insert_del) insert_or_del = min(current_row[i-1] + 1, last_row[i] + 1);
			else insert_or_del=1000;
        replace = (word[i-1] == ch) ? last_row[i-1] : (last_row[i-1] + 1);
 
        current_row[i] = min(insert_or_del, replace);
    }
 
    // When we find a cost that is less than the min_cost, is because
    // it is the minimum until the current row, so we update
    if ((current_row[sz-1] < min_cost) && (tree->word != "")) {
        min_cost = current_row[sz-1];
    }
 
    // If there is an element wich is smaller than the current minimum cost,
    //  we can have another cost smaller than the current minimum cost
	int min=*min_element(current_row.begin(), current_row.end());
    
		if(limit < min)
		{
			//printf("Give up");
		}
		else 
        for (trie::next_t::iterator it = tree->next.begin(); it != tree->next.end(); ++it) {
			search_impl(it->second, it->first, current_row, word, min_cost,limit, insert_del);
        }
    
}
 
extern "C" int search(trie tree,string word, int limit, int insert_del)
{
    word = string("$") + word;
     
    int sz = word.size();
	int    min_cost = 0x3f3f3f3f;
 
    vector<int> current_row(sz + 1);
 
    // Naive DP initialization
    for (int i = 0; i < sz; ++i) current_row[i] = i;
    current_row[sz] = sz;
     
     
    // For each letter in the root map wich matches with a
    //  letter in word, we must call the search
    for (int i = 0 ; i < sz; ++i) {
        if (tree.next.find(word[i]) != tree.next.end()) {
			search_impl(tree.next[word[i]], word[i], current_row, word, min_cost,limit, insert_del);
		if(limit > min_cost) 
				return min_cost;
        }
    }
 //   printf(" word %s %d", word,min_cost);
//cout << "word" << word << " "<<min_cost <<endl;
    return min_cost;
}
