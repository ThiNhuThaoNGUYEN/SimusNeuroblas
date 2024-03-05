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

#ifndef SIMULATION_PARAMS_H__
#define SIMULATION_PARAMS_H__

// ============================================================================
//                                   Includes
// ============================================================================
#include <cinttypes>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <string>
#include <list>
#include <vector>
#include <map>
#include <iostream>

#include "PopulationParams.h"
#include "CellParams.h"
#include "WorldSizeParams.h"
#include "CellType.h"
#include "InterCellSignal.h"
#include "NicheParams.h"
#include "movement/MoveBehaviour.h"

/*!
  \brief Parameters used to create the simulation
*/
class SimulationParams {
  friend class ParamFileReader;

 public :
  // ==========================================================================
  //                                Constructors
  // ==========================================================================
  SimulationParams() = default;
  SimulationParams(const SimulationParams&) = default;
  SimulationParams(SimulationParams&&) = default;

  // ==========================================================================
  //                                 Destructor
  // ==========================================================================
  virtual ~SimulationParams() = default;

  // ==========================================================================
  //                                  Getters
  // ==========================================================================
  bool autoseed() const { return autoseed_; };
  int32_t seed() const { return seed_; };
  double maxtime() const { return maxtime_; };
  double dt() const { return dt_; };
  double backup_dt() const { return backup_dt_; };
  // TODO(dpa) constness
  const std::list<PopulationParams>& pop_params() const { return pop_params_; };
  const NicheParams& niche_params() const { return niche_params_; };
  const CellParams& cell_params() const { return cell_params_; };
  const WorldSizeParams& worldsize_params() const { return worldsize_params_; };
  bool usecontactarea() const { return usecontactarea_; };
  bool output_orientation() const { return output_orientation_; };


  void setNicheParams(const char* formalism_str, double internal_radius);
  const std::list<InterCellSignal>& using_signals() const { return using_signals_; };
  const std::vector<std::string>& celltype_names() const { return celltype_names_; };

  // ==========================================================================
  //                               Public Methods
  // ==========================================================================
  void addPop(long size,
              const char* cell_type_str,
              const char* formalism_str,
              const char* move_behaviour_str,
              double doubling_time,
              double volume_min,
              const char* init_file);

 protected :
  // ==========================================================================
  //                              Protected Methods
  // ==========================================================================
  CellFormalism StrToFormalism(const std::string formalism_str);
  CellType StrToCellType(const std::string cell_type_str);
  MoveBehaviour::Type StrToMBType(const std::string mb_str);
  InterCellSignal StrToInterCellSignal(const std::string signal_str);

  // ==========================================================================
  //                                 Attributes
  // ==========================================================================
  bool autoseed_ = true;
  int32_t seed_ = 0;
  /** default time span */
  double maxtime_ = 200.0;
  /** default macroscopic time step */
  double dt_ = 0.2; // We aim at a code working with DT=0.5 or DT=1
  double backup_dt_ = 10.0; // Frequency of backups
  std::list<PopulationParams> pop_params_;
  NicheParams niche_params_;
  CellParams cell_params_;
  WorldSizeParams worldsize_params_;
  bool usecontactarea_ = true;
  bool output_orientation_ = false;

  /** Intercellular signals used in simulation */
  std::list<InterCellSignal> using_signals_;
  /** Names of cell types */
  std::vector<std::string> celltype_names_;

 public:
  /** Map cell formalism param file keywords to classIds */
  static std::map<std::string, CellFormalism>& CellFormalisms();

};

#endif // SIMULATION_PARAMS_H__
