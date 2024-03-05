#include <string>

#include "gtest/gtest.h"
#include "params/SimulationParams.h"
#include "params/PopulationParams.h"
#include "params/CellParams.h"
#include "params/ParamFileReader.h"
#include "plugins/Cell_SyncClock.h"
#include "Alea.h"

using std::string;


TEST(TestParamLoader, GeneralTest) {
  ParamFileReader reader("data/param.in");
  string indir = ".";
  string outdir = ".";

  reader.load();
  const SimulationParams simParams = reader.get_simParams();
  const CellParams& cellParams = simParams.cell_params();

  // Check simulation
  EXPECT_EQ(240, simParams.maxtime());
  EXPECT_EQ(0.05, simParams.dt());

  EXPECT_EQ(static_cast<CellFormalism>(Cell_SyncClock::classId_), simParams.niche_params().formalism());
  EXPECT_EQ(1.0, simParams.niche_params().internal_radius());

  EXPECT_EQ(static_cast<decltype(simParams.pop_params().size())>(1),
            simParams.pop_params().size());
  std::list<PopulationParams>::const_iterator it = simParams.pop_params().begin();
  EXPECT_EQ(20, it->nb_cells());
  EXPECT_EQ(STEM, it->cell_type());
  EXPECT_EQ(static_cast<CellFormalism>(Cell_SyncClock::classId_), it->formalism());
  EXPECT_EQ(10, it->doubling_time());
  EXPECT_EQ(1.0, it->volume_min());
  
  if (simParams.autoseed())
    Alea::seed(time(NULL));
  else
    Alea::seed(simParams.seed());
  EXPECT_EQ(64151, Alea::random(99999));
  EXPECT_EQ(37816, Alea::random(99999));

  // Check cells
  EXPECT_EQ(0.9, cellParams.radii_ratio());
}
