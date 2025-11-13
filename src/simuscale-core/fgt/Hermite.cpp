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

/* Hermite.cpp */

#include <cstdio>
#include "Hermite.h"

/* static member Hc_ needs to be declared */
constexpr long long Hermite::Hc_[22][11];

// The singleton instance
Hermite Hermite::instance_;

// ============================================================================
//                                Constructors
// ============================================================================
Hermite::Hermite() {
}

// ============================================================================
//                                   Methods
// ============================================================================


/** Hermite function */
H_Real Hermite::Function(H_Real t, size_t n) {
  return instance_.Polynomial(t,n)*exp(-t*t); 
}

/** Hermite 1D polynomial */
H_Real Hermite::Polynomial(H_Real t, size_t n) {

  /* Hc[n][k]: Coefficient of Hermite polynomials
     n in [0,19], k in [0,9]
     When n is even, Hc[n][k] is the coefficient
       of the term of degree 2*k
     When n is odd, Hc[n][k] is the coefficient
       of the term of degree 2*k+1
   */
  if ( n > max_order_ ) {
    fprintf(stderr,"n too big\n");
    exit(1);
  }
  H_Real H = 0; 
  for ( int  k = n/2 + 1; k--;  )
  {
    H = t*t*H + Hc_[n][k];
  }
  if ( n % 2 ) H *= t; /* if n is odd, then make degree odd */
  return H;  
}

