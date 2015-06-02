#include <iostream>
#include "freeling/morfo/semdb.h"
#include "freeling/morfo/language.h"
#include <utility>
#include <set>
#include <string>
#include <vector>
#include <unordered_map>

struct word_pos {
	const freeling::word &w;
	const freeling::sentence &s;
	int n_paragraph;
	int n_sentence;
	int position;

	word_pos(const freeling::word &w_p, const freeling::sentence &s_p, int n_paragraph, int n_sentence, int position);

    bool operator==(word_pos other) const;

	std::wstring toString() const;
};

struct related_words {
	const word_pos &w1;
	const word_pos &w2;
	double relatedness;

	related_words(const word_pos &w_p1, const word_pos &w_p2, double relatedness);

	std::wstring toString() const;
};


class relation {
public:

	const std::wstring label;

	relation(const std::wstring s, const std::wstring t);

	bool is_compatible(const freeling::word &w) const;

	virtual bool compute_word (const freeling::word &w, const freeling::sentence &s, const freeling::document &doc,
		int n_paragraph, int n_sentence, int position, std::list<word_pos> &words, std::list<related_words> &relations,
		std::unordered_map<std::wstring, std::pair<int, word_pos*> > &unique_words) const = 0;

	virtual double get_homogeneity_index(const std::list<word_pos> &words, const std::list<related_words> &relations,
		const std::unordered_map<std::wstring, std::pair<int, word_pos*> > &unique_words) = 0;

	virtual std::list<word_pos> order_words_by_weight(const std::unordered_map<std::wstring, std::pair<int, word_pos*> > &unique_words) const = 0;


protected:

	const freeling::regexp compatible_tag;
	std::wostream  * sout;
};

class SameWord : public relation {

public:

	SameWord(std::wostream &sout);

	double get_homogeneity_index(const std::list<word_pos> &words, const std::list<related_words> &relations,
		const std::unordered_map<std::wstring, std::pair<int, word_pos*> > &unique_words);

	bool compute_word (const freeling::word &w, const freeling::sentence &s, const freeling::document &doc,
		int n_paragraph, int n_sentence, int position, std::list<word_pos> &words, std::list<related_words> &relations,
		std::unordered_map<std::wstring, std::pair<int, word_pos*> > &unique_words) const;

	std::list<word_pos> order_words_by_weight(const std::unordered_map<std::wstring,
		std::pair<int, word_pos*> > &unique_words) const;
};

class Hypernymy : public relation {

public:

	Hypernymy(int k, const std::wstring &semfile, std::wostream &sout);

	double get_homogeneity_index(const std::list<word_pos> &words, const std::list<related_words> &relations,
		const std::unordered_map<std::wstring, std::pair<int, word_pos*> > &unique_words);

	bool compute_word (const freeling::word &w, const freeling::sentence &s, const freeling::document &doc,
		int n_paragraph, int n_sentence, int position, std::list<word_pos> &words, std::list<related_words> &relations,
		std::unordered_map<std::wstring, std::pair<int, word_pos*> > &unique_words) const;

	std::list<word_pos> order_words_by_weight(const std::unordered_map<std::wstring,
		std::pair<int, word_pos*> > &unique_words) const;

private:

	static freeling::semanticDB * semdb;
	static int depth;

	int hypernymyAux(std::wstring s1, std::wstring s2, int k) const;
	const word_pos &count_relations(int n, const std::list<related_words> &relations) const;
};


class SameCorefGroup : public relation {

public:

	SameCorefGroup(std::wostream &sout);

	double get_homogeneity_index(const std::list<word_pos> &words, const std::list<related_words> &relations,
		const std::unordered_map<std::wstring, std::pair<int, word_pos*> > &unique_words);

	bool compute_word (const freeling::word &w, const freeling::sentence &s, const freeling::document &doc,
		int n_paragraph, int n_sentence, int position, std::list<word_pos> &words, std::list<related_words> &relations,
		std::unordered_map<std::wstring, std::pair<int, word_pos*> > &unique_words) const;

	std::list<word_pos> order_words_by_weight(const std::unordered_map<std::wstring,
		std::pair<int, word_pos*> > &unique_words) const;
};

