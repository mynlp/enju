#ifndef __Supertagger_h__
#define __Supertagger_h__

#ifdef _MSC_VER
# include <float.h>
# include <math.h>
#endif

#include "LexAnalyzerPipe.h"
#include "LexiconTable.h"
#include "AmisModel.h"

namespace up {

    ////////////////////////////////////////////////////////////////////////////
    // Common routines
    ////////////////////////////////////////////////////////////////////////////
    namespace super {
        // vector of (relative position, index of word-element)
        typedef std::vector<std::pair<int, unsigned> > EventDef;

        // pair of (category-name, mask-vector)
        typedef std::pair<std::string, std::vector<int> > MaskDef;

        class ConfigFormatError : public std::runtime_error {
        public:
            ConfigFormatError(const std::string& msg)
                : std::runtime_error("configuration file format error: " + msg) {}
        };

        inline
        void readConfigFile(std::istream& conf_file,
                            EventDef& event_def,
                            std::vector<MaskDef>& masks)
        {
            event_def.clear();
            masks.clear();

            // read an event & mask definition file
            std::string line;
            while (std::getline(conf_file, line)) {
                if (line.empty() || line[0] == '#') {
                    continue;
                }
                std::istringstream iss(line);
                std::string tag;
                if (! (iss >> tag)) {
                    continue; // empty line
                }

                if (tag == "e") {
                    int relpos;
                    unsigned ix;
                    if (! (iss >> relpos >> ix)) {
                        throw ConfigFormatError("bad event definition: " + line);
                    }
                    event_def.push_back(std::make_pair(relpos, ix));
                }
                else if (tag == "m") {
                    std::string model_name;
                    std::string category_name;
                    if (! (iss >> model_name >> category_name)) {
                        throw ConfigFormatError("model or category name not found: " + line);
                    }

                    std::vector<int> v;
                    std::copy(std::istream_iterator<int>(iss),
                              std::istream_iterator<int>(),
                              std::back_inserter(v));

                    masks.push_back(std::make_pair(category_name, v));
                }
                else {
                    throw ConfigFormatError("unknown tag: " + tag);
                }
            }
        }

        struct LessBegin {
            LessBegin(const WordLattice& lattice_) : lattice(lattice_) {}
            bool operator()(unsigned i, unsigned j) const { return lattice[i].begin < lattice[j].begin; }
        private:
            const WordLattice& lattice;
        };

        // check if a word_lattice forms a linear-chain
        inline
        bool checkInput(const WordLattice& word_lattice, std::vector<unsigned>& path)
        {
            path.resize(word_lattice.size());
            for (unsigned i = 0; i < path.size(); ++i) {
                path[i] = i;
            }
            std::stable_sort(path.begin(), path.end(), LessBegin(word_lattice));

            // check connection
            for (unsigned i = 1; i < path.size(); ++i) {
                if (word_lattice[ path[i-1] ].end != word_lattice[ path[i] ].begin) {
                    return false;
                }
            }
            return true;
        }

        // make an event vector using EventDef
        inline
        bool makeEventVector(
            int target_position,
            const WordLattice& word_lattice,
            const std::vector<unsigned>& path,
            const EventDef& event_def,
            std::vector<std::string>& event)
        {
            event.clear();

            for (super::EventDef::const_iterator it = event_def.begin(); it != event_def.end(); ++it) {
                int position = target_position + it->first;
                if (position < 0) {
                    event.push_back("BOS");
                }
                else if (position >= (int) path.size()) {
                    event.push_back("EOS");
                }
                else {
                    const WordExtent& w = word_lattice[ path[position] ];
                    if (it->second >= w.word.size()) {
                        std::cerr << "WARN: input word structure does not match the supertagger event definition" << std::endl;
                        return false;
                    }

                    event.push_back(w.word[it->second]);
                }
            }

            return true;
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    // Event extraction
    ////////////////////////////////////////////////////////////////////////////
    class SupertaggerEventExtractor {
    public:
        SupertaggerEventExtractor(std::istream& lexicon_file,
                                  std::istream& conf_file)
        {
            lexicon = new LexiconTable(lexicon_file);

            // load the event definition and masks
            std::vector<super::MaskDef> masks;
            try {
                super::readConfigFile(conf_file, event_def, masks);
            }
            catch (std::runtime_error& e) {
                throw std::runtime_error(std::string("SupertaggerEventExtractor: ") + e.what());
            }
        }

        bool outputEvents(int deriv_id,
                          const WordLattice& word_lattice,
                          const std::vector<std::string>& tmpls,
                          std::ostream& output_file) const
        {
            std::vector<unsigned> path;
            if (! super::checkInput(word_lattice, path)) {
                std::cerr << "WARN: SupertaggerEventExtractor: input does not form a linear-chain" << std::endl;
                return false;
            }

            for (unsigned i = 0; i < path.size(); ++i) {

                std::string correct_tmpl = tmpls[ path[i] ];

                std::vector<std::string> candidates;
                if (! lexicon->lookup(word_lattice[ path[i] ].lookup_keys, candidates)) {
                    return false;
                }

                if (std::find(candidates.begin(), candidates.end(), correct_tmpl) == candidates.end()) {
                    std::cerr << deriv_id << ": template not covered: " << correct_tmpl << std::endl;
                    continue; // not covered
                }

                std::vector<std::string> event;
                if (! super::makeEventVector(i, word_lattice, path, event_def, event)) {
                    return false;
                }
                event.resize(event.size() + 1); // allocate one more element for a template name

                output_file << "event_" << deriv_id << "_" << i << '\n';
                for (std::vector<std::string>::const_iterator it = candidates.begin(); it != candidates.end(); ++it) {

                    std::string event_name;
                    event.back() = *it;
                    mayz::AmisModel::encode("uni", event, event_name);

                    output_file << (*it == correct_tmpl ? "1\t" : "0\t") << event_name << '\n';
                }
                output_file << '\n';
            }

            return true;
        }

    protected:
        LexiconTable *lexicon;
        super::EventDef event_def;
    };

    ////////////////////////////////////////////////////////////////////////////
    // Supertagging on POS-disambiguated input
    ////////////////////////////////////////////////////////////////////////////
    class SupertaggerViterbiMorph : public Supertagger {
    public:
        SupertaggerViterbiMorph(std::istream& lexicon_file,
                                   std::istream& model_file,
                                   std::istream& conf_file)
        {
            lexicon = new LexiconTable(lexicon_file);

            amis_model = new mayz::AmisModel();

            // load feature weights
            if (! amis_model->importModel(model_file)) {
                throw std::runtime_error("SupertaggerViterbiMorph: error in importing amis model");
            }

            // load an event definition and masks
            std::vector<super::MaskDef> masks;
            try {
                super::readConfigFile(conf_file, event_def, masks);
            }
            catch (std::runtime_error& e) {
                throw std::runtime_error(std::string("SupertaggerViterbiMorph: ") + e.what());
            }

            // register masks in the model
            for (std::vector<super::MaskDef>::const_iterator it = masks.begin(); it != masks.end(); ++it) {
                if (! amis_model->addFeatureMask(it->first, it->second)) {
                    throw std::runtime_error("SupertaggerViterbiMorph: error in feature mask definition");
                }
            }
        }

        ~SupertaggerViterbiMorph(void)
        {
            delete amis_model;
        }

        bool analyze(const WordLattice& word_lattice, std::vector<std::vector<LexTemplateFom> >& tmplss) const
        {
            tmplss.clear();
            tmplss.resize(word_lattice.size());

            std::vector<unsigned> path;
            if (! super::checkInput(word_lattice, path)) {
                std::cerr << "WARN: SupertaggerViterbiMorph: input does not form a linear-chain" << std::endl;
                return false;
            }

            for (unsigned i = 0; i < path.size(); ++i) {

                std::vector<std::string> event;
                if (! super::makeEventVector(i, word_lattice, path, event_def, event)) {
                    return false;
                }

                std::vector<std::string> tmpls;
                if (! lexicon->lookup(word_lattice[ path[i] ].lookup_keys, tmpls)) {
                    return false;
                }

                // allocate one more element for a template name
                event.resize(event.size() + 1);

                // get weights from the model
                std::vector<LexTemplateFom>& tmpl_foms = tmplss[ path[i] ];
                double sum = 0;
                for (std::vector<std::string>::const_iterator tmpl = tmpls.begin(); tmpl != tmpls.end(); ++tmpl) {
                    event.back() = *tmpl;
                    double weight = amis_model->eventWeight("uni", event);
                    tmpl_foms.push_back(LexTemplateFom(*tmpl, weight));
                    sum += std::exp(weight);
                }

                // normalize
                double logsum = std::log(sum);
                for (std::vector<LexTemplateFom>::iterator it = tmpl_foms.begin(); it != tmpl_foms.end(); ++it) {
                    it->fom -= logsum;
                }
            }

            return true;
        }

    protected:
        LexiconTable *lexicon;
        mayz::AmisModel *amis_model;
        super::EventDef event_def;
    };

    ////////////////////////////////////////////////////////////////////////////
    // Supertagging on ambiguous POS
    ////////////////////////////////////////////////////////////////////////////
    class SupertaggerMultiMorph : public Supertagger {
    private:
        enum Direction { LEFT = 0, RIGHT = 1 };

        struct Connection {
            std::vector<const WordExtent*> extents[2]; // index = LEFT or RIGHT
        };
        typedef std::vector<const WordExtent*>::const_iterator ConnItr;

        struct EventElemDef {
            unsigned word_elem_ix;
            unsigned event_elem_ix;

            EventElemDef() {}
            EventElemDef(unsigned wix, unsigned eix) : word_elem_ix(wix), event_elem_ix(eix) {}
        };
        typedef std::vector<EventElemDef> SubEventDef; // set of elements at each relative position

        struct SubEventProb {
            std::vector<std::string> sub_event;
            double log_prob;

            SubEventProb(void) : log_prob(0) {}
        };

    private:
        // maximum number of words (~= POS tags) on each span
        // NOTE: re-normalization within the active word extents is desirable but
        //      not implemented now; it requires forward-backward on the active part
        //      of the word-lattice
        unsigned maxAmbiguity;

    public:
        SupertaggerMultiMorph(std::istream& lexicon_file,
                                   std::istream& model_file,
                                   std::istream& conf_file,
                                   unsigned maxAmbiguity_)
            : maxAmbiguity(maxAmbiguity_)
        {
            lexicon = new LexiconTable(lexicon_file);

            amis_model = new mayz::AmisModel();

            if (! amis_model->importModel(model_file)) {
                throw std::runtime_error("error in importing amis model");
            }

            std::vector<super::MaskDef> masks;
            try {
                super::readConfigFile(conf_file, event_def, masks);
            }
            catch (std::runtime_error& e) {
                throw std::runtime_error(std::string("SupertaggerMultiMorph: ") + e.what());
            }

            // register masks in the model
            for (std::vector<super::MaskDef>::const_iterator it = masks.begin(); it != masks.end(); ++it) {
                if (! amis_model->addFeatureMask(it->first, it->second)) {
                    throw std::runtime_error("SupertaggerMultiMorph: error in feature mask definition");
                }
            }

            makeSubEventDefs(event_def, left_event_def, right_event_def, center_event_def);
        }

        ~SupertaggerMultiMorph(void)
        {
            delete amis_model;
        }

        bool analyze(const WordLattice& word_lattice, std::vector<std::vector<LexTemplateFom> >& tmplss) const
        {
            tmplss.clear();
            tmplss.resize(word_lattice.size());

            std::vector<bool> active;
            thresholdWords(word_lattice, active);

            std::vector<Connection> connections;
            analyzeConnections(word_lattice, active, connections);

            for (unsigned i = 0; i < word_lattice.size(); ++i) {

                if (! active[i]) {
                    continue;
                }

                const WordExtent& w = word_lattice[i];

                // look-up the lexicon
                std::vector<std::string> tmpls;
                if (! lexicon->lookup(w.lookup_keys, tmpls)) { // no templates for this extent
                    continue;
                }

                SubEventProb stack;
                std::vector<SubEventProb> left_event;
                followPath(w, LEFT, connections, left_event_def.begin(), left_event_def.end(), stack, left_event);

                std::vector<SubEventProb> right_event;
                followPath(w, RIGHT, connections, right_event_def.begin(), right_event_def.end(), stack, right_event);

                SubEventProb center_event;
                if (! addEventElem(w, center_event_def, center_event)) {
                    return false;
                }

                std::vector<std::string> event(event_def.size() + 1); // +1 for the target (= template) name

                // P(tmpl, this-extent) = Sum_ctxt P(tmpl | ctxt) P(ctxt)
                std::vector<double> prob(tmpls.size(), 0); // index: index in tmpls

                // log P(tmpl | ctxt)
                std::vector<double> cond_prob(tmpls.size(), 0);

                putElement(center_event_def, center_event.sub_event.begin(), event);
                for (unsigned j = 0; j < left_event.size(); ++j) {
                    // overlay left event elements
                    putElement(left_event_def, left_event[j].sub_event, event);

                    for (unsigned k = 0; k < right_event.size(); ++k) {
                        // overlay right event elements
                        putElement(right_event_def, right_event[k].sub_event, event);

                        double ctxt_fom = w.word_fom + left_event[j].log_prob + right_event[k].log_prob;
                        // get weights from the model
                        double sum = 0;
                        for (unsigned t = 0; t < tmpls.size(); ++t) {
                            event.back() = tmpls[t];
                            double weight = amis_model->eventWeight("uni", event);
                            cond_prob[t] = weight;
                            sum += std::exp(weight);
                        }
                        double logsum = std::log(sum);
#ifdef _MSC_VER
                        if (! _finite(logsum)) { // 2012-04-04: bugfix by matuzaki (missing "!" added)
                            cond_prob.clear();
                            cond_prob.resize(tmpls.size(), 0);
                            logsum = log((double)tmpls.size()); // uniform
                        }
#else
                        if (std::isinf(logsum)) {
                            cond_prob.clear();
                            cond_prob.resize(tmpls.size(), 0);
                            logsum = std::log(tmpls.size()); // uniform
                        }
#endif
                        // normalization of cond_prob, and accumulation over contexts
                        for (unsigned t = 0; t < tmpls.size(); ++t) {
                            prob[t] += std::exp(cond_prob[t] - logsum + ctxt_fom);
                        }
                    }
                }

                tmplss[i].resize(tmpls.size());
                for (unsigned t = 0; t < tmpls.size(); ++t) {
                    tmplss[i][t] = LexTemplateFom(tmpls[t], std::log(prob[t]));
                }
            }

            return true;
        }

        void followPath(const WordExtent& ext,
                        Direction dir,
                        const std::vector<Connection>& connections,
                        std::vector<SubEventDef>::const_iterator def,
                        std::vector<SubEventDef>::const_iterator end_def,
                        SubEventProb& stack,
                        std::vector<SubEventProb>& sub_event_table) const
        {
            if (def == end_def) { // no more event element from outside of this extent
                sub_event_table.push_back(stack);
                return;
            }

            const std::vector<const WordExtent*>& cs = connections[ (dir == LEFT) ? ext.begin : ext.end ].extents[dir];

            if (! cs.empty()) { // inner extent
                double orig_prob = stack.log_prob;
                unsigned orig_size = stack.sub_event.size();

                for (ConnItr e = cs.begin(); e != cs.end(); ++e) {

                    // exract elements from a next extent
                    addEventElem(**e, *def, stack);

                    // go further from the next extent
                    followPath(**e, dir, connections, def + 1, end_def, stack, sub_event_table);

                    // restore the original partial event
                    stack.sub_event.resize(orig_size);
                    stack.log_prob = orig_prob;
                }
            }
            else { // sentence-initial or -final extent
                unsigned orig_size = stack.sub_event.size();
                for ( ; def != end_def; ++def) {
                    stack.sub_event.resize(stack.sub_event.size() + def->size(), (dir == LEFT) ? "BOS" : "EOS");
                }
                sub_event_table.push_back(stack);
                stack.sub_event.resize(orig_size); // pop out BOS/EOS elements
            }
        }

        bool addEventElem(const WordExtent& ext, const SubEventDef& def, SubEventProb& ev) const
        {
            ev.log_prob += ext.word_fom;

            for (SubEventDef::const_iterator d = def.begin(); d != def.end(); ++d) {
                if (d->word_elem_ix >= ext.word.size()) {
                    std::cerr << "WARN: SupertaggerMultiMorph: too few word features" << std::endl;
                    return false;
                }
                ev.sub_event.push_back(ext.word[ d->word_elem_ix ]);
            }
            return true;
        }

        // re-organize the event definition to left/rigth/center sub-events
        //  ldef[0] = event elements from relative position -1
        //  ldef[1] = event elements from relative position -2
        //  ...
        //
        //  rdef[0] = event elements from relative position +1
        //  rdef[1] = event elements from relative position +2
        //  ...
        void makeSubEventDefs(const super::EventDef& edef,
                              std::vector<SubEventDef>& ldef,
                              std::vector<SubEventDef>& rdef,
                              SubEventDef& cdef)
        {
            ldef.clear();
            rdef.clear();
            cdef.clear();

            for (unsigned i = 0; i < edef.size(); ++i) {
                if (edef[i].first < 0) { // left context
                    unsigned ix = -(edef[i].first) - 1;
                    if (ldef.size() <= ix) ldef.resize(ix + 1);
                    ldef[ix].push_back(EventElemDef(edef[i].second, i));
                }
                else if (edef[i].first > 0) { // right context
                    unsigned ix = (edef[i].first) - 1;
                    if (rdef.size() <= ix) rdef.resize(ix + 1);
                    rdef[ix].push_back(EventElemDef(edef[i].second, i));
                }
                else { // target position
                    cdef.push_back(EventElemDef(edef[i].second, i));
                }
            }
        }

        struct GreaterWordFom {
            GreaterWordFom(const WordLattice &ws_) : ws(ws_) {}

            bool operator()(unsigned i, unsigned j) const
            {
                return ws[i].word_fom > ws[j].word_fom;
            }

        private:
            const WordLattice &ws;
        };

        void thresholdWords(const WordLattice &word_lattice, std::vector<bool> &active) const
        {
            active.clear();
            active.resize(word_lattice.size(), true);

            if (maxAmbiguity == 0) { // no thresholding
                return;
            }

            typedef std::map<std::pair<unsigned, unsigned>, std::vector<unsigned> > WordsBySpan;
            WordsBySpan spans;
            for (unsigned i = 0; i < word_lattice.size(); ++i) {
                const WordExtent &w = word_lattice[i];
                spans[ std::make_pair(w.begin, w.end) ].push_back(i);
            }

            for (WordsBySpan::iterator it = spans.begin(); it != spans.end(); ++it) {
                std::vector<unsigned> &ws = it->second;
                if (ws.size() > maxAmbiguity) {
                    std::stable_sort(ws.begin(), ws.end(), GreaterWordFom(word_lattice));
                    for (unsigned j = maxAmbiguity; j < ws.size(); ++j) {
                        active[ ws[j] ] = false;
                    }
                }
            }
        }

        void analyzeConnections(const WordLattice& word_lattice,
                                const std::vector<bool> &active,
                                std::vector<Connection>& connections) const
        {
            connections.clear();

            for (unsigned i = 0; i < word_lattice.size(); ++i) {
                if (! active[i]) {
                    continue;
                }

                const WordExtent &w = word_lattice[i];
                if (connections.size() <= std::max(w.begin, w.end)) {
                    connections.resize(std::max(w.begin, w.end) + 1);
                }
                connections[w.begin].extents[RIGHT].push_back(&w);
                connections[w.end  ].extents[LEFT ].push_back(&w);
            }
        }

        // pre-condition: ev has enough space
        void putElement(const std::vector<SubEventDef>& def,
                        const std::vector<std::string>& sub_ev,
                        std::vector<std::string>& ev) const
        {
            std::vector<std::string>::const_iterator sub_ev_itr = sub_ev.begin();
            for (std::vector<SubEventDef>::const_iterator d = def.begin(); d != def.end(); ++d) {
                sub_ev_itr = putElement(*d, sub_ev_itr, ev);
            }
        }

        std::vector<std::string>::const_iterator
            putElement(const SubEventDef& def,
                       std::vector<std::string>::const_iterator sub_ev_itr,
                       std::vector<std::string>& ev) const
        {
            for (SubEventDef::const_iterator d = def.begin(); d != def.end(); ++d) {
                ev[ d->event_elem_ix ] = *sub_ev_itr++;
            }
            return sub_ev_itr;
        }
        
    protected:
        LexiconTable *lexicon;
        mayz::AmisModel *amis_model;
        super::EventDef event_def;

        std::vector<SubEventDef> left_event_def;
        std::vector<SubEventDef> right_event_def;
        SubEventDef center_event_def;
    };
}

#endif // __Supertagger__
