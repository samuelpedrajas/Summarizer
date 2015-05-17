#include "config.h"
#include "freeling/output/output_freeling.h"
#include "freeling/morfo/configfile.h"
#include "freeling/morfo/traces.h"
#include "freeling/morfo/util.h"
#include "freeling/morfo/analyzer.h"

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
  cfg.TAGGER_ForceSelect=RETOK;
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