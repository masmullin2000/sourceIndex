#include <sqlite3.h>
#include <tclap/CmdLine.h>
#include "SqliteAdapterQuery.h"

#include "i_si.h"

using namespace std;
using namespace TCLAP;

int main( int argc, char** argv )
{
  try {
    CmdLine cmd("", ' ', "0.1");
    UnlabeledValueArg<string> fName("name","FileName",true,"int","string");

    cmd.add(fName);

    cmd.parse(argc,argv);

    string findName = fName.getValue();

    SqliteAdapterQuery saq(FILE_DATABASE,IDENT_DATABASE,LOCS_DATABASE);

    forward_list<tuple<string,uint16_t>>* foundList = saq.findExact(findName);
    if( foundList != nullptr ) {
      while( !foundList->empty() ) {
        tuple<string,uint16_t> tup = foundList->front();
        foundList->pop_front();

        cout << "ca " << get<0>(tup) << " " << get<1>(tup) << endl;
      }
      delete foundList;
    } else {
      cout << findName << ": Could not be found" << endl;
    }

  } catch (ArgException &e) {
    cerr << "error" << endl;
  }

  return 0;
}