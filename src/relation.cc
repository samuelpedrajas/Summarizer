#include "relation.h"

using namespace freeling;
using namespace std;

word_pos::word_pos(const word &w_p, const sentence &s_p, int n_paragraph, int n_sentence, int position) : w(w_p), s(s_p) {
	this->n_paragraph = n_paragraph;
	this->n_sentence = n_sentence;
	this->position = position;
}

bool word_pos::operator==(word_pos other) const {
    return n_paragraph == other.n_paragraph && n_sentence == other.n_sentence && position == other.position;
}

bool word_pos::operator<(word_pos other) const {
	if (n_sentence < other.n_sentence) return true;
    return n_sentence == other.n_sentence && position < other.position;
}

bool word_pos::operator>(word_pos other) const {
	if (n_sentence > other.n_sentence) return true;
    return n_sentence == other.n_sentence && position > other.position;
}

wstring word_pos::toString() const {
	wstring res = w.get_form() + L" [Paragraph " + to_wstring(n_paragraph) + L", Sentence " + to_wstring(n_sentence) + L", Position " + to_wstring(position) + L"]";
	return res;
}

related_words::related_words(const word_pos &w_p1, const word_pos &w_p2, double relatedness) : w1(w_p1), w2(w_p2) {
	this->relatedness = relatedness;
}

wstring related_words::toString() const {
	wstring res = w1.toString() + L" <-> " + w2.toString() + L" (Relatedness = " + to_wstring(relatedness) + L")";
	return res;
}

relation::relation(const wstring s, const wstring t) : label(s), compatible_tag(t) {

}

int relation::max_distance = 0;

bool relation::is_compatible(const word &w) const {
	return compatible_tag.search(w.get_tag());
}

bool relation::compute_word (const word &w, const sentence &s, const document &doc,
	int n_paragraph, int n_sentence, int position, list<word_pos> &words,
	list<related_words> &relations, unordered_map<wstring, pair<int, word_pos*> > &unique_words) const {

}

SameWord::SameWord(wostream &sout) : relation(L"Same Word", L"^(NP|VB|NN)") {
	this->sout = &sout;
}

bool SameWord::compute_word (const word &w, const sentence &s, const document &doc,
							 int n_paragraph, int n_sentence, int position, list<word_pos> &words,
list<related_words> &relations, unordered_map<wstring, pair<int, word_pos*> > &unique_words) const {
	bool found = false;
	word_pos * wp;

	if (words.size() > 0 && is_compatible(w)) {
		for (list<word_pos>::const_iterator it_w = words.begin(); it_w != words.end(); it_w++) {
			if (words.begin()->w.get_lc_form() == w.get_lc_form() && (n_sentence - it_w->n_sentence) <= max_distance) {
				if (!found) {
					wp = new word_pos(w, s, n_paragraph, n_sentence, position);
					found = true;
				}
				related_words rel_w(*wp, *it_w, 1);
				relations.push_back(rel_w);
			}
		}
	}
	if (found) {
		words.push_back(*wp);
		wstring form = w.get_lc_form();
		unordered_map<wstring, pair<int, word_pos*> >::iterator it_uw = unique_words.find(form);
		if (it_uw == unique_words.end()) unique_words[form] = pair<int, word_pos*>(1, wp);
		else (it_uw->second).first++;	}
	return found;
}

double SameWord::get_homogeneity_index(const list<word_pos> &words, const list<related_words> &relations,
									   const unordered_map<wstring, pair<int, word_pos*> > &unique_words) {
	return (1.0 - 1.0/(double)words.size());
}


list<word_pos> SameWord::order_words_by_weight(const unordered_map<wstring, pair<int, word_pos*> > &unique_words) const {
	list<word_pos> res;
	for (unordered_map<wstring, pair<int, word_pos*> >::const_iterator it = unique_words.begin();
			it != unique_words.end(); it++) {
		*sout << L"first : " << it->first << L"  " << it->second.first << endl;
		*sout << L"second : " << it->second.second->w.get_form() << endl;
		res.push_back(*(it->second).second);
	}
	*sout << L"WO? " << res.begin()->w.get_form() << endl;
	return res;
}

Hypernymy::Hypernymy(int k, double alpha, const wstring &semfile, wostream &sout) : relation(L"Hypernymy", L"^(VB|NN)") {
	if (semdb==NULL) semdb = new semanticDB(semfile);
	depth = k;
	this->alpha = alpha;
	this->sout = &sout;
}

semanticDB * Hypernymy::semdb = NULL;
int Hypernymy::depth = 0;
double Hypernymy::alpha = 0.9;

const word_pos &Hypernymy::count_relations(int n, const list<related_words> &relations) const {
	unordered_map<wstring, int> wp_count;
	const word_pos * max_wp;
	int max_freq = 0;
	for (list<related_words>::const_iterator it = relations.begin(); it != relations.end(); it++) {
		wstring key_1 = to_wstring(it->w1.n_paragraph) + L":" + to_wstring(it->w1.n_sentence) + L":" + to_wstring(it->w1.position);
		wstring key_2 = to_wstring(it->w2.n_paragraph) + L":" + to_wstring(it->w2.n_sentence) + L":" + to_wstring(it->w2.position);
		int freq1;
		int freq2;

		unordered_map<wstring, int>::iterator word_int2 = wp_count.find(key_2);
		if (word_int2 == wp_count.end()) {
			word_int2 = wp_count.insert(pair<wstring, int>(key_2, 1)).first;
			freq2 = 1;
		} else {
			word_int2->second++;
			freq2 = word_int2->second;
		}
		unordered_map<wstring, int>::iterator word_int1 = wp_count.find(key_1);
		if (word_int1 == wp_count.end()) {
			word_int1 = wp_count.insert(pair<wstring, int>(key_1, 1)).first;
			freq1 = 1;
		} else {
			word_int1->second++;
			freq1 = word_int1->second;
		}
		if (freq1 > max_freq && freq1 > freq2) {
			max_freq = freq1;
			max_wp = &(it->w1);
		} else if (freq2 > max_freq) {
			max_freq = freq2;
			max_wp = &(it->w2);
		}
	}
	return *max_wp;
}

double Hypernymy::get_homogeneity_index(const list<word_pos> &words, const list<related_words> &relations,
										const unordered_map<wstring, pair<int, word_pos*> > &unique_words) {
	int n = words.size();
	const word_pos &wp_core = count_relations(n, relations);

	vector<int> num_words_dist(depth+1, 0);
	num_words_dist[0] = 1;
	for (list<related_words>::const_iterator it = relations.begin(); it != relations.end(); it++) {
		if (it->w1 == wp_core || it->w2 == wp_core) {
			num_words_dist[it->relatedness]++;
		}
	}

	double res = 0;
	for (int i = 0; i < depth + 1; i++) {
		res += alpha * ((double)num_words_dist[i] / n);
		alpha *= 0.9;
		(*this->sout) << L"DISTANCIA " << i << L": " << num_words_dist[i] << endl;
	}

	return res;
}

bool order_by_score (const pair<int, word_pos*> &p1, const pair<int, word_pos*> &p2)
{
  return (p1.first >= p2.first);
}

list<word_pos> Hypernymy::order_words_by_weight(const unordered_map<wstring, pair<int, word_pos*> > &unique_words) const {
	list<pair<int, word_pos*> > lst_to_order;
	for (unordered_map<wstring, pair<int, word_pos*> >::const_iterator it = unique_words.begin();
			it != unique_words.end(); it++) {
		lst_to_order.push_back(it->second);
	} 
	lst_to_order.sort(order_by_score);
	list<word_pos> res;
	for (list<pair<int, word_pos*> >::const_iterator it = lst_to_order.begin(); it != lst_to_order.end(); it++) {
		res.push_back(*it->second);
	}
	return res;
}

int Hypernymy::hypernymyAux(wstring s1, wstring s2, int k) const {
	if (s1 == s2) {
		return k;
	}
	if (k < depth) {
		sense_info si = semdb->get_sense_info(s1);
		list<wstring> & parents = si.parents;
		for (list<wstring>::const_iterator it = parents.begin(); it != parents.end(); it++) {
			int k_ret = hypernymyAux(*it, s2, ++k);
			if (k_ret != -1) return k_ret;
		}
	}
	return -1;
}

bool Hypernymy::compute_word (const word &w, const sentence &s, const document &doc,
							  int n_paragraph, int n_sentence, int position, list<word_pos> &words,
							  list<related_words> &relations, unordered_map<wstring, pair<int, word_pos*> > &unique_words) const {

	bool inserted = FALSE;
	if (is_compatible(w)) {
		word_pos * wp;

		for (list<word_pos>::const_iterator it_w = words.begin(); it_w != words.end(); it_w++) {
			const word &w2 = it_w->w;

			if ((n_sentence - it_w->n_sentence) <= max_distance) {
				const list<pair<wstring,double>> & ss1 = w.get_senses();
				const list<pair<wstring,double>> & ss2 = w2.get_senses();
				if (ss1.empty() || ss2.empty()) return FALSE;
				wstring s1 = ss1.begin()->first;
				wstring s2 = ss2.begin()->first;
				int lvl = hypernymyAux(s1, s2, 0);
				if (lvl < 0) lvl = hypernymyAux(s2, s1, 0);
				if (lvl >= 0) {
					if (!inserted) {
						inserted = TRUE;
						wp = new word_pos(w, s, n_paragraph, n_sentence, position);
					}
					related_words rel_w(*wp, *it_w, lvl);
					relations.push_back(rel_w);
				}
			}
		}
		if (inserted) {	
			words.push_back(*wp);
			wstring form = w.get_lc_form();
			unordered_map<wstring, pair<int, word_pos*> >::iterator it_uw = unique_words.find(form);
			if (it_uw == unique_words.end()) unique_words[form] = pair<int, word_pos*>(1, wp);
			else (it_uw->second).first++;
		}
	}
	return inserted;
}

SameCorefGroup::SameCorefGroup(wostream &sout) : relation(L"Same Coreference Group", L"^(NP|NN|PRP|Z)") {
	this->sout = &sout;
}

double SameCorefGroup::get_homogeneity_index(const list<word_pos> &words, const list<related_words> &relations,
											 const unordered_map<wstring, pair<int, word_pos*> > &unique_words) {
	double hi = 0;
	bool prp_found = false;
	bool np_found = false;
	regexp re_prp(L"^(PRP|Z)");
	regexp re_np(L"^NP");
	regexp re_nn(L"^NN");
	for (list<word_pos>::const_iterator it = words.begin(); it != words.end(); it++) {
		if (!prp_found && re_prp.search((it->w).get_tag())) {
			hi++;
			prp_found = true;
		} else if (!np_found && re_np.search((it->w).get_tag())) {
			hi++;
			np_found = true;
		} else if (re_nn.search((it->w).get_tag())) {
			hi++;
		}
	}
	return (1.0 - (double) (hi/words.size()));
}

bool order_by_tag_and_score (const pair<int, word_pos*> &p1, const pair<int, word_pos*> &p2)
{
	wstring tag1 = p1.second->w.get_tag();
	wstring tag2 = p2.second->w.get_tag();
	regexp re_np(L"^NP");
	regexp re_nn(L"^NN");
	bool aux = re_np.search(tag2);
	if (re_np.search(tag1) && !aux) return true;
	if (aux) return false;
	aux = re_nn.search(tag2);
	if (re_nn.search(tag1) && !aux) return true;
	if (aux) return false;
	return (p1.first >= p2.first);
}

list<word_pos> SameCorefGroup::order_words_by_weight(const unordered_map<wstring, pair<int, word_pos*> > &unique_words) const {
	list<pair<int, word_pos*> > lst_to_order;
	for (unordered_map<wstring, pair<int, word_pos*> >::const_iterator it = unique_words.begin();
			it != unique_words.end(); it++) {
		lst_to_order.push_back(it->second);
	} 
	lst_to_order.sort(order_by_tag_and_score);
	list<word_pos> res;
	for (list<pair<int, word_pos*> >::const_iterator it = lst_to_order.begin(); it != lst_to_order.end(); it++) {
		res.push_back(*it->second);
		*sout << it->second->w.get_form() << L" ";
	}
	*sout << endl;
	return res;
}

bool SameCorefGroup::compute_word (const word &w, const sentence &s, const document &doc,
								   int n_paragraph, int n_sentence, int position, list<word_pos> &words,
								   list<related_words> &relations, unordered_map<wstring, pair<int, word_pos*> > &unique_words) const {

	if (words.size() > 0 && is_compatible(w)) {
		const word_pos &wp2 = *(words.begin());
		int group_w1 = -1;
		int group_w2 = -1;
		const list<int> &grps = doc.get_groups();
		for(list<int>::const_iterator it = grps.begin(); it != grps.end(); it++) {
			list<int> coref_id_mentions = doc.get_coref_id_mentions(*it);
			bool ffound = FALSE;
			bool sfound = FALSE;
			for(list<int>::const_iterator it_cid = coref_id_mentions.begin(); it_cid != coref_id_mentions.end(); it_cid++) {
				const mention &m = doc.get_mention(*it_cid);
				int s_mention = m.get_n_sentence();
				int pos_mention = m.get_head().get_position();
				if (s_mention == n_sentence && pos_mention == position) ffound = TRUE; // TODO: No comparar las dos posiciones de memoria
				if (s_mention == wp2.n_sentence && pos_mention == wp2.position) sfound = TRUE;
				if(ffound && sfound) {
					word_pos * wp;
					wp = new word_pos(w, s, n_paragraph, n_sentence, position);
					for (list<word_pos>::const_iterator it_w = words.begin(); it_w != words.end(); it_w++) {
						related_words rel_w(*wp, *it_w, 1);
						relations.push_back(rel_w);
					}
					words.push_back(*wp);
					wstring form = w.get_lc_form();
					unordered_map<wstring, pair<int, word_pos*> >::iterator it_uw = unique_words.find(form);
					if (it_uw == unique_words.end()) unique_words[form] = pair<int, word_pos*>(1, wp);
					else (it_uw->second).first++;

					return TRUE;
				}
			}
		}
	}
	return FALSE;
}