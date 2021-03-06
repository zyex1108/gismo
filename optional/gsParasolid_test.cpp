/** @file gsParasolid_test.cpp

    @brief Test for the connection to the Siemens's Parasolid geometric kernel

    This file is part of the G+Smo library.

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.

    Author(s): A. Mantzaflaris
*/

#include <iostream>

#include <gismo.h>

#include <gsParasolid/gsReadParasolid.h>
#include <gsParasolid/gsWriteParasolid.h>

using std::cout;
using std::endl;

using namespace gismo;
using extensions::gsWriteParasolid;


int main(int argc, char *argv[])
{
    std::string fn = GISMO_DATA_DIR "surfaces/simple.xml";

    gsCmdLine cmd("Hi, give me a file and I will read the contents to/from Parasolid.");
    cmd.addPlainString("filename", "G+SMO or Parasolid file", fn);
    const bool ok = cmd.getValues(argc,argv);
    if ( !ok )
    {
        gsWarn << "Something went wrong when reading the command line. Exiting.\n";
        return 1;
    }
    
    // Read in a surface
    cout << "Read in "<< gsFileData<>::getFilename(fn) <<"\n";
    gsMultiPatch<> * mp = gsReadFile<>(fn);
    cout << *mp <<"\n";
    
    // Get filename and extension
    std::string name = gsFileData<>::getBasename (fn);
    std::string ext  = gsFileData<>::getExtension(fn);

    if ( ext == "xml" )
    {
        // Write out in parasolid format

        gsWriteParasolid(*mp, name);        
        name += ".xmt_txt";
        cout << "Write out "<< name <<"\n";
    }
    else // if parasolid
    {
        // Write out in G+SMO format
        gsFileData<> fd;
        cout << "Write out "<< name <<".xml \n";
        fd<< *mp;
        fd.dump(name);
    }

    delete mp;
    
    return 0;
}
