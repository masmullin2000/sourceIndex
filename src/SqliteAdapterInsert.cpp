#include "SqliteAdapterInsert.h"
#include "FileProcessor.h"

SqliteAdapterInsert::SqliteAdapterInsert
(
  const string  &filesDbName,
  const string  &identsDbName,
  const string  &locsDbName
) : SqliteAdapter()
{
  openDatabases(filesDbName,identsDbName,locsDbName,false);

  // --Clear the tables--
  const char *sql = "DROP TABLE Files;";
  sqlite3_exec(_filesDb,sql,0,0,0);
  sql = "DROP TABLE Identifiers;";
  sqlite3_exec(_identsDb,sql,0,0,0);
  sql = "DROP TABLE Locations;";
  sqlite3_exec(_locsDb,sql,0,0,0);

  // --Create the tables
  sql = "CREATE TABLE Files("
        "pk     INTEGER PRIMARY KEY,"
        "name   TEXT            NOT NULL);";

  if( SQLITE_OK != sqlite3_exec(_filesDb,sql,0,0,0) ) {
    _state = false;
  }

  sql = "CREATE TABLE Identifiers("
        "pk       INTEGER PRIMARY KEY,"
        "name     TEXT    UNIQUE  NOT NULL);";

  if( SQLITE_OK != sqlite3_exec(_identsDb,sql,0,0,0) ) {
    _state = false;
    return;
  }

  sql = "CREATE TABLE Locations("
        "fk_id    INT             NOT NULL,"
        "fk_file  INT             NOT NULL,"
        "line     INT             NOT NULL);";

  if( SQLITE_OK != sqlite3_exec(_locsDb,sql,0,0,0) ) {
    _state = false;
    return;
  }

  // --Create the prepared statements--
  if( SQLITE_OK != sqlite3_prepare_v2(_filesDb,"INSERT INTO Files (pk,name)"
                                               "VALUES (?,?);",256,&_fileStmt,0) ) {
    _state = false;
    return;
  }

  if( SQLITE_OK != sqlite3_prepare_v2(_identsDb,"INSERT INTO Identifiers (pk,name)"
                                                "VALUES (?,?);",256,&_idStmt,0) ) {
    _state = false;
    return;
  }

  if( SQLITE_OK != sqlite3_prepare_v2(_locsDb,"INSERT INTO Locations (fk_id,fk_file,line)"
                                              "VALUES (?,?,?);",256,&_locStmt,0) ) {
    _state = false;
    return;
  }
}

uint32_t
SqliteAdapterInsert::storeFile
(
  const string &file
)
{
  sqlite3_bind_text(_fileStmt,2,file.c_str(),-1,SQLITE_STATIC);
  sqlite3_step(_fileStmt);
  sqlite3_reset(_fileStmt);

  return sqlite3_last_insert_rowid(_filesDb);
}

void
SqliteAdapterInsert::storeIdentifier
(
  const string    &name,
  const uint32_t  &key
)
{
  sqlite3_bind_int(_idStmt,1,key);
  sqlite3_bind_text(_idStmt,2,name.c_str(),-1,SQLITE_STATIC);
  sqlite3_step(_idStmt);
  sqlite3_reset(_idStmt);
}

void
SqliteAdapterInsert::storeLocation
(
  const Location  &l
)
{
  sqlite3_bind_int(_locStmt,1,l.fk_id);
  sqlite3_bind_int(_locStmt,2,l.fk_file);
  sqlite3_bind_int(_locStmt,3,l.line);
  sqlite3_step(_locStmt);
  sqlite3_reset(_locStmt);
}

void
SqliteAdapterInsert::indexLocations()
{
  sqlite3_exec(_locsDb,"CREATE INDEX fk_id_sort ON Locations(fk_id);",0,0,0);
}

void
SqliteAdapterInsert::startBulk
(
  const uint8_t    baseID
)
{
  sqlite3_exec(getDataBase(baseID),"BEGIN TRANSACTION",0,0,0);
}

void
SqliteAdapterInsert::endBulk
(
  const uint8_t    baseID
)
{
  sqlite3_exec(getDataBase(baseID),"END TRANSACTION",0,0,0);
}
