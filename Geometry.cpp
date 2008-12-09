/*
  This file is part of LOOS.

  LOOS (Lightweight Object-Oriented Structure library)
  Copyright (c) 2008, Alan Grossfield
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



#include <Geometry.hpp>



namespace loos {

  //const double DEGREES = 180 / M_PI ;

  greal Math::angle(const GCoord &a, const GCoord &b, const GCoord &c) {
    // TODO: check to make sure the sign is right
    GCoord ba = b - a;
    GCoord bc = b - c;
    greal cosine = (ba * bc) / (ba.length() * bc.length());
    return (acos(cosine) * DEGREES);
  }

  greal Math::angle(const pAtom a, const pAtom b, const pAtom c) {
    return(angle(a->coords(), b->coords(), c->coords()));
  }

  greal Math::torsion(const GCoord &a, const GCoord &b, const GCoord &c, 
                            const GCoord &d) {
    // TODO: check to make sure the sign is right
    GCoord ba = b - a;
    GCoord cb = c - b;
    GCoord dc = d - c;

    GCoord norm1 = ba.cross(cb);
    GCoord norm2 = cb.cross(dc);
    greal cosine = norm1*norm2 / (norm1.length() * norm2.length());
    greal angle = acos(cosine) * DEGREES;
    greal sign = ba * norm2;
    if (sign < 0) angle = -angle;
    return(angle);
  }

  greal Math::torsion(const pAtom a, const pAtom b, const pAtom c, 
                            const pAtom d) {
    return(torsion(a->coords(), b->coords(), c->coords(), d->coords()));
  }

}
