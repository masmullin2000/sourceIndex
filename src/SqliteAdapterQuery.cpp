#include "SqliteAdapterQuery.h"

SqliteAdapterQuery::SqliteAdapterQuery
(
  const string  &filesDbName,
  const string  &identsDbName,
  const string  &locsDbName
) : SqliteAdapter()
{
  openDatabases(filesDbName,identsDbName,locsDbName,true);

  if( (SQLITE_OK != sqlite3_prepare( _identsDb,"SELECT * FROM Identifiers where name GLOB ?",-1,&_idStmt,0)) ||
      (SQLITE_OK != sqlite3_prepare( _locsDb,"SELECT * FROM Locations where fk_id == ?",-1,&_locStmt,0)) ||
      (SQLITE_OK != sqlite3_prepare( _filesDb,"SELECT * FROM Files where pk == ?",-1,&_fileStmt,0)) ) {
    _state = false;
    return;
  }
}

forward_list<tuple<string,uint16_t>>*
SqliteAdapterQuery::find
(
  const string  &name
)
{
  sqlite3_bind_text(_idStmt,1,name.c_str(),-1,SQLITE_STATIC);
  uint32_t fk_id = -1;
  forward_list<tuple<string,uint16_t>> *rc = new forward_list<tuple<string,uint16_t>>();

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
        tuple<string,uint16_t> tup((const char*)fName,line);
        rc->push_front(tup);
      }
      sqlite3_reset(_fileStmt);
    }
    sqlite3_reset(_locStmt);
  }
  
  sqlite3_reset(_idStmt);

  return rc;
}
