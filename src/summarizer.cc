#include "summarizer.h"

using namespace freeling;
using namespace std;

summarizer::summarizer(const wstring &datFile) {
	config_file cfg; 
	enum sections {GENERAL, RELATIONS};
    cfg.add_section(L"General",GENERAL);
    cfg.add_section(L"Relations",RELATIONS);
    if (not cfg.open(datFile))
      ERROR_CRASH(L"Error opening file "+datFile);

    wstring line; 
    while (cfg.get_content_line(line)) {

      switch (cfg.get_section()) {
  
      case GENERAL: {
        wistringstream sin;
        sin.str(line);
        wstring key, value;
        sin >> key >> value; 
        if (key == L"RemoveUsedChains") {
        	remove_used_lexical_chains = (value == L"true");
        } else if (key == L"OnlyStrong") {
        	only_strong = (value == L"true" or value[0]==L'y' or value[0]==L'Y' or value==L"1");
        } else if (key == L"NumWords") {
        	num_words = stoi(value);
        } else if (key == L"SemDBPath") {
        	semdb_path = value;
        }
        break;
      }

      case RELATIONS: {
      	wistringstream sin;
        sin.str(line);
        wstring elem;
        sin >> elem;
        used_tags.insert(elem);
      	break;
      }

      default: break;
      }
    }
    cfg.close(); 

    TRACE(1,L"Module sucessfully loaded");
}

summarizer::~summarizer() {
	
}

// map should be const
double summarizer::average_scores(map<wstring, list<lexical_chain> > &chains) const {
	double avg = 0;
	int size = 0;
	for (set<wstring>::const_iterator it_tag = used_tags.begin(); it_tag != used_tags.end(); it_tag++) {
		list<lexical_chain> &lexical_chains = chains[*(it_tag)];
		size += lexical_chains.size();
		for (list<lexical_chain>::iterator it = lexical_chains.begin(); it != lexical_chains.end(); it++) {
			avg += it->get_score();
		}
	}
	return (double) avg/size;
}

// map should be const
double summarizer::standard_deviation_scores(map<wstring, list<lexical_chain> > &chains, const double avg) const {
	double sd = 0;
	int size = 0;
	for (set<wstring>::const_iterator it_tag = used_tags.begin(); it_tag != used_tags.end(); it_tag++) {
		list<lexical_chain> &lexical_chains = chains[*(it_tag)];
		size += lexical_chains.size();
		for (list<lexical_chain>::iterator it = lexical_chains.begin(); it != lexical_chains.end(); it++) {
			sd += (double) pow(it->get_score() - avg, 2);
		}
	}
	return sqrt(sd/size);
}

list<lexical_chain> summarizer::map_to_lists(map<wstring, list<lexical_chain> > &chains) const {
	list<lexical_chain> spliced_lists;
	for (set<wstring>::const_iterator it_tag = used_tags.begin(); it_tag != used_tags.end(); it_tag++) {
		list<lexical_chain> &lexical_chains = chains[*(it_tag)];
		spliced_lists.splice(spliced_lists.end(), lexical_chains);
	}
	return spliced_lists;
}

bool compare_lexical_chains (lexical_chain &first, lexical_chain &second)
{
  return (first.get_score() >= second.get_score());
}

list<word_pos> summarizer::first_word(wostream &sout, map<wstring, list<lexical_chain> > &chains) const {
	list<lexical_chain> lexical_chains = map_to_lists(chains);
	lexical_chains.sort(compare_lexical_chains);
	set<const sentence*> sent_set;
	list<word_pos> wp_list;
	int acc_n_words = 0;
	for (list<lexical_chain>::const_iterator it = lexical_chains.begin(); it != lexical_chains.end(); it++) {
		const list<word_pos> * wps = it->get_words();
		list<word_pos>::const_iterator end_wps = wps->end();
		if (remove_used_lexical_chains) {
			end_wps = wps->begin();
			end_wps++;
		}
		for(list<word_pos>::const_iterator it_wp = wps->begin(); it_wp != end_wps; it_wp++) {

			const word_pos wp = *it_wp;

			const sentence & s = wp.s;
			int s_size = s.get_words().size();

			if (s_size + acc_n_words <= num_words) {
				bool inserted = sent_set.insert(&s).second;
				if (inserted) {
					acc_n_words += s_size;
					wp_list.push_back(wp);
				}
			}
		}
	}
	return wp_list;
}

relation * summarizer::tag_to_rel(const wstring ws, wostream &sout) const { 
	relation * rel;
	if(ws == L"SW") { 
		rel = new SameWord(sout);
	} else if (ws == L"HN") {
		rel = new Hypernymy(2, semdb_path, sout);
	} else if (ws == L"SCG") {
		rel = new SameCorefGroup(sout);
	}
	return rel;
}

bool compare_word_pos (const word_pos& first, const word_pos& second) {
  if (first.n_paragraph < second.n_paragraph) return TRUE;
  else if (first.n_paragraph > second.n_paragraph) return FALSE;
  return first.n_sentence < second.n_sentence;
}

map<wstring, list<lexical_chain>> summarizer::build_lexical_chains(wostream &sout, const document &doc) {
	map<wstring, list<lexical_chain>> chains;
	int i = 0;
	for (set<wstring>::const_iterator it_t = used_tags.begin(); it_t != used_tags.end(); it_t++) {
		wstring tag = *it_t;
		relation * rel = tag_to_rel(tag, sout);
		for (list<paragraph>::const_iterator it_p = doc.begin(); it_p != doc.end(); it_p++) {
			int j = 0;
			for (list<sentence>::const_iterator it_s = it_p->begin(); it_s != it_p->end(); it_s++) {
				int k = 0;
				for (list<word>::const_iterator it_w = it_s->begin(); it_w != it_s->end(); it_w++) {
					list<lexical_chain> &lc = chains[tag];
					bool inserted = FALSE;
					for (list<lexical_chain>::iterator it_lc = lc.begin(); it_lc != lc.end(); it_lc++) {
						inserted = inserted || it_lc->compute_word((*it_w), (*it_s), doc, i, j, k, sout);
					}

					if (!inserted) {
						lexical_chain new_lc(rel, (*it_w), (*it_s), i, j, k);
						lc.push_back(new_lc);
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
			if(it->get_number_of_words() == 1) {
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
			if(it->get_score() <= (avg + 2 * sd)) {
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

list<word_pos> summarizer::summarize(wostream &sout, const document &doc) {
	// building lexical chains
	map<wstring, list<lexical_chain>> chains = build_lexical_chains(sout, doc);

	// remove one word lexical chains
	remove_one_word_lexical_chains(chains);

	if (only_strong) {
		remove_weak_lexical_chains(chains);
	}

	// print chains
	print_lexical_chains(chains, sout);

	list<word_pos> res = first_word(sout, chains);
	res.sort(compare_word_pos);
	return(res);
}
