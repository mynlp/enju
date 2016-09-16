#include <iostream>
#include <map>
#include <set>
#include <algorithm>
#include <iterator>
#include <liblilfes/CppPred.h>
#include <liblilfes/utility.h>

class Restrictor {
private:
    typedef std::vector<lilfes::cell> Fs;

public:
    typedef std::vector<const lilfes::type*> TypeRdef;
    typedef std::map<const lilfes::type*, TypeRdef> TypeRdefSet;
    typedef std::set<const lilfes::feature*> NoFollowPaths;
    typedef std::vector<NoFollowPaths> NoFollowPathsSet;
    typedef std::map<const lilfes::feature*, std::vector<unsigned> > PathLevelSwitchSet;

    typedef std::map<std::pair<const lilfes::type*, const lilfes::feature*>, unsigned> DepthIncPaths;
    typedef std::vector<unsigned> MaxDepthDef;
    typedef std::vector<std::vector<unsigned> > DepthLevelMap;

    struct Pattern {
        std::vector<Fs> _pattern;
        std::vector<const lilfes::feature*> _path;
        unsigned _level;

        Pattern(void) {}
        Pattern(const Fs &pattern, const std::vector<const lilfes::feature*> &path, unsigned level)
            : _pattern(1, pattern)
            , _path(path)
            , _level(level) {}
    };

    typedef std::vector<Pattern> PatternSet;

public:

    lilfes::machine *_mach;
    TypeRdefSet _typeRdefs;
    NoFollowPathsSet _noFollows;
    PathLevelSwitchSet _pathLevelSwitches;
    DepthIncPaths _depthIncPaths;
    MaxDepthDef _maxDepthDef;
    DepthLevelMap _depthLevelMap;
    PatternSet _patterns;
    bool _defError;

private:
    Restrictor(void) : _defError(false) {}

public:
    static Restrictor *getInstance()
    {
        static Restrictor *theInstance = 0;
        if (theInstance == 0) {
            theInstance = new Restrictor();
        }
        return theInstance;
    }

public:
    /// For debug
    void dump(std::ostream &ost)
    {
        lilfes::IPTrailStack iptrail(_mach);

        ost << "type restriction definitions:" << std::endl;
        for (TypeRdefSet::const_iterator it = _typeRdefs.begin(); it != _typeRdefs.end(); ++it) {
            ost << ' ' << it->first->GetSimpleName();
            for (unsigned i = 0; i < it->second.size(); ++i) {
                ost << " [" << (i + 1) << ']' << '\t' << (it->second)[i]->GetSimpleName();
            }
            ost << std::endl;
        }
        ost << std::endl;

        ost << "no follow paths:" << std::endl;
        for (unsigned i = 0; i < _noFollows.size(); ++i) {
            ost << " [" << (i + 1) << ']';
            for (NoFollowPaths::const_iterator it = _noFollows[i].begin(); it != _noFollows[i].end(); ++it) {
                ost << ' ' << (*it)->GetSimpleName();
            }
            ost << std::endl;
        }
        ost << std::endl;

        ost << "path level switches:" << std::endl;
        for (PathLevelSwitchSet::const_iterator it = _pathLevelSwitches.begin(); it != _pathLevelSwitches.end(); ++it) {
            ost << ' ' << it->first->GetSimpleName();
            for (unsigned i = 0; i < it->second.size(); ++i) {
                ost << " [" << (i + 1) << "] " << (it->second)[i];
            }
            ost << std::endl;
        }
        ost << std::endl;

        ost << "max depth definition:" << std::endl;
        for (unsigned i = 0; i < _maxDepthDef.size(); ++i) {
            ost << " [" << (i + 1) << "]: " << _maxDepthDef[i] << std::endl;
        }
        ost << std::endl;

        ost << "depth inc features:" << std::endl;
        for (DepthIncPaths::const_iterator it = _depthIncPaths.begin(); it != _depthIncPaths.end(); ++it) {
            ost << ' ' << it->first.first->GetSimpleName() << "::" << it->first.second->GetSimpleName()
                << " => +" << it->second << std::endl;
        }
        ost << std::endl;

        ost << "depth level map:" << std::endl;
        for (unsigned i = 0; i < _depthLevelMap.size(); ++i) {
            ost << " [" << i << "]:";
            for (unsigned j = 0; j < _depthLevelMap[i].size(); ++j) {
                ost << ' ' << _depthLevelMap[i][j];
            }
            ost << std::endl;
        }
        ost << std::endl;

        ost << "pattern:" << std::endl;
        for (PatternSet::const_iterator it = _patterns.begin(); it != _patterns.end(); ++it) {
            ost << " pattern:" << std::endl;
            for (std::vector<Fs>::const_iterator p = it->_pattern.begin(); p != it->_pattern.end(); ++p) {
                if (p != it->_pattern.begin()) {
                    ost << "--or--" << std::endl;
                }
                lilfes::FSP pat(_mach, *p);
                ost << pat.DisplayAVM();
            }
            ost << " path: ";
            for (std::vector<const lilfes::feature*>::const_iterator f = it->_path.begin();
                 f != it->_path.end();
                 ++f) {
                ost << (*f)->GetSimpleName() << "\\";
            }
            ost << std::endl;
            ost << " level: " << it->_level;
            ost << " ---------" << std::endl;
        }
    }
    
    /// Setters
    bool addTypeRdef(const lilfes::type* derived, const TypeRdef &rdef)
    {
        const lilfes::type *last = lilfes::bot;
        for (TypeRdef::const_iterator it = rdef.begin(); it != rdef.end(); ++it) {
            if (! derived->IsSubType(*it)) {
                return false;
            }
            if (! (*it)->IsSubType(last)) {
                return false;
            }
            last = *it;
        }

        _typeRdefs[derived] = rdef;

        return true;
    }

    bool initNoFollows(const NoFollowPathsSet &noFollows)
    {
        for (unsigned i = 0; i < noFollows.size(); ++i) {
            if (i > 0) {
                const NoFollowPaths &p = noFollows[i - 1]; // previous
                const NoFollowPaths &c = noFollows[i];     // current
                if (! std::includes(p.begin(), p.end(), c.begin(), c.end())) {
                    return false;
                }
            }
        }
        _noFollows = noFollows;

        return true;
    }

    bool addPathLevelSwitch(
        const lilfes::feature *feature,
        const std::vector<unsigned> &levels
    ) {
        if (levels.empty()) { /// we assume this case means levels = [0,1,2,...]
            return true;
        }

        for (unsigned i = 0; i < levels.size(); ++i) {
            if (levels[i] > i) {
                return false;
            }
            if (i > 0 && levels[i - 1] > levels[i]) {
                return false;
            }
        }

        _pathLevelSwitches[feature] = levels;

        return true;
    }

    bool initMaxDepthDef(const std::vector<unsigned> &def)
    {
        unsigned last = 0;
        for (unsigned i = 0; i < def.size(); ++i) {
            if (last > def.size()) {
                return false;
            }
            last = def[i];
        }
        _maxDepthDef = def;
        return true;
    }

    void addDepthIncPath(
        const lilfes::type *type,
        const lilfes::feature *feature,
        unsigned inc
    ) {
        _depthIncPaths[std::make_pair(type, feature)] = inc;
    }

    bool initDepthLevelMap(const DepthLevelMap &dlm)
    {
        for (unsigned i = 0; i < dlm.size(); ++i) {
            assert(! dlm[i].empty());

            unsigned rowLast = i;
            for (unsigned j = 0; j < dlm[i].size(); ++j) {
                if (rowLast < dlm[i][j]) {
                    return false;
                }
                rowLast = dlm[i][j];

                unsigned colLast = (i == 0) ? 0 :
                                   (j < dlm[i-1].size()) ? dlm[i-1][j] : dlm[i-1].back();
                if (colLast > dlm[i][j]) {
                    return false;
                }
            }
        }
        _depthLevelMap = dlm;
        return true;
    }

    void setError(void)
    {
        _defError = true;
    }

    void addPattern(const Fs &pattern, const std::vector<const lilfes::feature*> &path, unsigned level)
    {
        for (unsigned i = 0; i < _patterns.size(); ++i) {
            if (path == _patterns[i]._path && level == _patterns[i]._level) {
                _patterns[i]._pattern.push_back(pattern);
                return;
            }
        }
        _patterns.push_back(Pattern(pattern, path, level));
    }

    void setMachine(lilfes::machine *mach)
    {
        _mach = mach;
    }

private:
    typedef std::map<lilfes::core_p, lilfes::core_p> NodeMap;
    typedef std::map<lilfes::core_p, unsigned> LevelMap;
    typedef std::map<lilfes::core_p, unsigned> DepthMap;

public:
    /// Restriction
    bool restrict(
        lilfes::machine &machine,
        unsigned level,
        lilfes::FSP in,
        lilfes::FSP out
    ) {
        if (_defError) {
            return false;
        }

        lilfes::FSP canonical = in.CanonicalCopy();

        LevelMap specialNodes;
        checkPattern(canonical, specialNodes);

        DepthMap depthMap;
        makeDepthMap(0, canonical, depthMap);

        LevelMap levelMap;
        const std::vector<unsigned> &depthToLevel
            = (level < _depthLevelMap.size()) ? _depthLevelMap[level] : std::vector<unsigned>(1, level);
        makeLevelMap(level, canonical, depthMap, specialNodes, depthToLevel, levelMap);

        NodeMap visit;
        lilfes::FSP outNonCanonical(machine);
        restrictItr(machine, canonical, outNonCanonical, visit, levelMap, depthMap);

        out.Unify(outNonCanonical.CanonicalCopy());

        return true;
    }

    bool restrict(
        lilfes::machine &machine,
        unsigned level,
        lilfes::FSP in,
        Fs &out
    ) {
        lilfes::FSP outFSP(machine);

        if (! restrict(machine, level, in, outFSP)) {
            return false;
        }

        outFSP.Serialize(out);
        return true;
    }

private:
    void checkPattern(lilfes::FSP canonical, LevelMap &patternResult)
    {
        for (PatternSet::const_iterator p = _patterns.begin(); p != _patterns.end(); ++p) {
            bool match = false;
            for (std::vector<Fs>::const_iterator pp = p->_pattern.begin();
                 ! match && pp != p->_pattern.end(); ++pp) {

                _mach->SetTrailPoint();
                lilfes::FSP pattern(_mach, *pp);
                match = canonical.Unify(pattern);
                _mach->TrailBack();
            }

            if (! match) {
                continue;
            }

            lilfes::FSP curr = canonical.Deref();
            std::vector<const lilfes::feature*>::const_iterator f = p->_path.begin();
            while (f != p->_path.end()) {
                const lilfes::type *type = curr.GetType();
                unsigned nfeatures = type->GetNFeatures();
                bool followed = false;
                for (unsigned i = 0; i < nfeatures; ++i) {
                    if (type->Feature(i) == *f) {
                        curr = curr.FollowNth(i);
                        followed = true;
                        break;
                    }
                }
                if (followed) {
                    ++f;
                }
                else {
                    break;
                }
            }
            if (f == p->_path.end()) {
                lilfes::core_p addr = curr.Deref().GetAddress();
                LevelMap::iterator it = patternResult.insert(std::make_pair(addr, p->_level)).first;
                it->second = std::min(it->second, p->_level);
            }
        }
    }

    void makeDepthMap(unsigned depth, lilfes::FSP in, DepthMap &m)
    {
        lilfes::FSP derefed = in.Deref();
        std::pair<DepthMap::iterator, bool> ib = m.insert(std::make_pair(derefed.GetAddress(), depth));

        if (! ib.second && ib.first->second <= depth) { /// already visited with lower or equal depth
            return;
        }

        ib.first->second = depth;

        if (derefed.IsLeaf()) {
            return;
        }

        const lilfes::type *type = derefed.GetType();
        int nfeatures = type->GetNFeatures();
        for (int i = 0; i < nfeatures; ++i) {
            const lilfes::feature *f = type->Feature(i);

            DepthIncPaths::const_iterator it = _depthIncPaths.find(std::make_pair(type, f));
            unsigned inc = (it != _depthIncPaths.end()) ? it->second : 0;

            makeDepthMap(depth + inc, derefed.FollowNth(i), m);
        }
    }

    void makeLevelMap(
        unsigned level,
        lilfes::FSP in,
        const DepthMap &depthMap,
        const LevelMap &specialNodes,
        const std::vector<unsigned> &depthToLevel,
        LevelMap &m
    ) {
        lilfes::FSP derefed = in.Deref();

        LevelMap::const_iterator special = specialNodes.find(derefed.GetAddress());
        if (special != specialNodes.end()) {
            level = std::min(level, special->second);
        }

        std::pair<LevelMap::iterator, bool> ib = m.insert(std::make_pair(derefed.GetAddress(), level));

        if (! ib.second && ib.first->second >= level) { /// already visited with higher or equal level
            return;
        }

        DepthMap::const_iterator depthItr = depthMap.find(derefed.GetAddress());
        assert(depthItr != depthMap.end());
        unsigned depth = depthItr->second;

        unsigned newLevel = std::min(level, (depth < depthToLevel.size()) ? depthToLevel[depth] : depthToLevel.back());

        ib.first->second = newLevel;

        if (derefed.IsLeaf()) {
            return;
        }

        const lilfes::type *tin = derefed.GetType();
        const lilfes::type *tout = findOutType(newLevel, tin);

        int nfeatures = tin->GetNFeatures();
        for (int i = 0; i < nfeatures; ++i) {
            const lilfes::feature *f = tin->Feature(i);

            if (tout->GetIndex(f) == -1) {
                continue;
            }

            if (isNoFollow(level, f)) {
                continue;
            }

            makeLevelMap(findNextLevel(newLevel, f), derefed.FollowNth(i), depthMap, specialNodes, depthToLevel, m);
        }
    }

    const lilfes::type *findOutType(
        unsigned level, const lilfes::type *inType
    ) {
        TypeRdefSet::const_iterator titr = _typeRdefs.find(inType);
        if (titr == _typeRdefs.end() || titr->second.empty()) {
            /// default is to copy the type
            return inType;
        }
        else {
            const TypeRdef &ts = titr->second;
            return (level < ts.size()) ? ts[level] : ts.back();
        }
    }

    bool isNoFollow(unsigned level, const lilfes::feature *f)
    {
        if (level < _noFollows.size()) {
            if (_noFollows[level].find(f) != _noFollows[level].end()) {
                return true;
            }
        }
        else if (! _noFollows.empty()) {
            if (_noFollows.back().find(f) != _noFollows.back().end()) {
                return true;
            }
        }

        return false;
    }

    bool exceedMaxDepth(unsigned depth, unsigned level)
    {
        if (level >= _maxDepthDef.size()) {
            return false;
        }
        else {
            return depth > _maxDepthDef[level];
        }
    }

    unsigned findNextLevel(unsigned level, const lilfes::feature *f)
    {
        /// assert: level-switches are not empty
        PathLevelSwitchSet::const_iterator it = _pathLevelSwitches.find(f);
        if (it != _pathLevelSwitches.end() && level < it->second.size()) {
            return it->second[level];
        }
        else {
            return level;
        }
    }

    void restrictItr(
        lilfes::machine &machine,
        lilfes::FSP in,
        lilfes::FSP out,
        NodeMap &map,
        const LevelMap &levelMap,
        const DepthMap &depthMap
    ) {
        lilfes::FSP derefed = in.Deref();
        std::pair<NodeMap::iterator, bool> ib = map.insert(std::make_pair(derefed.GetAddress(), out.GetAddress()));

        if (! ib.second) {  /// visited node
            if (! out.Unify(lilfes::FSP(machine, ib.first->second))) {
                throw std::logic_error("unify failed");
            }
            return;
        }

        LevelMap::const_iterator levelItr = levelMap.find(derefed.GetAddress());
        assert(levelItr != levelMap.end());
        unsigned level = levelItr->second;

        /// unseen node
        const lilfes::type *tin = derefed.GetType();
        const lilfes::type *tout = findOutType(level, tin);

        /// PENDING: data leaves
        if (lilfes::IsDAT(derefed.ReadCell())) {
            if (! out.Unify(derefed.Copy())) {
                throw std::logic_error("data copy failed");
            }
        }
        else if (! out.Coerce(tout)) {
            throw std::logic_error(std::string("coercion failed: orig type: ") + out.GetType()->GetSimpleName()
                                   + ", outtype: " + tout->GetSimpleName());
        }
        if (derefed.IsLeaf()) {
            return;
        }

        DepthMap::const_iterator depthItr = depthMap.find(derefed.GetAddress());
        assert(depthItr != depthMap.end());
        unsigned depth = depthItr->second;

        int nfeatures = tout->GetNFeatures();
        for (int i = 0; i < nfeatures; ++i) {
            const lilfes::feature *f = tout->Feature(i);

            if (isNoFollow(level, f)) {
                continue;
            }

            DepthIncPaths::const_iterator it = _depthIncPaths.find(std::make_pair(tin, f));
            unsigned inc = (it != _depthIncPaths.end()) ? it->second : 0;
            if (exceedMaxDepth(depth + inc, level)) {
                continue;
            }

            /// recursion
            restrictItr(machine, derefed.Follow(f), out.Follow(f), map, levelMap, depthMap);
        }
    }

public: /// Predicate classes
    template<int Arity>
    class DefErrorCheck : public lilfes::Pred<Arity> {
    public:
        DefErrorCheck(const std::string &name) : lilfes::Pred<Arity>(name) {}
        bool exec(lilfes::machine &m, lilfes::FSP argv[])
        {
            if (execImpl(m, argv)) {
                return true;
            }
            else {
                Restrictor::getInstance()->setError();
                return false;
            }
        }

        virtual bool execImpl(lilfes::machine &m, lilfes::FSP argv[]) = 0;
    };

    class AddTypeRdef : public DefErrorCheck<2> {
    public:
        AddTypeRdef(void) : DefErrorCheck<2>("type_rdef") {}

		bool execImpl(lilfes::machine &m, lilfes::FSP argv[])
		{
            const lilfes::type *derived = argv[0].GetType();

            /// list of types
            std::vector<lilfes::FSP> ts;
            if (! lilfes::list_to_vector(m, argv[1], ts)) {
                std::cerr << "second argument of type_rdef/2 must be a list of types" << std::endl;
                return false;
            }

            TypeRdef typeRdef;
            for (std::vector<lilfes::FSP>::const_iterator t = ts.begin(); t != ts.end(); ++t) {
                typeRdef.push_back(t->GetType());
            }

            if (! Restrictor::getInstance()->addTypeRdef(derived, typeRdef)) {
                std::cerr
                    << "second argument of type_rdef/2 must be a general-to-specific chain of types "
                    << "(leading to the first argument)" << std::endl;
                return false;
            }

			return true;
		}
    };

	class PathRdef : public DefErrorCheck<1> {
	public:
		PathRdef(void) : DefErrorCheck<1>("path_rdef") {}

		bool execImpl(lilfes::machine &m, lilfes::FSP argv[])
		{
            /// list of lists of features
            std::vector<std::vector<lilfes::FSP> > fss;
            if (! lilfes::list_to_vector(m, argv[0], fss)) {
                std::cerr << "first argument of path_rdef/1 must be a list of lists of features" << std::endl;
                return false;
            }

            NoFollowPathsSet noFollows(fss.size());
            for (unsigned i = 0; i < fss.size(); ++i) {
                for (std::vector<lilfes::FSP>::const_iterator it = fss[i].begin(); it != fss[i].end(); ++it) {
                    const lilfes::feature *f = lilfes::feature::Search(it->GetType());
                    if (! f) {
                        std::cerr << "first argument of path_rdef/1 must be a list of lists of features" << std::endl;
                        return false;
                    }
                    noFollows[i].insert(f);
                }
            }

            if (! Restrictor::getInstance()->initNoFollows(noFollows)) {
                std::cerr << "first argument of path_rdef/1 must form a decreasing chain of "
                          << "sets of features" << std::endl;
                return false;
            }

			return true;
		}
	};

	class PathSdef : public DefErrorCheck<2> {
	public:
		PathSdef(void) : DefErrorCheck<2>("path_sdef") {}

		bool execImpl(lilfes::machine &m, lilfes::FSP argv[])
		{
            const lilfes::feature *feature = lilfes::feature::Search(argv[0].GetType());
            if (! feature) {
                std::cerr << "first argument of path_sdef/2 must be a name of feature" << std::endl;
                return false;
            }

            std::vector<int> levels;
            if (! lilfes::list_to_vector(m, argv[1], levels)) {
                std::cerr << "second argument of path_sdef/2 must be a list of non-negative integers"
                    << std::endl;
                return false;
            }

            std::vector<unsigned> ulevels;
            for (std::vector<int>::const_iterator it = levels.begin(); it != levels.end(); ++it) {
                if (*it < 0) {
                    std::cerr << "second argument of path_sdef/2 must be a list of non-negative integers"
                        << std::endl;
                    return false;
                }
                ulevels.push_back(static_cast<unsigned>(*it));
            }

            if (! Restrictor::getInstance()->addPathLevelSwitch(feature, ulevels)) {
                std::cerr << "second argument of path_sdef/2 must be an increasing sequence of non-negative "
                          << "integers and each element must be less than or equal to its index number"
                          << std::endl;
                return false;
            }

			return true;
		}
	};

    class MaxDepth : public DefErrorCheck<1> {
    public:
        MaxDepth(void) : DefErrorCheck<1>("max_depth") {}

		bool execImpl(lilfes::machine &m, lilfes::FSP argv[])
		{
            std::vector<int> depths;
            if (! lilfes::list_to_vector(m, argv[0], depths)) {
                std::cerr << "argument of max_depth/1 must be a list of non-negative integers" << std::endl;
                return false;
            }

            std::vector<unsigned> udepths;
            for (std::vector<int>::const_iterator it = depths.begin(); it != depths.end(); ++it) {
                if (*it < 0) {
                    std::cerr << "argument of max_depth/1 must be a list of non-negative integers" << std::endl;
                    return false;
                }
                udepths.push_back(static_cast<unsigned>(*it));
            }

            if (! Restrictor::getInstance()->initMaxDepthDef(udepths)) {
                std::cerr << "argument of max_depth/1 must be an increasing sequence of non-negative integers"
                          << std::endl;
                return false;
            }

			return true;
		}
    };

    class DepthIncFeatDef : public DefErrorCheck<3> {
    public:
        DepthIncFeatDef(void) : DefErrorCheck<3>("depth_inc_feat") {}

		bool execImpl(lilfes::machine &, lilfes::FSP argv[])
		{
            const lilfes::type *type = argv[0].GetType();
            const lilfes::feature *feature = lilfes::feature::Search(argv[1].GetType());
            if (! feature) {
                std::cerr << "second argument of depth_inc_feat/3 must be a name of feature" << std::endl;
                return false;
            }

            int inc = argv[2].ReadInteger();
            if (inc <= 0) {
                std::cerr << "third argument of depth_inc_feat/3 must be a positive integer" << std::endl;
                return false;
            }

            Restrictor::getInstance()->addDepthIncPath(type, feature, inc);

			return true;
        }
    };

	class DepthLevelMapDef : public DefErrorCheck<1> {
	public:
		DepthLevelMapDef(void) : DefErrorCheck<1>("depth_ldef") {}

		bool execImpl(lilfes::machine &m, lilfes::FSP argv[])
		{
            /// list of lists of integers
            std::vector<std::vector<int> > nss;
            if (! lilfes::list_to_vector(m, argv[0], nss)) {
                std::cerr << "first argument of depth_ldef/1 must be a list of lists of integers" << std::endl;
                return false;
            }

            std::vector<std::vector<unsigned> > uss(nss.size());
            for (unsigned i = 0; i < nss.size(); ++i) {
                for (unsigned j = 0; j < nss[i].size(); ++j) {
                    if (nss[i][j] < 0) {
                        std::cerr << "first argument of depth_ldef/1 must be a list of lists of non-negative integers"
                                  << std::endl;
                        return false;
                    }
                    uss[i].push_back(nss[i][j]);
                }
                if (uss[i].empty()) {
                    uss[i].push_back(i); // -> equivalent to [i, i, ... ]
                }
            }

            if (! Restrictor::getInstance()->initDepthLevelMap(uss)) {
                std::cerr << "first argument of depth_ldef/1 must be a list of increasing sequence of "
                          << "non-negative integers" << std::endl;
                return false;
            }

			return true;
		}
	};

    class RestrictionPattern : public DefErrorCheck<3> {
    public:
        RestrictionPattern(void) : DefErrorCheck<3>("restriction_pattern") {}

		bool execImpl(lilfes::machine &m, lilfes::FSP argv[])
		{
            Fs pattern;
            argv[0].Serialize(pattern);

            /// list of features
            std::vector<lilfes::FSP> fs;
            if (! lilfes::list_to_vector(m, argv[1], fs)) {
                std::cerr << "second argument of restriction_pattern/3 must be a lists of features" << std::endl;
                return false;
            }

            std::vector<const lilfes::feature*> path;
            for (std::vector<lilfes::FSP>::const_iterator it = fs.begin(); it != fs.end(); ++it) {
                const lilfes::feature *f = lilfes::feature::Search(it->GetType());
                if (! f) {
                    std::cerr << "second argument of restriction_pattern/3 must be a lists of features" << std::endl;
                    return false;
                }
                path.push_back(f);
            }

            // level
            if (! argv[2].IsInteger() || argv[2].ReadInteger() < 0) {
                std::cerr << "third argument of restriction_pattern/3 must be a non-negative integer" << std::endl;
            }
            unsigned level = argv[2].ReadInteger();

            Restrictor::getInstance()->addPattern(pattern, path, level);

			return true;
		}
    };

    /// for debug
    class Dump : public lilfes::Pred<0> {
    public:
        Dump(void) : lilfes::Pred<0>("dump_restrictor_def") {}
		bool exec(lilfes::machine &)
        {
            Restrictor::getInstance()->dump(std::cerr);
            return true;
        }
    };

    /// restrict_sign(+$Level, +$InFs, -$OutFs)
    class RestrictSign : public lilfes::Pred<3> {
    public:
        RestrictSign(void) : lilfes::Pred<3>("restrict_sign") {}
		bool exec(lilfes::machine &machine, lilfes::FSP argv[])
        {
            P0("restrict_sign");

            if (! argv[0].IsInteger()) {
                std::cerr << "first argument of restrict_sign/3 must be an integer"
                          << std::endl;
                return false;
            }

            int level = argv[0].ReadInteger();
            if (level < 0) {
                std::cerr << "first argument of restrict_sign/3 must be a non-negative integer"
                         << " but got " << level
                         << std::endl;
                return false;
                // level = 0;
            }

            lilfes::FSP out(machine);
            if (! Restrictor::getInstance()->restrict(machine, level, argv[1], out)) {
                std::cerr << "restrict_sign/3: error in the restrictor definition" << std::endl;
                return false;
            }

            return argv[2].Unify(out);
            // return argv[2].Unify(out.CanonicalCopy());
        }
    };

    template<int Arity>
    static void makeBuiltin(lilfes::CppPredImpl::PredBase<Arity> *pred)
    {
        lilfes::type *t = new lilfes::type(pred->PredName(), lilfes::module::BuiltinModule());
        t->SetAsChildOf(lilfes::t_pred[Arity]);
        t->Fix();
        lilfes::proc_builtin *proc = new lilfes::proc_builtin(t, Arity, pred);
        proc = 0; /// dummy to stop warning from compliers
    }

    static void init(lilfes::machine *mach)
    {
#if 0
        lilfes::ModuleDecl mod("restrictor");
        lilfes::Registrator reg0(mod, new Restrictor::AddTypeRdef());
        lilfes::Registrator reg1(mod, new Restrictor::PathRdef());
        lilfes::Registrator reg2(mod, new Restrictor::PathSdef());
        lilfes::Registrator reg3(mod, new Restrictor::Dump());
        lilfes::Registrator reg4(mod, new Restrictor::RestrictSign());
        lilfes::Registrator reg5(mod, new Restrictor::MaxDepth());
        lilfes::Registrator reg6(mod, new Restrictor::DepthIncFeatDef());
        lilfes::Registrator reg7(mod, new Restrictor::DepthLevelMapDef());
#endif
        Restrictor::getInstance()->setMachine(mach);

        makeBuiltin<2>(new Restrictor::AddTypeRdef());
        makeBuiltin<1>(new Restrictor::PathRdef());
        makeBuiltin<2>(new Restrictor::PathSdef());
        //makeBuiltin<>(new Restrictor::Dump());
        makeBuiltin<3>(new Restrictor::RestrictSign());
        makeBuiltin<1>(new Restrictor::MaxDepth());
        makeBuiltin<3>(new Restrictor::DepthIncFeatDef());
        makeBuiltin<1>(new Restrictor::DepthLevelMapDef());
        makeBuiltin<3>(new Restrictor::RestrictionPattern());
    }
};


