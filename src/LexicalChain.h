#include "Relation.h"

////////////////////////////////////////////////////////////////
///  Class LexicalChain represents a lexical chain and computes 
/// words and stores (or not) them into the structures.
////////////////////////////////////////////////////////////////

class LexicalChain {
public:

	/// Constructor
	LexicalChain(Relation * r, const freeling::word &w, const freeling::sentence &s,
	              int n_paragraph, int n_sentence, int position);

	/// Destructor
	~LexicalChain();

	/// Computes a word, if the word an be added to the lexical chain, this method stores it in its
	/// structures and return true. Otherwise, it does nothing and returns false.
	bool compute_word(const freeling::word &w, const freeling::sentence &s, const freeling::document &doc,
	                  int n_paragraph, int n_sentence, int position, std::wostream &sout);

	/// Get the score of the lexical chain
	double get_score();

	/// Get the number of words inside the lexical chain
	int get_number_of_words() const;

	/// Get all the words embedded in a word_pos struct of the lexical chain
	const std::list<word_pos> &get_words() const;

	/// Get all the words ordered by frequency
	std::list<word_pos> get_ordered_words() const;

	/// Get a string representation of the lexical chain to debug
	std::wstring toString();
private:

	/// Lexical chains score.
	double score;
	/// Structure to keep track of the words frequency
	std::unordered_map<std::wstring, std::pair<int, word_pos*> > unique_words;
	/// List of words embedded in a word_pos struct
	std::list<word_pos> words;
	/// Pointer to the relation that this lexical chain uses
	Relation * rel;
	/// List of relations between words
	std::list<related_words> relations;
};