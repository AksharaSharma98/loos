/*
  This file is part of LOOS.

  LOOS (Lightweight Object-Oriented Structure library)
  Copyright (c) 2008, Tod D. Romo, Alan Grossfield
  Department of Biochemistry and Biophysics
  School of Medicine & Dentistry, University of Rochester

  This package (LOOS) is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation under version 3 of the License.

  This package is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include <pdb.hpp>
#include <utils.hpp>
#include <Fmt.hpp>

#include <iomanip>
#include <tr1/unordered_set>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>


namespace loos {

  // Assume we're only going to find spaces in a PDB file...
  bool PDB::emptyString(const std::string& s) {
    std::string::const_iterator i;

    for (i = s.begin(); i != s.end(); ++i)
      if (*i != ' ')
        return(false);

    return(true);
  }


  // Special handling for REMARKs to ignore the line code, if
  // present... 

  void PDB::parseRemark(const std::string& s) {
    std::string t;

    if (s[6] == ' ' && isdigit(s[7]))
      t = s.substr(11, 58);
    else
      t = s.substr(7, 62);

    _remarks.add(t);
  }


  // Parse an ATOM or HETATM record...

  void PDB::parseAtomRecord(const std::string& s) {
    greal r;
    gint i;
    std::string t;
    GCoord c;
    pAtom pa(new Atom);
  
    t = parseStringAs<std::string>(s, 0, 6);
    pa->recordName(t);

    i = parseStringAs<int>(s, 6, 5);
    pa->id(i);

    t = parseStringAs<std::string>(s, 12, 4);
    pa->name(t);

    t = parseStringAs<std::string>(s, 16, 1);
    pa->altLoc(t);

    t = parseStringAs<std::string>(s, 17, 4);
    pa->resname(t);

    t = parseStringAs<std::string>(s, 21, 1);
    pa->chainId(t);

    i = parseStringAs<int>(s, 22, 4);
    pa->resid(i);
  
    t = parseStringAs<std::string>(s, 26, 1);

    // Special handling of resid field since it may be frame-shifted by
    // 1 col in some cases...
    if (strictness_policy) {
      char c = t[0];
    
      if (c != ' ' && !isalpha(c))
        throw(std::runtime_error("Non-alpha character in iCode column of PDB"));
    } else {
      char c = t[0];

      if (c != ' ' && isdigit(c)) {
        i = parseStringAs<int>(s, 22, 5);
        pa->resid(i);
        t = " ";
      }

    }
    pa->iCode(t);

    c[0] = parseStringAs<float>(s, 30, 8);
    c[1] = parseStringAs<float>(s, 38, 8);
    c[2] = parseStringAs<float>(s, 46, 8);
    pa->coords(c);
  
    r = parseStringAs<float>(s, 54, 6);
    pa->occupancy(r);

    r = parseStringAs<float>(s, 60, 6);
    pa->bfactor(r);

    t = parseStringAs<std::string>(s, 72, 4);
    pa->segid(t);

    t = parseStringAs<std::string>(s, 76, 2);
    pa->PDBelement(t);


    // Charge is not currently handled...
    t = parseStringAs<std::string>(s, 78, 2);

    append(pa);
  }



  // Convert an Atom to a string with a PDB format...

  std::string PDB::atomAsString(const pAtom p) const {
    std::ostringstream s;

    // Float formatter for coords
    Fmt crdfmt(3);
    crdfmt.width(8);
    crdfmt.right();
    crdfmt.trailingZeros(true);
    crdfmt.fixed();

    // Float formatter for B's and Q's...
    Fmt bqfmt(2);
    bqfmt.width(6);
    bqfmt.right();
    bqfmt.trailingZeros(true);
    bqfmt.fixed();

    // We don't worry about strings exceeding field-widths (yet),
    // but do check for numeric overflows...
    s << std::setw(6) << std::left << p->recordName();
    s << std::setw(5) << std::right << fixedSizeFormat(p->id(), 5) << " ";
    s << std::setw(4) << std::left << p->name();

    s << std::setw(1) << p->altLoc();
    s << std::setw(4) << std::left << p->resname();
    
    s << std::setw(1) << std::right << p->chainId();
    s << std::setw(4) << fixedSizeFormat(p->resid(), 4);
    s << std::setw(2) << p->iCode();
    s << "  ";
        
    s << crdfmt(p->coords().x());
    s << crdfmt(p->coords().y());
    s << crdfmt(p->coords().z());
    s << bqfmt(p->occupancy());
    s << bqfmt(p->bfactor());
    s << "      ";
    s << std::setw(4) << std::left << p->segid();
    s << std::setw(2) << std::right << p->PDBelement();
    if (_show_charge)
      s << std::setw(2) << p->charge();
    else
      s << "  ";

    return(s.str());
  }


  // Parse CONECT records, updating the referenced atoms...
  // Couple of issues:
  //
  //    Will accept up to 8 bound atoms and considers them all equal,
  // but the PDB standard says some are h-bonded and some are
  // salt-bridged... 
  //
  //    No check is made for overflow of fields...

  void PDB::parseConectRecord(const std::string& s) {
    int bound_id = parseStringAs<int>(s, 6, 5);
    pAtom bound = findById(bound_id);
    if (bound == 0)
      throw(PDB::BadConnectivity("Cannot find primary atom " + s.substr(6, 5)));

    // This currently includes fields designated as H-bond indices...
    // Should we do this? or separate them out?  Hmmm...
    for (int i=0; i<8; ++i) {
      int j = i * 5 + 11;
      std::string t = s.substr(j, 5);
      if (emptyString(t))
        break;
      int id = parseStringAs<int>(t);
      pAtom boundee = findById(id);
      if (boundee == 0)
        throw(PDB::BadConnectivity("Cannot find bound atom " + t));
      bound->addBond(boundee);
    }
  }


  void PDB::parseCryst1Record(const std::string& s) {
    greal r;
    gint i;
    std::string t;

    r = parseStringAs<float>(s, 6, 9);
    cell.a(r);
    r = parseStringAs<float>(s, 15, 9);
    cell.b(r);
    r = parseStringAs<float>(s, 24, 9);
    cell.c(r);

    r = parseStringAs<float>(s, 33, 7);
    cell.alpha(r);
    r = parseStringAs<float>(s, 40, 7);
    cell.beta(r);
    r = parseStringAs<float>(s, 47, 7);
    cell.gamma(r);

    // Special handling in case of mangled CRYST1 record...
    if (s.length() < 66) {
      t = s.substr(55);
      cell.spaceGroup(t);
      cell.z(-1);   // ??? 
    } else {
      t = parseStringAs<std::string>(s, 55, 11);
      cell.spaceGroup(t);
      i = parseStringAs<int>(s, 66, 4);
      cell.z(i);
    }

    _has_cryst = true;
  }


  //! Top level parser...
  //! Reads a PDB from an input stream
  void PDB::read(std::istream& is) {
    std::string input;
    bool has_cryst = false;
    std::tr1::unordered_set<std::string> seen;

    while (getline(is, input)) {
      if (input.substr(0, 4) == "ATOM" || input.substr(0,6) == "HETATM")
        parseAtomRecord(input);
      else if (input.substr(0, 6) == "REMARK")
        parseRemark(input);
      else if (input.substr(0,6) == "CONECT")
        parseConectRecord(input);
      else if (input.substr(0, 6) == "CRYST1") {
        parseCryst1Record(input);
        has_cryst = true;
      } else if (input.substr(0,3) == "TER")
        ;
      else if (input.substr(0,3) == "END")
        break;
      else {
        int space = input.find_first_of(' ');
        std::string record = input.substr(0, space);
        if (seen.find(record) == seen.end()) {
          std::cerr << "Warning - unknown PDB record " << record << std::endl;
          seen.insert(record);
        }
      }
    }

    // Do some post-extraction...
    if (loos::remarksHasBox(_remarks)) {
      GCoord c = loos::boxFromRemarks(_remarks);
      periodicBox(c);
    } else if (has_cryst) {
      GCoord c(cell.a(), cell.b(), cell.c());
      periodicBox(c);
    }

    // Force atom id's to be monotonic if there was an overflow event...
    if (atoms.size() >= 100000)
      renumber();
  }


  std::ostream& FormattedUnitCell(std::ostream& os, const UnitCell& u) {
    os << "CRYST1";
    Fmt dists(3);
    dists.width(9).right().trailingZeros(true).fixed();
    Fmt angles(2);
    angles.width(7).right().trailingZeros(true).fixed();

    os << dists(u.a()) << dists(u.b()) << dists(u.c());
    os << angles(u.alpha()) << angles(u.beta()) << angles(u.gamma());
    os << " " << std::setw(10) << std::left << u.spaceGroup() << std::setw(4) << u.z();

    return(os);
  }

  std::ostream& XTALLine(std::ostream& os, const GCoord& box) {
    os << "REMARK  XTAL "
       << box.x() << " "
       << box.y() << " "
       << box.z();
    return(os);
  }


  std::ostream& FormatConectRecords(std::ostream& os, PDB& p) {
    AtomicGroup::iterator ci;

    // We first have to make sure that the base AtomicGroup is sorted
    // since we will be verifying bound atoms exist by searching for
    // their ID...this would force a sort while we have iterators
    // pointing to the vector of atoms which would be bad...
    p.sort();
  
    for (ci = p.atoms.begin(); ci != p.atoms.end(); ++ci) {
      if ((*ci)->checkProperty(Atom::bondsbit)) {
        int donor = (*ci)->id();

        os << boost::format("CONECT%5d") % donor;
        int i = 0;

        std::vector<int> bonds = (*ci)->getBonds();
        std::vector<int>::const_iterator cj;
        for (cj = bonds.begin(); cj != bonds.end(); ++cj) {
          if (++i > 4) {
            i = 1;
            os << boost::format("\nCONECT%5d") % donor;
          }
          int bound_id = *cj;
          pAtom pa = p.findById(bound_id);
          if (pa != 0)
            os << boost::format("%5d") % bound_id;
        }
        os << std::endl;
      }
    }

    return(os);
  }


  //! Output the group as a PDB...
  /**
   * There are some formatting changes that occur when the group has a
   * large number of atoms or resids.  The most significant is when
   * you have 100,000 or more, in which case you lose the altloc and
   * chainid fields on output.  However, the output PDB will load into
   * pymol...
   *
   */
  std::ostream& operator<<(std::ostream& os, PDB& p) {
    AtomicGroup::iterator i;

    os << p._remarks;
    if (p.isPeriodic())
      XTALLine(os, p.periodicBox()) << std::endl;
    if (p._has_cryst) 
      FormattedUnitCell(os, p.cell) << std::endl;
    for (i = p.atoms.begin(); i != p.atoms.end(); ++i)
      os << p.atomAsString(*i) << std::endl;

    if (p.hasBonds()) {
      int maxid = 0;
      for (i = p.atoms.begin(); i != p.atoms.end(); ++i)
        if ((*i)->id() > maxid)
          maxid = (*i)->id();

      if (maxid <= 99999)
        FormatConectRecords(os, p);
    }
  
    if (p._auto_ter)
      os << "TER     \n";

    return(os);
  }

  PDB PDB::copy(void) const {
    AtomicGroup grp = this->AtomicGroup::copy();
    PDB p(grp);
    
    p._show_charge = _show_charge;
    p._auto_ter = _auto_ter;
    p._has_cryst = _has_cryst;
    p._remarks = _remarks;
    p.cell = cell;
    
    return(p);
  }

  PDB* PDB::clone(void) const {
    return(new PDB(*this));
  }

  PDB PDB::fromAtomicGroup(const AtomicGroup& g) {
    PDB p(g);
    
    return(p);
  }


  bool PDB::showCharge(void) const { return(_show_charge); }
  void PDB::showCharge(bool b) { _show_charge = b; }
  bool PDB::strict(void) const { return(strictness_policy); }
  void PDB::strict(const bool b) { strictness_policy = b; }

  bool PDB::autoTerminate(void) const { return(_auto_ter); }

  void PDB::autoTerminate(bool b) { _auto_ter = b; }

  Remarks& PDB::remarks(void) { return(_remarks); }
  void PDB::remarks(const Remarks& r) { _remarks = r; }

  UnitCell& PDB::unitCell(void) { return(cell); }
  void PDB::unitCell(const UnitCell& c) { cell = c; }



}
