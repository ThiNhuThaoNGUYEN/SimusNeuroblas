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

#include <getopt.h>
#include <gsl/gsl_rng.h>

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cinttypes>
#include <cassert>
#include <iomanip>
#include <iostream>
#include <string>

#include "config.h"
#include "params/ParamFileReader.h"
#include "Simulation.h"
#include "Alea.h"




using std::string;
using std::cout;
using std::endl;


void interpret_cmd_line_options(int argc, char* argv[],
                                string& input_dir,
                                string& output_dir,
                                double& resume_time);
void print_help(char* prog_path);
void print_version();



int main(int argc, char* argv[]) {
  cout << "main"<< endl;
  string input_dir;
  string output_dir;
  double resume_time = -1;

  interpret_cmd_line_options(argc, argv, input_dir, output_dir, resume_time);

  // Create the simulation
  if (resume_time == -1) {
    ParamFileReader paramFileReader(input_dir + "/param.in");
    paramFileReader.load();
    cout << "loaded parameters" << endl;

    // Simulation setup
    Simulation::Setup(paramFileReader.get_simParams(), output_dir);
  }
  else {
    printf("Resuming simulation from dir \"%s\" at time %f\n", input_dir.c_str(), resume_time);
    Simulation::Load(input_dir, resume_time, output_dir);
  }

  // Run simulation
  Simulation::Run();

  cout << endl;
  cout << "End of simulation." << endl;

  return 0;
}


void interpret_cmd_line_options(int argc, char* argv[],
                                string& input_dir,
                                string& output_dir,
                                double& resume_time) {
  // 1) Initialize command-line option variables with default values
  input_dir = "";
  output_dir = "";


  // 2) Define allowed options
  const char* options_list = "hVi:o:r:";
  static struct option long_options_list[] = {
      {"help",     no_argument,        NULL, 'h'},
      {"version",  no_argument,        NULL, 'V'},
      {"in",       required_argument,  NULL, 'i'},
      {"out",      required_argument,  NULL, 'o'},
      {"resume",   required_argument,  NULL, 'r'},
      {0, 0, 0, 0}
  };


  // 3) Get actual values of the command-line options
  int option;
  while ((option = getopt_long(argc, argv,
                               options_list,
                               long_options_list, NULL)) != -1) {
    switch (option)
    {
      case 'h' :
      case '?' : {
        // Print help message when requested, or when an option is not recognized
        print_help(argv[0]);
        exit(EXIT_SUCCESS);
      }
      case 'V' :{
        print_version();
        exit(EXIT_SUCCESS);
      }
      case 'i' : {
        input_dir = string(optarg);
        break;
      }
      case 'o' : {
        output_dir = string(optarg);
        break;
      }
      case 'r' : {
        resume_time = atof(optarg);
        break;
      }
      default : {
        // We never get here
        break;
      }
    }
  }

  // 4) Set undefined command line parameters to default values
  if (input_dir == "") {
    input_dir = string(".");
  }
  if (output_dir == "") {
    output_dir = string(".");
  }
}

void print_help(char* prog_path) {
  // Get the program file-name in prog_name (strip prog_path of the path)
  char* prog_name; // No new, it will point to somewhere inside prog_path
  if ((prog_name = strrchr(prog_path, '/'))) prog_name++;
  else prog_name = prog_path;

  cout << "SiMuScale - Multi-scale simulation framework\n\n"
      << "Usage: " << prog_name << " -h or --help\n"
      << "   or: " << prog_name << " -V or --version\n"
      << "   or: " << prog_name << " [-i INDIR] [-o OUTDIR] [-r TIME]\n\n"
      << "Options:\n"
      << "  -h, --help\n\tprint this help, then exit\n"
      << "  -V, --version\n\tprint version number, then exit\n"
      << "  -i, --in INDIR\n\tspecify input directory\n"
      << "  -o, --out OUTDIR\n\tspecify output directory\n"
      << "  -r, --resume TIME\n\tresume simulation at a given time\n";
}

void print_version() {
  cout << PROJECT_NAME << " " << PROJECT_VERSION << endl;
}
