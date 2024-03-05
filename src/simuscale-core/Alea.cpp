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
#include "Alea.h"

#include <cstdio>
#include <cstdlib>
#include <cassert>

#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>



// =================================================================
//                    Definition of static attributes
// =================================================================
// The singleton instance
Alea Alea::instance_;


// =================================================================
//                             Constructors
// =================================================================
Alea::Alea(void) {
  random_number_generator_ = gsl_rng_alloc(gsl_rng_default);
}


// =================================================================
//                             Destructor
// =================================================================
Alea::~Alea(void) {
  gsl_rng_free(random_number_generator_);
}


// =================================================================
//                            Public Methods
// =================================================================
void Alea::seed(int32_t seed) {
  gsl_rng_set(instance_.random_number_generator_, (unsigned long int)seed);
}

void Alea::Save(gzFile backup_file) const {
  gzwrite(backup_file,
          gsl_rng_state(random_number_generator_),
          gsl_rng_size(random_number_generator_));
}

void Alea::Load(gzFile backup_file) {
  gzread(backup_file,
         gsl_rng_state(random_number_generator_),
         gsl_rng_size(random_number_generator_));
}

void Alea::Save(FILE* backup_file) const {
  assert(gsl_rng_fwrite(backup_file, random_number_generator_) == 0);
}

void Alea::Load(FILE* backup_file) {
  assert(gsl_rng_fread(backup_file, random_number_generator_) == 0);
}




// =================================================================
//                           Protected Methods
// =================================================================



// =================================================================
//                          Non inline accessors
// =================================================================
