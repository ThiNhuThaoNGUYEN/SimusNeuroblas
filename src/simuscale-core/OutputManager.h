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

#ifndef SIMUSCALE__OUPUT_MANAGER_H__
#define SIMUSCALE__OUPUT_MANAGER_H__

#define FL_FMT "%.2f"

// ============================================================================
//                                   Includes
// ============================================================================
#include <cinttypes>
#include <cstdio>
#include <cstdlib>
#include <cassert>

#include <string>
#include <vector>
#include <map>



// ============================================================================
//                         Class declarations, Using etc
// ============================================================================
using std::string;
using std::map;






class OutputManager
{
  // ==========================================================================
  //                                  Enums
  // ==========================================================================
  enum Output {
    STATS,
    TRAJECTORY,
    SIGNALS
  };

 public :
  // ==========================================================================
  //                               Constructors
  // ==========================================================================
  OutputManager(void);// = default; //< Default ctor
  OutputManager(const OutputManager&) = delete; //< Copy ctor
  OutputManager(OutputManager&&) = delete; //< Move ctor

  // ==========================================================================
  //                                Destructor
  // ==========================================================================
  virtual ~OutputManager(void); //< Destructor

  // ==========================================================================
  //                                 Getters
  // ==========================================================================
  const string& output_dir() const {return output_dir_;};

  // ==========================================================================
  //                                 Setters
  // ==========================================================================

  // ==========================================================================
  //                                Operators
  // ==========================================================================

  // ==========================================================================
  //                              Public Methods
  // ==========================================================================
  void Setup(const string& output_dir);
  void SetupForResume(const string& input_dir, const string& output_dir);
  void PrintTimeStepOutputs();
  void PrintSimulationEndOutputs();

  static string CoerceTrailingSlash(string path);




 protected :
  // ==========================================================================
  //                            Protected Methods
  // ==========================================================================
  void PrepareFilesForResume(const string& input_dir); // TODO check trailing /
  void PrepareFileForResume(const string& input_file_path,
                            const string& output_file_path) const;

  void InitOutputDir(const string& output_dir);
  void OpenFiles(void);
  void CloseFiles(void);
  void PrintHeaders(void);
#ifdef PRINT_SIGNALS
  void PrintStatsHeader(void);
  void PrintSignalsHeader(void);
  void PrintSignals();
#endif
  void PrintTrajectoryHeader(void);
  void PrintTrajectory();
  void PrintNormalization();
  void PrintTrees(void);

  // ==========================================================================
  //                               Attributes
  // ==========================================================================
  string output_dir_ = "./"; // TODO add trailing / if missing

  map<Output, FILE*> outputs_;
  static map<Output, const string> output_file_names_;

  /* end-of-simulation output files */
  FILE* normalization_file_ = NULL;
  FILE* newick_file_ = NULL;
  FILE* tabtree_file_ = NULL;
  static const string normalization_file_name_;
  static const string newick_file_name_;
  static const string tabtree_file_name_;

};


// ============================================================================
//                           Getters' definitions
// ============================================================================

// ============================================================================
//                           Setters' definitions
// ============================================================================

// ============================================================================
//                          Operators' definitions
// ============================================================================

// ============================================================================
//                       Inline functions' definition
// ============================================================================

#endif // SIMUSCALE__OUPUT_MANAGER_H__
