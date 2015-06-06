#include "freeling/morfo/analyzer.h"
#include "freeling/morfo/util.h"
#include "freeling/output/input_conll.h"
#include "freeling/output/output_conll.h"

#include "config.h"

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

  // crear output_conll
  output_conll out(L"./output.cfg");

  /// load document to analyze
  wstring text = L"";  
  wstring line;

  while (getline(wcin,line) and  line!=L"<<<END-OF-TEXT>>>") {
    text = text + line + L"\n";
  }
  
  /// load document to analyze
  wstring an_text = L"";  
  while (getline(wcin,line)) 
    an_text = an_text + line + L"\n";

  wcout << L"---- original text ----" << endl;
  wcout << text;
  wcout << L"----end original text ---" << endl;

  wcout << L"---- conll annotations ----" << endl;
  wcout << an_text;
  wcout << L"---- end conll annotations ----" << endl;

  input_conll ip;

  document doc;
  ip.input_document(an_text,doc,true);
  
  wcout << L"---- loaded conll annotations ----" << endl;
  out.PrintResults(wcout,doc);
  wcout << L"---- end loaded conll annotations ----" << endl;

  wcout << L"---- sentences from original spans ----" << endl;
  for (list<paragraph>::const_iterator p = doc.begin(); p != doc.end(); p++) {
    for (list<sentence>::const_iterator s = p->begin(); s != p->end(); s++) {
      sentence::const_iterator b= s->begin();
      sentence::const_iterator e= --s->end();
      wcout << L"SPANS: " << b->get_span_start() << L" - " << e->get_span_finish() << endl;
      wcout << L"<sentence>";
      for (int i = b->get_span_start(); i < e->get_span_finish(); i++) {
        wcout << text[i];
      }
      wcout << L"</sentence>" << endl;
    }
  }
  wcout << L"---- end sentences from original spans ----" << endl;

}
