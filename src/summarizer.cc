#include "summarizer.h"

using namespace freeling;
using namespace std;

summarizer::summarizer(const wstring &datFile) {

	// Default configuration
	hypernymy_depth = 2;
	alpha = 0.9;
	remove_used_lexical_chains = FALSE;
	only_strong = FALSE;
	num_words = 50;
	this->semdb_path = semdb_path;
	heuristic = L"FirstWord"; // FirstWord, FirstMostWeightedWord, SumOfChainWeights
	relation::max_distance = 50;
	//used_tags = {L"SameWord", L"Hypernymy", L"SameCoreferenceGroup"};

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
				relation::max_distance = stoi(value);
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
			used_tags.insert(elem);
			break;
		}

		default: break;
		}
	}
	cfg.close();

	if (used_tags.size() == 0) used_tags = {L"SameWord", L"Hypernymy",
		                                        L"SameCoreferenceGroup"
		                                       };

	TRACE(1, L"Module sucessfully loaded");
}

summarizer::~summarizer() {

}

double summarizer::average_scores(map<wstring, list<lexical_chain> > &chains) const {
	double avg = 0;
	int size = 0;
	for (set<wstring>::const_iterator it_tag = used_tags.begin();
	        it_tag != used_tags.end(); it_tag++) {
		list<lexical_chain> &lexical_chains = chains[*(it_tag)];
		size += lexical_chains.size();
		for (list<lexical_chain>::iterator it = lexical_chains.begin();
		        it != lexical_chains.end(); it++) {
			avg += it->get_score();
		}
	}
	return (double) avg / size;
}

double summarizer::standard_deviation_scores(map<wstring, list<lexical_chain> > &chains,
        const double avg) const {
	double sd = 0;
	int size = 0;
	for (set<wstring>::const_iterator it_tag = used_tags.begin();
	        it_tag != used_tags.end(); it_tag++) {
		list<lexical_chain> &lexical_chains = chains[*(it_tag)];
		size += lexical_chains.size();
		for (list<lexical_chain>::iterator it = lexical_chains.begin();
		        it != lexical_chains.end(); it++) {
			sd += (double) pow(it->get_score() - avg, 2);
		}
	}
	return sqrt(sd / size);
}

list<lexical_chain> summarizer::map_to_lists(map<wstring, list<lexical_chain> > &chains) const {
	list<lexical_chain> spliced_lists;
	for (set<wstring>::const_iterator it_tag = used_tags.begin();
	        it_tag != used_tags.end(); it_tag++) {
		list<lexical_chain> &lexical_chains = chains[*(it_tag)];
		spliced_lists.splice(spliced_lists.end(), lexical_chains);
	}
	return spliced_lists;
}

bool compare_lexical_chains (lexical_chain &first, lexical_chain &second)
{
	return (first.get_score() >= second.get_score());
}

void summarizer::compute_sentence(const list<word_pos> &wps, list<word_pos> &wp_list,
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

list<word_pos> summarizer::first_word(wostream &sout,
                                      map<wstring, list<lexical_chain> > &chains) const {
	list<lexical_chain> lexical_chains = map_to_lists(chains);
	lexical_chains.sort(compare_lexical_chains);
	set<const sentence*> sent_set;
	list<word_pos> wp_list;
	int acc_n_words = 0;
	for (list<lexical_chain>::const_iterator it = lexical_chains.begin();
	        it != lexical_chains.end(); it++) {
		const list<word_pos> &wps = it->get_words();

		compute_sentence(wps, wp_list, sent_set, acc_n_words);
	}
	return wp_list;
}

list<word_pos> summarizer::first_most_weighted_word(wostream &sout,
        map<wstring, list<lexical_chain> > &chains) const {
	list<lexical_chain> lexical_chains = map_to_lists(chains);
	lexical_chains.sort(compare_lexical_chains);
	set<const sentence*> sent_set;
	list<word_pos> wp_list;
	int acc_n_words = 0;
	for (list<lexical_chain>::const_iterator it = lexical_chains.begin();
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

list<word_pos> summarizer::sum_of_chain_weights(wostream &sout,
        map<wstring, list<lexical_chain> > &chains) const {
	list<lexical_chain> lexical_chains = map_to_lists(chains);
	lexical_chains.sort(compare_lexical_chains);

	// The key is the sentence number, the first value of the pair is the score and the second is the
	// first word_pos that reference the sentence of the key
	unordered_map<int, pair<int, const word_pos*> > sentence_scores;

	// Here we score the sentences that have at least one word in a lexical chain
	for (list<lexical_chain>::const_iterator it = lexical_chains.begin();
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

relation * summarizer::tag_to_rel(const wstring ws, wostream &sout) const {
	relation * rel;
	if (ws == L"SameWord") {
		rel = new SameWord(sout);
	} else if (ws == L"Hypernymy") {
		rel = new Hypernymy(hypernymy_depth, alpha, semdb_path, sout);
	} else if (ws == L"SameCoreferenceGroup") {
		rel = new SameCorefGroup(sout);
	}
	return rel;
}

map<wstring, list<lexical_chain>> summarizer::build_lexical_chains(wostream &sout, const document &doc) {
	map<wstring, list<lexical_chain>> chains;
	for (set<wstring>::const_iterator it_t = used_tags.begin(); it_t != used_tags.end(); it_t++) {
		wstring tag = *it_t;
		relation * rel = tag_to_rel(tag, sout);
		int i = 0;
		int j = 0;
		for (list<paragraph>::const_iterator it_p = doc.begin(); it_p != doc.end(); it_p++) {
			for (list<sentence>::const_iterator it_s = it_p->begin(); it_s != it_p->end(); it_s++) {
				int k = 0;
				for (list<word>::const_iterator it_w = it_s->begin(); it_w != it_s->end(); it_w++) {
					list<lexical_chain> &lc = chains[tag];
					bool inserted = FALSE;
					for (list<lexical_chain>::iterator it_lc = lc.begin(); it_lc != lc.end(); it_lc++) {
						inserted = inserted || it_lc->compute_word((*it_w), (*it_s), doc, i, j, k, sout);
					}

					if (!inserted) {
						if (rel->is_compatible(*it_w)) {
							lexical_chain new_lc(rel, (*it_w), (*it_s), i, j, k);
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

void summarizer::remove_one_word_lexical_chains(map<wstring, list<lexical_chain>> &chains) {
	for (set<wstring>::const_iterator it_t = used_tags.begin(); it_t != used_tags.end(); it_t++) {
		wstring tag = *it_t;
		list<lexical_chain> &lc = chains[tag];
		list<lexical_chain>::iterator it = lc.begin();

		while (it != lc.end())
		{
			if (it->get_number_of_words() == 1) {
				it = lc.erase(it);
			} else it++;
		}
	}
}

void summarizer::remove_weak_lexical_chains(map<wstring, list<lexical_chain>> &chains) {
	double avg = average_scores(chains);
	double sd = standard_deviation_scores(chains, avg);

	for (set<wstring>::const_iterator it_tag = used_tags.begin(); it_tag != used_tags.end(); it_tag++) {
		list<lexical_chain> &lexical_chains = chains[*(it_tag)];
		list<lexical_chain>::iterator it = lexical_chains.begin();

		while (it != lexical_chains.end())
		{
			if (it->get_score() <= (avg + 2.0 * sd)) {
				it = lexical_chains.erase(it);
			} else it++;
		}
	}
}

void summarizer::print_lexical_chains(map<wstring, list<lexical_chain>> &chains, wostream &sout) {
	for (set<wstring>::const_iterator it_tag = used_tags.begin(); it_tag != used_tags.end(); it_tag++) {
		list<lexical_chain> &lexical_chains = chains[*(it_tag)];
		for (list<lexical_chain>::iterator it = lexical_chains.begin(); it != lexical_chains.end(); it++)
		{
			sout << "-------------------------------" << endl;
			sout << it->toString();
		}
	}
}

list<const sentence*> summarizer::wp_to_sp(list<word_pos> &wp_l) {
	list<const sentence*> res;
	for (list<word_pos>::const_iterator it = wp_l.begin(); it != wp_l.end(); it++) {
		res.push_back(&(it->s));
	}
	return res;
}

list<const sentence*> summarizer::summarize(wostream &sout, const document &doc) {
	// building lexical chains
	map<wstring, list<lexical_chain>> chains = build_lexical_chains(sout, doc);

	// remove one word lexical chains
	remove_one_word_lexical_chains(chains);

	if (only_strong) {
		remove_weak_lexical_chains(chains);
	}

	// print chains
	// print_lexical_chains(chains, sout);

	list<word_pos> wp_res;
	if (heuristic == L"FirstMostWeightedWord") {
		wp_res = first_most_weighted_word(sout, chains);
	} else if (heuristic == L"SumOfChainWeights") {
		wp_res = sum_of_chain_weights(sout, chains);
	} else {
		wp_res = first_word(sout, chains);
	}

	wp_res.sort();

	list<const sentence*> res = wp_to_sp(wp_res);

	return (res);
}
