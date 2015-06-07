#include "lexical_chain.h"

using namespace freeling;
using namespace std;

lexical_chain::lexical_chain(relation * r, const word &w, const sentence &s,
                             int n_paragraph, int n_sentence, int position) {
	rel = r;
	word_pos * wp = new word_pos(w, s, n_paragraph, n_sentence, position);
	words.push_back(*wp);
	unique_words[w.get_lc_form()] = pair<int, word_pos*>(1, wp);
	score = -1;
}

lexical_chain::~lexical_chain() {

}

double lexical_chain::get_score() {
	if (score < 0)
		score = (double)words.size() * rel->get_homogeneity_index(words, relations, unique_words);
	return score;
}

const list<word_pos> &lexical_chain::get_words() const {
	return words;
}

list<word_pos> lexical_chain::get_ordered_words() const {
	return rel->order_words_by_weight(this->unique_words);
}

int lexical_chain::get_number_of_words() const {
	return words.size();
}

wstring lexical_chain::toString() {
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

bool lexical_chain::compute_word(const word &w, const sentence &s, const document &doc,
                                 int n_paragraph, int n_sentence, int position, wostream &sout) {
	score = -1;
	return rel->compute_word(w, s, doc, n_paragraph, n_sentence, position, this->words,
	                         this->relations, this->unique_words);
}