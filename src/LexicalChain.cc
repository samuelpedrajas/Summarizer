#include "LexicalChain.h"

using namespace freeling;
using namespace std;

LexicalChain::LexicalChain(Relation * r, const word &w, const sentence &s,
                             int n_paragraph, int n_sentence, int position) {
	rel = r;
	word_pos * wp = new word_pos(w, s, n_paragraph, n_sentence, position);
	words.push_back(*wp);
	unique_words[w.get_lc_form()] = pair<int, word_pos*>(1, wp);
	score = -1;
}

LexicalChain::~LexicalChain() {

}

double LexicalChain::get_score() {
	// the score is computed only the first time
	if (score < 0)
		score = (double)words.size() * rel->get_homogeneity_index(words, relations, unique_words);
	return score;
}

const list<word_pos> &LexicalChain::get_words() const {
	return words;
}

list<word_pos> LexicalChain::get_ordered_words() const {
	return rel->order_words_by_weight(this->unique_words);
}

int LexicalChain::get_number_of_words() const {
	return words.size();
}

wstring LexicalChain::toString() {
	wstring res;
	res += rel->label + L" Lexical Chain\n";
	res += L"	Word list:\n";
	for (list<word_pos>::const_iterator it = words.begin(); it != words.end(); it++) {
		res += L"		" + it->toString() + L"\n";
	}
	res += L"	Relation list:\n";
	for (list<related_words>::const_iterator it = relations.begin(); it != relations.end(); it++) {
		res += L"		" + it->toString() + L"\n";
	}
	res += L"	Score: " + to_wstring(get_score()) + L"\n";
	return res;
}

bool LexicalChain::compute_word(const word &w, const sentence &s, const document &doc,
                                 int n_paragraph, int n_sentence, int position, wostream &sout) {
	score = -1; // score needs to be computed again
	return rel->compute_word(w, s, doc, n_paragraph, n_sentence, position, this->words,
	                         this->relations, this->unique_words);
}