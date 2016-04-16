#include <sqlite3.h>
#include <tclap/CmdLine.h>
#include "SqliteAdapterQuery.h"

#include "i_si.h"

using namespace std;
using namespace TCLAP;

int main( int argc, char** argv )
{
  try {
    char* editor = getenv("EDITOR");
    if( editor == nullptr ) {
      editor = (char*)"vi";
    }

    CmdLine cmd("", ' ', "0.1");
    UnlabeledValueArg<string> idName("name","Identifier",true,"int","string");
    SwitchArg fileSearch("f","file_search","Search for a file rather than an index",false);

    cmd.add(idName);
    cmd.add(fileSearch);    

    cmd.parse(argc,argv);

    string findName = idName.getValue();
    bool fLook = fileSearch.getValue();

    SqliteAdapterQuery saq(FILE_DATABASE,IDENT_DATABASE,LOCS_DATABASE);

    if( fLook ) {
      forward_list<string>* foundList = saq.findFile(findName);
      if( foundList != nullptr ) {
        while( !foundList->empty() ) {
          string s = foundList->front();
          foundList->pop_front();
          
          cout << editor << " " << s << endl;
        }
      }
    } else {
      forward_list<tuple<string,uint32_t>>* foundList = saq.findId(findName);
      if( foundList != nullptr ) {
        while( !foundList->empty() ) {
          tuple<string,uint32_t> tup = foundList->front();
          foundList->pop_front();

          cout << editor << " " << get<0>(tup) << " " << get<1>(tup) << endl;
        }
        delete foundList;
      } else {
        cout << findName << ": Could not be found" << endl;
      }
    }
  } catch (ArgException &e) {
    cerr << "error" << endl;
  }

  return 0;
}