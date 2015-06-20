#include "Summarizer.h"

using namespace freeling;
using namespace std;

Summarizer::Summarizer(const wstring &datFile, bool debug) {

	// Default configuration
	this->debug = debug;
	hypernymy_depth = 2;
	alpha = 0.9;
	remove_used_lexical_chains = FALSE;
	only_strong = FALSE;
	num_words = 50;
	this->semdb_path = L"/usr/local/share/freeling/en/semdb.dat";
	heuristic = L"FirstWord"; // FirstWord, FirstMostWeightedWord, SumOfChainWeights
	Relation::max_distance = 50;

	config_file cfg;
	enum sections {GENERAL, RELATIONS};
	cfg.add_section(L"General", GENERAL);
	cfg.add_section(L"Relations", RELATIONS);
	if (not cfg.open(datFile))
		ERROR_CRASH(L"Error opening file " + datFile);

	wstring line;
	while (cfg.get_content_line(line)) {

		switch (cfg.get_section()) {

		case GENERAL: {
			wistringstream sin;
			sin.str(line);
			wstring key, value;
			sin >> key >> value;
			if (key == L"RemoveUsedChains") {
				remove_used_lexical_chains = (value == L"true" or value[0] == L'y'
				                              or value[0] == L'Y' or value == L"1");
			} else if (key == L"OnlyStrong") {
				only_strong = (value == L"true" or value[0] == L'y'
				               or value[0] == L'Y' or value == L"1");
			} else if (key == L"NumWords") {
				num_words = stoi(value);
			} else if (key == L"SemDBPath") {
				semdb_path = value;
			} else if (key == L"Heuristic") {
				heuristic = value;
			} else if (key == L"MaxDistanceBetweenWords") {
				Relation::max_distance = stoi(value);
			}
			break;
		}

		case RELATIONS: {
			wistringstream sin;
			sin.str(line);
			wstring elem;
			sin >> elem;
			if (elem == L"Hypernymy") {
				wstring value1, value2;
				sin >> value1 >> value2;
				hypernymy_depth = stoi(value1);
				alpha = stof(value2);
			}
			used_relations.insert(elem);
			break;
		}

		default: break;
		}
	}
	cfg.close();

	if (used_relations.size() == 0)
		used_relations = {L"SameWord", L"Hypernymy", L"SameCoreferenceGroup"};

	TRACE(1, L"Module sucessfully loaded");
}

Summarizer::~Summarizer() {

}

double Summarizer::average_scores(map<wstring, list<LexicalChain> > &chains) const {
	double avg = 0;
	int size = 0;
	for (set<wstring>::const_iterator it_tag = used_relations.begin();
	        it_tag != used_relations.end(); it_tag++) {
		list<LexicalChain> &lexical_chains = chains[*(it_tag)];
		size += lexical_chains.size();
		for (list<LexicalChain>::iterator it = lexical_chains.begin();
		        it != lexical_chains.end(); it++) {
			avg += it->get_score();
		}
	}
	return (double) avg / size;
}

double Summarizer::standard_deviation_scores(map<wstring, list<LexicalChain> > &chains,
        const double avg) const {
	double sd = 0;
	int size = 0;
	for (set<wstring>::const_iterator it_tag = used_relations.begin();
	        it_tag != used_relations.end(); it_tag++) {
		list<LexicalChain> &lexical_chains = chains[*(it_tag)];
		size += lexical_chains.size();
		for (list<LexicalChain>::iterator it = lexical_chains.begin();
		        it != lexical_chains.end(); it++) {
			sd += (double) pow(it->get_score() - avg, 2);
		}
	}
	return sqrt(sd / size);
}

list<LexicalChain> Summarizer::map_to_lists(map<wstring, list<LexicalChain> > &chains) const {
	list<LexicalChain> spliced_lists;
	for (set<wstring>::const_iterator it_tag = used_relations.begin();
	        it_tag != used_relations.end(); it_tag++) {
		list<LexicalChain> &lexical_chains = chains[*(it_tag)];
		spliced_lists.splice(spliced_lists.end(), lexical_chains);
	}
	return spliced_lists;
}

bool compare_lexical_chains (LexicalChain &first, LexicalChain &second)
{
	return (first.get_score() >= second.get_score());
}

void Summarizer::compute_sentence(const list<word_pos> &wps, list<word_pos> &wp_list,
                                  set<const sentence*> &sent_set, int &acc_n_words) const {
	bool brk = false;
	for (list<word_pos>::const_iterator it_wp = wps.begin(); it_wp != wps.end() &&
	        acc_n_words < num_words && !brk; it_wp++) {
		const word_pos wp = *it_wp;
		const sentence & s = wp.s;
		if (sent_set.find(&s) == sent_set.end()) {
			// Counting the number of words (here we exclude commas,
			// points, exclamation symbols, etc...)
			int s_size = 0;
			for (sentence::const_iterator it_s = s.words_begin(); it_s != s.words_end(); it_s++)
				if (it_s->get_tag()[0] != L'F') s_size++;

			if (s_size + acc_n_words <= num_words) {
				sent_set.insert(&s);
				acc_n_words += s_size;
				wp_list.push_back(wp);
				if (remove_used_lexical_chains) brk = true;
			}
		}
	}
}

list<word_pos> Summarizer::first_word(wostream &sout,
                                      map<wstring, list<LexicalChain> > &chains) const {
	list<LexicalChain> lexical_chains = map_to_lists(chains);
	lexical_chains.sort(compare_lexical_chains);
	set<const sentence*> sent_set;
	list<word_pos> wp_list;
	int acc_n_words = 0;
	for (list<LexicalChain>::const_iterator it = lexical_chains.begin();
	        it != lexical_chains.end(); it++) {
		const list<word_pos> &wps = it->get_words();

		compute_sentence(wps, wp_list, sent_set, acc_n_words);
	}
	return wp_list;
}

list<word_pos> Summarizer::first_most_weighted_word(wostream &sout,
        map<wstring, list<LexicalChain> > &chains) const {
	list<LexicalChain> lexical_chains = map_to_lists(chains);
	lexical_chains.sort(compare_lexical_chains);
	set<const sentence*> sent_set;
	list<word_pos> wp_list;
	int acc_n_words = 0;
	for (list<LexicalChain>::const_iterator it = lexical_chains.begin();
	        it != lexical_chains.end(); it++) {
		list<word_pos> wps = it->get_ordered_words();

		compute_sentence(wps, wp_list, sent_set, acc_n_words);
	}
	return wp_list;
}

bool order_by_scores (const pair<int, const word_pos*> &sc_wp1,
                      const pair<int, const word_pos*> &sc_wp2)
{
	return sc_wp1.first > sc_wp2.first;
}

list<word_pos> Summarizer::sum_of_chain_weights(wostream &sout,
        map<wstring, list<LexicalChain> > &chains) const {
	list<LexicalChain> lexical_chains = map_to_lists(chains);
	lexical_chains.sort(compare_lexical_chains);

	// The key is the sentence number, the first value of the pair is the score and the second is the
	// first word_pos that reference the sentence of the key
	unordered_map<int, pair<int, const word_pos*> > sentence_scores;

	// Here we score the sentences that have at least one word in a lexical chain
	for (list<LexicalChain>::const_iterator it = lexical_chains.begin();
	        it != lexical_chains.end(); it++) {
		const list<word_pos> &wps = it->get_words();

		for (list<word_pos>::const_iterator it_wp = wps.begin(); it_wp != wps.end(); it_wp++) {
			unordered_map<int, pair<int, const word_pos*> >::iterator it_ss =
			    sentence_scores.find(it_wp->n_sentence);
			if (it_ss != sentence_scores.end()) {
				(it_ss->second).first++;
			} else {
				sentence_scores[it_wp->n_sentence] = make_pair(1, &*it_wp);
			}
		}
	}

	// We insert every pair in a list and then we order by the score
	list<pair<int, const word_pos*> > score_wp_list;
	for (unordered_map<int, pair<int, const word_pos*> >::const_iterator it = sentence_scores.begin();
	        it != sentence_scores.end(); it++) {
		score_wp_list.push_back(it->second);
		sout << L"SENTENCE: " << it->second.second->n_sentence << L" SCORE: " <<
		     it->second.first << endl;
	}

	score_wp_list.sort(order_by_scores);

	// Here we select the sentences with best score and insert them in a list
	// (until the desired number of words is reached)
	int acc_n_words = 0;
	list<word_pos> wp_list;
	for (list<pair<int, const word_pos*> >::const_iterator it = score_wp_list.begin();
	        it != score_wp_list.end() && acc_n_words < num_words; it++) {
		int s_size = 0;
		const word_pos *wp = it->second;
		const sentence &s = (wp->s);

		// Counting the number of words (here we exclude commas, points,
		// exclamation symbols, etc...)
		for (sentence::const_iterator it_s = s.words_begin();
		        it_s != s.words_end(); it_s++)
			if (it_s->get_tag()[0] != L'F') s_size++;

		sout << L"NUM words: " << s_size << endl;
		if (s_size + acc_n_words <= num_words) {
			acc_n_words += s_size;
			wp_list.push_back(*wp);
		}
	}
	return wp_list;
}

Relation * Summarizer::label_to_relation(const wstring ws, wostream &sout) const {
	Relation * rel;
	if (ws == L"SameWord") {
		rel = new SameWord(sout);
	} else if (ws == L"Hypernymy") {
		rel = new Hypernymy(hypernymy_depth, alpha, semdb_path, sout);
	} else if (ws == L"SameCoreferenceGroup") {
		rel = new SameCorefGroup(sout);
	}
	return rel;
}

map<wstring, list<LexicalChain>> Summarizer::build_lexical_chains(wostream &sout, const document &doc) {
	map<wstring, list<LexicalChain>> chains;
	for (set<wstring>::const_iterator it_t = used_relations.begin(); it_t != used_relations.end(); it_t++) {
		wstring tag = *it_t;
		Relation * rel = label_to_relation(tag, sout);
		int i = 0;
		int j = 0;
		for (list<paragraph>::const_iterator it_p = doc.begin(); it_p != doc.end(); it_p++) {
			for (list<sentence>::const_iterator it_s = it_p->begin(); it_s != it_p->end(); it_s++) {
				int k = 0;
				for (list<word>::const_iterator it_w = it_s->begin(); it_w != it_s->end(); it_w++) {
					if (rel->is_compatible(*it_w)) {
						list<LexicalChain> &lc = chains[tag];
						bool inserted = false;

						// computing the word in every lexical chain
						for (list<LexicalChain>::iterator it_lc = lc.begin(); it_lc != lc.end(); it_lc++) {
							inserted = inserted || it_lc->compute_word((*it_w), (*it_s), doc, i, j, k, sout);
						}

						// if the word was not inserted in any chain, then we build a new one
						if (!inserted) {
								LexicalChain new_lc(rel, (*it_w), (*it_s), i, j, k);
								lc.push_back(new_lc);
						}						
					}
					k++;
				}
				j++;
			}
			i++;
		}
	}
	return chains;
}

void Summarizer::remove_one_word_lexical_chains(map<wstring, list<LexicalChain>> &chains) {
	for (set<wstring>::const_iterator it_t = used_relations.begin(); it_t != used_relations.end(); it_t++) {
		wstring tag = *it_t;
		list<LexicalChain> &lc = chains[tag];
		list<LexicalChain>::iterator it = lc.begin();

		while (it != lc.end())
		{
			if (it->get_number_of_words() == 1) {
				it = lc.erase(it);
			} else it++;
		}
	}
}

void Summarizer::remove_weak_lexical_chains(map<wstring, list<LexicalChain>> &chains) {
	double avg = average_scores(chains);
	double sd = standard_deviation_scores(chains, avg);

	for (set<wstring>::const_iterator it_tag = used_relations.begin(); it_tag != used_relations.end(); it_tag++) {
		list<LexicalChain> &lexical_chains = chains[*(it_tag)];
		list<LexicalChain>::iterator it = lexical_chains.begin();

		while (it != lexical_chains.end())
		{
			if (it->get_score() <= (avg + 1.0 * sd)) {
				it = lexical_chains.erase(it);
			} else it++;
		}
	}
}

void Summarizer::print_lexical_chains(map<wstring, list<LexicalChain>> &chains, wostream &sout) {
	for (set<wstring>::const_iterator it_tag = used_relations.begin(); it_tag != used_relations.end(); it_tag++) {
		list<LexicalChain> &lexical_chains = chains[*(it_tag)];
		for (list<LexicalChain>::iterator it = lexical_chains.begin(); it != lexical_chains.end(); it++)
		{
			sout << "-------------------------------" << endl;
			sout << it->toString();
		}
	}
}

list<const sentence*> Summarizer::wp_to_sp(list<word_pos> &wp_l) {
	list<const sentence*> res;
	for (list<word_pos>::const_iterator it = wp_l.begin(); it != wp_l.end(); it++) {
		res.push_back(&(it->s));
	}
	return res;
}

list<const sentence*> Summarizer::summarize(wostream &sout, const document &doc) {
	// building lexical chains
	map<wstring, list<LexicalChain>> chains = build_lexical_chains(sout, doc);

	// remove one word lexical chains
	remove_one_word_lexical_chains(chains);

	// remove weak lexical chains
	if (only_strong)
		remove_weak_lexical_chains(chains);

	// print chains
	if (debug)
		print_lexical_chains(chains, sout);

	// select most relevant sentences using one heuristic
	list<word_pos> wp_res;
	if (heuristic == L"FirstMostWeightedWord") {
		wp_res = first_most_weighted_word(sout, chains);
	} else if (heuristic == L"SumOfChainWeights") {
		wp_res = sum_of_chain_weights(sout, chains);
	} else {
		wp_res = first_word(sout, chains);
	}

	// sorts the sentences by position in the original text
	wp_res.sort();

	list<const sentence*> res = wp_to_sp(wp_res);

	return (res);
}
