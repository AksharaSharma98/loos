// cluster-kgs.cpp
// Kelly, Gardner, and Sutcliffe, Prot. Eng. 9 11 1063-1065 (1996)
// hereafter KGS
// To perform exactly the analysis specified there, one must first apply
// one of the all-to-all rmsd tolls (such as rmsds or multi-rmsds) before
// running this tool. Those tools write their RMSD matricies to cout, and
// this tool reads from cin, so the effect can be achieved through a pipe.
#include "Clustering.hpp"
#include <iostream>
#include <string>

using std::cout;
using std::cin;
using std::endl;
using std::vector;

using namespace Clustering;

const std::string helpstr = "XXX";
const std::string indent = "  ";
int main(int argc, char* argv[])
{
  for (int i = 0; i < argc; i++) {
    if (argv[i] == "-h" || argv[i] == "--help") {
      cout << helpstr << endl;
      exit(0);
    }
  }
  
  Eigen::Matrix<dtype, Eigen::Dynamic, Eigen::Dynamic> similarityScores = readMatrixFromStream<dtype>(cin);
  KGS clusterer(similarityScores);
  cout << similarityScores;
  clusterer.cluster();
  idxT optStg = clusterer.cutoff();
  vector<idxT> exemplars =
    getExemplars(clusterer.clusterTraj[optStg], similarityScores);
  // below here is output stuff. All quantities of interest have been obtained.
  cout << "{";
  cout << indent + "\"optimal stage\": " << optStg << "," << endl;
  cout << indent + "\"penalties\": ";
  containerAsOneLineJSONArr<Eigen::Matrix<dtype, Eigen::Dynamic, 1>>(clusterer.penalties, cout);
  cout << "," << endl;
  cout << indent + "\"clusters\": ";
  vectorVectorsAsJSONArr<idxT>((clusterer.clusterTraj)[optStg], cout, "  ");
  cout << "," << endl;
  cout << indent + "\"exemplars\": ";
  containerAsJSONArr<vector<idxT>>(exemplars, cout, "  ");
  cout << endl;
  cout << "}";
}