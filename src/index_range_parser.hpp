/*
  This file is part of LOOS.

  LOOS (Lightweight Object-Oriented Structure library)
  Copyright (c) 2016, Tod D. Romo, Alan Grossfield
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



#if !defined(LOOS_INDEX_RANGE_PARSER)
#define LOOS_INDEX_RANGE_PARSER


#include <vector>
#include <string>

namespace loos {

  std::vector<uint> parseIndexRange(const std::string& input, const uint maxsize);


  template <typename T>
  std::vector<T> uniquifyVector(const std::vector<T>& list) {
    std::set<T> indices;
    std::insert_iterator< std::set<T> > ii(indices, indices.begin());
    std::copy(list.begin(), list.end(), ii);

    std::vector<T> uniques(indices.size());
    std::copy(indices.begin(), indices.end(), uniques.begin());
    return uniques;
  }

}


#endif 
