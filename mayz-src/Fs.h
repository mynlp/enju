/*
 * $Id: Fs.h,v 1.3 2011-05-02 08:35:20 matuzaki Exp $
 *
 *  Copyright (c) 2005, Tsujii Laboratory, The University of Tokyo.
 *  All rights reserved.
 *
 */

#ifndef __Fs_h
#define __Fs_h

#include <liblilfes/cell.h>
#include <vector>

namespace up {
    
    typedef std::vector<lilfes::cell> Fs;
    
    // TODO: Replace Fs with LilfesFs
    class LilfesFs {
    public:
        LilfesFs(void) {} // safe?
        LilfesFs(const LilfesFs &fs) : cells(fs.cells) {}
        LilfesFs(lilfes::FSP fsp) { fsp.Serialize(cells); }

        lilfes::FSP makeFSP(lilfes::machine *machine) const { return lilfes::FSP(machine, cells); }

        size_t size() const { return cells.size(); }
        void clear() { return cells.clear(); }

        bool operator==(const LilfesFs &fs) const { return cells == fs.cells; }

    private:
        std::vector<lilfes::cell> cells;
    };
    
}

#endif // __Fs_h

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.2  2009/01/06 11:35:31  matuzaki
 *
 * fix for parsing with up
 *
 * fix for making 'up -nofom' work
 *
 * Revision 1.1.1.1  2007/04/17 12:12:25  yusuke
 *
 * Start Enju 2.2
 *
 *
 * Revision 1.3  2005/06/21 10:17:12  yusuke
 *
 * copyright を書きました．
 *
 * Revision 1.2  2005/06/16 07:25:36  yusuke
 *
 * integrate-up を統合．
 * version 2.0
 *
 * Revision 1.1.2.1  2005/06/05 10:12:18  ninomi
 * upをmayz/srcの下に移しはじめました。
 *
 * Revision 1.2  2004/11/16 09:13:54  ninomi
 * Artistic Licenseを頭につけました。
 *
 * Revision 1.1  2004/06/30 07:50:09  ninomi
 * up 0.2のrelease versionです。
 *
 */
