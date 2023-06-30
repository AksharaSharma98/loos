/*
  Testing code for HDF5 support 

  (c) 2023 Alan Grossfield
           Department of Biochemistry and Biophysics
           University of Rochester School of Medicine and Dentistry


*/


/*
  This file is part of LOOS.

  LOOS (Lightweight Object-Oriented Structure library)
  Copyright (c) 2023 Alan Grossfield
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

using namespace loos;

int main(int argc, char *argv[]) {

    std::string filename = argv[1];
    MDTraj mdtraj_system(filename);
    //std::cerr << "size = " << mdtraj_system.size() << std::endl;
    //std::cerr <<" centroids: " << "\t" 
    //          << mdtraj_system.centroid() << std::endl;
    AtomicGroup ligand = selectAtoms(mdtraj_system, std::string("resname=='LIG'"));
    //std::cerr << "ligand size = " << ligand.size() << std::endl;
    //std::cerr << *(ligand[0]) << std::endl;

    AtomicGroup system = createSystem(filename);
    //std::cerr << "size = " << system.size() << std::endl;
    //std::cerr << "centroid from ag = " << system.centroid() << std::endl;

    MDTrajTraj traj(filename, system.size());
    traj.updateGroupCoords(system);
    //std::cerr << "centroid: " << system.centroid() << std::endl;

    pTraj traj2 = createTrajectory(filename, system);
    traj2->updateGroupCoords(system);
    //std::cerr << "centroid: " << system.centroid() << std::endl;
    //std::cerr << "box: " << system.periodicBox() << std::endl;

    std::cout << "#" << traj2->nframes() << std::endl;
    while (traj2->readFrame()) {
      traj2->updateGroupCoords(system);
      GCoord centroid = system.centroid();
      std::cout << traj2->currentFrame() << "\t" 
                << centroid.x() << "\t"
                << centroid.y() << "\t"
                << centroid.z() << "\t"
                << std::endl;
    }
  
    return 0;
}