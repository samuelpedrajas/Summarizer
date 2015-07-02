#include "lexical_chain.h"
#include "freeling/windll.h"
#include "freeling/output/output_freeling.h"
#include "freeling/morfo/configfile.h"
#include "freeling/morfo/traces.h"
#include "freeling/morfo/util.h"


#include <algorithm>
#include <math.h>

#undef MOD_TRACENAME
#define MOD_TRACENAME L"ANALYZER"

////////////////////////////////////////////////////////////////
///  summarizer class summarizes a document using the lexical
/// chains method.
////////////////////////////////////////////////////////////////

class summarizer {
public:
    /// Constructor
    summarizer(const std::wstring &datFile);

    /// Destructor
    ~summarizer();

    /// Summarizes a document and returns the list of sentences that composes the summary.
    std::list<const freeling::sentence*> summarize(std::wostream &sout, 
                                                   const freeling::document &doc,
                                                   int num_words) const;

private:

    /// If true, the used lexical_chains will be removed.
    bool remove_used_lexical_chains;
    /// If true, the summarizer will use only strong chains
    bool only_strong;
    /// Maximum hypernymy depth
    int hypernymy_depth;
    /// Parameter to compute the homogeinity index in the hypernymy relation.
    double alpha;
    /// Path to the semantic DB.
    std::wstring semdb_path;
    /// A set with the relations that will be used.
    std::set<relation*> used_relations;
    /// A string that indicates the heuristic that will be used.
    std::wstring heuristic;

    /// Builds all the lexical chains.
    std::map<std::wstring, std::list<lexical_chain>> build_lexical_chains(std::wostream &sout,
            const freeling::document &doc) const;

    /// Remove the lexical chains with only one word
    void remove_one_word_lexical_chains(std::map<std::wstring, std::list<lexical_chain>> &chains) const;

    /// Remove the lexical chains which does not satisfy the strength criterion.
    void remove_weak_lexical_chains(std::map<std::wstring, std::list<lexical_chain>> &chains) const;

    /// Print the lexical chains. Only for debugging.
    void print_lexical_chains(std::map<std::wstring, std::list<lexical_chain>> &chains,
                              std::wostream &sout) const;

    /// Counts the number of occurences of the word w in the document doc.
    int count_occurences(const freeling::word &w, const freeling::document &doc) const;

    /// Computes and returns the average scores of the lexical chains.
    double average_scores(std::map<std::wstring, std::list<lexical_chain> > &chains_type) const;

    /// Computes and returns the standard deviation of the lexical chains scores.
    double standard_deviation_scores(std::map<std::wstring, std::list<lexical_chain> > &chains_type,
                                     const double avg) const;

    /// Concatenate all the lists in the map chains_type into a single list.
    std::list<lexical_chain> map_to_lists(std::map<std::wstring,
                                         std::list<lexical_chain> > &chains_type) const;

    /// Auxiliar function for first_word and first_most_weighted_word function. Computes
    /// a sentence to include it in the summary or not.
    void compute_sentence(const std::list<word_pos> &wps, std::list<word_pos> &wp_list,
                          std::set<const freeling::sentence*> &sent_set, int &acc_n_words,
                          int num_words) const;

    /// Returns the list of sentences embedded in a word_pos struct which composes the
    /// summary using the heuristic FirstWord.
    std::list<word_pos> first_word(std::wostream &sout, std::map<std::wstring,
                                   std::list<lexical_chain> > &chains_type, int num_words) const;

    /// Returns the list of sentences embedded in a word_pos struct which composes the
    /// summary using the heuristic FirstMostWeightedWord.
    std:: list<word_pos> first_most_weighted_word(std::wostream &sout,
            std::map<std::wstring, std::list<lexical_chain> > &chains, int num_words) const;

    /// Returns the list of sentences embedded in a word_pos struct which composes the
    /// summary using the heuristic SumOfChainWeights.
    std::list<word_pos> sum_of_chain_weights(std::wostream &sout,
            std::map<std::wstring, std::list<lexical_chain> > &chains, int num_words) const;

    /// Creates a relation of the subclass specified by ws and returns a pointer to it.
    relation * label_to_relation(const std::wstring ws, std::wstring expr) const;

    /// Transforms a list of word_pos to a list of sentences.
    std::list<const freeling::sentence*> wp_to_sp(std::list<word_pos> &wp_l) const;
};
