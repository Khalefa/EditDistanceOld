
#include "core.h"
#include <cstring>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <vector>
#include <bitset>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>
using namespace std;
unordered_map<std::string,int> words;
bitset<26> inital_letters;
///////////////////////////////////////////////////////////////////////////////////////////////
int WordValue(const char * word, int lq){
	int val=0;
	for(int i=0;i<lq;i++)
		val+=word[i]-'a';

	return val;
}

int strcmp_(const char* s1, const char* s2)
{
	while(*s1 && (*s1==*s2))
		s1++,s2++;
	if(*s1=='\0' && *s2==' ') return 0;
	return *(const unsigned char*)s1-*(const unsigned char*)s2;
}

int EditDistance(const char* a, int na, const char* b, int nb, int limit)
{
	int oo=10;
	int diff=nb-na;
	if (diff<0) diff=-diff;
	if(diff>3 ) return 4;

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
	int count;
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


struct vector_less {
	bool operator ()(Query const& a, Query const& b) const {
		if (a.count > b.count) return true;
		return false;
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////

ErrorCode StartQuery(QueryID query_id, const char* query_str, MatchType match_type, unsigned int match_dist)
{
	Query query;
	query.query_id=query_id;
	strcpy(query.str, query_str);
	query.match_type=match_type;
	query.match_dist=match_dist;
	//	printf("Q_id %d %d %d\n", query_id, match_type, match_dist);
	//check if the words are new
	int count=0;
	int iq=0;
	while(query.str[iq] )
	{
		while(query.str[iq]==' ') iq++;
		if(!query.str[iq]) break;
		char* qword=&query.str[iq];

		int lq=iq;
		while(query.str[iq] && query.str[iq]!=' ') iq++;
		char qt=query.str[iq];
		query.str[iq]=0;
		lq=iq-lq;
		count=++words[qword];
		query.str[iq]=qt;
	}
	query.count=count;
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
			Query *query=&queries[i];

			int iq=0;
			while(query->str[iq]){
				while(query->str[iq]==' ') iq++;
				if(!query->str[iq]) break;
				char* qword=&query->str[iq];

				int lq=iq;
				while(query->str[iq] && query->str[iq]!=' ') iq++;
				char qt=query->str[iq];
				query->str[iq]=0;
				lq=iq-lq;
				int l=words[qword]--;
				if(l==0)  {
					words.erase (qword);        
				}
				query->str[iq]=qt;
			}


			queries.erase(queries.begin()+i);
			break;
		}
	}
	return EC_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////
//two ideas
//remove from doc all non matched words
//edit distancce
//get query length

bitset<32> QueryLength(){
	bitset<32> lengths; 
	unsigned int i, n=queries.size();

	for(i=0;i<n;i++)
	{
		Query* quer=&queries[i];
		//fail quickly if any word of query is in not_found
		int iq=0;
		while(quer->str[iq])
		{
			while(quer->str[iq]==' ') iq++;
			if(!quer->str[iq]) break;
			char* qword=&quer->str[iq];

			int lq=iq;
			while(quer->str[iq] && quer->str[iq]!=' ') iq++;
			lq=iq-lq;
			if(quer->match_type==MT_EDIT_DIST)
			{
				for(int len=lq-quer->match_dist; len <lq+quer->match_dist;len++)
				{
					if(len>=4&& len<=31)
						lengths.set(len);
				}

			} else 	lengths.set(lq);
		}
	}
	return lengths;
}
void doc(char * dst_doc, const char *doc_str){
	float removed=0;
	bitset<32> lengths=QueryLength();
	initals();
	int id=0;
	int did=0;
	while(doc_str[id] )
	{
		while(doc_str[id]==' ') id++;
		if(!doc_str[id]) break;
		int s_id=id;

		
		while(doc_str[id] && doc_str[id]!=' '){id++; }

		if(lengths[id-s_id] && exist_initals(&doc_str[s_id]))
		{//need to remove it
			//removed+=id-ld;
			for(int i=s_id;i<=id;i++,did++) 
				dst_doc[did]=doc_str[i];

		} else removed+=id-s_id;
	}
	dst_doc[did]='\0';
//	printf("Removed %f %f\n",removed, removed/id*100);
}

int exist_initals(const char *a){
	if(inital_letters[a[0]-'a'] || inital_letters[a[1]-'a']|| inital_letters[a[2]-'a']|| inital_letters[a[3]-'a']) return true;
	return false;
}
void printQueries(){
	unsigned int i, n=queries.size();
	printf("Queries LIST\n");
	for(i=0;i<n;i++){
		Query* q=&queries[i];
		printf("Q_id %d count %d\n", q->query_id, q->count);

	}
	printf("\n");
}
ErrorCode MatchDocument(DocID doc_id, const char* doc_str)
{
	char cur_doc_str[MAX_DOC_LENGTH];
	strcpy(cur_doc_str, doc_str);
	std::sort(queries.begin(), queries.end(),vector_less());

//	doc(cur_doc_str,doc_str);
//	if(doc_id==3)	printf("%s\n",cur_doc_str);
	bitset<32> query_length=QueryLength();	
	bitset<(31-4)*26> f_mask;

	unsigned int i, n=queries.size();
	vector<unsigned int> query_ids;
	std::unordered_set<std::string> found_words;
	std::unordered_set<std::string> not_found_words;

	//printQueries();
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
				int qval=WordValue(qword,lq);
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
				int id=0;
				while(cur_doc_str[id] && !matching_word)
				{
					while(cur_doc_str[id]==' ') id++;
					if(!cur_doc_str[id]) break;
					const char* dword=&cur_doc_str[id];

					int ld=id;
					while(cur_doc_str[id] &&cur_doc_str[id]!=' ') id++;
					ld=id-ld;

					unsigned int edit_dist=EditDistance(qword, lq, dword, ld,quer->match_dist);
					if(edit_dist<=quer->match_dist) matching_word=true;			
				}
			} else {
				/* HAMMING or EXACT*/
				int id=0;
				char *qw=qword;
				int qval=WordValue(qword,lq);
				if(f_mask[qval]==1)
					if(found_words.find(qword)!=found_words.end()) {
						matching_word=true;
					}

					while(cur_doc_str[id] && !matching_word) {
						while(cur_doc_str[id]==' ') id++;
						if(!cur_doc_str[id]) break;

						unsigned int num_mismatches=0;
						int fail=false;
						while(cur_doc_str[id] && cur_doc_str[id]!=' '&& *qw) {
							if(*qw!=cur_doc_str[id]) {
								num_mismatches++; 
								if (quer->match_dist+1==num_mismatches){ fail=true; break;}
							}
							id++; qw++;
						}
						if(!*qw){ 
							if (cur_doc_str[id]!=' ' &&cur_doc_str[id]) fail=true;
						}
						else
							fail=true;

						if(fail) 
							while(cur_doc_str[id] && cur_doc_str[id]!=' ') id++;

						qw=qword;
						matching_word=!fail;
						if(matching_word&& quer->match_type==MT_EXACT_MATCH){
							found_words.insert(qword);
							f_mask.set(qval);

						}
					}
			}
			//done with qword

			if(!matching_word)
			{
				// This query has a word that does not match any word in the document
				matching_query=false;

				if(quer->match_type==MT_EXACT_MATCH){
					not_found_words.insert(qword);
					//nf_mask.set(WordValue(qword,lq));
				}
			}
			quer->str[iq]=qt;
		}/*while(quer->str[iq] && matching_query)*/

		if(matching_query)
		{
			// This query matches the document
			query_ids.push_back(quer->query_id);
		}

	}
	sort(query_ids.begin(), query_ids.end());
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


void initals(){
	inital_letters.reset();

	int i, n=queries.size();
	for(i=0;i<n;i++)
	{
		Query* quer=&queries[i];
		int iq=0;

		while(quer->str[iq])
		{
			while(quer->str[iq]==' ') iq++;
			if(!quer->str[iq]) break;
			int lq=iq;
			while(quer->str[iq] && quer->str[iq]!=' '){ 
				if(iq-lq<=4) inital_letters.set(quer->str[iq]-'a');
				iq++;
			
			}

		}
	}
}
