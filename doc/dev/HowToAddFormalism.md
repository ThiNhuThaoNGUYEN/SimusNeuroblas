Create a new cell formalism for SiMuScale
==========================================

1. Write your plugin (using the easy or the hard way described below)

2. Add Cell_XXX to the list of plugins


## Write your plugin the easy way
1. cd into the plugins directory
2. Copy the boilerplate files Cell_Boilerplate.h and Cell_Boilerplate.cpp into,
e.g., Cell_XXX for your XXX formalism
3. Rename the class Cell_Boilerplate and the include guard
SIMUSCALE_CELL_BOILERPLATE_H__
4. Specify a keyword and a classID for your class (attributes classKW_
and classId_).
They must both be unique among the plugins
5. Look for the <TODO> tags and insert your code there


## Write your plugin the hard way
### Create class Cell_XXX, derived from base class Cell
class Cell_XXX: public Cell

### Generate an id for your new class
This id can be e.g. the CRC32 hash of the file content
In Cell_XXX.h:
`static constexpr CellFormalism classId_ = <CRC32_hash>;`
In Cell_XXX.cpp:
`constexpr CellFormalism Cell_ODE::classId_;`

### Create 2 factories (factory and loader-factory) for your class
The first factory (Cell::CellFactory) will be used to create instances of your
class at initialization time
The second one (Cell::CellLoader) will be used to restore saved instances from
a backup file
NOTA: You will most probably need a couple of constructors taking the same list
of parameters as their factory counterpart

### Register your class with its Id, keyword, factory and loader-factory

#### Copy the following lines in your class declaration
 private:
  /** dummy attribute - allows to register class in Simuscale statically */
  static bool registered_;

#### Actually register your class
bool Cell_ODE::registered_ =
    Cell::RegisterClass(classId_, classKeyWd, factory, loader);

### Override pure abstract methods of base class
This is where you actually define the specifics of the formalism you are adding

