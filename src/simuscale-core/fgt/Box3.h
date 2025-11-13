#ifndef SIMUSCALE_BOX3_H__
#define SIMUSCALE_BOX3_H__

#include <cstdlib>
#include <cmath>
#include <vector>
#include <array>
#include <stdint.h>

using namespace std;

class Box3 {

 public:
  
  using real_type_ = float;
  using uint_type_ = uint32_t;
  Box3(); // cor
  Box3(uint_type_ id, array<real_type_, 3> c);
  virtual ~Box3() = default;

  //------- Accessors --------------
  vector<uint_type_>& p() { return p_; };
  uint_type_ index() { return index_; };
  array<real_type_, 3>& center() { return center_; };
  size_t n() { return p_.size(); };
  vector<real_type_>& B() { return B_; };

  //------- Setters ----------------
  void set_index(uint_type_ id) { index_ = id; };
  void set_center(array<real_type_, 3> c);

  //-------- Methods ---------------
  void push(const uint_type_ p);

 protected:
  vector<uint_type_> p_;          /* list of point indices */
  uint_type_ index_;              /* index of box */
  array<real_type_, 3> center_;   /* coordinates of box center */
  vector<real_type_> B_;          /* Taylor coefficients for target boxes */
};

#endif // SIMUSCALE_BOX3_H__
