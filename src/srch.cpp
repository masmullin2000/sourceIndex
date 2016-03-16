#include <sqlite3.h>
#include <tclap/CmdLine.h>

#include "i_si.h"

using namespace std;
using namespace TCLAP;

int main( int argc, char** argv )
{
  try {
    CmdLine cmd("", ' ', "0.1");
    UnlabeledValueArg<string> fName("name","unlab",true,"int","string");

    cmd.add(fName);

    cmd.parse(argc,argv);

    string findName = fName.getValue();

    sqlite3   *_filesDb;
    sqlite3   *_identsDb;
    sqlite3   *_locsDb;

    sqlite3_enable_shared_cache(1);
    int flags =  SQLITE_OPEN_READONLY|SQLITE_OPEN_NOMUTEX;
    if( sqlite3_open_v2(FILE_DATABASE,&_filesDb,flags,nullptr) ) {
      return 1;
    }
    if( sqlite3_open_v2(IDENT_DATABASE,&_identsDb,flags,nullptr) ) {
      return 1;
    }
    if( sqlite3_open_v2(LOCS_DATABASE,&_locsDb,flags,nullptr) ) {
      return 1;
    }

    sqlite3_stmt *_idStmt;
    sqlite3_stmt *_lStmt;
    sqlite3_stmt *_fStmt;

    sqlite3_prepare( _identsDb,"SELECT * FROM Identifiers where name == ?",-1,&_idStmt,0 );
    sqlite3_prepare( _locsDb,"SELECT * FROM Locations where fk_id == ?",-1,&_lStmt,0 );
    sqlite3_prepare( _filesDb,"SELECT * FROM Files where pk == ?",-1,&_fStmt,0 );

    sqlite3_bind_text(_idStmt,1,findName.c_str(),-1,SQLITE_STATIC);
    uint32_t fk_id = -1;
    if( SQLITE_ROW == sqlite3_step(_idStmt) ) {
      fk_id = sqlite3_column_int(_idStmt,0);
    } else {
      return 1;
    }
    sqlite3_reset(_idStmt);

    sqlite3_bind_int(_lStmt,1,fk_id);
    while( SQLITE_ROW == sqlite3_step(_lStmt) ) {
      int pk = sqlite3_column_int(_lStmt,2);
      int line = sqlite3_column_int(_lStmt,3);

      sqlite3_bind_int(_fStmt,1,pk);
      const unsigned char* fName;
      if( SQLITE_ROW == sqlite3_step(_fStmt) ) {
        fName = sqlite3_column_text(_fStmt,1);
      }

      cout << fName << " " << line << endl;
      sqlite3_reset(_fStmt);
    }

  } catch (ArgException &e) {
    cerr << "error" << endl;
  }

  return 0;
}