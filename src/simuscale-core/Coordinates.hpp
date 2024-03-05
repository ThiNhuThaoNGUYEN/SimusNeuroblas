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

// =================================================================
//                              Includes
// =================================================================
#include "Coordinates.h"

#include <cmath>


// =================================================================
//                    Definition of static attributes
// =================================================================



// =================================================================
//                             Constructors
// =================================================================
template<typename T>
Coordinates<T>::Coordinates() {
  x = y = z = 0.0;
}

template<typename T>
Coordinates<T>::Coordinates(T x, T y, T z) {
  this->x = x;
  this->y = y;
  this->z = z;
}


// =================================================================
//                             Destructor
// =================================================================


// =================================================================
//                             Accessors
// =================================================================


// =================================================================
//                             Operators
// =================================================================
template<typename T>
Coordinates<T> operator-(const Coordinates<T>& rhs){
  return Coordinates<T>(-rhs.x, -rhs.y, -rhs.z);
}

template<typename T>
Coordinates<T>& Coordinates<T>::operator+=(const Coordinates& rhs) {
  x += rhs.x;
  y += rhs.y;
  z += rhs.z;

  return *this;
}

template<typename T>
Coordinates<T>& Coordinates<T>::operator=(const Coordinates& rhs) {
  x = rhs.x;
  y = rhs.y;
  z = rhs.z;

  return *this;
}

template<typename T>
bool operator==(const Coordinates<T>& lhs, const Coordinates<T>& rhs) {
  return lhs.x == rhs.x and lhs.y == rhs.y and lhs.z == rhs.z;
}

template<typename T>
bool operator!=(const Coordinates<T>& lhs, const Coordinates<T>& rhs) {
  return (lhs.x != rhs.x or lhs.y != rhs.y or lhs.z != rhs.z);
}

template<typename T>
Coordinates<T> operator+(const Coordinates<T>& lhs,
                         const Coordinates<T>& rhs) {
  return Coordinates<T>(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z);
}

template<typename T>
Coordinates<T> operator-(const Coordinates<T>& lhs,
                         const Coordinates<T>& rhs) {
  return Coordinates<T>(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z);
}

template<typename T>
Coordinates<T> operator*(double factor,
                         const Coordinates<T>& rhs) {
  return Coordinates<T>(factor * rhs.x, factor * rhs.y, factor * rhs.z);
}

template<typename T>
Coordinates<T> operator*(const Coordinates<T>& lhs, double factor) {
  return factor * lhs;
}

template<typename T>
Coordinates<T> operator/(const Coordinates<T>& lhs,
                         double divisor) {
  return Coordinates<T>(lhs.x / divisor, lhs.y / divisor, lhs.z / divisor);
}


// =================================================================
//                            Public Methods
// =================================================================
/** Reset coordinates to (0, 0, 0) */
template<typename T>
void Coordinates<T>::reset() {
  x = y = z = 0.0;
}

/** Return the norm of vector (x, y, z) */
template<typename T>
double Coordinates<T>::norm() const {
  return sqrt(x*x + y*y + z*z);
}

/** Assert no coordinate is not a number */
template<typename T>
void Coordinates<T>::assert_no_nan() const {
  assert(not std::isnan(x));
  assert(not std::isnan(y));
  assert(not std::isnan(z));
}

template<typename T>
void Coordinates<T>::Save(gzFile backup_file) const {
  gzwrite(backup_file, &x, sizeof(x));
  gzwrite(backup_file, &y, sizeof(y));
  gzwrite(backup_file, &z, sizeof(z));
}

template<typename T>
void Coordinates<T>::Load(gzFile backup_file) {
  gzread(backup_file, &x, sizeof(x));
  gzread(backup_file, &y, sizeof(y));
  gzread(backup_file, &z, sizeof(z));
}
