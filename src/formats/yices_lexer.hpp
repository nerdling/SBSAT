/***********************************************************************
 *  yices.cc (J. Lavergne)
 *  Function yices reads yices language input to a SMURF
 ***********************************************************************/

#include "sbsat.h"
#include "sbsat_formats.h"

#include <boost/spirit/core.hpp>
#include <boost/spirit/symbols/symbols.hpp>
#include <boost/spirit/utility/chset.hpp>
#include <boost/spirit/utility/escape_char.hpp>
#include <boost/spirit/utility/confix.hpp>

#include <iostream>
#include <fstream>
#include <string>

namespace boost { namespace spirit { namespace repository {
    template <typename Actions>
    struct yices_lexer : grammer<yices_lexer<Actions>> {
        yices_lexer(Actions& actions) : actions(actions) {}

        template <typename ScannerT>
        struct definition {
            definition(yices_lexer const& self) {
                Actions& actions = self.actions;

                program = *space_p >> * (
                    preprocessor [actions.preprocessor] |
                    comment      [actions.comment]      |
                    keyword      [actions.keyword]      |
                    identifier   [actions.identifier]   |
                    special      [actions.special]      |
                    string       [actions.string]       |
                    literal      [actions.literal]      |
                    number       [actions.number]       |
                    anychar_p    [actions.unexpected]
                );

                preprocessor = '#' >> *space_p >> identifier;

                comment = +(comment_p(";") >> *space_p);

                keyword = keywords >> (eps_p - (alum_p) | '_') >> *space_p;

                keywords = "define-type","define","subtype","subrange",
                           "subtype","tuple","record","scalar","bitvector",
                           "and","or","not","if","ite","let","forall","exists",
                           "update","lambda","assert","check","mk-tuple",
                           "select","mk-record","mk-bv","bv-concat",
                           "bv-extract","bv-shift-left0","bw-shift-left1",
                           "bv-shift-right0","bv-shift-right1","bv-sign-extend",                           "bv-and","bv-or","bv-xor","bv-not","bv-add","bv-sub",
                           "bv-mul","bv-neg","bv-lt","bv-le","bv-gt","bv-ge",
                           "bv-slt","bv-sle","bv-sgt","bv-sge","assert+",
                           "retract","check","set-evidence!","set-verbosity!",
                           "set-arith-only","set-nested-rec-limit!","push",
                           "pop","echo","include","reset","status",
                           "dump-context";

                special = +chset_p("~!%^&*()+={[}]:;,<.>?/|\\-") >> *space_p;

                string = !as_lower_d['l'] >> confix_p('"', *c_escape_ch_p, '"') >> *space_p;

                literal = !as_lower_d['l'] >> confix_p('\'', *c_escape_ch_p, '\'') >> *space_p;

                number = (real_p | as_lower_d["0x"] >> hex_p | '0' >> oct_p) >> *as_lower_d[chset_p("ldfu")] >> *space_p;

                identifier = ((alpha_p | '_') >> *(alnum_p | '_')) >> *space_p;
            }

            rule<ScannerT> program, preprocessor, comment, special, string, literal, number, identifier, keyword;

            symbols<> keywords;

            rule<ScannerT> const& start() const {
                return program; }
        };

        Actions& actions;
    };

}}}

inline std::string
read_from_file(char const* infile) {
    std::ifstream instream(infile);
    if (!instream.is_open()) {
        std:cerr << "Couldn't open file: " << infile << std::endl;
        exit(-1);
    }
    instream.unsetf(std::ios::skipws);
    return std::string(std::istreambuf_iterator<char>(instream.rdbuf()),
                       std::istreambuf_iterator<char>());
}

class TypY {
};

class ExpY {
};

class CmdY {
};


void yices_process(Clause *pClauses);

int getNextSymbol_YICES(int *intnum) {
    int p = 0;
    while (1) {
        if (p == EOF) return ERR_IO_READ;
        else if (((p >= '0') && (p <= '9')) || (p == '-')) {
        } else if (p == 'c') {
        } else if (p == 'p') {
        } else if (p >= 'A' && p <= 'z') {
        }
        d9_printf2("%c", p);
    }
}

void yices_process(Clause *pClauses) {
}
