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

#ifndef __CPARAM_LOADER_H__
#define __CPARAM_LOADER_H__


// =================================================================
//                              Libraries
// =================================================================
#include <inttypes.h>
#include <stdio.h>

#include <string>


// =================================================================
//                            Project Files
// =================================================================
#include "Cell.h"
#include "params/CellParams.h"
#include "params/SimulationParams.h"



// =================================================================
//                          Class declarations
// =================================================================
class f_line {
 public :
  f_line();

  int16_t nb_words;
  char    words[50][255];
};




class ParamFileReader {
 public :
  // =================================================================
  //                             Constructors
  // =================================================================
  ParamFileReader();
  ParamFileReader(const ParamFileReader &model) = delete;
  ParamFileReader(const std::string& file_name);

  // =================================================================
  //                             Destructor
  // =================================================================
  virtual ~ParamFileReader();

  // =================================================================
  //                              Accessors
  // =================================================================
  inline const SimulationParams& get_simParams() const;
  inline const CellParams& get_cellParams() const;

  // =================================================================
  //                            Public Methods
  // =================================================================
  void load();

  // =================================================================
  //                           Public Attributes
  // =================================================================


 protected :
  // =================================================================
  //                           Protected Methods
  // =================================================================
  void open_input_file();
  f_line* get_line();
  static void format_line(f_line* formatted_line, char* line,
                          bool* line_is_interpretable);
  void interpret_line(f_line* line);


  // =================================================================
  //                          Protected Attributes
  // =================================================================
  std::string _param_file_name = "param.in";
  FILE* _param_file;
  int   _cur_line = 0;

  SimulationParams simParams;
};


// =====================================================================
//                          Accessors definitions
// =====================================================================
inline const SimulationParams& ParamFileReader::get_simParams() const {
  return simParams;
}

inline const CellParams& ParamFileReader::get_cellParams() const {
  return simParams.cell_params();
}

// =====================================================================
//                       Inline functions' definition
// =====================================================================

#endif // __CPARAM_LOADER_H__
