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

#ifndef SIMUSCALE_NICHEPARAMS_H__
#define SIMUSCALE_NICHEPARAMS_H__


// ============================================================================
//                                   Includes
// ============================================================================

/**
 *
 */
class NicheParams {
  friend class SimulationParams;

 public :
  // ==========================================================================
  //                               Constructors
  // ==========================================================================
  NicheParams() = default; //< Default ctor
  NicheParams(const NicheParams&) = default; //< Copy ctor
  NicheParams(NicheParams&&) = default; //< Move ctor

  // ==========================================================================
  //                                Destructor
  // ==========================================================================
  virtual ~NicheParams() = default; //< Destructor

  // ==========================================================================
  //                                Operators
  // ==========================================================================
  /// Copy assignment
  NicheParams& operator=(const NicheParams& other) = default;

  /// Move assignment
  NicheParams& operator=(NicheParams&& other) = default;

  // ==========================================================================
  //                              Public Methods
  // ==========================================================================


  // ==========================================================================
  //                                Accessors
  // ==========================================================================
  const CellFormalism& formalism() const { return cell_formalism_; };
  double internal_radius() const { return internal_radius_; };

 protected :
  // ==========================================================================
  //                            Protected Methods
  // ==========================================================================

  // ==========================================================================
  //                               Attributes
  // ==========================================================================
  CellFormalism cell_formalism_;
  double internal_radius_;
};

#endif // SIMUSCALE_NICHEPARAMS_H__
