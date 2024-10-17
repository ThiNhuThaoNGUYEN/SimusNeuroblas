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
#include "Simulation.h"

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cassert>

#include <algorithm>
#include <iostream>
#include <iomanip>

#include <zlib.h>

#include "params/SimulationParams.h"
#include "params/PopulationParams.h"
#include "params/CellParams.h"
#include "Alea.h"
#include "Population.h"
#include "Grid.h"
#include "Cell.h"

using std::cerr;
using std::cout;
using std::endl;


// =================================================================
//                    Definition of static attributes
// =================================================================
// The singleton instance
Simulation Simulation::instance_;




// =================================================================
//                             Constructors
// =================================================================
Simulation::Simulation() {
  grid_ = new Grid();
  pop_ = new Population();

  time_ = 0.0;
  timestep_ = 0;
  tree_ = new CellTree(time_);
}


// =================================================================
//                             Destructor
// =================================================================
Simulation::~Simulation() {
  delete pop_;
  delete grid_;
}


// =================================================================
//                            Public Methods
// =================================================================
void Simulation::Setup(const SimulationParams& simParams,
                       const string& output_dir) {
  instance_.DoSetup(simParams, output_dir);
}

void Simulation::Run() {
  instance_.DoRun();
}

void Simulation::Save() {
  instance_.DoSave();
}

void Simulation::Load(const string& input_dir,
                      int32_t timestep,
                      const string& output_dir) {
  instance_.DoLoad(input_dir, timestep, output_dir);
}


// =================================================================
//                           Protected Methods
// =================================================================
void Simulation::DoSetup(const SimulationParams& simParams,
                         const string& output_dir) {

  // Simulation parameters
  dt_ = simParams.dt();
  backup_dtimestep_ =
      static_cast<int32_t>(round(simParams.backup_dt() / simParams.dt()));
  max_timestep_ =
      static_cast<int32_t>(round(simParams.maxtime() / simParams.dt()));
  max_pop_ = simParams.maxpop() ; 

  // Seed PRNG
  if (simParams.autoseed())
    Alea::seed(static_cast<int32_t>(time(NULL)));
  else
    Alea::seed(simParams.seed());


  // Cell parameters
  Cell::set_volume_max_min_ratio(simParams.cell_params().volume_max_min_ratio());
  Cell::set_radii_ratio(simParams.cell_params().radii_ratio());

  // WorldSize parameters
  WorldSize::set_worldsize(simParams.worldsize_params().size());
  WorldSize::set_worldmargin(simParams.worldsize_params().margin());

  // Add cells to the population
  for (const PopulationParams& popParams : simParams.pop_params()) {
    pop_->AddCells(popParams, simParams.cell_params());
  }


  // Generate niche
  pop_->GenerateNiche(simParams.niche_params());


  // Place cells on the grid
  for (Cell* cell : pop_->cell_list()) {
    grid_->AddCell(cell);
  }

  // Add cells in the tree at root
  for (Cell* cell : pop_->cell_list()) {
    tree_->AddTreeNode(tree_->root(),time_,max_timestep_*dt_,cell->id(),cell->cell_type()); 
  }


  // Intercellular signals to momitor
  using_signals_ = simParams.using_signals();

  usecontactarea_ = simParams.usecontactarea(); 
  output_orientation_ = simParams.output_orientation(); 

  max_signals_.resize(using_signals_.size());
  min_signals_.resize(using_signals_.size());
  std::fill(max_signals_.begin(),max_signals_.end(),-DBL_MAX);
  std::fill(min_signals_.begin(),min_signals_.end(), DBL_MAX);

  output_manager_.Setup(output_dir);
  output_manager_.PrintTimeStepOutputs();
}

void Simulation::DoRun() {
  while(timestep_ < max_timestep_) {
    Update();
    if (Simulation::pop().Size() >= max_pop_) break;
  }
  Finalize();
}

void Simulation::Update() {
  ComputeInteractions();
  ApplyUpdate();
  
  if (timestep_ % backup_dtimestep_ == 0) {
    Save();
  }
  if (timestep_ % static_cast<int>(round(1/std::min(dt_,1.0))) == 0) {
    cout << '*';
    cout.flush();
  }

  output_manager_.PrintTimeStepOutputs();
}

void Simulation::Finalize() {
  output_manager_.PrintSimulationEndOutputs();
}

void Simulation::DoSave() {
  // Open backup file
  char backup_file_path[255];
  sprintf(backup_file_path,
          "%s/backup_%06" PRId32,
          output_manager_.output_dir().c_str(),
          static_cast<int32_t>(round(time_)));
  gzFile backup_file = gzopen(backup_file_path, "w");
  if (backup_file == Z_NULL) {
    printf("%s:%d: error: could not open backup file %s\n",
            __FILE__, __LINE__, backup_file_path);
    exit(EXIT_FAILURE);
  }

  // Write file-type signature ?

  // Save PRNG
  Alea::instance().Save(backup_file);

  // Save simulation
  gzwrite(backup_file, &time_, sizeof(time_));
  gzwrite(backup_file, &timestep_, sizeof(timestep_));
  gzwrite(backup_file, &max_timestep_, sizeof(backup_file));
  gzwrite(backup_file, &dt_, sizeof(dt_));
  gzwrite(backup_file, &backup_dtimestep_, sizeof(backup_dtimestep_));
  pop_->Save(backup_file);
  grid_->Save(backup_file);

  // Close backup file
  gzclose(backup_file);
}

void Simulation::DoLoad(const string& input_dir,
                        double time,
                        const string& output_dir) {
  // Open backup file
  char backup_file_path[255];
  sprintf(backup_file_path,
          "%s/backup_%06" PRId32,
          input_dir.c_str(),
          static_cast<int32_t>(round(time)));
  gzFile backup_file = gzopen(backup_file_path, "r");
  if (backup_file == Z_NULL) {
    printf("%s:%d: error: could not open backup file %s\n",
            __FILE__, __LINE__, backup_file_path);
    exit(EXIT_FAILURE);
  }

  // Check file-type signature ?

  // Load PRNG
  Alea::instance().Load(backup_file);

  // Load simulation
  gzread(backup_file, &time_, sizeof(time_));
  gzread(backup_file, &timestep_, sizeof(timestep_));
  gzread(backup_file, &max_timestep_, sizeof(backup_file));
  gzread(backup_file, &dt_, sizeof(dt_));
  gzread(backup_file, &backup_dtimestep_, sizeof(backup_dtimestep_));

  pop_->Load(backup_file);
  grid_->Load(backup_file);

  // Re-place all the cells in the grid
  for(Cell* cell : pop_->cell_list()) {
    grid_->AddCell(cell);
  }

  // Close backup file
  gzclose(backup_file);

  // Open output files
  output_manager_.SetupForResume(input_dir, output_dir);
}

// =================================================================
//                           Private Methods
// =================================================================
/**
 * Compute the cell-neighbourhood of each cell
 */
void Simulation::ComputeNeighbourhood() {

  // For each cell, compute neighbourhood
  for (Cell* cell : pop_->cell_list_) {
    cell->ResetNeighbours();

    // Determine which voxel the cell is in
    auto cellCoords = grid_->GridCoords(cell->pos());

    // For each cell within the neighbouring voxels, add it to the current
    // cell's neighbor list if they are in contact with each other
    for (int16_t dX = -1; dX <= 1; dX++)
        for (int16_t dY = -1; dY <= 1; dY++)
        for (int16_t dZ = -1; dZ <= 1; dZ++) {
      // Replace all this by grid_->getClosebyCells ou getNeigbouringCells
      Coordinates<int16_t> curCoords{static_cast<int16_t>(cellCoords.x + dX),
                                     static_cast<int16_t>(cellCoords.y + dY),
                                     static_cast<int16_t>(cellCoords.z + dZ)};

      // If voxel is valid...
      if (grid_->IsValid(curCoords)) {
        // For each cell in voxel
        for (Cell* neighbCell : grid_->getCellsInVoxel(curCoords)) {
          // Skip over the current cell itself
          if (neighbCell->id() == cell->id()) continue;

          // If cells are in contact, add neighbCell to neighbours
          if (cell->Distance(neighbCell) <
              neighbCell->external_radius() + cell->external_radius()) {
            cell->AddNeighbour(neighbCell);
          }
        }
      }
    }
  }
}

/**
 * DEBUG only
 *
 * Compute the cell-neighborhood of each cell without any restriction 
 * regarding voxels and check that the neighborhood is the same
 */
void Simulation::CheckNeighbourhood() {
  cout << "Checking neighborhood at time " << time_ << endl;
  // Locate neighbors
  for (Cell* cell : pop_->cell_list())
  {
    std::vector<Cell*> neighbours;

    // For each cell add it the the current cell's neighbor list if they are
    // in contact with each other
    for (Cell* neighbCell : pop_->cell_list())
    {
      // Skip over the current cell itself
      if (neighbCell->id() == cell->id()) continue;

      if (cell->Distance(neighbCell) <
          neighbCell->external_radius() + cell->external_radius())
      {
        neighbours.push_back(neighbCell);
      }
    }

    if (neighbours.size() != cell->neighbours().size())
    {
      cout << "Error for cell " << cell->id() << endl;
      cout << "Neighbors:";
      for (auto& neighbour : neighbours) {
        cout << " " << neighbour->id();
      }
      cout << endl << "Computed:";
      for (auto& neighbour : cell->neighbours())
      {
        cout << " " << neighbour->id();
      }
      cout << endl;
    }
  }
}


/**
 * Compute mechanical and biochemical interactions between cells
 */
void Simulation::ComputeInteractions() {
  // Compute the cell-neighbourhood of each cell
  ComputeNeighbourhood();

  // For each cell compute the cumulative action of all its neighbours on it,
  // both mechanically and chemically
  for (Cell* cell : pop_->cell_list()) {
    cell->ComputeInteractions();
  }
}

/**
 * Actually apply the update
 *
 * Update cells, possibly kill them or make them divide.
 * Move them according to mechanical forces
 * and take a step in time
 */
void Simulation::ApplyUpdate() {
  list<Cell*> newCells;
  auto cell_it = std::begin(pop_->cell_list());

  // for (Cell* cell : pop_->cell_list()) {
  /** 
   *  sb:
   *  it is not possible to use range-based for loops
   *  when the elements of the list are removed. 
   *  When an element of the list is removed (with std::list::remove)
   *  the iterator becomes invalidated, and this results in 
   *  undefined behavior. 
   *  Solution: use a while loop and increment the iterator before
   *  the element of the list is being removed.
   */ 
  while ( cell_it != std::end(pop_->cell_list()) ) {
    Cell* cell = *cell_it; // get the cell 
    ++cell_it;             // safe to increment--cell_it is not used anywhere else in the loop
    cell->Update(dt_);

    UpdateMinMaxSignals(cell);

    // If cell marked as "to die", remove it from the grid and from the pop,
    // then step to the next cell
    if (cell->isDying()) {
      tree_->GetNodeFromId(cell->id())->set_time_of_tip(time_);
      grid_->RemoveCell(cell);
      pop_->RemoveCell(cell);
      continue;
    }

    // If cell marked as "to divide", make it do so
    if (cell->isDividing()) {
      Cell* newCell = cell->Divide();
      newCells.push_back(newCell);
      grid_->AddCell(newCell);
      tree_->AddTreeNode(tree_->GetNodeFromId(cell->id()),time_,max_timestep_*dt_,newCell->id(),newCell->cell_type());
    }
  }

  // Add new cells to population (if any)
  if (newCells.size() > 0) {
    pop_->AddCells(newCells);
  }

  time_ += dt_;
  timestep_ ++;
}

void Simulation::UpdateMinMaxSignals(Cell* cell) {
  int i = 0;
  for ( auto signal : Simulation::using_signals() ) { 
    if ( cell->get_output(signal) > max_signals_[i] ) {
      max_signals_[i] = cell->get_output(signal);
    }
    if ( cell->get_output(signal) < min_signals_[i] ) {
      min_signals_[i] = cell->get_output(signal);
    }
    i++;
  }
}
