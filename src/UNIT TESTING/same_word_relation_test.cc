#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Fixtures
#include <boost/test/unit_test.hpp>

#include "../lexical_chain.h"
#include "../analyzer_config_options.cc"

using namespace std;
using namespace freeling;

struct SameWordFixture {
	lexical_chain * lc;
	document * doc;

	SameWordFixture () {
		wstring freeling_path = L"/usr/local/share/freeling/";


		/// set config options (which modules to create, with which configuration)
		analyzer::config_options cfg = fill_config(freeling_path);
		/// create analyzer
		analyzer anlz(cfg);

		senses * sens = new senses(cfg.SENSE_ConfigFile);
		ukb * dsb = new ukb(cfg.UKB_ConfigFile);

		/// set invoke options (which modules to use)
		analyzer::invoke_options ivk = fill_invoke();
		/// load invoke options into analyzer
		anlz.set_current_invoke_options(ivk);

		/// load document to analyze
		wstring text = L"This is a test.\
						This test consist in many sentences containing the word test many times.\
						The word 'word' also appears many times.";

		/// analyze text, leave result in doc
		anlz.analyze(text,*doc,true);
	}

	~SameWordFixture () {
	}

};

BOOST_FIXTURE_TEST_SUITE(SameWordTestSuite, SameWordFixture)
 
BOOST_AUTO_TEST_CASE(specialTheory)
{
	BOOST_CHECK_EQUAL(4,4);
}
 
BOOST_AUTO_TEST_SUITE_END()