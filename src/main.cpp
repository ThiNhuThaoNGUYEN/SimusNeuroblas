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
                                double& backup_time,
                                int& dump);
void print_help(char* prog_path);
void print_version();



int main(int argc, char* argv[]) {
  // cout << "main"<< endl;
  string input_dir;
  string output_dir;
  double backup_time = -1;
  int dump = 0; /* 0: no dump, 1: dump json, 2: dump lineage tree, 3: dump json and lineage tree */

  interpret_cmd_line_options(argc, argv, input_dir, output_dir, backup_time, dump);

  // Create the simulation
  if (backup_time == -1) {
    ParamFileReader paramFileReader(input_dir + "/param.in");
    paramFileReader.load();
    cout << "Parameters loaded" << endl;

    // Simulation setup
    Simulation::Setup(paramFileReader.get_simParams(), output_dir);
  }
  else if (dump == 0) {
    printf("Resuming simulation from dir \"%s\" at time %f\n", input_dir.c_str(), backup_time);
    Simulation::Load(input_dir, backup_time, output_dir);
  }
  else { // dump == true
    // printf("Dumping simulation from dir \"%s\" at time %f\n", input_dir.c_str(), backup_time);
    Simulation::Dump(input_dir, backup_time, dump);
    return 0; // do not run the simulation
  }

  // Run simulation
  cout << "Running simulation" << endl;
  Simulation::Run();

  cout << endl;
  cout << "End of simulation." << endl;

  return 0;
}


void interpret_cmd_line_options(int argc, char* argv[],
                                string& input_dir,
                                string& output_dir,
                                double& backup_time,
                                int& dump) {
  // 1) Initialize command-line option variables with default values
  input_dir = "";
  output_dir = "";


  // 2) Define allowed options
  const char* options_list = "hVi:o:r:j:l:d:";
  static struct option long_options_list[] = {
      {"help",     no_argument,        NULL, 'h'},
      {"version",  no_argument,        NULL, 'V'},
      {"in",       required_argument,  NULL, 'i'},
      {"out",      required_argument,  NULL, 'o'},
      {"resume",   required_argument,  NULL, 'r'},
      {"json",     required_argument,  NULL, 'j'},
      {"lineage",  required_argument,  NULL, 'l'},
      {"dump",     required_argument,  NULL, 'd'},
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
        backup_time = atof(optarg);
        break;
      }
      case 'j' : {
        backup_time = atof(optarg);
        dump = 1;
        break;
      }
      case 'l' : {
        backup_time = atof(optarg);
        dump = 2;
        break;
      }
      case 'd' : {
        backup_time = atof(optarg);
        dump = 3;
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

  cout << "Simuscale - Multiscale simulation framework\n\n"
      << "Usage: " << prog_name << " -h or --help\n"
      << "   or: " << prog_name << " -V or --version\n"
      << "   or: " << prog_name << " [-i INDIR] [-o OUTDIR] [-r TIME]\n"
      << "   or: " << prog_name << " [-i INDIR] [-d TIME]\n\n"
      << "Options:\n"
      << "  -h, --help\n\tprint this help, then exit\n"
      << "  -V, --version\n\tprint version number, then exit\n"
      << "  -i, --in INDIR\n\tspecify input directory\n"
      << "  -o, --out OUTDIR\n\tspecify output directory\n"
      << "  -r, --resume TIME\n\tresume simulation at a given time from backup\n"
      << "  -j, --json TIME\n\tprint simulation state time TIME from backup\n"
      << "  -l, --lineage TIME\n\tprint lineage tree at time TIME from backup\n"
      << "  -d, --dump TIME\n\tprint simulation state (json + lineage tree) at time TIME from backup\n";
}

void print_version() {
  cout << PROJECT_NAME << " " << PROJECT_VERSION << endl;
}
