
#include "core.h"
#include <cstring>
#include <string>
#include <cstdlib>
#include <utility>
#include <iostream>

#include <cstdio>
#include <vector>
#include <unordered_set>
using namespace std;

///////////////////////////////////////////////////////////////////////////////////////////////
int strcmp_(const char* s1, const char* s2)
{
	while(*s1 && (*s1==*s2))
		s1++,s2++;
	if(*s1=='\0' && *s2==' ') return 0;
	return *(const unsigned char*)s1-*(const unsigned char*)s2;
}
/*******************/
int min(int a, int b, int c){
int m=(a>b)?b:a;
m=(c>m)?m:c;
return m;
}
int editDistance(const char *a, int na, const char * b, int nb, int k) {
int oo=10;

static int H[31+1];
int top = k +1;
for (int i=0;i<=nb;i++)  H[i] = i;	
for(int j=1;j<=na;j++){
int  c = 0;
for(int i=1;i<=top;i++){
int e=c;
if(a[i-1]!=b[j-1]) e=min(H[i-1], H[i], c)+1;
c=H[i];H[i]=e;
}
while (H[top] >k)  top--;
if (top == nb ) 
return H[top];

else top++;
}
return oo;
}
int EditDistance(const char *a, int na, const char * b, int nb, int k) {
int diff=nb-na;
if (diff<0) diff=-diff;
if(diff>k) return 100;

if(na>nb) 
return	editDistance(a, na, b,nb,  k);
else return editDistance(b, nb, a,na,  k);
}
int oldEditDistance(const char* a, int na, const char* b, int nb, int limit)
{
	int oo=10;
	int diff=nb-na;
	if (diff<0) diff=-diff;
	if(diff>limit ) return limit+1;

	static int T[2][MAX_WORD_LENGTH+1];

	int ia, ib;

	int cur=0;
	ia=0;

	for(ib=0;ib<=nb;ib++)
		T[cur][ib]=ib;

	cur=1-cur;

	for(ia=1;ia<=na;ia++)
	{
		for(ib=0;ib<=nb;ib++)
			T[cur][ib]=oo;

		int ib_st=0;
		int ib_en=nb;

		if(ib_st==0)
		{
			ib=0;
			T[cur][ib]=ia;
			ib_st++;
		}
		int min=oo;
		for(ib=ib_st;ib<=ib_en;ib++)
		{
			int ret=oo;

			int d1=T[1-cur][ib]+1;
			int d2=T[cur][ib-1]+1;
			int d3=T[1-cur][ib-1]; if(a[ia-1]!=b[ib-1]) d3++;

			if(d1<ret) ret=d1;
			if(d2<ret) ret=d2;
			if(d3<ret) ret=d3;
			if(ret<min)min=ret;
			T[cur][ib]=ret;
		}
		if(min>limit) return limit+1;
		cur=1-cur;
	}

	int ret=T[1-cur][nb];

	return ret;
}

///////////////////////////////////////////////////////////////////////////////////////////////

// Computes Hamming distance between a null-terminated string "a" with length "na"
//  and a null-terminated string "b" with length "nb" 
unsigned int HammingDistance(const char* a, int na, const char* b, int nb, int limit)
{
	int j, oo=0x7FFFFFFF;
	if(na!=nb) return oo;

	unsigned int num_mismatches=0;
	for(j=0;j<na;j++) if(a[j]!=b[j]) {num_mismatches++;if (num_mismatches==limit+1) return 100;}

	return num_mismatches;
}

///////////////////////////////////////////////////////////////////////////////////////////////

// Keeps all information related to an active query
struct Query
{
	QueryID query_id;
	char str[MAX_QUERY_LENGTH];
	MatchType match_type;
	unsigned int match_dist;
};

///////////////////////////////////////////////////////////////////////////////////////////////

// Keeps all query ID results associated with a dcoument
struct Document
{
	DocID doc_id;
	unsigned int num_res;
	QueryID* query_ids;
};

///////////////////////////////////////////////////////////////////////////////////////////////

// Keeps all currently active queries
vector<Query> queries;

// Keeps all currently available results that has not been returned yet
vector<Document> docs;

///////////////////////////////////////////////////////////////////////////////////////////////

ErrorCode InitializeIndex(){return EC_SUCCESS;}

///////////////////////////////////////////////////////////////////////////////////////////////

ErrorCode DestroyIndex(){return EC_SUCCESS;}

///////////////////////////////////////////////////////////////////////////////////////////////

ErrorCode StartQuery(QueryID query_id, const char* query_str, MatchType match_type, unsigned int match_dist)
{
	Query query;
	query.query_id=query_id;
	strcpy(query.str, query_str);
	query.match_type=match_type;
	query.match_dist=match_dist;
	//	printf("Q_id %d %d %d\n", query_id, match_type, match_dist);
	// Add this query to the active query set
	queries.push_back(query);
	return EC_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////

ErrorCode EndQuery(QueryID query_id)
{
	// Remove this query from the active query set
	unsigned int i, n=queries.size();
	for(i=0;i<n;i++)
	{
		if(queries[i].query_id==query_id)
		{
			queries.erase(queries.begin()+i);
			break;
		}
	}
	return EC_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////
int cmp(const char *a, const char *b, int l, int dist){
	int num_mismatches=0;
	int fail=false;
	for(int i=0;i<l;i++) {
		if(a[i]!=b[i]) {
			num_mismatches++; 
			if (dist+1==num_mismatches){ fail=true; break;}
		}
	}
	return !fail;
}
void GetWords(const char *doc_str, unordered_set<string> &words){
	int id=0;
	char word[31];
	while(doc_str[id]) {
		while(doc_str[id]==' ') id++;
		if(!doc_str[id]) break;
		int ld=id;
		while(doc_str[id] && doc_str[id]!=' '){word[id-ld]=doc_str[id]; id++;}
		word[id-ld]=0;
		ld=id-ld;
		words.insert(word);
	}
}
ErrorCode MatchDocument(DocID doc_id, const char* doc_str)
{
	unsigned int i, n=queries.size();
	vector<unsigned int> query_ids;
	//store found words
	unordered_set<string> found_words;
	unordered_set<string> words;
	unordered_set<string> not_found_words;

	GetWords(doc_str, words);
	// Iterate on all active queries to compare them with this new document
	for(i=0;i<n;i++)
	{
		bool matching_query=true;
		Query* quer=&queries[i];
		//fail quickly if any word of query is in not_found
		int iq=0;
		if(quer->match_type==MT_EXACT_MATCH) {
			while(quer->str[iq] && matching_query)
			{
				while(quer->str[iq]==' ') iq++;
				if(!quer->str[iq]) break;
				char* qword=&quer->str[iq];

				int lq=iq;
				while(quer->str[iq] && quer->str[iq]!=' ') iq++;
				char qt=quer->str[iq];
				quer->str[iq]=0;
				lq=iq-lq;

				if(not_found_words.find(qword)!=not_found_words.end()) {
					matching_query=false;
					quer->str[iq]=qt;
					break;
				}
				quer->str[iq]=qt;
			}
		}
		if(!matching_query)continue;

		iq=0;
		while(quer->str[iq] && matching_query)
		{
			while(quer->str[iq]==' ') iq++;
			if(!quer->str[iq]) break;
			char* qword=&quer->str[iq];

			int lq=iq;
			while(quer->str[iq] && quer->str[iq]!=' ') iq++;
			char qt=quer->str[iq];
			quer->str[iq]=0;
			lq=iq-lq;

			bool matching_word=false;
			if (quer->match_type==MT_EDIT_DIST) {
				for(unordered_set<string>::iterator it=words.begin(); it!=words.end();it++){
					unsigned int edit_dist=oldEditDistance(qword, lq, it->c_str(), it->length(),quer->match_dist);
					if(edit_dist<=quer->match_dist) {matching_word=true; break;}			
				}
			} else {
				/* HAMMING or EXACT*/
				if(found_words.find(qword)!=found_words.end()) {
					matching_word=true;
				}
				if(!matching_word) {
					for(unordered_set<string>::iterator it=words.begin(); it!=words.end();it++){
						if (lq!=it->length()) continue;
						if(cmp(it->c_str(),qword,lq,quer->match_dist)) {
							matching_word=true;
							found_words.insert(qword);
							break;
						}
					}	
				}
			}

			if(!matching_word)	{
				// This query has a word that does not match any word in the document
				matching_query=false;

				if(quer->match_type==MT_EXACT_MATCH){
					not_found_words.insert(qword);
				}
			}
			quer->str[iq]=qt;
		}/*while(quer->str[iq] && matching_query)*/

		if(matching_query)	{
			// This query matches the document
			query_ids.push_back(quer->query_id);
		}
	}

	Document doc;
	doc.doc_id=doc_id;
	doc.num_res=query_ids.size();
	doc.query_ids=0;
	if(doc.num_res) doc.query_ids=(unsigned int*)malloc(doc.num_res*sizeof(unsigned int));
	for(i=0;i<doc.num_res;i++) doc.query_ids[i]=query_ids[i];
	// Add this result to the set of undelivered results
	docs.push_back(doc);

	return EC_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////

ErrorCode GetNextAvailRes(DocID* p_doc_id, unsigned int* p_num_res, QueryID** p_query_ids)
{
	// Get the first undeliverd resuilt from "docs" and return it
	*p_doc_id=0; *p_num_res=0; *p_query_ids=0;
	if(docs.size()==0) return EC_NO_AVAIL_RES;
	*p_doc_id=docs[0].doc_id; *p_num_res=docs[0].num_res; *p_query_ids=docs[0].query_ids;
	docs.erase(docs.begin());
	return EC_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////
