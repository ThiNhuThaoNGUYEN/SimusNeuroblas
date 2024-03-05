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
#include "ParamFileReader.h"

#include <cstdlib>
#include <cstring>
#include <cassert>
#include <ctime>

#include "Alea.h"
#include "Simulation.h"
#include "Cell.h"




//##############################################################################
//                                                                             #
//                          Class ParamFileReader                              #
//                                                                             #
//##############################################################################

// =================================================================
//                    Definition of static attributes
// =================================================================
const char kTabChar = 0x09;

// =================================================================
//                             Constructors
// =================================================================
ParamFileReader::ParamFileReader() {
  open_input_file();
}


ParamFileReader::ParamFileReader(const string& file_name) {
  _param_file_name = file_name;
  open_input_file();
}

// =================================================================
//                             Destructor
// =================================================================
ParamFileReader::~ParamFileReader() {
  fclose(_param_file);
}

// =================================================================
//                            Public Methods
// =================================================================
void ParamFileReader::interpret_line(f_line* line) {
  // SIMULATION PARAMETERS
  if (strcmp(line->words[0], "PRNG_SEED") == 0) {
    if(strcmp(line->words[1], "AUTO") == 0) {
      simParams.autoseed_ = true;
    }
    else {
      simParams.autoseed_ = false;
      simParams.seed_ = atol(line->words[1]);
    }
  }
  else if (strcmp(line->words[0], "MAXTIME") == 0) {
    simParams.maxtime_ = atof(line->words[1]);
  }
  else if (strcmp(line->words[0], "DT") == 0) {
    simParams.dt_ = atof(line->words[1]);
  }
  else if (strcmp(line->words[0], "BACKUP_DT") == 0) {
    simParams.backup_dt_ = atof(line->words[1]);
  }
  else if (strcmp(line->words[0], "NICHE") == 0) {
    if (line->nb_words != 3) {
      printf("ERROR in param file \"%s\" on line %" PRId32
                 ": incorrect number of parameters for keyword \"%s\".\n",
             _param_file_name.c_str(), _cur_line, line->words[0]);
      exit(EXIT_FAILURE);
    }
    try {
      simParams.setNicheParams(line->words[1], atof(line->words[2]));
    }
    catch (const string& error) {
      printf("ERROR in param file \"%s\" on line %" PRId32
                 ": %s.\n",
             _param_file_name.c_str(), _cur_line, error.c_str());
      exit(EXIT_FAILURE);
    }
  }
  else if (strcmp(line->words[0], "ADD_POPULATION") == 0) {
    if (line->nb_words < 7 | line->nb_words > 8) {
      printf("ERROR in param file \"%s\" on line %" PRId32
                 ": incorrect number of parameters for keyword \"%s\".\n",
             _param_file_name.c_str(), _cur_line, line->words[0]);
      exit(EXIT_FAILURE);
    }

    try {
      if ( line->nb_words == 7 ) {
        simParams.addPop(atol(line->words[1]),    /* number of cells    */
                       line->words[2],            /* cell type          */ 
                       line->words[3],            /* formalism (plugin) */
                       line->words[4],            /* movement behavior  */ 
                       atof(line->words[5]),      /* doubling time      */ 
                       atof(line->words[6]),      /* volume min         */ 
                       "");                       /* init file          */
      }
      else {
        simParams.addPop(atol(line->words[1]),    /* number of cells    */
                       line->words[2],            /* cell type          */ 
                       line->words[3],            /* formalism (plugin) */
                       line->words[4],            /* movement behavior  */ 
                       atof(line->words[5]),      /* doubling time      */ 
                       atof(line->words[6]),      /* volume min         */ 
                       line->words[7]);           /* init file          */
      }
    }
    catch (const string& error) {
      printf("ERROR in param file \"%s\" on line %" PRId32
                 ": %s.\n",
             _param_file_name.c_str(), _cur_line, error.c_str());
      exit(EXIT_FAILURE);
    }
  }

  // CELL UNIVERSAL PARAMETERS
  else if (strcmp(line->words[0], "R_RATIO") == 0) {
    simParams.cell_params_.radii_ratio_ = atof(line->words[1]);
  }
  
  // CELL UNIVERSAL PARAMETERS
  else if (strcmp(line->words[0], "VOL_MAX_MIN_RATIO") == 0) {
    try {
      if ( atof(line->words[1]) > 2.0 ) {
        simParams.cell_params_.volume_max_min_ratio_ = atof(line->words[1]);
      }
      else {
        printf("ERROR in param file \"%s\" on line %" PRId32
                 ": %s.\n",
             _param_file_name.c_str(), _cur_line, "VOL_MAX_MIN_RATIO must be larger than 2.0");
        exit(EXIT_FAILURE);
      }
    }
    catch (const string& error) {
      printf("ERROR in param file \"%s\" on line %" PRId32
                 ": %s.\n",
             _param_file_name.c_str(), _cur_line, error.c_str());
      exit(EXIT_FAILURE);
    }
  }

  // LIST OF SIGNALS TO RECORD TO OUTPUT FILES 
  else if (strcmp(line->words[0], "SIGNAL") == 0) {
    try {
      simParams.using_signals_.push_back (simParams.StrToInterCellSignal(line->words[1]));
    }
    catch (const string& error) {
      printf("ERROR in param file \"%s\" on line %" PRId32
                 ": %s.\n",
             _param_file_name.c_str(), _cur_line, error.c_str());
      exit(EXIT_FAILURE);

    }
  }

  // WORLD SIZE 
  else if (strcmp(line->words[0], "WORLDSIZE") == 0) {
    try {
      simParams.worldsize_params_.size_.x = atof(line->words[1]);
      simParams.worldsize_params_.size_.y = atof(line->words[2]);
      simParams.worldsize_params_.size_.z = atof(line->words[3]);
    }
    catch (const string& error) {
      printf("ERROR in param file \"%s\" on line %" PRId32
                 ": %s.\n",
             _param_file_name.c_str(), _cur_line, error.c_str());
      exit(EXIT_FAILURE);

    }

    if ( simParams.worldsize_params_.size_.x > 100 ||
         simParams.worldsize_params_.size_.y > 100 ||
         simParams.worldsize_params_.size_.z > 100 ) {
      printf("ERROR in param file \"%s\" on line %" PRId32
                 ": max world size is 100.\n",
             _param_file_name.c_str(), _cur_line);
      exit(EXIT_FAILURE);
    }
  }
  
  // WORLD MARGIN
  else if (strcmp(line->words[0], "WORLDMARGIN") == 0) {
    try {
      simParams.worldsize_params_.margin_.x = atof(line->words[1]);
      simParams.worldsize_params_.margin_.y = atof(line->words[2]);
      simParams.worldsize_params_.margin_.z = atof(line->words[3]);
    }
    catch (const string& error) {
      printf("ERROR in param file \"%s\" on line %" PRId32
                 ": %s.\n",
             _param_file_name.c_str(), _cur_line, error.c_str());
      exit(EXIT_FAILURE);

    }
  }

  // Use contact area (1, default) for weighing interactions
  // or a constant weight of one (0)
  else if (strcmp(line->words[0], "USECONTACTAREA") == 0) {
    try {
      simParams.usecontactarea_ = static_cast<bool>(atol(line->words[1]));
    }
    catch (const string& error) {
      printf("ERROR in param file \"%s\" on line %" PRId32
                 ": %s.\n",
             _param_file_name.c_str(), _cur_line, error.c_str());
      exit(EXIT_FAILURE);

    }
  }

  // Write orientation vector in trajectory file (default=0)
  else if (strcmp(line->words[0], "WRITEORIENTATION") == 0) {
    try {
      simParams.output_orientation_ = static_cast<bool>(atol(line->words[1]));
    }
    catch (const string& error) {
      printf("ERROR in param file \"%s\" on line %" PRId32
                 ": %s.\n",
             _param_file_name.c_str(), _cur_line, error.c_str());
      exit(EXIT_FAILURE);

    }
  }

  // CELL_ODE PARAMETERS
  else if (strcmp(line->words[0], "ODESYSTEMSIZE") == 0) {
    printf("ERROR in param file \"%s\" on line %d:"
           " key word no longer supported \"%s\"\n",
           _param_file_name.c_str(), _cur_line, line->words[0]);
    exit(EXIT_FAILURE);
  }
  else if (strcmp(line->words[0], "ODENBPARAMS") == 0) {
    printf("ERROR in param file \"%s\" on line %d:"
           " key word no longer supported \"%s\"\n",
           _param_file_name.c_str(), _cur_line, line->words[0]);
    exit(EXIT_FAILURE);
  }

  // ERRONEOUS KEYWORD
  else {
    printf("ERROR in param file \"%s\" on line %d:"
           " undefined key word \"%s\"\n",
           _param_file_name.c_str(), _cur_line, line->words[0]);
    exit(EXIT_FAILURE);
  }
}

void ParamFileReader::load() {
  // This rewind is only necessary when using multiple param files
  rewind(_param_file);

  _cur_line = 0;
  f_line* line;

  // TODO : write line = new f_line(_param_file) => f_line::f_line(char*)
  while ((line = get_line()) != NULL) {
    interpret_line(line);
    delete line;
  }
}



// =================================================================
//                           Protected Methods
// =================================================================
void ParamFileReader::open_input_file() {
  _param_file = fopen(_param_file_name.c_str(),  "r");
  if (_param_file == NULL) {
    printf("ERROR : couldn't open file %s\n",
           _param_file_name.c_str());
    exit(EXIT_FAILURE);
  }
}

void ParamFileReader::format_line(f_line* formatted_line,
                                  char* line,
                                  bool* line_is_interpretable) {
  int16_t i = 0;
  int16_t j;

  // Parse line
  while (line[i] != '\n' && line[i] != '\0' && line[i] != '\r') {
    j = 0;

    // Flush white spaces and tabs
    while (line[i] == ' ' || line[i] == kTabChar) i++;

    // Check comments
    if (line[i] == '#') break;

    // If we got this far, there is content in the line
    *line_is_interpretable = true;

    // Parse word
    while (line[i] != ' ' && line[i] != kTabChar && line[i] != '\n' &&
        line[i] != '\0' && line[i] != '\r') {
      formatted_line->words[formatted_line->nb_words][j++] = line[i++];
    }

    // Add '\0' at end of word if it's not empty (line ending with space or tab)
    if (j != 0) {
      formatted_line->words[formatted_line->nb_words++][j] = '\0';
    }
  }
}

f_line* ParamFileReader::get_line() {
  char line[255];
  f_line* formated_line = new f_line();

  // Found line that is neither a comment nor empty
  bool found_interpretable_line = false;

  while (!feof(_param_file) && !found_interpretable_line) {
    if (!fgets(line, 255, _param_file)) {
      delete formated_line;
      return NULL;
    }
    _cur_line++;
    format_line(formated_line, line, &found_interpretable_line);
  }

  if (found_interpretable_line) {
    return formated_line;
  }
  else {
    delete formated_line;
    return NULL;
  }
}

f_line::f_line() {
  nb_words = 0;
}


