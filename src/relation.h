#include <iostream>
#include "freeling/morfo/semdb.h"
#include "freeling/morfo/language.h"
#include <utility>
#include <set>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>

////////////////////////////////////////////////////////////////
/// Struct that allow us to compare words easily.
////////////////////////////////////////////////////////////////

struct word_pos {
	const freeling::word &w;
	const freeling::sentence &s;
	int n_paragraph;
	int n_sentence;
	int position;

	word_pos(const freeling::word &w_p, const freeling::sentence &s_p,
	         int n_paragraph, int n_sentence, int position);

	/// Two words are equal if they are in the same position, phrase and paragraph.
	bool operator==(word_pos other) const;

	/// One word is smaller than other one if it appears later in the text.
	bool operator<(word_pos other) const;

	/// One word is greater than other one if it appears sooner in the text.
	bool operator>(word_pos other) const;

	/// Get a string representation of the word_pos to debug
	std::wstring toString() const;
};

////////////////////////////////////////////////////////////////
/// Struct that represents a relationship between two words.
////////////////////////////////////////////////////////////////

struct related_words {
	const word_pos &w1;
	const word_pos &w2;
	/// Relatedness represents the strength of the relationship.
	double relatedness;

	related_words(const word_pos &w_p1, const word_pos &w_p2, double relatedness);

	/// Get a string representation of the relationship to debug
	std::wstring toString() const;
};

////////////////////////////////////////////////////////////////
///  Class relation is a non-instantiable class which defines
/// many virtual methods to check if a word is compatible with
/// the relation or if a word can be stored in the structures
/// of a lexical_chain.
////////////////////////////////////////////////////////////////

class relation {
public:

	/// Label with the name of the related. It is used for debugging.
	const std::wstring label;
	/// The maximum distance in phrases between two words to be related
	static int max_distance;

	/// Constructor
	relation(const std::wstring s, const std::wstring t);

	/// Destructor
	~relation();

	/// True if the words tag is compatible with the relation
	bool is_compatible(const freeling::word &w) const;

	virtual bool compute_word (const freeling::word &w, const freeling::sentence &s,
	                           const freeling::document &doc, int n_paragraph,
	                           int n_sentence, int position, std::list<word_pos> &words,
	                           std::list<related_words> &relations, std::unordered_map<std::wstring,
	                           std::pair<int, word_pos*> > &unique_words) const = 0;

	virtual double get_homogeneity_index(const std::list<word_pos> &words,
	                                     const std::list<related_words> &relations,
	                                     const std::unordered_map<std::wstring,
	                                     std::pair<int, word_pos*> > &unique_words) = 0;

	virtual std::list<word_pos> order_words_by_weight(const std::unordered_map<std::wstring,
	        std::pair<int, word_pos*> > &unique_words) const = 0;


protected:

	/// If a word tag matchs with compatible_tag, then the word is compatible
	/// with the relation.
	const freeling::regexp compatible_tag;
	/// Pointer to a wostream to debug.
	std::wostream  * sout;
};

////////////////////////////////////////////////////////////////
///  Class SameWord represents the same word relation: two words
/// are related if they are the same word.
////////////////////////////////////////////////////////////////

class SameWord : public relation {

public:

	/// Constructor
	SameWord(std::wostream &sout);

	/// Computes the homogeinity index of the given structures using the specific formula
	/// of this relation.
	double get_homogeneity_index(const std::list<word_pos> &words,
	                             const std::list<related_words> &relations,
	                             const std::unordered_map<std::wstring,
	                             std::pair<int, word_pos*> > &unique_words);

	/// Returns true and stores the word w in the list words, list relations and
	/// unordered_map unique_words if w is compatible with the words in these
	/// structures using this relation.
	bool compute_word (const freeling::word &w, const freeling::sentence &s,
	                   const freeling::document &doc, int n_paragraph,
	                   int n_sentence, int position, std::list<word_pos> &words,
	                   std::list<related_words> &relations, std::unordered_map<std::wstring,
	                   std::pair<int, word_pos*> > &unique_words) const;

	/// In SameWord, the words in unique_words are not sorted because there is just
	/// one word. It returns the word_pos in unique_words in a list.
	std::list<word_pos> order_words_by_weight(const std::unordered_map<std::wstring,
	        std::pair<int, word_pos*> > &unique_words) const;
};

////////////////////////////////////////////////////////////////
///  Class Hypernymy represents the hypernymy relation: two words
/// are related if one is an hypernymy of the other and the
/// hypernymy depth is smaller or equal than a given maximum.
////////////////////////////////////////////////////////////////

class Hypernymy : public relation {

public:

	/// Constructor
	Hypernymy(int k, double alpha, const std::wstring &semfile, std::wostream &sout);

	/// Computes the homogeinity index of the given structures using the specific formula
	/// of this relation.
	double get_homogeneity_index(const std::list<word_pos> &words,
	                             const std::list<related_words> &relations,
	                             const std::unordered_map<std::wstring,
	                             std::pair<int, word_pos*> > &unique_words);

	/// Returns true and stores the word w in the list words, list relations and
	/// unordered_map unique_words if w is compatible with the words in these
	/// structures using this relation.
	bool compute_word (const freeling::word &w, const freeling::sentence &s,
	                   const freeling::document &doc, int n_paragraph, int n_sentence,
	                   int position, std::list<word_pos> &words, std::list<related_words> &relations,
	                   std::unordered_map<std::wstring, std::pair<int, word_pos*> > &unique_words) const;

	/// Sorts the words in unique_words by word frequency and returns a list with them.
	std::list<word_pos> order_words_by_weight(const std::unordered_map<std::wstring,
	        std::pair<int, word_pos*> > &unique_words) const;

private:

	/// Semantic DB to check the hypernymy.
	static freeling::semanticDB * semdb;
	/// Maximum depth.
	static int depth;
	/// Used to compute the homogeinity index.
	static double alpha;

	/// Recursive auxiliar function to check if two words are related.
	int hypernymyAux(std::wstring s1, std::wstring s2, int k) const;

	/// Gets the word which appears in most relations.
	const word_pos &count_relations(int n, const std::list<related_words> &relations) const;
};

////////////////////////////////////////////////////////////////
///  Class SameCorefGroup represents the same coreference group
/// relation: two words are related if they are in the same
/// coreference group.
////////////////////////////////////////////////////////////////

class SameCorefGroup : public relation {

public:

	/// Constructor
	SameCorefGroup(std::wostream &sout);

	/// Computes the homogeinity index of the given structures using the specific formula
	/// of this relation.
	double get_homogeneity_index(const std::list<word_pos> &words,
	                             const std::list<related_words> &relations,
	                             const std::unordered_map<std::wstring,
	                             std::pair<int, word_pos*> > &unique_words);

	/// Returns true and stores the word w in the list words, list relations and
	/// unordered_map unique_words if w is compatible with the words in these
	/// structures using this relation.
	bool compute_word (const freeling::word &w, const freeling::sentence &s,
	                   const freeling::document &doc, int n_paragraph, int n_sentence,
	                   int position, std::list<word_pos> &words, std::list<related_words> &relations,
	                   std::unordered_map<std::wstring, std::pair<int, word_pos*> > &unique_words) const;

	/// Sorts the words in unique_words by word frequency and returns a list with them.
	std::list<word_pos> order_words_by_weight(const std::unordered_map<std::wstring,
	        std::pair<int, word_pos*> > &unique_words) const;
};

