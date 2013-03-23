#ifndef __SIGMOD_TRIE_H_
#define __SIGMOD_TRIE_H_

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
using namespace std;
 

#ifdef __cplusplus
extern "C" {
#endif
 
/*
 * Algorithm: Edit distance using a trie-tree (Dynamic Programming)
 * Author: Murilo Adriano Vasconcelos <muriloufg@gmail.com>
 */
 

// Trie's node
struct trie
{
    typedef map<char, trie*> next_t;
 
    // The set with all the letters which this node is prefix
    next_t next;
 
    // If word is equal to "" is because there is no word in the
    //  dictionary which ends here.
    string word;
 
    trie() : next(map<char, trie*>()) {}
    void free(){
		
		for(next_t::iterator it=next.begin();it!=next.end();it++){
			if(it->second!=NULL){
				it->second->free();
				delete it->second;
			}	

		}
		
		next.clear();
		//next=NULL;
    }
    void print(){
			for(next_t::iterator it=next.begin();it!=next.end();it++){
				printf(" %c [", it->first );
				if(it->second!=NULL)it->second->print();
				printf(" ]\n" );
		}
	
	}
    void insert(string w)
    {
        w = string("$") + w;
         
        int sz = w.size();
         
        trie* n = this;
        for (int i = 0; i < sz; ++i) {
            if (n->next.find(w[i]) == n->next.end()) {
                n->next[w[i]] = new trie();
            }
 
            n = n->next[w[i]];
        }
 
        n->word = w;
    }
};
 
 
// The minimum cost of a given word to be changed to a word of the dictionary
//
void search_impl(trie* tree, char ch, vector<int> last_row, const string& word, int &min,int limit);

int search(trie tree,string word, int limit);



#ifdef __cplusplus
}
#endif

#endif // __SIGMOD_CORE_H_
