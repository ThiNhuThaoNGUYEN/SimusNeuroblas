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

/* FastGaussTransform3D.h */


// <TODO> Modify the include guard </TODO>
#ifndef SIMUSCALE_FASTGAUSSTRANSFORM_H__
#define SIMUSCALE_FASTGAUSSTRANSFORM_H__


// ============================================================================
//                                   Includes
// ============================================================================


#include <cstdlib>
#include <cmath>
#include <vector>
#include <array>
#include <set>
#include <stdint.h>

#include "Hermite.h"
#include "Box3.h"
#include "Simulation.h"

using namespace std;

/**
 * 
 */
class FastGaussTransform3D {

 public :
  // ==========================================================================
  //                               Types
  // ==========================================================================

  using real_type_ = float;
  using uint_type_ = uint32_t;
  using point_type_ = array<real_type_, 3>;

  // ==========================================================================
  //                               Constructors
  // ==========================================================================
  FastGaussTransform3D() = default; //< Default ctor
  FastGaussTransform3D(vector<real_type_> q, vector<point_type_> s, 
      vector<point_type_> t, real_type_ delta, real_type_ epsilon);

  // ==========================================================================
  //                                Destructor
  // ==========================================================================
  virtual ~FastGaussTransform3D() = default;

  // ==========================================================================
  //                                Operators
  // ==========================================================================

  // ==========================================================================
  //                              Public Methods
  // ==========================================================================

  void Setup(const SimulationParams& simParams);
  void init_transform(InterCellSignal signal);
  void fast_transform();
  void direct_transform();
  void finish_transform();

  void Save(gzFile backup_file) const;
  void Load(gzFile backup_file);

  // ==========================================================================
  //                                Accessors
  // ==========================================================================
  vector<real_type_>& G() { return G_; };
  vector<real_type_>& Gexact() { return Gexact_; };
  array<uint32_t, 4>& evals() { return evals_; };
  const list<InterCellSignal>& using_diffusive_signals() const { return using_diffusive_signals_; }
  real_type_ delta() { return delta_; }

  // ==========================================================================
  //                               Attributes
  // ==========================================================================


 protected :
  // ==========================================================================
  //                            Protected Methods
  // ==========================================================================


  // ==========================================================================
  //                            Protected Attributes
  // ==========================================================================

  static constexpr uint_type_   d_ = 3;         /* space dimension */
  uint_type_  N_;
  uint_type_  M_;
  uint_type_  N_side_;                          /* nbr box in each dimensions */
  real_type_  r_;                               /* r largest value < 1/2 such that N_side is a integer */
  uint_type_  n_;                               /* nbr box span in each direction */
  uint_type_  p_;

  uint_type_  N_F_;                             /* N_F = O(_p^(d-1)) Cut-off for number of sources per box */
  uint_type_  M_L_;                             /* M_L = O(_p^(d-1)) Cut-off for number of targets per box */

  vector<Box3> Bs_;                              /* array of lists of source points in each box */
  vector<Box3> Bt_;                              /* array of lists of target points in each box */

  vector<real_type_>  A_alpha_;                   /* Hermite Coeffs */

  set<uint32_t>    ilist_; 
  set<uint32_t>    taylor_box_list_; 
  set<uint32_t>    non_empty_source_list_; 
  set<uint32_t>    non_empty_target_list_; 
  set<uint32_t>    non_empty_box_list_; 

  
  vector<real_type_>  G_;
  vector<real_type_>  Gexact_;
  vector<real_type_>  q_;
  vector<point_type_>  s_;
  vector<point_type_>  t_;

  real_type_          scale_;                      /* renormalization scale delta = delta/scale^2 */
  real_type_          delta_;                      /* renormalization scale delta = delta/scale^2 */
  real_type_          epsilon_ = 1e-3;

  // evaluation counts
  array<uint32_t, 4> evals_;


  // cell list from Simulation
  const list<Cell*>* cell_list_; 

  /** Intercellular signals used as diffusive (long-range) signals */
  list<InterCellSignal> using_diffusive_signals_;
  vector<real_type_> diffusive_delta_;
  vector<real_type_> diffusive_epsilon_;

  InterCellSignal signal_;

 private:
  // ==========================================================================
  //                            Private Methods
  // ==========================================================================

  /* return index of box containing point p */
  uint_type_ p2b(point_type_ p); 

  /** space renormalization */
  void renormalize();



  /** integral power */
  real_type_ ipow(const real_type_ x, uint_type_ n);

  /** square norm of difference between p1 and p2 */
  real_type_ dist2(point_type_ p1, point_type_ p2);


  /** form_interaction_list: list the indices of target boxes */
  void form_interaction_list(uint_type_ i);


  /** Direct summation of sources, direct evaluation at targets */
  void direct_direct(Box3 source, Box3 target);

  /** Transform all field to Taylor series */
  void direct_taylor(Box3 source, Box3& target);

  /** Compute Hermite expansion */
  void hermite_coeffs(Box3 source);

  /** Truncated Hermite expansion, direct evaluation */
  void hermite_direct(point_type_ sB, Box3 target );

  /** Truncated Hermite expansion to Taylor series */
  void hermite_taylor(Box3& target, point_type_ sB);

  // ==========================================================================
  //                            Private Attributes
  // ==========================================================================

  static constexpr unsigned long long factorial_[20] = {
                           1,1,2,6,24,120,720,5040,40320,362880,
                           3628800,39916800,479001600,6227020800,
                           87178291200,1307674368000,20922789888000,
                           355687428096000,6402373705728000,121645100408832000};

};

#endif // SIMUSCALE_FASTGAUSSTRANSFORM_H__

