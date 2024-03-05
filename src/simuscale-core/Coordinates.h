// ****************************************************************************
//
//              SiMuScale - Multi-scale simulation framework
//
// ****************************************************************************
//
// Copyright: See the AUTHORS file provided with the package
// E-mail: simuscale-contact@lists.gforge.inria.fr
// Original Authors : Samuel Bernard, Carole Knibbe, David Parsons
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// ****************************************************************************

#ifndef SIMUSCALE_COORDINATES_H__
#define SIMUSCALE_COORDINATES_H__


// =================================================================
//                              Includes
// =================================================================
#include <cmath>
#include <cassert>
#include <cstdio>

#include <zlib.h>


// =================================================================
//                          Class declarations
// =================================================================



/*!
  \brief This template class manages (x, y, z) coordinates
*/
template<typename T>
class Coordinates {
 public :
  // =================================================================
  //                             Constructors
  // =================================================================
  Coordinates();
  Coordinates(const Coordinates&) = default;
  Coordinates(Coordinates&&) = default;
  Coordinates(T x, T y, T z);


  // =================================================================
  //                             Destructor
  // =================================================================
  virtual ~Coordinates(void) = default;


  // =================================================================
  //                              Accessors
  // =================================================================


  // =================================================================
  //                              Operators
  // =================================================================
  Coordinates& operator+=(const Coordinates& rhs);
  Coordinates& operator=(const Coordinates& rhs);
  // non-member operators : see below

  // =================================================================
  //                            Public Methods
  // =================================================================
  inline void reset();
  inline double norm() const;
  inline void assert_no_nan() const;

  void Save(gzFile backup_file) const;
  void Load(gzFile backup_file);


  // =================================================================
  //                           Public Attributes
  // =================================================================
  T x;
  T y;
  T z;
};

// ===================================================================
//                        Non member operators
// ===================================================================
template<typename T>
bool operator==(const Coordinates<T>& lhs, const Coordinates<T>& rhs);

template<typename T>
bool operator!=(const Coordinates<T>& lhs, const Coordinates<T>& rhs);

template<typename T>
Coordinates<T> operator-(const Coordinates<T>& rhs);

template<typename T>
Coordinates<T> operator+(const Coordinates<T>& lhs, const Coordinates<T>& rhs);

template<typename T>
Coordinates<T> operator-(const Coordinates<T>& lhs, const Coordinates<T>& rhs);

template<typename T>
Coordinates<T> operator*(double factor, const Coordinates<T>& rhs);

template<typename T>
Coordinates<T> operator*(const Coordinates<T>& lhs, double factor) ;

template<typename T>
Coordinates<T> operator/(const Coordinates<T>& lhs, double divisor);

#include "Coordinates.hpp"

#endif // SIMUSCALE_COORDINATES_H__
