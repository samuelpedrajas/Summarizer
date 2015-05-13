#include "relation.h"

class lexical_chain {
public:

	lexical_chain(relation * r, const freeling::word &w, const freeling::sentence &s, int n_paragraph, int n_sentence, int position);

	~lexical_chain();

    bool compute_word(const freeling::word &w, const freeling::sentence &s, const freeling::document &doc, int n_paragraph, int n_sentence, int position, std::wostream &sout);

    double get_score();

    int get_number_of_words() const;

    const std::list<word_pos> * get_words() const;

    std::wstring toString();
private:

	double score;
	std::set<std::wstring> unique_words;
	std::list<word_pos> words;
	relation * rel;
	std::list<related_words> relations; 
};