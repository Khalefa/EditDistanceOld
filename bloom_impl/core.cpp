#include "core.h"
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <vector>
#include <map>
#include <set>
#define WORD_LIST
using namespace std;

///////////////////////////////////////////////////////////////////////////////////////////////

// Computes edit distance between a null-terminated string "a" with length "na"
//  and a null-terminated string "b" with length "nb" 
int EditDistance(const char* a, int na, const char* b, int nb)
{
	int oo=0x7FFFFFFF;

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

		for(ib=ib_st;ib<=ib_en;ib++)
		{
			int ret=oo;

			int d1=T[1-cur][ib]+1;
			int d2=T[cur][ib-1]+1;
			int d3=T[1-cur][ib-1]; if(a[ia-1]!=b[ib-1]) d3++;

			if(d1<ret) ret=d1;
			if(d2<ret) ret=d2;
			if(d3<ret) ret=d3;

			T[cur][ib]=ret;
		}

		cur=1-cur;
	}

	int ret=T[1-cur][nb];

	return ret;
}

///////////////////////////////////////////////////////////////////////////////////////////////

// Computes Hamming distance between a null-terminated string "a" with length "na"
//  and a null-terminated string "b" with length "nb" 
unsigned int HammingDistance(const char* a, int na, const char* b, int nb)
{
	int j, oo=0x7FFFFFFF;
	if(na!=nb) return oo;

	unsigned int num_mismatches=0;
	for(j=0;j<na;j++) if(a[j]!=b[j]) num_mismatches++;

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
#ifdef WORD_LIST
	vector<unsigned int> words;
#endif
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

#ifdef WORD_LIST
//a flat word list
typedef struct _word{
	char str[MAX_WORD_LENGTH+1];
	vector <int> queries_id;
} Word;

vector<Word> words;

int findInwords(char* str){
	for (int i=0;i< words.size() ;  i++){
		Word *w=&words[i];
		if(strcmp(w->str, str)==0) 
			return i;
	}
	return -1;
}
#endif
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
#ifdef WORD_LIST
	int iq=0;
	while(query.str[iq]){
		while(query.str[iq]==' ') iq++;
		if(!query.str[iq]) break;
		char* qword=&query.str[iq];

		int lq=iq;
		while(query.str[iq] && query.str[iq]!=' ') iq++;
		char qt=query.str[iq];
		query.str[iq]=0;
		lq=iq-lq;

		//check if qword is already there
		int  i=findInwords(qword);
		if (i>=0){
			words[i].queries_id.push_back(query_id);
			query.words.push_back(i);
		} else {
			Word w;
			strcpy(w.str,qword);
			w.queries_id.push_back(query_id);
			words.push_back(w);
			query.words.push_back(findInwords(qword));
		}
		printf("i%d %s\n",findInwords(qword), qword);
		query.str[iq]=qt;
	}
#endif	
	// Add this query to the active query set
	queries.push_back(query);
printf("Query %d words %d\n", query.query_id, query.words.size());
	return EC_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////

ErrorCode EndQuery(QueryID query_id)
{
	// Remove this query from the active query set
	printf("Del Query %d\n",query_id);
	unsigned int i, n=queries.size();
	for(i=0;i<n;i++)
	{
		if(queries[i].query_id==query_id)
		{
			queries.erase(queries.begin()+i);
			break;
		}
	}
	for(i=0;i<n;i++)

		return EC_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////


int GetWordValue(const char * s, int n){
	int value=0;
	for(int i=0;i<n;i++){
		char c=s[i]-'A';
		if(c>=32)c=c-32;
		value+=c;
	}
	return value;
}
int MatchWord(const char *dword, const char *qword, int ld, int lq, int dword_value, const Query * quer){
	int matching_word=false;

	int qword_value=GetWordValue(qword,lq);            	
	int delta=dword_value-qword_value;
	if(delta<0)delta=-delta;
	if(delta<=26*quer->match_dist){
		if(quer->match_type==MT_EXACT_MATCH)
		{
			if(ld==lq) //same length
				if(strcmp(qword, dword)==0) matching_word=true;
		}
		else if(quer->match_type==MT_HAMMING_DIST)
		{
			if(ld==lq){ 						
				unsigned int num_mismatches=HammingDistance(qword, lq, dword, ld);
				if(num_mismatches<=quer->match_dist) matching_word=true;
			}
		}
		else if(quer->match_type==MT_EDIT_DIST)
		{
			unsigned int edit_dist=EditDistance(qword, lq, dword, ld);
			if(edit_dist<=quer->match_dist) matching_word=true;
		}
	}

	return matching_word;
}
ErrorCode MatchDocument(DocID doc_id, const char* doc_str)
{
	char cur_doc_str[MAX_DOC_LENGTH];
	strcpy(cur_doc_str, doc_str); //I really want to remove this
	vector<unsigned int> query_ids;
	set <unsigned int> foundWords;

	unsigned int i, n=queries.size();
#if 1	
	map<unsigned int,int> lqueries;
	for(i=0;i<n;i++)
	{
		Query* quer=&queries[i];
		int qid=quer->query_id;	
		int q_length=0;
		int iq=0;
		while(quer->str[iq] )
		{
			while(quer->str[iq]==' ') iq++;
			if(!quer->str[iq]) break;

			int lq=iq;
			while(quer->str[iq] && quer->str[iq]!=' ') iq++;
			lq=iq-lq;
			q_length+=lq;
		}
		lqueries[qid]=q_length;
	}	
#endif
	// Start scaning the document
	int id=0;
	while(cur_doc_str[id])
	{
		while(cur_doc_str[id]==' ') id++;
		if(!cur_doc_str[id]) break;
		char* dword=&cur_doc_str[id];

		int ld=id;
		while(cur_doc_str[id] && cur_doc_str[id]!=' ') id++;
		char dt=cur_doc_str[id];
		cur_doc_str[id]=0;
		ld=id-ld;

		int dword_value=GetWordValue(dword,ld);

		for(i=0;i<n ;i++)
		{
			int qid =queries[i].query_id;
			if (lqueries[qid]<=0)continue;

			Query* quer=&queries[i];

			int iq=0;
			while(quer->str[iq])
			{
				int matching_word=false;

				while(quer->str[iq]==' ') iq++;
				if(!quer->str[iq]) break;
				char* qword=&quer->str[iq];

				int lq=iq;
				while(quer->str[iq] && quer->str[iq]!=' ') iq++;
				char qt=quer->str[iq];
				quer->str[iq]=0;
				lq=iq-lq;
				matching_word=MatchWord(dword,qword,  ld,  lq, dword_value, quer);
				if (matching_word){
					// update statisics
					lqueries[qid]-=lq;
					int i=findInwords(qword); 
					foundWords.insert(i);
				}
				quer->str[iq]=qt;
			}
		}
		cur_doc_str[id]=dt;
	}

	for(int i=0;i<n;i++){
		unsigned int qid=queries[i].query_id;
		if(lqueries[qid]<=0)  {
			int to_insert=1;
			//make sure all words have been found
			int nn=queries[i].words.size();
			for(int j=0;j<nn;j++){
				if (foundWords.find(queries[i].words[j])==foundWords.end())
					{to_insert=0; break;}
				
			}
			if(to_insert)
				query_ids.push_back(qid);

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
