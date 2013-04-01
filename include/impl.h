#ifndef _IMPL_H
#define _IMPL_H
#ifdef __cplusplus
extern "C" {
#endif


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
extern vector<Query> queries;

// Keeps all currently available results that has not been returned yet
extern vector<Document> docs;

#ifdef __cplusplus
}
#endif


#endif
