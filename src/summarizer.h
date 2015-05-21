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

class summarizer {
public:
    /// Constructor
    summarizer(const std::wstring &datFile);

    /// Destructor
    ~summarizer();

    std::list<word_pos> summarize(std::wostream &sout, const freeling::document &doc);

private:
    bool remove_used_lexical_chains;
    bool only_strong;
    int num_words, hypernymy_depth;
    std::wstring semdb_path;
    std::set<std::wstring> used_tags;
    std::wstring heuristic;

    std::map<std::wstring, std::list<lexical_chain>> build_lexical_chains(std::wostream &sout, const freeling::document &doc);

    void remove_one_word_lexical_chains(std::map<std::wstring, std::list<lexical_chain>> &chains);

    void remove_weak_lexical_chains(std::map<std::wstring, std::list<lexical_chain>> &chains);

    void print_lexical_chains(std::map<std::wstring, std::list<lexical_chain>> &chains, std::wostream &sout);

    int count_occurences(const freeling::word &w, const freeling::document &doc) const;

	double average_scores(std::map<std::wstring, std::list<lexical_chain> > &chains_type) const;

	double standard_deviation_scores(std::map<std::wstring, std::list<lexical_chain> > &chains_type, const double avg) const;

	std::list<lexical_chain> map_to_lists(std::map<std::wstring, std::list<lexical_chain> > &chains_type) const;

	std::list<word_pos> first_word(std::wostream &sout, std::map<std::wstring, std::list<lexical_chain> > &chains_type) const;

    std::list<word_pos> sum_of_chain_weights(std::wostream &sout, std::map<std::wstring, std::list<lexical_chain> > &chains) const;

	relation * tag_to_rel(const std::wstring ws, std::wostream &sout) const;
};