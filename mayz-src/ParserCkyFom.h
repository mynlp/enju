/*
 * $Id: ParserCkyFom.h,v 1.12 2011-05-06 13:00:52 matuzaki Exp $
 *
 *  Copyright (c) 2005, Tsujii Laboratory, The University of Tokyo.
 *  All rights reserved.
 *
 */

#ifndef __ParserCkyFom_h
#define __ParserCkyFom_h

#include <limits>
#include <vector>
#include <liblilfes/machine.h>
#include <liblilfes/ftype.h>
#include <liblilfes/proc.h>
#include <liblilfes/structur.h>
#include "AmisModel.h"
#include "ParserBasic.h"

namespace up {
    
    class ParserCkyFom : public ParserBasic {
    protected:
        typedef ParserBasic base_type;

        struct LexEntState {
            unsigned word_id;
            lilfes::FSP lexent;
            double fom;
            unsigned state;

            LexEntState() {}
            LexEntState(unsigned word_id_, lilfes::FSP lexent_, double fom_, unsigned state_)
                : word_id(word_id_)
                , lexent(lexent_)
                , fom(fom_)
                , state(state_) {}
        };

        template<class ElemT>
        struct GreaterFom {
            GreaterFom(const std::vector<ElemT>& vec_) : vec(vec_) {}

            bool operator()(size_t serial1, size_t serial2) const
            {
                return vec[serial1].getFom() > vec[serial2].getFom();
            }

        private:
            const std::vector<ElemT> &vec;
        };

        struct GreaterLexEnt {
            bool operator()(const LexEntState& lex1, const LexEntState& lex2) const
            {
                return lex1.fom > lex2.fom;
            }
        };

        struct Beam {
            unsigned lexent_num;
            double   lexent_width;
            unsigned edge_num;
            double   edge_width;
            double   global_edge_width;

            void increment(const Beam& diff)
            {
                lexent_num   += diff.lexent_num;
                lexent_width += diff.lexent_width;
                edge_num     += diff.edge_num;
                edge_width   += diff.edge_width;
                global_edge_width += diff.global_edge_width;
            }

            bool exceed(const Beam& limit)
            {
                return lexent_num   > limit.lexent_num
                    || lexent_width > limit.lexent_width
                    || edge_num     > limit.edge_num
                    || edge_width   > limit.edge_width
                    || global_edge_width > limit.global_edge_width;
            }

            static Beam noBeam(void)
            {
                Beam b;
                b.lexent_num = 0;
                b.lexent_width = 0;
                b.edge_num = 0;
                b.edge_width = 0;
                b.global_edge_width = 0;

                return b;
            }
        };

        enum GlobalThresholdingType {
            GT_NONE, // disabled
            GT_LIGHT,    // global_thresholding_light
            GT_FULL      // global_thresholding
        };

    protected:

        static const unsigned default_chart_size = 150;
        
        std::vector<std::vector<std::vector<eserial> > > chart;
        std::vector<std::vector<bool> > alive;
        std::vector<std::vector<std::vector<LexEntState> > > lexent_chart;
        
        GlobalThresholdingType global_thresholding_type;
        unsigned current_iteration_number;
        bool reuse_iterative_parse;

        Beam beam_start;
        Beam beam_step;
        Beam beam_end;

    private:
        void globalThresholdingForward(std::vector<double>& f, int n)
        {
            f[0] = 0.0;
            for (int start = 0; start <= n-1; ++start) {
                double left = f[start];
                for (int j = start+1; j <= n; ++j) {
                    const std::vector<eserial>& sij = chart[start][j];
                    if (! sij.empty()) {
                        double score = left + edge_vec[sij.front()].getFom();
                        f[j] = std::max(f[j], score);
                    }
                }
            }
        }

        void globalThresholdingBackward(std::vector<double>& b, int n)
        {
            b[n] = 0.0;
            for (int start = n-1; start >= 0; --start) {
                for (int j = start+1 ; j <= n ; ++j) {
                    const std::vector<eserial>& sij = chart[start][j];
                    if (! sij.empty()) {
                        double right = b[j];
                        double score = right + edge_vec[sij.front()].getFom();
                        b[start] = std::max(b[start], score);
                    }
                }
            }
        }

    protected:
        void globalThresholding(int len, int n, double width)
        {
            P2("ParserCkyFom::globalThresholding");

            std::vector<double> f(n+1, -std::numeric_limits<double>::infinity());
            std::vector<double> b(n+1, -std::numeric_limits<double>::infinity());

            globalThresholdingForward(f, n);
            globalThresholdingBackward(b, n);
            double bestprob = f[n];
        
            for (int i = 0 ; i < n ; ++i) {
                for (int j = i+1; j <= i+len && j <= n ; ++j) {
                    const std::vector<eserial>& sij = chart[i][j];
                    std::vector<eserial>::const_iterator it = sij.begin();
                    std::vector<eserial>::const_iterator last = sij.end();
                    for (; it != last ; ++it) {
                        double left = f[i];
                        double right = b[j];
                        double total = left + edge_vec[*it].getFom() + right;
                        if (total < bestprob - width) {
                            break;
                        }
                    }
                    for (; it != last ; ++it) {
                        if (edge_vec[*it].getState() > current_iteration_number) break;
                        edge_vec[*it].setState(current_iteration_number + 1);
                    }
                }
            }
        }

        void globalThresholdingLight(int len, int n, double width)
        {
            P2("ParserCkyFom::globalThresholdingLight");

            double best = -std::numeric_limits<double>::infinity();
            for (int i = 0 ; i <= n - len; ++i) {
                int j = i + len;
                const std::vector<eserial>& sij = chart[i][j];
                if (! sij.empty()) {
                    double score = edge_vec[sij.front()].getFom();
                    best = std::max(best, score);
                }
            }
            for (int i = 0 ; i <= n - len; ++i) {
                int j = i + len;
                const std::vector<eserial>& sij = chart[i][j];
                std::vector<eserial>::const_iterator it = sij.begin();
                std::vector<eserial>::const_iterator last = sij.end();
                for (; it != last ; ++it) {
                    double score = base_type::edge_vec[*it].getFom();
                    if (score < best - width) {
                        break;
                    }
                }
                for (; it != last ; ++it) {
                    edge_vec[*it].setState(current_iteration_number + 1);
                }
            }
        }
        
        int thresholdOutEdges(std::vector<eserial>& sij, int num, double width)
        {
            P2("ParserCkyFom::thresholdOutEdges");
        
            int size = sij.size();
            if (size == 0) return 0;

            int num_pruned = 0;
            std::stable_sort(sij.begin(), sij.end(), GreaterFom<Edge>(edge_vec));

            double max_fom = edge_vec[sij.front()].getFom();
            if ( num > 0 ) {
                for (int k = 0; k < size - num; ++k) {
                    int target = size - num_pruned - 1;
                    edge_vec[sij[target]].setState( current_iteration_number + 1 );
                    ++num_pruned;
                }
            }

            if ( width > 0.0 ) {
                while (true) {
                    assert(sij.size() > 0);
                    int target = size - num_pruned - 1;
                    if (edge_vec[sij[target]].getFom() < max_fom - width) {
                        edge_vec[sij[target]].setState( current_iteration_number + 1 );
                        ++num_pruned;
                    }
                    else {
                        break;
                    }
                }
            }
            return num_pruned;
        }
        
        // Pre-condition: lexents is sorted in the descending order of fom
        int thresholdOutLexEnts(std::vector<LexEntState>& lexents, int num, double width)
        {
            P2("ParserCkyFom::thresholdOutLexEnt");
        
            int size = lexents.size();
            if (size == 0) return 0;

            int num_pruned = 0;

            if ( num > 0 ) {
                for (int k = 0; k < size - num; ++k) {
                    int target = size - num_pruned - 1;
                    lexents[target].state = current_iteration_number + 1;
                    ++num_pruned;
                }
            }

            if ( width > 0.0 ) {
                double limit = lexents.front().fom - width;
                for (int target = size - num_pruned - 1; target >= 0; --target) {
                    if (lexents[target].fom < limit) {
                        lexents[target].state = current_iteration_number + 1 ;
                        ++num_pruned;
                    }
                    else {
                        break;
                    }
                }
            }
            return num_pruned;
        }

        bool parseUnary(int i, int j, std::vector<eserial> &queue)
        {
            P2("ParserCkyFom::parseUnary");

            while (! queue.empty() ) {
                static std::vector<Fs*> signs;
                static std::vector<lserial> links;
                signs.clear();
                links.clear();
            
                grammar->applyIdSchemaUnary(queue.back(), signs, links, edge_vec, link_vec);
                queue.pop_back();
                storeEdge(i, j, signs, links, queue);
            }
            return true;
        }

        bool parseBinary(int i, int k, int j)
        {
            P2("ParserCkyFom::parseBinary");
        
            static std::vector<eserial> queue;
            static std::vector<Fs*> signs;
            static std::vector<lserial> links;

            lilfes::code *oldIP = mach->GetIP();
            lilfes::core_p TrailP = mach->SetTrailPoint(NULL);
        
                
            std::vector<eserial>& sik = chart[i][k];
            std::vector<eserial>& skj = chart[k][j];
                
            // copy signs in sik and skj
            std::vector<eserial>::iterator
                v = sik.begin(), vlast = sik.end(),
                w = skj.begin(), wlast = skj.end();
            std::vector<std::pair<eserial, lilfes::FSP> > xx, yy;
            
            for (; v != vlast ; ++v) {
                P2("ParserCkyFom parseIJ copy v");
                if (edge_vec[*v].getState() <= current_iteration_number) {
                    xx.push_back(std::make_pair(*v, lilfes::FSP(mach, edge_vec[*v].getSign())));
                }
                else {
                    break;
                }
            }
            for (; w != wlast ; w++) {
                P2("ParserCkyFom parseIJ copy w");
                if (edge_vec[*w].getState() <= current_iteration_number) {
                    yy.push_back(std::make_pair(*w, lilfes::FSP(mach, edge_vec[*w].getSign())));
                }
                else {
                    break;
                }
            }
            
            std::vector<std::pair<eserial, lilfes::FSP> >::iterator
                x = xx.begin(), xlast = xx.end(),
                y,              ylast = yy.end();

            for (; x != xlast ; x++) {

                eserial xesn = x->first;
                lilfes::FSP xfs = x->second;
                    
                for (y = yy.begin(); y != ylast ; y++) {
                    eserial yesn = y->first;
                    lilfes::FSP yfs = y->second;
                    {
                        P2("ParserCkyFom::parseXY");

                        if ((edge_vec[xesn].getState() != current_iteration_number) &&
                            (edge_vec[yesn].getState() != current_iteration_number)) continue;
        
                        queue.clear();
                        signs.clear();
                        links.clear();

                        grammar->applyIdSchemaBinary(xesn, yesn, xfs, yfs, signs, links,
                                                    edge_vec, link_vec, getSentenceLength());

                        if (! storeEdge(i, j, signs, links, queue)) {
                            return false; // TrailBack is not necessary? (2009-11-05 matuzaki)
                        }
                    }
                }
            }
        
            mach->TrailBack(TrailP);
            mach->SetIP(oldIP);
        
            return true;
        }
    
        bool storeEdge(int i, int j, std::vector<Fs*>& signs, std::vector<lserial>& links, std::vector<eserial>& queue)
        {
            P2("ParserCkyFom::StoreEdge");
            ASSERT(0 <= i);
            ASSERT(i < j);
            ASSERT(j < (getLimitSentenceLength() + 1));

            std::vector<lserial>::iterator it = links.begin();
            std::vector<lserial>::iterator last = links.end();
            std::vector<Fs*>::iterator it_sign = signs.begin();
    
            for (; it != last ; ++it, ++it_sign) {

                Fs* x = *it_sign;
        
                std::vector<eserial> &xxx = chart[i][j];
                std::vector<eserial>::iterator cell_it = xxx.begin();
                std::vector<eserial>::iterator cell_last = xxx.end();

                bool factored = false;

                // if factored, store edge_info
                for (; cell_it != cell_last ; ++cell_it) {
                    Edge& e = edge_vec[*cell_it];
                    Fs& y = e.getSign();
                    if ( (*x) == y ) {
                        e.addLink(*it);
                        eserial esn = e.getSerialNo();
                        link_vec[*it].setMyEdgeSerialNo(esn);
                        delete x;
                        factored = true;

                        // fom
                        double efom = edge_vec[esn].getFom();
                        double lfom = link_vec[*it].getFom();
                        if (lfom > efom) {
                            edge_vec[esn].setState( current_iteration_number ); // ver. 2.1.1
                            edge_vec[esn].setFom(lfom);
                        }
                        break;
                    }
                }

                // not factored
                if (! factored ) {

                    unsigned limit_edgenum = getLimitEdgeNumber();
                    if (limit_edgenum > 0 && edge_vec.size() >= limit_edgenum ) {
                        delete x;
                        parse_status = EDGE_LIMIT;
                        return false;
                    }

                    eserial esn = edge_vec.size();
                    edge_vec.push_back(Edge(esn, std::make_pair(i, j),
                                       *x, link_vec[*it].getFom(), current_iteration_number));
                    delete x;
                
                    edge_vec[esn].addLink(*it);
                    link_vec[*it].setMyEdgeSerialNo(esn);
                
                    chart[i][j].push_back(esn);
                    queue.push_back(esn);
                }
            }
            return true;
        }

    protected:
        void inhibitCrossBracketing()
        {
            unsigned n = getSentenceLength();

            for (Brackets::const_iterator it = brackets.begin(); it != brackets.end(); ++it) {
                // constitunts crossing the left boundary
                for (unsigned x = 0; x < it->begin; ++x) {
                    for (unsigned y = it->begin + 1; y < it->end; ++y) {
                        alive[x][y] = false;
                    }
                }
                // constitunts crossing the right boundary
                for (unsigned x = it->begin + 1; x < it->end; ++x) {
                    for (unsigned y = it->end + 1; y <= n ; ++y) {
                        alive[x][y] = false;
                    }
                }
            }
        }

        void inhibitLargeConstituents(int s, int n)
        {
            if (s <= 0) {
                return;
            }

            for (int i = 1; i < n - s; ++i) {
                for (int j = i + s + 1; j < n; ++j) {
                    alive[i][j] = false;
                }
            }
        }

        void resizeChart(unsigned n)
        {
            chart.resize(n);
            alive.resize(n);
            lexent_chart.resize(n);
            for (unsigned i = 0 ; i < n ; ++i) {
                chart[i].resize(n+1);
                alive[i].resize(n+1, true);
                lexent_chart[i].resize(n+1);
            }
        }

        void initState()
        {
            P2("ParserCkyFom::init state");

            edge_vec.clear();
            link_vec.clear();
            current_iteration_number = 0;

            grammar->clearQuickCheckArray();

            unsigned n = getSentenceLength();
            if (n > chart.size()) {
                resizeChart(n);
            }

            for (unsigned j = 1; j <= n; ++j) {
                for (unsigned i = 0; i < j; ++i) {
                    chart[i][j].clear();
                    alive[i][j] = true;
                    lexent_chart[i][j].clear();
                }
            }

            // initialization of alive[][]: set alive/dead condition of each cell
            inhibitCrossBracketing();
            inhibitLargeConstituents(getLimitConstituentSize(), getSentenceLength());

            // initialization of lexent_chart: c++ data -> lilfes lexent type
            {
                P2("ParserCkyFom::init lexent_chart");
                for (unsigned word_id = 0; word_id < lexent_lattice.size(); ++word_id) {
                    const LexEntExtent &ext = lexent_lattice[word_id];
                    lilfes::FSP word = grammar->makeWordFSP(ext.word, ext.begin, ext.end);
                    std::vector<LexEntState>& cell = lexent_chart[ ext.begin ][ ext.end ];
                    for (std::vector<LexTemplateFom>::const_iterator it = ext.tmpls.begin(); it != ext.tmpls.end(); ++it) {
                        lilfes::FSP lexent = grammar->makeLexicalEntryFSP(word, it->tmpl);
                        cell.push_back(LexEntState(word_id, lexent, it->fom, 0));
                    }
                }
                for (unsigned j = 1; j <= n ; ++j) {
                    for (unsigned i = 0; i < j; ++i) {
                        std::stable_sort(lexent_chart[i][j].begin(), lexent_chart[i][j].end(), GreaterLexEnt());
                    }
                }
            }
        }

        void resetState()
        {
            edge_vec.clear();
            link_vec.clear();
            current_iteration_number = 0;

            grammar->clearQuickCheckArray();

            // clear chart
            for (unsigned j = 1; j <= getSentenceLength(); ++j) {
                for (unsigned i = 0; i < j; ++i) {
                    chart[i][j].clear();
                }
            }

            // reset lexent states
            for (unsigned j = 1; j <= getSentenceLength(); ++j) {
                for (unsigned i = 0; i < j; ++i) {
                    std::vector<LexEntState>& c = lexent_chart[i][j];
                    for (std::vector<LexEntState>::iterator it = c.begin(); it != c.end(); ++it) {
                        it->state = 0;
                    }
                }
            }
        }
        
    protected:
    
        bool parseLex(int lexent_beam_threshold_num, double lexent_beam_threshold_width)
        {
            P2("ParserCkyFom::parseLex");
            lilfes::IPTrailStack iptrail1(mach);
        
            static std::vector<eserial> queue;
            static std::vector<Fs*> signs;
            static std::vector<lserial> links;

            int n = getSentenceLength();

            // cut terminal edges
            for (int i = 0 ; i < n ; i++) {
                for (int j = i + 1; j <= n ; j++) {
                    thresholdOutLexEnts(lexent_chart[i][j], lexent_beam_threshold_num, lexent_beam_threshold_width);
                }
            }
        
            // restore terminal
            for (int i = 0 ; i < n ; i++) {
                for (int j = i + 1; j <= n ; j++) {
                    const std::vector<LexEntState>& sij = lexent_chart[i][j];
                    std::vector<LexEntState>::const_iterator lex = sij.begin(), last = sij.end();
                    for (; lex != last; ++lex) {
                        if ( lex->state > current_iteration_number) break;
                        if ( lex->state == current_iteration_number) {
                            queue.clear();
                            signs.clear();
                            links.clear();
                            if (! grammar->lookupLexicalEntry(lex->word_id, lex->lexent, signs, links, link_vec, lex->fom)) continue;
                            if (! storeEdge(i, j, signs, links, queue)) {
                                return false;
                            }

                            if (grammar->isUnary()) {
                                parseUnary(i, j, queue);
                            }
                        }
                    }
                }
            }
            return true;
        }

        bool parsePhrase(int beam_num, double beam_width, double global_width)
        {
            P2("ParserCkyFom::parsePhrase");
        
            int n = getSentenceLength();

            for ( int len = 2; len <= n ; len++) {
                int maxi = n - len;
                for (int i = 0 ; i <= maxi; i++) {

                    if (! checkElapsedTime()) { // check for timeout
                        return false;
                    }

                    int j = i + len;

                    if (! alive[i][j] ) continue;

                    for ( int k = i + 1; k < j ; k++ ) {
                        if (! parseBinary(i, k, j)) {
                            return false;
                        }
                    }
                    
                    if (grammar->isUnary()) {
                        std::vector<eserial> queue = chart[i][j];
                        parseUnary(i, j, queue);
                    }

                    std::vector<eserial>& sij = chart[i][j];

                    // beam_thresholding
                    thresholdOutEdges(sij, beam_num, beam_width);

                    // sort edge_infos
                    std::vector<eserial>::iterator it = sij.begin(), last = sij.end();
                    for (; it != last ; it++) {
                        std::vector<lserial> &ei = edge_vec[*it].getLinks();
                        std::stable_sort(ei.begin(), ei.end(), GreaterFom<Link>(link_vec));
                    }
                }

                if (isGlobalThresholding()) {
                    globalThresholding(len, n, global_width);
                }
            
                if (isGlobalThresholdingLight()) {
                    globalThresholdingLight(len, n, global_width);
                }
            }
        
            // fom root
            std::vector<eserial>& root = chart[0][n];
            std::vector<eserial>::iterator it = root.begin(), last = root.end();
            for ( ; it != last ; it++) {
                lilfes::IPTrailStack iptrail(mach);
                lilfes::FSP sign(mach, edge_vec[*it].getSign()); lilfes::FSP fom(mach);
                if (grammar->fomRoot(sign, fom)) {
                    edge_vec[*it].setFom(edge_vec[*it].getFom() + fom.ReadFloat());
                }
            }
            std::stable_sort(root.begin(), root.end(), GreaterFom<Edge>(edge_vec));
            
            return true;
        }

        bool checkRootCondition()
        {
            P2("ParserCkyFom::checkRootCondition");

            lilfes::IPTrailStack iptrail(mach);
            const std::vector<eserial> &ee = chart[0][ getSentenceLength() ];
            for (std::vector<eserial>::const_iterator it = ee.begin(); it != ee.end(); ++it) {
                lilfes::FSP root = lilfes::FSP(mach, edge_vec[*it].getSign());
                if ( grammar->rootSign( root ) ) {
                    return true;
                }
            }

            return false;
        }

        // virtual functions
        bool parseImpl()
        {
            if (! checkSentenceLength()) {
                return false;
            }

            initState();
            Beam beam = beam_start;

            while ( true ) {
                if (! parseLex(beam.lexent_num, beam.lexent_width)) {
                    return false;
                }
                if (! parsePhrase(beam.edge_num, beam.edge_width, beam.global_edge_width)) {
                    return false;
                }

                if (checkRootCondition()) {
                    return true;
                }
            
                // retry with wider thresholds
                beam.increment(beam_step);
                if (beam.exceed(beam_end)) {
                    break;
                }
            
                ++current_iteration_number;
                if (! isReuseIterativeParse()) {
                    resetState();
                }
            }

            parse_status = SEARCH_LIMIT;
            return false;
        }

	public: 
        void getEdges(unsigned i, unsigned j, std::vector<eserial>& edges) const
        {
            edges.clear();

            if (i < j && j <= getSentenceLength() && j <= getLimitSentenceLength() ) {
                edges = chart[i][j];
            }
        }
        
    public:
        ParserCkyFom()
            : global_thresholding_type(GT_NONE)
            , current_iteration_number(0)
            , reuse_iterative_parse(false)
        {
            // default beam start
            beam_start.lexent_num = 12;
            beam_start.lexent_width = 6.0;
            beam_start.edge_num = 12;
            beam_start.edge_width = 6.0;
            beam_start.global_edge_width = 6.0;

            // default beam step
            beam_step.lexent_num = 6;
            beam_step.lexent_width = 3.0;
            beam_step.edge_num = 6;
            beam_step.edge_width = 3.0;
            beam_step.global_edge_width = 3.0;

            // default beam end
            beam_end.lexent_num = 30;
            beam_end.lexent_width = 15.0;
            beam_end.edge_num = 30;
            beam_end.edge_width = 15.0;
            beam_end.global_edge_width = 15.0;
        }

        ~ParserCkyFom() { }

        bool init(lilfes::machine* m, Grammar* g)
        {
            if (! base_type::init(m, g) ) return false;
            if (getLimitSentenceLength() > 0) {
                resizeChart(getLimitSentenceLength());
            }
            else {
                resizeChart(default_chart_size);
            }
            sentence_length = 0;
            return true;
        }

        bool isGlobalThresholding() { return (global_thresholding_type == GT_FULL); }
        void enableGlobalThresholding()  { global_thresholding_type = GT_FULL; }
        void disableGlobalThresholding() { global_thresholding_type = GT_NONE; }
        
        bool isGlobalThresholdingLight() { return (global_thresholding_type == GT_LIGHT); }
        void enableGlobalThresholdingLight()  { global_thresholding_type = GT_LIGHT; }
        void disableGlobalThresholdingLight() { global_thresholding_type = GT_NONE; }
        
        void showParserMode(std::ostream &s) const
        {
            base_type::showParserMode(s);
            
            switch (global_thresholding_type) {
                case GT_NONE:  s << "global thresholding: off" << std::endl; break;
                case GT_LIGHT: s << "global thresholding light: on" << std::endl; break;
                case GT_FULL:  s << "global thresholding: on" << std::endl; break;
                default: s << "global thresholding: internal error" << std::endl; break;
            }

            s << "iterative parse reuse: " << (reuse_iterative_parse ? "on" : "off") << std::endl;
            
            s << "lexent beam threshold num: "
              << beam_start.lexent_num << "-("
              << beam_step.lexent_num << ")-"
              << beam_end.lexent_num << std::endl;
            
            s << "lexent beam threshold width: "
              << beam_start.lexent_width << "-("
              << beam_step.lexent_width << ")-"
              << beam_end.lexent_width << std::endl;
            
            s << "beam threshold num: "
              << beam_start.edge_num << "-("
              << beam_step.edge_num << ")-"
              << beam_end.edge_num << std::endl;
            
            s << "beam threshold width: "
              << beam_start.edge_width << "-("
              << beam_step.edge_width << ")-"
              << beam_end.edge_width << std::endl;
            
            s << "global beam threshold width: "
              << beam_start.global_edge_width << "-("
              << beam_step.global_edge_width << ")-"
              << beam_end.global_edge_width << std::endl;
        }

        void enableIterativeParse( unsigned lexent_num_start, double lexent_width_start, unsigned num_start, double width_start, double global_width_start,
                                   unsigned lexent_num_end,   double lexent_width_end,   unsigned num_end,   double width_end,   double global_width_end,
                                   unsigned lexent_num_step,  double lexent_width_step,  unsigned num_step,  double width_step,  double global_width_step ) {
            beam_start.lexent_num        = lexent_num_start;
            beam_start.lexent_width      = lexent_width_start;
            beam_start.edge_num          = num_start;
            beam_start.edge_width        = width_start;
            beam_start.global_edge_width = global_width_start;

            beam_end.lexent_num        = lexent_num_end;
            beam_end.lexent_width      = lexent_width_end;
            beam_end.edge_num          = num_end;
            beam_end.edge_width        = width_end;
            beam_end.global_edge_width = global_width_end;

            beam_step.lexent_num        = lexent_num_step;
            beam_step.lexent_width      = lexent_width_step;
            beam_step.edge_num          = num_step;
            beam_step.edge_width        = width_step;
            beam_step.global_edge_width = global_width_step;
        }

        void disableIterativeParse()
        {
            beam_start.lexent_num        = 0;
            beam_start.lexent_width      = 0;
            beam_start.edge_num          = 0;
            beam_start.edge_width        = 0;
            beam_start.global_edge_width = 0;

            beam_end.lexent_num        = 0;
            beam_end.lexent_width      = 0;
            beam_end.edge_num          = 0;
            beam_end.edge_width        = 0;
            beam_end.global_edge_width = 0;

            beam_step.lexent_num        = 1; // run the parsing loop only once
            beam_step.lexent_width      = 1;
            beam_step.edge_num          = 1;
            beam_step.edge_width        = 1;
            beam_step.global_edge_width = 1;
        }

        bool isReuseIterativeParse() const { return reuse_iterative_parse; }
        void enableReuseIterativeParse() { reuse_iterative_parse = true; }
        void disableReuseIterativeParse() { reuse_iterative_parse = false; }

        // for debug
        void dump(std::ostream& o)
        {
            unsigned n = getSentenceLength();

            o << "---------------------" << std::endl;
            o << "lex" << std::endl;
            o << "---------------------" << std::endl;
            for (LexEntLattice::const_iterator ext = lexent_lattice.begin(); ext != lexent_lattice.end(); ++ext) {
                o << "[" << ext->begin << "," << ext->end << "):";
                for (unsigned i = 0; i < ext->tmpls.size(); ++i) {
                    o << ' ' << ext->tmpls[i].tmpl << ':' << ext->tmpls[i].fom;
                }
                o << std::endl;
            }
            o << "---------------------" << std::endl;
            o << "chart" << std::endl;
            o << "---------------------" << std::endl;
            for (unsigned l = 1; l <= n; ++l) {
                for (unsigned i = 0; i + l <= n; ++i) {
                    const std::vector<eserial> &c = chart[i][i+l];
                    o << "[" << i << "][" << i+l << "]:";
                    for (unsigned j = 0; j < c.size(); ++j) {
                        Edge& e = edge_vec[ c[j] ];
                        o << ' ' << c[j] << ':' << e.getFom() << ':' << e.getState();
                        if (l == 1) {
                            std::vector<lserial> ls = e.getLinks();
                            for (unsigned j = 0; j < ls.size(); ++j) {
                                if (link_vec[ls[j]].isTerminal()) {
                                    lilfes::FSP lexname(mach, link_vec[ls[j]].getLexName());
                                    o << ":" << lexname.DisplayAVM();
                                }
                            }
                        }
                    }
                    o << std::endl;
                }
            }
        }
    };
}
#endif // __ParserCkyFom_h

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.11  2009/12/18 12:59:32  hkhana
 * make small changes for compilation with GCC-4.1
 *
 * Revision 1.10  2009/12/15 07:34:42  matuzaki
 *
 * bug fix for forestmaker
 *
 * Revision 1.9  2009/12/03 23:33:28  matuzaki
 *
 * clean-up branch initial version
 *
 * clean-up branch initial version
 *
 *
 * clean-up branch initial version
 *
 * clean-up branch initial version
 *
 * clean-up branch initial version
 *
 * clean-up branch initial version
 *
 * Revision 1.8  2009/01/06 11:35:31  matuzaki
 *
 * fix for parsing with up
 *
 * fix for making 'up -nofom' work
 *
 * Revision 1.7  2008/07/21 06:19:39  mkurita
 * Modified to execute on AIX
 *
 * Revision 1.6  2008/06/05 20:24:11  matuzaki
 *
 * Merge mogura
 *
 * Merge mogura, separation of Supertagger
 *
 * addtion: lexmerge
 * update: derivtoxml
 *
 * separation of supertagger
 *
 * separation of supertagger
 *
 * Separation of Supertagger
 *
 * Revision 1.5.6.1  2008/04/28 06:30:52  matuzaki
 *
 * initial version of mogura parser
 *
 * initial version of mogura parser
 *
 *
 * Separate supertagger class
 *
 * Separate supertagger class
 *
 * Revision 1.5  2007/10/14 06:51:55  ninomi
 * fix the problems in time interfaces. (e.g., removed get_preprocessing_time)
 *
 * Revision 1.4  2007/09/07 00:23:53  ninomi
 * grammar-refine-1 is merged to trunk.
 *
 */
