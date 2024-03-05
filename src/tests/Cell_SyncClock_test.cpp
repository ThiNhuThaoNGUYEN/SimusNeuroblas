#include "gtest/gtest.h"

#include <string>
#include <iostream>

#include "CellType.h"
#include "plugins/Cell_SyncClock.h"
#include "Alea.h"
#include "movement/Mobile.h"
#include "movement/Immobile.h"

// using std::string;
// using std::cout;
// using std::endl;

/**
 * This class does nothing but expose protected members of its parent class
 * so that we can test it more thoroughly and precisely
 */
class ExposedCell_SyncClock : public Cell_SyncClock
{
 public:
  ExposedCell_SyncClock(CellType type,
                  const MoveBehaviour& move_behaviour,
                  Coordinates<double> pos,
                  double initial_volume,
                  double volume_min,
                  double doubling_time) :
    Cell_SyncClock(type, move_behaviour,
             pos, initial_volume, volume_min,
             doubling_time) {
  };

  double per_scal(){return per_scal_;};
  double* internal_state(){return internal_state_;};
  double volume(){return size_.volume();};
  double volume_min(){return size_.volume_min();};
  static int do_compute_dydt(double t,
                             const double y[],
                             double f[],
                             void *params) {
    return compute_dydt(t, y, f, params);
  };
  void do_ODE_update(const double dt) {
    ODE_update(dt);
    // TODO <david.parsons@inria.fr> Check how to remove this dt without
    // further coupling Cell and Simulation
  };
};



class TestCell_SyncClock : public testing::Test {
 protected:
  virtual void SetUp() {
    Alea::seed(155);
    Cell::set_volume_max_min_ratio(2.0);
    double cell_radius = 0.5 / 0.95;
    double initial_volume = 4 * M_PI / 3 * pow(cell_radius, 3);
    double volume_min = 0.5;
    double doubling_time = 0.02;

    cell = new ExposedCell_SyncClock(STEM,
                               Immobile::instance(),
                               Coordinates<double>(3.0, 3.0, 3.0),
                               initial_volume,
                               volume_min,
                               doubling_time);
  };
  virtual void TearDown() {
    delete cell;
  };

  ExposedCell_SyncClock* cell;
};


TEST_F(TestCell_SyncClock, ParamInit) {
  EXPECT_FLOAT_EQ(1.0022254, cell->per_scal());
  EXPECT_FLOAT_EQ(0.61069983, cell->volume());
  EXPECT_TRUE(cell->volume() >= cell->volume_min());
}

TEST_F(TestCell_SyncClock, IntStateInit) {
  EXPECT_FLOAT_EQ(0.032616317, cell->internal_state()[Cell_SyncClock::CY_X]);
  EXPECT_FLOAT_EQ(0.018155437, cell->internal_state()[Cell_SyncClock::CY_Z]);
  EXPECT_FLOAT_EQ(0.046056148, cell->internal_state()[Cell_SyncClock::CD_DEATH]);
  EXPECT_FLOAT_EQ(0.027637674, cell->internal_state()[Cell_SyncClock::Y1]);
  EXPECT_FLOAT_EQ(0.0086572953, cell->internal_state()[Cell_SyncClock::Y2]);
  EXPECT_FLOAT_EQ(0.014810348, cell->internal_state()[Cell_SyncClock::Y3]);
  EXPECT_FLOAT_EQ(0.031031255, cell->internal_state()[Cell_SyncClock::Y4]);
  EXPECT_FLOAT_EQ(0.01454461, cell->internal_state()[Cell_SyncClock::Y5]);
  EXPECT_FLOAT_EQ(0.0053313337, cell->internal_state()[Cell_SyncClock::Y6]);
  EXPECT_FLOAT_EQ(0.0075390814, cell->internal_state()[Cell_SyncClock::Y7]);
  EXPECT_FLOAT_EQ(0.0092718983, cell->internal_state()[Cell_SyncClock::V]);
  EXPECT_FLOAT_EQ(0.041629981, cell->internal_state()[Cell_SyncClock::X1]);
  EXPECT_FLOAT_EQ(0.012005323, cell->internal_state()[Cell_SyncClock::X2]);
}

TEST_F(TestCell_SyncClock, ODEUpdate) {
  cell->do_ODE_update(0.01);

  // Check internal state
  EXPECT_FLOAT_EQ(0.032638807, cell->internal_state()[Cell_SyncClock::CY_X]);
  EXPECT_FLOAT_EQ(0.018201733, cell->internal_state()[Cell_SyncClock::CY_Z]);
  EXPECT_FLOAT_EQ(0.041673325, cell->internal_state()[Cell_SyncClock::CD_DEATH]);
  EXPECT_FLOAT_EQ(0.028275104, cell->internal_state()[Cell_SyncClock::Y1]);
  EXPECT_FLOAT_EQ(0.0086227823, cell->internal_state()[Cell_SyncClock::Y2]);
  EXPECT_FLOAT_EQ(0.014811833, cell->internal_state()[Cell_SyncClock::Y3]);
  EXPECT_FLOAT_EQ(0.030691033, cell->internal_state()[Cell_SyncClock::Y4]);
  EXPECT_FLOAT_EQ(0.014543349, cell->internal_state()[Cell_SyncClock::Y5]);
  EXPECT_FLOAT_EQ(0.0053794421, cell->internal_state()[Cell_SyncClock::Y6]);
  EXPECT_FLOAT_EQ(0.0075338664, cell->internal_state()[Cell_SyncClock::Y7]);
  EXPECT_FLOAT_EQ(0.0089930352, cell->internal_state()[Cell_SyncClock::V]);
  EXPECT_FLOAT_EQ(0.039997648, cell->internal_state()[Cell_SyncClock::X1]);
  EXPECT_FLOAT_EQ(0.012317484, cell->internal_state()[Cell_SyncClock::X2]);
}

TEST_F(TestCell_SyncClock, internalODEUpdate) {
  double d_int_st[13];
  cell->do_compute_dydt(0., cell->internal_state(), d_int_st, cell);

  // Check that internal state hasn't changed
  EXPECT_FLOAT_EQ(0.032616317, cell->internal_state()[Cell_SyncClock::CY_X]);
  EXPECT_FLOAT_EQ(0.018155437, cell->internal_state()[Cell_SyncClock::CY_Z]);
  EXPECT_FLOAT_EQ(0.046056148, cell->internal_state()[Cell_SyncClock::CD_DEATH]);
  EXPECT_FLOAT_EQ(0.027637674, cell->internal_state()[Cell_SyncClock::Y1]);
  EXPECT_FLOAT_EQ(0.0086572953, cell->internal_state()[Cell_SyncClock::Y2]);
  EXPECT_FLOAT_EQ(0.014810348, cell->internal_state()[Cell_SyncClock::Y3]);
  EXPECT_FLOAT_EQ(0.031031255, cell->internal_state()[Cell_SyncClock::Y4]);
  EXPECT_FLOAT_EQ(0.01454461, cell->internal_state()[Cell_SyncClock::Y5]);
  EXPECT_FLOAT_EQ(0.0053313337, cell->internal_state()[Cell_SyncClock::Y6]);
  EXPECT_FLOAT_EQ(0.0075390814, cell->internal_state()[Cell_SyncClock::Y7]);
  EXPECT_FLOAT_EQ(0.0092718983, cell->internal_state()[Cell_SyncClock::V]);
  EXPECT_FLOAT_EQ(0.041629981, cell->internal_state()[Cell_SyncClock::X1]);
  EXPECT_FLOAT_EQ(0.012005323, cell->internal_state()[Cell_SyncClock::X2]);

  // Check value issued by func (=> d_int_st)
  EXPECT_FLOAT_EQ(0.0022497026, d_int_st[Cell_SyncClock::CY_X]);
  EXPECT_FLOAT_EQ(0.0046307072, d_int_st[Cell_SyncClock::CY_Z]);
  EXPECT_FLOAT_EQ(-0.46056148, d_int_st[Cell_SyncClock::CD_DEATH]);
  EXPECT_FLOAT_EQ(0.063788474, d_int_st[Cell_SyncClock::Y1]);
  EXPECT_FLOAT_EQ(-0.0034646899, d_int_st[Cell_SyncClock::Y2]);
  EXPECT_FLOAT_EQ(0.0001549007, d_int_st[Cell_SyncClock::Y3]);
  EXPECT_FLOAT_EQ(-0.034210019, d_int_st[Cell_SyncClock::Y4]);
  EXPECT_FLOAT_EQ(-8.6901346e-05, d_int_st[Cell_SyncClock::Y5]);
  EXPECT_FLOAT_EQ(0.0048190523, d_int_st[Cell_SyncClock::Y6]);
  EXPECT_FLOAT_EQ(-0.00052404142, d_int_st[Cell_SyncClock::Y7]);
  EXPECT_FLOAT_EQ(-0.028430296, d_int_st[Cell_SyncClock::V]);
  EXPECT_FLOAT_EQ(-0.16651992, d_int_st[Cell_SyncClock::X1]);
  EXPECT_FLOAT_EQ(0.035934269, d_int_st[Cell_SyncClock::X2]);
}

TEST_F(TestCell_SyncClock, divide) {
  ExposedCell_SyncClock* newCell = (ExposedCell_SyncClock*) cell->Divide();

  // Check that internal state hasn't changed
  EXPECT_FLOAT_EQ(0.032616317, cell->internal_state()[Cell_SyncClock::CY_X]);
  EXPECT_FLOAT_EQ(0.018155437, cell->internal_state()[Cell_SyncClock::CY_Z]);
  EXPECT_FLOAT_EQ(0.046056148, cell->internal_state()[Cell_SyncClock::CD_DEATH]);
  EXPECT_FLOAT_EQ(0.027637674, cell->internal_state()[Cell_SyncClock::Y1]);
  EXPECT_FLOAT_EQ(0.0086572953, cell->internal_state()[Cell_SyncClock::Y2]);
  EXPECT_FLOAT_EQ(0.014810348, cell->internal_state()[Cell_SyncClock::Y3]);
  EXPECT_FLOAT_EQ(0.031031255, cell->internal_state()[Cell_SyncClock::Y4]);
  EXPECT_FLOAT_EQ(0.01454461, cell->internal_state()[Cell_SyncClock::Y5]);
  EXPECT_FLOAT_EQ(0.0053313337, cell->internal_state()[Cell_SyncClock::Y6]);
  EXPECT_FLOAT_EQ(0.0075390814, cell->internal_state()[Cell_SyncClock::Y7]);
  EXPECT_FLOAT_EQ(0.0092718983, cell->internal_state()[Cell_SyncClock::V]);
  EXPECT_FLOAT_EQ(0.041629981, cell->internal_state()[Cell_SyncClock::X1]);
  EXPECT_FLOAT_EQ(0.012005323, cell->internal_state()[Cell_SyncClock::X2]);

  // Check that internal state was correctly copied to daughter cell
  EXPECT_FLOAT_EQ(0.032616317, newCell->internal_state()[Cell_SyncClock::CY_X]);
  EXPECT_FLOAT_EQ(0.018155437, newCell->internal_state()[Cell_SyncClock::CY_Z]);
  EXPECT_FLOAT_EQ(0.046056148, newCell->internal_state()[Cell_SyncClock::CD_DEATH]);
  EXPECT_FLOAT_EQ(0.027637674, newCell->internal_state()[Cell_SyncClock::Y1]);
  EXPECT_FLOAT_EQ(0.0086572953, newCell->internal_state()[Cell_SyncClock::Y2]);
  EXPECT_FLOAT_EQ(0.014810348, newCell->internal_state()[Cell_SyncClock::Y3]);
  EXPECT_FLOAT_EQ(0.031031255, newCell->internal_state()[Cell_SyncClock::Y4]);
  EXPECT_FLOAT_EQ(0.01454461, newCell->internal_state()[Cell_SyncClock::Y5]);
  EXPECT_FLOAT_EQ(0.0053313337, newCell->internal_state()[Cell_SyncClock::Y6]);
  EXPECT_FLOAT_EQ(0.0075390814, newCell->internal_state()[Cell_SyncClock::Y7]);
  EXPECT_FLOAT_EQ(0.0092718983, newCell->internal_state()[Cell_SyncClock::V]);
  EXPECT_FLOAT_EQ(0.041629981, newCell->internal_state()[Cell_SyncClock::X1]);
  EXPECT_FLOAT_EQ(0.012005323, newCell->internal_state()[Cell_SyncClock::X2]);

  // Check cell's and newCell's resp. positions
  EXPECT_FLOAT_EQ(2.9262195, cell->pos_x());
  EXPECT_FLOAT_EQ(2.9822352, cell->pos_y());
  EXPECT_FLOAT_EQ(3.0776541, cell->pos_z());
  EXPECT_FLOAT_EQ(3.0737805, newCell->pos_x());
  EXPECT_FLOAT_EQ(3.0177648, newCell->pos_y());
  EXPECT_FLOAT_EQ(2.9223459, newCell->pos_z());

  // Check cell's and newCell's resp. radii and volumes
  EXPECT_FLOAT_EQ(0.39685026, cell->internal_radius());
  EXPECT_FLOAT_EQ(0.41773713, cell->external_radius());
  EXPECT_FLOAT_EQ(0.30534992, cell->volume());
  EXPECT_FLOAT_EQ(0.39685026, newCell->internal_radius());
  EXPECT_FLOAT_EQ(0.41773713, newCell->external_radius());
  EXPECT_FLOAT_EQ(0.30534992, newCell->volume());
}
