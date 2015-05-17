#include "summarizer.h"
#include "analyzer_config_options.cc"

using namespace std;
using namespace freeling;


//////////////   MAIN PROGRAM  /////////////////////

int main (int argc, char **argv) {

  //// set locale to an UTF8 compatible locale 
  util::init_locale(L"default");

  /// read FreeLing installation path if given, use default otherwise
  wstring ipath;
  if (argc < 2) ipath = L"/usr/local";
  else ipath = util::string2wstring(argv[1]);

  /// set config options (which modules to create, with which configuration)
  analyzer::config_options cfg = fill_config(ipath+L"/share/freeling/");
  /// create analyzer
  analyzer anlz(cfg);

  senses * sens = new senses(cfg.SENSE_ConfigFile);
  ukb * dsb = new ukb(cfg.UKB_ConfigFile);

  /// set invoke options (which modules to use)
  analyzer::invoke_options ivk = fill_invoke();
  /// load invoke options into analyzer
  anlz.set_current_invoke_options(ivk);

  /// load document to analyze
  wstring text;  
  wstring line;
  while (getline(wcin,line))
    text = text + line + L"\n";

  /// analyze text, leave result in doc
  document doc;
  anlz.analyze(text,doc,true);
  
  for (list<paragraph>::const_iterator it_p = doc.begin(); it_p != doc.end(); it_p++) {
    int j = 0;
    for (list<sentence>::const_iterator it_s = it_p->begin(); it_s != it_p->end(); it_s++) {
      int k = 0;
      for (list<word>::const_iterator it_w = it_s->begin(); it_w != it_s->end(); it_w++) {
        wcout << it_w->get_form() << L" -> " << it_w->get_tag() << endl; 
      }
    }
  }
  
  
  summarizer sum(L"./summarizer.dat");
  list<word_pos> selected_sentences = sum.summarize(wcout, doc);
  for (list<word_pos>::const_iterator it = selected_sentences.begin(); it != selected_sentences.end(); it++) {
    const sentence & s = it->s;
    for (sentence::const_iterator s_it = s.words_begin(); s_it != s.words_end(); s_it++) {
      wcout << s_it->get_form() << L" ";
    }
    wcout << endl;
  }
}