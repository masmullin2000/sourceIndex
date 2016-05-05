#include "SqliteAdapterQuery.h"

#include <sstream>
#include <iostream>

using namespace std;

SqliteAdapterQuery::SqliteAdapterQuery
(
  const string  &filesDbName,
  const string  &identsDbName,
  const string  &locsDbName
) : SqliteAdapter()
{
  openDatabases(filesDbName,identsDbName,locsDbName,true);
  
  _idStmt = _locStmt = _fileStmt = _fLook = nullptr;

  if( (SQLITE_OK != sqlite3_prepare( _identsDb,"SELECT * FROM Identifiers where name GLOB ?",-1,&_idStmt,0)) ||
      (SQLITE_OK != sqlite3_prepare( _locsDb,"SELECT * FROM Locations where fk_id == ?",-1,&_locStmt,0)) ||
      (SQLITE_OK != sqlite3_prepare( _filesDb,"SELECT * FROM Files where pk == ?",-1,&_fileStmt,0)) ) {
    _state = false;
    return;
  }
  
  if( SQLITE_OK != sqlite3_prepare( _filesDb,"SELECT name FROM Files where name GLOB ?",-1,&_fLook,0) ) {
    _state = false;
  }
}

SqliteAdapterQuery::~SqliteAdapterQuery()
{
  sqlite3_finalize(_fLook);
}

void
SqliteAdapterQuery::findId
(
  const string              &name,
  function<void(string&,uint32_t&)>  cb
)
{
  if( !_state ) return;

  sqlite3_bind_text(_idStmt,1,name.c_str(),-1,SQLITE_STATIC);
  uint32_t fk_id = -1;

  while( SQLITE_ROW == sqlite3_step(_idStmt) ) {
    fk_id = sqlite3_column_int(_idStmt,0);
    string idName = (char*)sqlite3_column_text(_idStmt,1);

    sqlite3_bind_int(_locStmt,1,fk_id);
    while( SQLITE_ROW == sqlite3_step(_locStmt) ) {
      cout << idName;
      int pk = sqlite3_column_int(_locStmt,1);
      uint32_t line = sqlite3_column_int(_locStmt,2);

      sqlite3_bind_int(_fileStmt,1,pk);
      
      if( SQLITE_ROW == sqlite3_step(_fileStmt) ) {
        string fName = (char*)sqlite3_column_text(_fileStmt,1);
        
        cb(fName,line);
      }
      sqlite3_reset(_fileStmt);
    }
    sqlite3_reset(_locStmt);
  }
  sqlite3_reset(_idStmt);
}

forward_list<tuple<string,uint32_t>>*
SqliteAdapterQuery::findId
(
  const string  &name
)
{
  if( !_state ) return nullptr;

  sqlite3_bind_text(_idStmt,1,name.c_str(),-1,SQLITE_STATIC);
  uint32_t fk_id = -1;
  forward_list<tuple<string,uint32_t>> *rc = new forward_list<tuple<string,uint32_t>>();

  while( SQLITE_ROW == sqlite3_step(_idStmt) ) {
    fk_id = sqlite3_column_int(_idStmt,0);

    sqlite3_bind_int(_locStmt,1,fk_id);
    while( SQLITE_ROW == sqlite3_step(_locStmt) ) {
      int pk = sqlite3_column_int(_locStmt,1);
      int line = sqlite3_column_int(_locStmt,2);

      sqlite3_bind_int(_fileStmt,1,pk);
      char* fName;
      if( SQLITE_ROW == sqlite3_step(_fileStmt) ) {
        fName = (char*)sqlite3_column_text(_fileStmt,1);
        tuple<string,uint32_t> tup((const char*)fName,line);
        rc->push_front(tup);
      }
      sqlite3_reset(_fileStmt);
    }
    sqlite3_reset(_locStmt);
  }

  sqlite3_reset(_idStmt);

  return rc;
}

forward_list<string>*
SqliteAdapterQuery::findFile
(
  const string    &name
)
{
  if( !_state ) return nullptr;
  stringstream ss;
  
  ss << "*" << name;
  
  const char *q = ss.str().c_str();

  sqlite3_bind_text(_fLook,1,q,-1,SQLITE_STATIC);
  
  forward_list<string>* rc = new forward_list<string>();
  
  char* fName;
  while( SQLITE_ROW == sqlite3_step(_fLook) ) {
    fName = (char*)sqlite3_column_text(_fLook,0);
    rc->push_front(fName);
  }
  sqlite3_reset(_fLook);
  
  return rc;
}
