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

// ============================================================================
//                                   Includes
// ============================================================================
#include "OutputManager.h"
#include "Population.h"

#include <iostream>

#include "params/NicheParams.h"
#include "Simulation.h"
#include "WorldSize.h"
#include "CellType.h"
#include "InterCellSignal.h"

using std::cout;
using std::cerr;
using std::endl;

#define PRINT_CELLTYPE
#define PRINT_LIVINGSTATUS
#define PRINT_DIFFUSIVESIGNALS

//##############################################################################
//                                                                             #
//                             Class OutputManager                             #
//                                                                             #
//##############################################################################

// ============================================================================
//                       Definition of static attributes
// ============================================================================
// const string OutputManager::output_file_names_[SIGNALS] = "signals.txt";
// const string OutputManager::output_file_names_[STATS] = "stats.txt";
// const string OutputManager::output_file_names_[TRAJECTORY] = "trajectory.txt";
map<OutputManager::Output, const string> OutputManager::output_file_names_ = {
#ifdef PRINT_SIGNALS
  {SIGNALS, "signals.txt"},
  {STATS, "stats.txt"},
#endif
  {TRAJECTORY, "trajectory.txt"},
};
const string OutputManager::normalization_file_name_ = "normalization.txt";
const string OutputManager::newick_file_name_ = "newick.tree";
const string OutputManager::tabtree_file_name_ = "treelist.tgf";

// ============================================================================
//                                Constructors
// ============================================================================

OutputManager::OutputManager(void) {
#ifdef PRINT_SIGNALS
  outputs_[SIGNALS] = NULL;
  outputs_[STATS] = NULL;
#endif
  outputs_[TRAJECTORY] = NULL;
}

// ============================================================================
//                                 Destructor
// ============================================================================

OutputManager::~OutputManager() {
  CloseFiles();
}

// ============================================================================
//                                   Methods
// ============================================================================

void OutputManager::Setup(const string& output_dir) {
  InitOutputDir(output_dir);
  OpenFiles();
}

void OutputManager::SetupForResume(const string& input_dir, const string& output_dir) {
  InitOutputDir(output_dir);

  // Make output files ready before resuming simulation
  PrepareFilesForResume(CoerceTrailingSlash(input_dir));
}

void OutputManager::InitOutputDir(const string& output_dir) {
  output_dir_ = CoerceTrailingSlash(output_dir);
}

void OutputManager::OpenFiles() {
  for (auto entry : outputs_) {
    FILE* file = entry.second;

    // File must not already be opened
    assert(file == NULL);

    // Open file and check if it was done succesfully
    outputs_[entry.first] =
            fopen((output_dir_ + output_file_names_[entry.first]).c_str(), "w");
    if (outputs_[entry.first] == NULL) {
      cerr << "Error: impossible to open the file "
              << output_dir_ + output_file_names_[entry.first] << endl;
      exit(EXIT_FAILURE);
    }
  }

  PrintHeaders();
}

void OutputManager::CloseFiles() {
  for (auto entry : outputs_) {
    FILE* file = entry.second;
    if (file) {
      fclose(file);
    }
  }
}

void OutputManager::PrepareFilesForResume(const string& input_dir) {
  // TODO <david.parsons@inria.fr> Faire une méthode pour les rename en ".bak"
  for (auto entry : output_file_names_) {
    string file_name = entry.second;

    string input_file_path = input_dir + file_name;
    string output_file_path = output_dir_ + file_name;

    if (output_dir_ == input_dir) {
        // Try renaming input_file_path (add ".bak" postfix)
        // Note that input_file_path == output_file_path
        input_file_path += ".bak";
        if (rename(output_file_path.c_str(), input_file_path.c_str()) != 0) {
            cerr << "Error: Could not rename " << output_file_path << " into "
                    << input_file_path << endl;
            exit(EXIT_FAILURE);
        }
    }

    PrepareFileForResume(input_file_path, output_file_path);

    outputs_[entry.first] = fopen(output_file_path.c_str(), "a");
    if (outputs_[entry.first] == NULL) {
        cerr << "Error: impossible to open the file "
                << output_file_names_[entry.first] << endl;
        exit(EXIT_FAILURE);
    }
  }
}

void OutputManager::PrepareFileForResume(const string& input_file_path,
        const string& output_file_path) const {
    // Open both old file (read mode) and new file (write mode)
    FILE* input_file = fopen(input_file_path.c_str(), "r");
    FILE* output_file = fopen(output_file_path.c_str(), "w");
    if (input_file == NULL) {
        cerr << "Error: unable to open file " + input_file_path << endl;
        exit(EXIT_FAILURE);
    }
    if (output_file == NULL) {
        cerr << "Error: unable to open file " + output_file_path << endl;
        exit(EXIT_FAILURE);
    }

    // Declare buffer
    int buf_size = 255;
    char line[buf_size];

    // ===== Read first line =====
    if (fgets(line, buf_size, input_file) == NULL) {
        cerr << "Error reading first line of file " + input_file_path << endl;
        exit(EXIT_FAILURE);
    }

    // ===== Copy file header =====
    while (!feof(input_file) && (line[0] == '#' || line[0] == '!' || line[0] == '$')) {
        fputs(line, output_file);
        if (fgets(line, buf_size, input_file) == NULL) {
            cerr << "Error reading header line of file " + input_file_path << endl;
            exit(EXIT_FAILURE);
        }
    }

    // ===== Copy lines until current timestep (included) =====
    // We test the time we read from file against time + dt/2 to get rid of
    // rounding problems
    double time_upper_bound = Simulation::sim_time() + Simulation::dt() / 2;
    while (atof(line) <= time_upper_bound && !feof(input_file)) {
        while (strlen(line) == static_cast<size_t> (buf_size - 1)) {
            fputs(line, output_file);
            fgets(line, buf_size, input_file);
        }
        fputs(line, output_file);
        if (fgets(line, buf_size, input_file) == NULL) {
            cerr << "Error reading from file " + input_file_path << endl;
            exit(EXIT_FAILURE);
        }
    }

    fclose(input_file);
    fclose(output_file);
}

void OutputManager::PrintTimeStepOutputs() {
#ifdef PRINT_SIGNALS
    PrintStats();
    PrintSignals();
#endif
    PrintTrajectory();
}


void OutputManager::PrintHeaders() {
#ifdef PRINT_SIGNALS
    PrintStatsHeader();
    PrintSignalsHeader();
#endif 
    PrintTrajectoryHeader();
}

void OutputManager::PrintSimulationEndOutputs() {
    PrintNormalization();
    PrintTrees();
}

#ifdef PRINT_SIGNALS
void OutputManager::PrintStatsHeader() {
  int16_t column_nbr = 4;
  fprintf(outputs_[STATS], "# Column headers :\n");
  fprintf(outputs_[STATS], "# 1 : time\n");
  fprintf(outputs_[STATS], "# 2 : number of cells in the population\n");
  fprintf(outputs_[STATS], "# 3 : avg number of interactions\n");
  for ( auto signal : Simulation::using_signals() ) {
    fprintf(outputs_[STATS], "# %" PRId32 " : avg signal level %s\n", column_nbr++, InterCellSignal_Names.at(signal).c_str());
  }
}

void OutputManager::PrintStats() {
  const Population& pop = Simulation::pop();
  fprintf(outputs_[STATS], FL_FMT " ", Simulation::sim_time());
  fprintf(outputs_[STATS], "%" PRId32 " ", pop.Size());
  fprintf(outputs_[STATS], FL_FMT " ", pop.getAvgNbInteractions());
  for ( auto signal : Simulation::using_signals() ) {
    fprintf(outputs_[STATS], FL_FMT " ", pop.getAvgSignal(signal));
  }
  fprintf(outputs_[STATS], "\n");
  fflush(outputs_[STATS]);
}
#endif

void OutputManager::PrintTrajectoryHeader(void) {
  fprintf(outputs_[TRAJECTORY], "! " FL_FMT " " FL_FMT " " FL_FMT "\n",
          WorldSize::size().x, WorldSize::size().y, WorldSize::size().z);
  fprintf(outputs_[TRAJECTORY], "# Column headers :\n");
  fprintf(outputs_[TRAJECTORY], "# 1 : time\n");
  fprintf(outputs_[TRAJECTORY], "# 2 : number of cells in the population\n");
  fprintf(outputs_[TRAJECTORY], "# 3 : cell ID\n");
  fprintf(outputs_[TRAJECTORY], "# 4 : x position\n");
  fprintf(outputs_[TRAJECTORY], "# 5 : y position\n");
  fprintf(outputs_[TRAJECTORY], "# 6 : z position\n");
  int16_t column_nbr = 7;
  if ( Simulation::output_orientation() ) {
    fprintf(outputs_[TRAJECTORY], "# 7 : x orientation\n");
    fprintf(outputs_[TRAJECTORY], "# 8 : y orientation\n");
    fprintf(outputs_[TRAJECTORY], "# 9 : z orientation\n");
    column_nbr += 3;
  }
  fprintf(outputs_[TRAJECTORY], "# %" PRId32 " : external radius \n",column_nbr++);
#ifdef PRINT_CELLTYPE
    fprintf(outputs_[TRAJECTORY], "# %" PRId32 " : cell type\n",column_nbr++);
#endif
#ifdef PRINT_LIVINGSTATUS
    fprintf(outputs_[TRAJECTORY], "# %" PRId32 " : living status\n",column_nbr++);
#endif
  for ( auto signal : Simulation::using_signals() ) {
    fprintf(outputs_[TRAJECTORY], "$ %" PRId32 " = %s\n", column_nbr++, InterCellSignal_Names.at(signal).c_str() );
  }
#ifdef PRINT_DIFFUSIVESIGNALS
  for ( auto signal : Simulation::fgt()->using_diffusive_signals() ) {
    fprintf(outputs_[TRAJECTORY], "$ %" PRId32 " = %s_D_ diffusive\n", column_nbr++, InterCellSignal_Names.at(signal).c_str() );
  }
#endif
}

void OutputManager::PrintTrajectory(void) {
  // Each line contains : 
  //   * the timestamp
  //   * the number of cells
  //   * the current cell id
  //   * its x, y, z coordinates
  //   * its external radius
  //   * its intrinsic output (direct output to other cells)
  for (auto cell : Simulation::pop().cell_list()) {
    fprintf(outputs_[TRAJECTORY],
            "" FL_FMT " %" PRId32 " ",
            Simulation::sim_time(),
            Simulation::pop().Size());
    fprintf(outputs_[TRAJECTORY],
            "%d " FL_FMT " " FL_FMT " " FL_FMT " ",
            cell->id(),
            cell->pos_x(), cell->pos_y(), cell->pos_z());
    if ( Simulation::output_orientation() ) {
      fprintf(outputs_[TRAJECTORY],
              FL_FMT " " FL_FMT " " FL_FMT " ",
              cell->orientation_x(), cell->orientation_y(), cell->orientation_z());
    }
    fprintf(outputs_[TRAJECTORY], FL_FMT " ", cell->external_radius());
#ifdef PRINT_CELLTYPE
    fprintf(outputs_[TRAJECTORY],
            "%s ", CellType_Names.at(cell->cell_type()).c_str());
#endif
#ifdef PRINT_LIVINGSTATUS
    fprintf(outputs_[TRAJECTORY],
            "%c ", cell->isDying() ? 'D' : 'A');
#endif
    for ( auto signal : Simulation::using_signals() ) {
      fprintf(outputs_[TRAJECTORY], FL_FMT " ", cell->get_output(signal) );
    }
#ifdef PRINT_DIFFUSIVESIGNALS
    for ( auto signal : Simulation::fgt()->using_diffusive_signals() ) {
      fprintf(outputs_[TRAJECTORY], FL_FMT " ", cell->gaussian_field_weight(signal) );
    }
#endif
    fprintf(outputs_[TRAJECTORY], "\n");
  }
}

void OutputManager::PrintNormalization(void) {
  normalization_file_ =
      fopen((output_dir_ + normalization_file_name_).c_str(), "w");
  size_t i = 0;
  if (normalization_file_ == NULL) {
    cerr << "Error: impossible to open the file " << normalization_file_name_
         << endl;
    exit(EXIT_FAILURE);
  }

  fprintf(normalization_file_, "# MAX_SIGNAL MIN_SIGNAL\n");
  for ( auto signal : Simulation::using_signals() ) {
    fprintf(normalization_file_,
            "%s " FL_FMT " " FL_FMT "\n",
            InterCellSignal_Names.at(signal).c_str(),
            Simulation::max_signals().at(i),
            Simulation::min_signals().at(i));
    i++;
  }
#ifdef PRINT_DIFFUSIVESIGNALS
  for ( auto signal : Simulation::fgt()->using_diffusive_signals() ) {
    fprintf(normalization_file_,
            "%s_D_ " FL_FMT " " FL_FMT "\n",
            InterCellSignal_Names.at(signal).c_str(),
            Simulation::max_signals().at(i),
            Simulation::min_signals().at(i));
    i++;
  }
#endif

    fclose(normalization_file_);
}

void OutputManager::PrintTrees(void) {
  newick_file_ =
      fopen((output_dir_ + newick_file_name_).c_str(), "w");
  if (newick_file_ == NULL) {
    cerr << "Error: impossible to open the file " << newick_file_name_
         << endl;
    exit(EXIT_FAILURE);
  }
  
  Simulation::tree().PrintNewick(newick_file_);

  fclose(newick_file_);

  tabtree_file_ =
      fopen((output_dir_ + tabtree_file_name_).c_str(), "w");
  if (tabtree_file_ == NULL) {
    cerr << "Error: impossible to open the file " << tabtree_file_name_
         << endl;
    exit(EXIT_FAILURE);
  }
  
  Simulation::tree().PrintTabularTree(tabtree_file_);

  fclose(tabtree_file_);
}

#ifdef PRINT_SIGNALS
void OutputManager::PrintSignalsHeader(void) {
  fprintf(outputs_[SIGNALS], "! " FL_FMT " " FL_FMT " " FL_FMT "\n",
          WorldSize::size().x, WorldSize::size().y, WorldSize::size().z);
  fprintf(outputs_[SIGNALS], "# Column headers :\n");
  fprintf(outputs_[SIGNALS], "# 1 : time\n");
  fprintf(outputs_[SIGNALS], "# 2 : number of cells in the population\n");
  fprintf(outputs_[SIGNALS], "# 3 : ID of cell i\n");
  int16_t column_nbr = 4;
  for ( auto signal : Simulation::using_signals() ) {
    fprintf(outputs_[SIGNALS], "# %" PRId32 " : signal %s of cell i\n", column_nbr++, InterCellSignal_Names.at(signal).c_str() );
  }
}

void OutputManager::PrintSignals(void) {
    /* Each line is one timestep and columns are ordered in the following way:
     time popsize  (for each cell : signal[0] signal[1]) */

  for (Cell* cell : Simulation::pop().cell_list()){
    // TODO : Loop based on MAXINTRISICSIGNALS
    fprintf(outputs_[SIGNALS], "%" PRId32 , cell->id()); 
    for ( auto signal : Simulation::using_signals() ) {
      fprintf(outputs_[SIGNALS], " " FL_FMT " ", cell->get_output(signal) );
    }
  }

}
#endif

string OutputManager::CoerceTrailingSlash(string path) {
    // Add a trailing "/" if needed
    assert(not path.empty());
    if (path.back() != '/') path += '/';

    return path;
}


// ============================================================================
//                            Non inline accessors
// ============================================================================
