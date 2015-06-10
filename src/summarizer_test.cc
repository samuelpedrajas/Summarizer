#include "freeling/morfo/analyzer.h"
#include "freeling/morfo/util.h"

#include "config.h"
#include "summarizer.h"

using namespace std;
using namespace freeling;

/// predeclarations
analyzer::config_options fill_config(const wstring &path);
analyzer::invoke_options fill_invoke();


//////////////   MAIN PROGRAM  /////////////////////

int main (int argc, char **argv) {

  //// set locale to an UTF8 compatible locale
  util::init_locale(L"default");

  /// read FreeLing installation path if given, use default otherwise
  wstring ipath;
  if (argc < 2) ipath = L"/usr/local";
  else ipath = util::string2wstring(argv[1]);

  /// set config options (which modules to create, with which configuration)
  analyzer::config_options cfg = fill_config(ipath + L"/share/freeling/");
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
  while (getline(wcin, line))
    text = text + line + L"\n";

  /// analyze text, leave result in doc
  document doc;
  anlz.analyze(text, doc, true);

  //sens->analyze(doc);
  //dsb->analyze(doc);


  for (list<paragraph>::const_iterator it_p = doc.begin(); it_p != doc.end(); it_p++) {
    int j = 0;
    for (list<sentence>::const_iterator it_s = it_p->begin(); it_s != it_p->end(); it_s++) {
      int k = 0;
      for (list<word>::const_iterator it_w = it_s->begin(); it_w != it_s->end(); it_w++) {
        wcout << it_w->get_form() << L" -> " << it_w->get_tag() << endl;
      }
    }
  }


  summarizer sum(L"/home/samuel/Summarizer/src/summarizer.dat");
  list<const sentence*> selected_sentences = sum.summarize(wcout, doc);

  for (list<const sentence*>::const_iterator it = selected_sentences.begin();
       it != selected_sentences.end(); it++) {
    const sentence * s = *it;
    sentence::const_iterator b_it = s->begin();
    sentence::const_iterator e_it = --s->end();
    unsigned long span_start = b_it->get_span_start();
    unsigned long span_finish = e_it->get_span_finish();

    for (int i = span_start; i < span_finish; i++) {
      wcout << text[i];
    }

    wcout << endl;
  }
}


///////////////////////////////////////////////////
/// Load an ad-hoc set of configuration options

analyzer::config_options fill_config(const wstring &path) {

  analyzer::config_options cfg;

  /// Language of text to process
  cfg.Lang = L"en";

  // path to language specific data
  wstring lpath = path + L"/" + cfg.Lang + L"/";

  /// Tokenizer configuration file
  cfg.TOK_TokenizerFile = lpath + L"tokenizer.dat";
  /// Splitter configuration file
  cfg.SPLIT_SplitterFile = lpath + L"splitter.dat";
  /// Morphological analyzer options
  cfg.MACO_Decimal = L".";
  cfg.MACO_Thousand = L",";
  cfg.MACO_LocutionsFile = lpath + L"locucions.dat";
  cfg.MACO_QuantitiesFile = lpath + L"quantities.dat";
  cfg.MACO_AffixFile = lpath + L"afixos.dat";
  cfg.MACO_ProbabilityFile = lpath + L"probabilitats.dat";
  cfg.MACO_DictionaryFile = lpath + L"dicc.src";
  cfg.MACO_NPDataFile = lpath + L"np.dat";
  cfg.MACO_PunctuationFile = path + L"common/punct.dat";
  cfg.MACO_ProbabilityThreshold = 0.001;

  /// NEC config file
  cfg.NEC_NECFile = lpath + L"nerc/nec/nec-ab-poor1.dat";
  /// Sense annotator and WSD config files
  cfg.SENSE_ConfigFile = lpath + L"senses.dat";
  cfg.UKB_ConfigFile = lpath + L"ukb.dat";
  /// Tagger options
  cfg.TAGGER_HMMFile = lpath + L"tagger.dat";
  cfg.TAGGER_ForceSelect = RETOK;
  /// Chart parser config file
  cfg.PARSER_GrammarFile = lpath + L"chunker/grammar-chunk.dat";
  /// Dependency parsers config files
  cfg.DEP_TxalaFile = lpath + L"dep_txala/dependences.dat";
  cfg.DEP_TreelerFile = lpath + L"dep_treeler/labeled/dependences.dat";
  /// Coreference resolution config file
  cfg.COREF_CorefFile = lpath + L"coref/relaxcor/relaxcor.dat";

  return cfg;
}


///////////////////////////////////////////////////
/// Load an ad-hoc set of invoke options

analyzer::invoke_options fill_invoke() {

  analyzer::invoke_options ivk;

  /// Level of analysis in input and output
  ivk.InputLevel = TEXT;
  ivk.OutputLevel = COREF;

  /// activate/deactivate morphological analyzer modules
  ivk.MACO_UserMap = false;
  ivk.MACO_AffixAnalysis = true;
  ivk.MACO_MultiwordsDetection = true;
  ivk.MACO_NumbersDetection = true;
  ivk.MACO_PunctuationDetection = true;
  ivk.MACO_DatesDetection = true;
  ivk.MACO_QuantitiesDetection  = true;
  ivk.MACO_DictionarySearch = true;
  ivk.MACO_ProbabilityAssignment = true;
  ivk.MACO_CompoundAnalysis = false;
  ivk.MACO_NERecognition = true;
  ivk.MACO_RetokContractions = false;

  ivk.PHON_Phonetics = false;
  ivk.NEC_NEClassification = true;

  ivk.SENSE_WSD_which = UKB;
  ivk.TAGGER_which = HMM;
  ivk.DEP_which = TREELER;

  return ivk;
}
