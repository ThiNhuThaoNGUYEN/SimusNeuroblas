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

#ifndef SIMUSCALE_ALEA_H__
#define SIMUSCALE_ALEA_H__


// =================================================================
//                              Libraries
// =================================================================
#include <stdio.h>

#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <zlib.h>


// =================================================================
//                            Project Files
// =================================================================





// =================================================================
//                          Class declarations
// =================================================================



class Alea {
 public :
  // =================================================================
  //                        Singleton management
  // =================================================================
 private:
  static Alea instance_;
  Alea(void);
  Alea (const Alea&) = delete;
  Alea& operator= (const Alea&) = delete;
  virtual ~Alea(void);
 public:
  static Alea& instance() {return instance_;};

  // =================================================================
  //                              Accessors
  // =================================================================

  // =================================================================
  //                              Operators
  // =================================================================

  // =================================================================
  //                            Public Methods
  // =================================================================
  /** Initializes (or 'seeds') the random number generator */
  static void seed(int32_t seed);

  /** Generates a random number in uniformly distributed in [0, max) */
  static inline int32_t random(int32_t max);

  /** Generates a random number in uniformly distributed in [0, 1) */
  static inline double random(void);

  /** Generates a Gaussian random number with 0 mean and unit variance
   *  using polar form of the Box-Muller transformation */
  static inline double gaussian_random(void);
  static inline double exponential_random(double param);

  void Save(gzFile backup_file) const;
  void Load(gzFile backup_file);
  void Save(FILE* backup_file) const;
  void Load(FILE* backup_file);


  // =================================================================
  //                           Public Attributes
  // =================================================================

 protected :
  // =================================================================
  //                           Protected Methods
  // =================================================================

  // =================================================================
  //                          Protected Attributes
  // =================================================================
  gsl_rng* random_number_generator_;
};

/*
 * Returns a pseudo-random int32_t, uniform distribution in [0,max)
 */
int32_t Alea::random(int32_t max) {
  // Since max is an int32, it is safe to cast the result to an int32
  return static_cast<int32_t>(gsl_rng_uniform_int(
      instance_.random_number_generator_, (unsigned long int) max));
};

/*
 * Returns a pseudo-random double, uniform distribution in [0,1)
 */
double Alea::random(void) {
  return gsl_rng_uniform(instance_.random_number_generator_);
}


double Alea::gaussian_random(void) {
  return gsl_ran_gaussian(instance_.random_number_generator_, 1.0);
}
double Alea::exponential_random(double param) {
  return gsl_ran_exponential(instance_.random_number_generator_, param);
}

#endif // SIMUSCALE_ALEA_H__
