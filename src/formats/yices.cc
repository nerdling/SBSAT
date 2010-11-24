/***********************************************************************
 *  yices.cc (J. Lavergne)
 *  Function yices reads yices language input to a SMURF
 ***********************************************************************/

#include "sbsat.h"
#include "sbsat_formats.h"

#include <iostream>
#include <fstream>
#include <vector>
#include "yices_lexer.hpp"

using namespace std;
using namespace boost::spirit;
using namespace boost::spirit::repository;

namespace {

    struct something {
        something(ostream& out)
            : preprocessor("preprocessor", out)
            , comment("comment", out)
            , keyword("keyword", out)
            , identifier("identifier", out)
            , special("special", out)
            , string("string", out)
            , literal("literal", out)
            , number("number", out)
            , unexpected(out) {}

        process preprocessor, comment, keyword, identifier,
                special, string, literal, number;

        unexpected_char unexpected;
    };
}

static int
parse(istream& in,
      ostream& out,
      string const& title,
      string const& css) {
    in.unsetf(ios::skipws); // Turn off whitespace skipping on the stream

    vector<char> vec;
    std::copy(
        istream_iterator<char>(in),
        istream_iterator<char>(),
        std:back_inserter(vec));

    vector<char>::const_iterator first = vec.begin();
    vector<char>::const_iterator last = vec.end();

    // something
    something actions(out);
    yices_lexer<something> p(actions);
    parse_info<vector<char>::const_iterator> info = parse(first, last, p);

    if (!info.full) {
        cerr << "parsing error\n"
        cerr << string(info.stop, last);
        return -1;
    }

    return 0;
}

void yices_process(Clause *pClauses) {
    if(feof(finputfile)) return ERR_IO_READ;

    if (int i = fscanf(finputfile, "%d %d\n", &numYICESVariables, &nNumClauses) != 2) {
        std::cerr << "Error while parsing Yices input: bad header "
                  << i << " " << nNumYICESVariables << " " << nNumClauses
                  << std::endl;
        exit(1);
    }

    d9_printf3(" yices %d %d\n", nNumYICESVariables, nNumClauses);

    int nOrigNumClauses = nNumClauses;

    Clause *pClauses = (Clause*)yices_calloc(nNumClauses, sizeof(Clause), 9, "read_yices::pClauses");
    int tempint_max = 100;
    int *tempint = (int *)yices_callc(tempint_max, sizeof(int *), 9, "read_yices()::tempint");

    // Get and store the Yices clauses
    int print_variable_warning = 1;
    int x = 0;
    while (1) {
    }

    d2_printf3("\rReading Yices %d/%d            \n", nNumClauses, nNumClauses);

    yices_process(pClauses);

    yices_free((void**)&tempint); tempint_max = 0;

    d3_printf2("Number of Deaths - %d\n", nmbrFunctions);

    free_clauses(pClauses);

    return NO_ERROR;
}
