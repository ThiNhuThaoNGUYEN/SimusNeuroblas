#include <string>
#include <iostream>

#include "gtest/gtest.h"
#include "Simulation.h"
#include "params/ParamFileReader.h"
#include "params/SimulationParams.h"

// using std::string;
// using std::cout;
// using std::endl;



class TestSimulation : public testing::Test
{
 protected:
  virtual void SetUp()
  {
    Alea prng;

    // Create the simulation
    ParamFileReader pl("data/param.in");
    pl.load();

    sim = new Simulation(string("."), string("."), &prng);
    sim->setup(pl.get_simParams());
  }
  virtual void TearDown()
  {

  }

  Simulation* sim;
};


TEST_F(TestSimulation, ParamInit)
{
  EXPECT_FLOAT_EQ(0.0, 0.0);
}
