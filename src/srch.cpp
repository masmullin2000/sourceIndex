#include <sqlite3.h>
#include <tclap/CmdLine.h>
#include "SqliteAdapterQuery.h"

#include "i_si.h"

using namespace std;
using namespace TCLAP;

class cb : public identifierCallback {
public:
  virtual void operator()(string file, uint32_t line) const {
    cout << file << line << endl;
  }
};

int main( int argc, char** argv )
{
  try {
    char* editor = getenv("C_EDITOR");
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
      auto cBack = [editor](string file, uint32_t line) {
        cout << editor << " " << file << " " << line << endl;
      };
      saq.findId(findName, cBack);
    }
  } catch (ArgException &e) {
    cerr << "error" << endl;
  }

  return 0;
}