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

/* Hermite.h */

#ifndef SIMUSCALE_HERMITE_H_
#define SIMUSCALE_HERMITE_H_

#include <cstdlib>
#include <cmath>
#include <stdint.h>

typedef double H_Real;

class Hermite {

  // ==========================================================================
  // Singleton management 
  // ==========================================================================

 private :
  static Hermite instance_;
  Hermite(); //< default cor
  Hermite(const Hermite&) = delete; //< copy cor : no copy possible
  Hermite& operator= (const Hermite&) = delete; //< copy assign : no assign possible
  // virtual ~Hermite(); //< destructor
 public :
  static Hermite &instance() noexcept {
    return instance_;
  };

  // ==========================================================================
  //                                Operators
  // ==========================================================================

  // ==========================================================================
  //                              Public Methods
  // ==========================================================================

 public :

  /** Hermite function */
  static H_Real Function(H_Real t, size_t n);

  // ==========================================================================
  //                                Accessors
  // ==========================================================================

  size_t maxOrder() { return instance_.max_order_; };


 protected :
  // ==========================================================================
  //                            Protected Methods
  // ==========================================================================

  /** Hermite 1D polynomial */
  H_Real Polynomial(H_Real t, size_t n);

  // ==========================================================================
  //                               Attributes
  // ==========================================================================

 public :


  using real_type_ = H_Real;

 protected : 

  static constexpr size_t max_order_ = 21;


 private :

  static constexpr long long Hc_[22][11] = {{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
                        {2, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
                        {-2, 4, 0, 0, 0, 0, 0, 0, 0, 0, },
                        {-12, 8, 0, 0, 0, 0, 0, 0, 0, 0, },
                        {12, -48, 16, 0, 0, 0, 0, 0, 0, 0, },
                        {120, -160, 32, 0, 0, 0, 0, 0, 0, 0, },
                        {-120, 720, -480, 64, 0, 0, 0, 0, 0, 0, },
                        {-1680, 3360, -1344, 128, 0, 0, 0, 0, 0, 0, },
                        {1680, -13440, 13440, -3584, 256, 0, 0, 0, 0, 0, },
                        {30240, -80640, 48384, -9216, 512, 0, 0, 0, 0, 0, },
                        {-30240, 302400, -403200, 161280, -23040, 1024, 0, 0, 0, 0, },
                        {-665280, 2217600, -1774080, 506880, -56320, 2048, 0, 0, 0, 0, },
                        {665280, -7983360, 13305600, -7096320, 1520640, -135168, 4096, 0, 0, 0, },
                        {17297280, -69189120, 69189120, -26357760, 4392960, -319488, 8192, 0, 0, 0, },
                        {-17297280, 242161920, -484323840, 322882560, -92252160, 12300288, -745472, 16384, 0, 0, },
                        {-518918400, 2421619200, -2905943040, 1383782400, -307507200, 33546240, -1720320, 32768, 0, 0, },
                        {518918400, -8302694400, 19372953600, -15498362880, 5535129600, -984023040, 89456640, -3932160, 65536, 0, },
                        {17643225600, -94097203200, 131736084480, -75277762560, 20910489600, -3041525760, 233963520, -8912896, 131072, 0, },
                        {-17643225600, 317578060800, -846874828800, 790416506880, -338749931520, 75277762560, -9124577280, 601620480, -20054016, 262144, },
                        {-670442572800, 4022655436800, -6436248698880, 4290832465920, -1430277488640, 260050452480, -26671841280, 1524105216, -44826624, 524288, },
                        {670442572800, -13408851456000, 40226554368000, -42908324659200, 21454162329600, -5721109954560, 866834841600, -76205260800, 3810263040, -99614720, 1048576, },
                        {28158588057600, -187723920384000, 337903056691200, -257449947955200, 100119424204800, -21844238008320, 2800543334400, -213374730240, 9413591040, -220200960, 2097152, },
                        };

};


#endif // SIMUSCALE_HERMITE_H_
