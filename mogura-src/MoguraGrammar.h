#ifndef MoguraGrammar_h__
#define MoguraGrammar_h__

#include "Grammar.h"
#include "mayzutil.h"

namespace mogura {

class Grammar : public up::Grammar {
private:
    typedef up::Grammar base_type;

private:
    bool _is_open;
    lilfes::procedure *_get_sign_label;
    lilfes::procedure *_get_schema_list;
    lilfes::procedure *_get_enju_feature;
    lilfes::procedure *_get_val_feature;
    lilfes::procedure *_get_model_key_index;
    lilfes::procedure *_get_pos_index;
    lilfes::procedure *_get_input_word_index;
    lilfes::procedure *_get_cfg_grammar_file;
    lilfes::procedure *_get_restricted_cfg_grammar_file;
    lilfes::procedure *_get_model_file;

    // the index in word-feature vector to select a disambiguation model
    unsigned _modelKeyIndex;

    // the index of POS feature field in word vec
    unsigned _posIndex;

    // the index of input word feature field in word vec
    unsigned _inputWordIndex;

public:
    Grammar(void)
        : base_type()
        , _is_open(false)
        , _get_sign_label(0)
        , _get_schema_list(0)
        , _get_enju_feature(0)
        , _get_val_feature(0)
        , _get_model_key_index(0)
        , _get_pos_index(0)
        , _get_input_word_index(0)
        , _get_cfg_grammar_file(0)
        , _get_restricted_cfg_grammar_file(0)
        , _get_model_file(0)
        , _modelKeyIndex(0)
    {}

    lilfes::machine *getMachine(void) { return mach; }

    bool init(lilfes::machine *m)
    {
        if (! base_type::init(m)) {
            return false;
        }

        if (_is_open) {
            return true;
        }

        lilfes::module *moguraInterface = lilfes::module::SearchModule("mogura:grammar");
        if (! moguraInterface) {
            std::cerr << "module not found: mogura:parser" << std::endl;
            return false;
        }

        bool ok = up::init_proc(moguraInterface, _get_sign_label, "get_sign_label", 2)
               && up::init_proc(moguraInterface, _get_schema_list, "get_schema_list", 3)
               && up::init_proc(moguraInterface, _get_enju_feature, "get_enju_feature", 3)
               && up::init_proc(moguraInterface, _get_val_feature, "get_val_feature", 2)
               && up::init_proc(moguraInterface, _get_model_key_index, "get_model_key_index", 1)
               && up::init_proc(moguraInterface, _get_pos_index, "get_pos_index", 1)
               && up::init_proc(moguraInterface, _get_input_word_index, "get_input_word_index", 1)
               && up::init_proc(moguraInterface, _get_cfg_grammar_file, "get_cfg_grammar_file", 1)
               && up::init_proc(moguraInterface, _get_restricted_cfg_grammar_file, "get_restricted_cfg_grammar_file", 1)
               && up::init_proc(moguraInterface, _get_model_file, "get_model_file", 1)
               ;

        if (! ok) {
            return false;
        }

        try {
            _modelKeyIndex  = getIndexConfig(_get_model_key_index, "model-key", "get_model_key_index/1");
            _posIndex       = getIndexConfig(_get_pos_index, "pos-field", "get_pos_index/1");
            _inputWordIndex = getIndexConfig(_get_input_word_index, "input-word-field", "get_input_word_index/1");
        }
        catch (std::runtime_error &e) {
            std::cerr << e.what() << std::endl;
            return false;
        }

        _is_open = true;
        return true;
    }

    bool getSignLabel(lilfes::FSP sign, std::string &label)
    {
        lilfes::FSP labelFsp(mach);
        if (! up::call_proc(mach, _get_sign_label, sign, labelFsp)) {
            std::cerr << "WARN: get_sign_label failed" << std::endl;
            return false;
        }

        if (! labelFsp.IsString()) {
            return false;
        }

        label = labelFsp.ReadString();

        return true;
    }

    bool getSchemaList(lilfes::FSP leftHead, lilfes::FSP rightHead, lilfes::FSP unary)
    {
        return up::call_proc(mach, _get_schema_list, leftHead, rightHead, unary);
    }

    bool getEnjuFeature(lilfes::FSP left, lilfes::FSP right, lilfes::FSP fv)
    {
        return up::call_proc(mach, _get_enju_feature, left, right, fv);
    }

    bool getValFeature(lilfes::FSP sign, lilfes::FSP fv)
    {
        return up::call_proc(mach, _get_val_feature, sign, fv);
    }

    bool applyIdSchemaUnary(const lilfes::type *schema, lilfes::FSP dtr, lilfes::FSP mother)
    {
        up::ScopeProf prof("mogura::Grammar::applyIdSchemaUnary");

        lilfes::code *ip = mach->GetIP();
        lilfes::core_p trailPtr = mach->SetTrailPoint(NULL);

        lilfes::FSP dcp(mach);
        lilfes::FSP schemaFsp(mach);
        schemaFsp.Coerce(schema);

        bool ok = idSchemaUnary(schemaFsp, dtr, mother, dcp)
               && lilfes::builtin::lilfes_call(*mach, dcp)
               && mach->GetIP() != NULL
               && mach->Execute(mach->GetIP());

        if (ok) {
            return true;
        }
        else {
            mach->TrailBack(trailPtr);
            mach->SetIP(ip);
            return false;
        }
    }

    bool applyIdSchemaBinary(const lilfes::type *schema, lilfes::FSP left, lilfes::FSP right, lilfes::FSP mother)
    {
        up::ScopeProf prof("mogura::Grammar::applyIdSchemaBinary");

        lilfes::code *ip = mach->GetIP();
        lilfes::core_p trailPtr = mach->SetTrailPoint(NULL);

        lilfes::FSP dcp(mach);
        lilfes::FSP schemaFsp(mach);
        schemaFsp.Coerce(schema);

        bool ok = idSchemaBinary(schemaFsp, left, right, mother, dcp)
               && lilfes::builtin::lilfes_call(*mach, dcp)
               && mach->GetIP() != NULL
               && mach->Execute(mach->GetIP());

        if (ok) {
            return true;
        }
        else {
            mach->TrailBack(trailPtr);
            mach->SetIP(ip);
            return false;
        }
    }

    unsigned getModelKeyIndex(void) const { return _modelKeyIndex; }
    unsigned getPosIndex(void) const { return _posIndex; }
    unsigned getInputWordIndex(void) const { return _inputWordIndex; }

    bool getCfgGrammarFile(std::string &fname)
    {
        return getFileNameImpl(fname, _get_cfg_grammar_file);
    }

    bool getRestrictedCfgGrammarFile(std::string &fname)
    {
        return getFileNameImpl(fname, _get_restricted_cfg_grammar_file);
    }

    bool getModelFile(std::string &fname)
    {
        return getFileNameImpl(fname, _get_model_file);
    }

private:
    bool getFileNameImpl(std::string &fname, lilfes::procedure *getter)
    {
        lilfes::FSP fnameFsp(mach);
        if (up::call_proc(mach, getter, fnameFsp) && fnameFsp.IsString()) {
            fname = fnameFsp.ReadString();
            return true;
        }
        else {
            return false;
        }
    }

    unsigned getIndexConfig(lilfes::procedure *proc,
                            const std::string &fieldName,
                            const std::string &predName)
    {
        lilfes::IPTrailStack iptrail(mach);

        lilfes::FSP indexFSP(mach);
        if (! up::call_proc(mach, proc, indexFSP) || ! indexFSP.IsInteger()) {
            throw std::runtime_error("could not get " + fieldName + " from " + predName);
        }
        return indexFSP.ReadInteger();
    }
};

} // namespace mogura

#endif // MoguraGrammar_h__
