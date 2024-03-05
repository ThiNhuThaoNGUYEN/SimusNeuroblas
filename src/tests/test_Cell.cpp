#include "gtest/gtest.h"

#include "Cell.h"
#include "Alea.h"
#include "movement/Mobile.h"
#include "movement/Immobile.h"


/*
 * Proxy class overriding abstract methods of Cell
 */

class CellProxy : public Cell {
protected:
  CellProxy() = delete;
  CellProxy(const CellProxy &model) = delete;

public:
  CellProxy(CellType cellType,
            const MoveBehaviour& move_behaviour,
            const Coordinates<double>& pos,
            double initial_volume,
            double volume_min,
            double doubling_time) :
          Cell(cellType, move_behaviour,
               pos, CellSize(initial_volume, volume_min),
               doubling_time) {}

  void doMove(Coordinates<double> dpos) {
    return Move(dpos);
  }

  void setPos(Coordinates<double> pos) {
    pos_ = pos;
  }

  Cell* Divide() {
    return nullptr;
  }

  bool isCycling() const {
    return true;
  }

  bool isDividing() const {
    return true;
  }

  bool isDying() const {
    return true;
  }

  double get_output(InterCellSignal signal) const {
    return 0.0;
  }

  void UpdateCyclingStatus() {}

  bool StopCycling() {
    return true;
  }

  bool StartCycling() {
    return true;
  }

  void InternalUpdate(const double& dt) {}

  void doAddMechForce(const CellProxy* other) {
    return AddMechForce(other);
  }


};


class TestCellMovement : public testing::Test {
protected:
  virtual void SetUp() {
    Alea::seed(155);
    Cell::set_volume_max_min_ratio(2.0);
    double radius = 0.5;
    double initial_volume = CellSize::compute_volume(radius);
    double volume_min = 0.5;
    double growth_rate = 0.2;
    initial_pos = new Coordinates<double>(3.0, 3.0, 3.0);
    cell = new CellProxy(STEM,
                         Mobile::instance(),
                         *initial_pos,
                         initial_volume,
                         volume_min,
                         growth_rate);
  }

  virtual void TearDown() {
    delete cell;
  }

  CellProxy* cell;
  Coordinates<double>* initial_pos;
};

TEST_F(TestCellMovement, NoMovement)
{
  Coordinates<double> movement = Coordinates<double>(0., 0., 0.);
  ASSERT_DOUBLE_EQ(3.0, cell->pos_x());
  ASSERT_DOUBLE_EQ(3.0, cell->pos_y());
  ASSERT_DOUBLE_EQ(3.0, cell->pos_z());
  cell->doMove(movement);
  ASSERT_DOUBLE_EQ(3.0, cell->pos_x());
  ASSERT_DOUBLE_EQ(3.0, cell->pos_y());
  ASSERT_DOUBLE_EQ(3.0, cell->pos_z());
}

TEST_F(TestCellMovement, SimpleMovement)
{
  Coordinates<double> movement = Coordinates<double>(4., 2., 10.);
  cell->doMove(movement);
  ASSERT_DOUBLE_EQ(initial_pos->x + movement.x, cell->pos_x());
  ASSERT_DOUBLE_EQ(initial_pos->y + movement.y, cell->pos_y());
  ASSERT_DOUBLE_EQ(initial_pos->z + movement.z, cell->pos_z());
}

TEST_F(TestCellMovement, MovementOutOfBoundariesInX)
{
  Coordinates<double> movement = Coordinates<double>(100., 2., 10.);
  cell->doMove(movement);
  ASSERT_DOUBLE_EQ(initial_pos->x, cell->pos_x());
  ASSERT_DOUBLE_EQ(initial_pos->y + movement.y, cell->pos_y());
  ASSERT_DOUBLE_EQ(initial_pos->z + movement.z, cell->pos_z());
}

TEST_F(TestCellMovement, MovementOutOfBoundariesInY)
{
  Coordinates<double> movement = Coordinates<double>(5., -4., 10.);
  cell->doMove(movement);
  ASSERT_DOUBLE_EQ(initial_pos->x + movement.x, cell->pos_x());
  ASSERT_DOUBLE_EQ(initial_pos->y, cell->pos_y());
  ASSERT_DOUBLE_EQ(initial_pos->z + movement.z, cell->pos_z());
}

TEST_F(TestCellMovement, MovementOutOfBoundariesInZ)
{
  Coordinates<double> movement = Coordinates<double>(30., 1., 40.);
  cell->doMove(movement);
  ASSERT_DOUBLE_EQ(initial_pos->x + movement.x, cell->pos_x());
  ASSERT_DOUBLE_EQ(initial_pos->y + movement.y, cell->pos_y());
  ASSERT_DOUBLE_EQ(initial_pos->z, cell->pos_z());
}

TEST_F(TestCellMovement, MovementOutOfBoundariesLimit)
{
  Coordinates<double> movement = Coordinates<double>(-2., 10., 37.);
  cell->doMove(movement);
  ASSERT_DOUBLE_EQ(initial_pos->x + movement.x, cell->pos_x());
  ASSERT_DOUBLE_EQ(initial_pos->y + movement.y, cell->pos_y());
  ASSERT_DOUBLE_EQ(initial_pos->z, cell->pos_z());
}

TEST_F(TestCellMovement, GravitationMovementWithoutImpulsion)
{
  double dt = 0.5;
  cell->Move(dt);
  ASSERT_DOUBLE_EQ(initial_pos->x, cell->pos_x());
  ASSERT_DOUBLE_EQ(initial_pos->y, cell->pos_y());
  ASSERT_GT(initial_pos->z, cell->pos_z());
}

TEST_F(TestCellMovement, NoGravitationMovementOnTheGround)
{
  Coordinates<double> initial_pos = Coordinates<double>(10.0, 20.0, 0.0);
  double dt = 0.5;
  cell->setPos(initial_pos);
  cell->Move(dt);
  ASSERT_DOUBLE_EQ(initial_pos.x, cell->pos_x());
  ASSERT_DOUBLE_EQ(initial_pos.y, cell->pos_y());
  ASSERT_DOUBLE_EQ(initial_pos.z, cell->pos_z());
}

class TestCellMechInteraction : public testing::Test {
protected:
  virtual void SetUp() {
    Alea::seed(155);
    Cell::set_volume_max_min_ratio(2.0);
    double radius = 0.5;
    double initial_volume = CellSize::compute_volume(radius);
    double volume_min = 0.5;
    double growth_rate = 0.2;
    Coordinates<double> initial_coordinates = Coordinates<double>(3.0, 3.0, 3.0);
    first_cell = new CellProxy(STEM,
                               Mobile::instance(),
                               Coordinates<double>(10.0, 10.0, 0.0),
                               initial_volume,
                               volume_min,
                               growth_rate);

    second_cell = new CellProxy(STEM,
                                Mobile::instance(),
                                Coordinates<double>(3.0, 3.0, 0.0),
                                initial_volume,
                                volume_min,
                                growth_rate);

  }

  virtual void TearDown() {
    delete first_cell;
    delete second_cell;
  }

  CellProxy* first_cell;
  CellProxy* second_cell;
};

TEST_F(TestCellMechInteraction, CellsMustGetCloser) {
  double dt = 0.5;
  Coordinates<double> first_cell_initial_pos = Coordinates<double>(15.0, 15.0, 0.0);
  Coordinates<double> second_cell_initial_pos = Coordinates<double>(35.0, 35.0, 0.0);

  first_cell->setPos(first_cell_initial_pos);
  second_cell->setPos(second_cell_initial_pos);

  double initial_distance = first_cell->Distance(second_cell);

  first_cell->doAddMechForce(second_cell);
  second_cell->doAddMechForce(first_cell);

  ASSERT_DOUBLE_EQ(initial_distance, first_cell->Distance(second_cell));

  first_cell->Move(dt);
  second_cell->Move(dt);

  ASSERT_GT(initial_distance, first_cell->Distance(second_cell));
}
