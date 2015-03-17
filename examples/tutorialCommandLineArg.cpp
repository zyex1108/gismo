/** @file tutorialCommandLineArg.cpp

    @brief Tutorial on how to use command line parser in G+Smo.

    This file is part of the G+Smo library.

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.

    Author(s): J. Speh
*/

#include <iostream>
#include <string>
#include <gismo.h>

using namespace gismo;

int main(int argc, char* argv[])
{
    // Variables that will take values from the command line
    std::string string("none");  // string variable default value
    real_t flNumber = 1.0;       // flNumber variable default value
    int number = 1;              // number variable default value
    bool boolean = false;        // boolean variable default value
    std::string plainString;     // argument of reading plain string

    // -----------------------------------------------------------------
    // First we Initialize the object that sets up and parses command line arguments
    //
    // This defines by default 3 arguments that can be readily used:
    //
    // --,  --ignore_rest
    //  Ignores the rest of the labeled arguments following this flag.
    //
    // --version
    // Displays version information and exits.
    //
    // -h,  --help
    // Displays usage information for all other arguments and exits.
    //
    gsCmdLine cmd("Tutorial Command Line Arguments");
    
    // -----------------------------------------------------------------
    // General syntax to add an argument:
    // cmd.addType("f", "--flag", "Description", destination)
    // "f"    is the short flag: -f
    // "flag" is the long  flag: --flag (same effect as "-f")
    // "Description" describes what this argument is about
    // destination is the variable that will have the value of the input argument

    // -----------------------------------------------------------------
    // Adding a string argument, given by the "-s" (or "--stringArg") flag
    // If set, string is updated to the input value, otherwise string remains untouched
    cmd.addString("s", "stringArg", 
                  "Description of string command line argument.",
                  string);

    // -----------------------------------------------------------------
    // Adding a string argument, given by the "-i" (or "--num") flag
    // If set, number is updated to the input value, otherwise number remains untouched
    cmd.addInt   ("i", "num", 
                  "Description of int command line argument", 
                  number);

    // -----------------------------------------------------------------
    // Adding a float argument, given by the "-r" (or "--real") flag
    // If set, flNumber is updated to the input value, otherwise flNumber remains untouched
    cmd.addReal  ("r", "real", 
                  "Description of float command line argument", 
                  flNumber);

    // -----------------------------------------------------------------
    // Adding a switch argument, given by the "--bool" flag
    // If set, boolean is updated to the input value, otherwise boolean remains untouched
    cmd.addSwitch("bool","Description of the switch argument.", boolean);
                
    // -----------------------------------------------------------------
    // Extra plain argument (manually defined):
    // Plain arguments are given without a flag. They need to be
    // defined by making a "gsArgValPlain" argument object, taking the
    // cmd object in the constructor
    std::string name = "plain";
    std::string desc =  "Description of the plain command line argument.";
    bool req = false; // whether the argument is required
    std::string value = "default_plain_value"; 
    std::string typeDesc = "string"; // type description    
    gsArgValPlain<std::string> plainArg(name, desc, req, value, typeDesc, cmd);

    // Note: Another manually defined argument is
    //   gsArgMultiVal
    // which reads several values (i.e. a vector) with one flag

    // -----------------------------------------------------------------
    // Reading the arguments: values string, number, flNumber, boolean
    // are updated with the inputs, if given. If "true" is returned, then reading succeeded.
    bool ok = cmd.getValues(argc,argv);

    // -----------------------------------------------------------------
    // The extra (manually defined) arguments are not fetched with the
    //above command. The user must call "getValue" for each manually
    //defined argument.
    plainString = plainArg.getValue();

    if ( !ok ) 
    {
        std::cout << "Something went wrong when reading the command line. Exiting.\n";
        return 0;
    }

    std::cout << "Printing command line arguments:\n\n\n"
              << "Plain string: " << plainString << "\n\n"
              << "String:       " << string << "\n\n"
              << "Float:        " << flNumber << "\n\n"
              << "Integer:      " << number << "\n\n"
              << "Switch:       " << boolean << "\n" << std::endl;

    return 0;
}







