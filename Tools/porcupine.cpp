/*
  porcupine
  
  (c) 2009 Tod D. Romo, Grossfield Lab
  Department of Biochemistry
  University of Rochester School of Medicine and Dentistry



  Creates "porcupine" plots by placing atoms at the endpoints of the
  vectors and adding a bond between them.  This is all written out a
  PDB file with CONECT records.

  Notes:

    The eigenvectors are taken to be column-vectors from the
  input matrix.  Since SVD's are often computed over subsets of a
  structure, the eigenvectors can be mapped back onto the appropriate
  atoms from a larger model by using a map file.  This is just a list
  of the eigenvector id's (i.e. column #'s) and the corresponding
  atomid's.  For example:
     0     30
     1     42
     2     57
     3     66

*/



/*

  This file is part of LOOS.

  LOOS (Lightweight Object-Oriented Structure library)
  Copyright (c) 2009, Tod D. Romo
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



#include <loos.hpp>
#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <limits>

using namespace std;
using namespace loos;


namespace po = boost::program_options;
typedef Math::Matrix<float, Math::ColMajor>   Matrix;


string vec_name;
vector<uint> cols;
vector<double> scales;
double global_scale;
bool uniform;
bool double_sided;
string model_name;
string map_name;

const string porcupine_tag("POR");


void parseOptions(int argc, char *argv[]) {

  try {
    vector<string> strings;

    po::options_description generic("Allowed options");
    generic.add_options()
      ("help", "Produce this help message")
      ("columns,c", po::value< vector<string> >(&strings), "Columns to use")
      ("scale,s", po::value< vector<double> >(&scales), "Scale the requested columns")
      ("global,g", po::value<double>(&global_scale)->default_value(1.0), "Global scaling")
      ("uniform,u", "Scale all elements uniformly")
      ("map,M", po::value<string>(&map_name), "Use a map file to map LSV/eigenvectors to atomids")
      ("double_sided,d", "Use double-sided vectors");

    po::options_description hidden("Hidden options");
    hidden.add_options()
      ("model", po::value<string>(&model_name), "Model name")
      ("lsv", po::value<string>(&vec_name), "Left singular vector matrix");

    po::options_description command_line;
    command_line.add(generic).add(hidden);

    po::positional_options_description p;
    p.add("model", 1);
    p.add("lsv", 1);

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).
              options(command_line).positional(p).run(), vm);
    po::notify(vm);

    if (vm.count("help") || !(vm.count("model") && vm.count("lsv"))) {
      cerr << "Usage- " << argv[0] << " [options] model eigenvector_matrix\n";
      cerr << generic;
      exit(-1);
    }

    if (vm.count("double_sided"))
      double_sided = true;
    else
      double_sided = false;

    if (vm.count("uniform"))
      uniform = true;
    else
      uniform = false;

    if (!strings.empty())
      cols = parseRangeList<uint>(strings);
    else
      cols.push_back(0);

    if (!scales.empty()) {
      if (scales.size() != cols.size()) {
        cerr << "ERROR - You must have the same number of scalings as columns or rely on the global scaling\n";
        exit(-1);
      }
    } else {
      for (uint i=0; i<cols.size(); ++i)
        scales.push_back(1.0);
    }

  }
  catch(exception& e) {
    cerr << "Error - " << e.what() << endl;
    exit(-1);
  }
}



string generateSegid(const uint n) {
  stringstream s;

  s << boost::format("P%03d") % n;
  return(s.str());
}


vector<int> readMap(const string& name) {
  ifstream ifs(name.c_str());
  string line;
  uint lineno = 0;
  vector<int> atomids;

  while (!getline(ifs, line).eof()) {
    stringstream ss(line);
    int a, b;
    if (!(ss >> a >> b)) {
      cerr << "ERROR - cannot parse map at line " << lineno << " of file " << name << endl;
      exit(-10);
    }
    atomids.push_back(b);
  }

  return(atomids);
}


vector<int> fakeMap(const AtomicGroup& g) {
  AtomicGroup::const_iterator ci;
  vector<int> atomids;

  for (ci = g.begin(); ci != g.end(); ++ci)
    atomids.push_back((*ci)->id());

  return(atomids);
}



int main(int argc, char *argv[]) {
  string hdr = invocationHeader(argc, argv);

  parseOptions(argc, argv);
  // First, read in the LSVs
  Matrix U;
  readAsciiMatrix(vec_name, U);
  uint m = U.rows();

  // Read in the average structure...
  AtomicGroup avg = createSystem(model_name);

  vector<int> atomids;
  if (map_name.empty())
    atomids = fakeMap(avg);
  else
    atomids = readMap(map_name);

  int atomid = 1;
  int resid = 1;
  AtomicGroup spines;

  for (uint j=0; j<cols.size(); ++j) {
    double k = global_scale * scales[j];
    uint col = cols[j];

    string segid = generateSegid(col);

    for (uint i=0; i<m; i += 3) {
      GCoord v(U(i,col), U(i+1,col), U(i+2,col));
      if (uniform)
        v /= v.length();

      v *= k;
      pAtom pa = avg.findById(atomids[i/3]);
      GCoord c = pa->coords();

      pAtom atom1(new Atom(atomid++, porcupine_tag, c+v));
      atom1->resid(resid);
      atom1->resname(porcupine_tag);
      atom1->segid(segid);

      pAtom atom2;
      if (double_sided)
        atom2 = pAtom(new Atom(atomid++, porcupine_tag, c-v));
      else
        atom2 = pAtom(new Atom(atomid++, porcupine_tag, c));

      atom2->resid(resid++);
      atom2->resname(porcupine_tag);
      atom2->segid(segid);

      atom1->addBond(atom2);
      atom2->addBond(atom1);

      spines.append(atom1);
      spines.append(atom2);
    }

  }

  PDB outpdb = PDB::fromAtomicGroup(spines);
  outpdb.remarks().add(hdr);
  cout << outpdb;
}
