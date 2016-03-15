#include "SqliteAdapter.h"
#include "FileProcessor.h"

#include <iostream>
#include <sstream>

using namespace std;

SqliteAdapter::SqliteAdapter
(
  string  filesDbName,
  string  identsDbName,
  string  locsDbName
)
{
  _state = true;

  // --Init the databases--
  sqlite3_enable_shared_cache(1);
  int flags =  SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE|SQLITE_OPEN_NOMUTEX;
  if( sqlite3_open_v2(filesDbName.c_str(),&_filesDb,flags,nullptr) ) {
    _state = false;
    return;
  }
  if( sqlite3_open_v2(identsDbName.c_str(),&_identsDb,flags,nullptr) ) {
    _state = false;
    return;
  }
  if( sqlite3_open_v2(locsDbName.c_str(),&_locsDb,flags,nullptr) ) {
    _state = false;
    return;
  }

  setPragmas(_filesDb);
  setPragmas(_identsDb);
  setPragmas(_locsDb);

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
        "pk       INTEGER PRIMARY KEY,"
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

  if( SQLITE_OK != sqlite3_prepare_v2(_locsDb,"INSERT INTO Locations (pk,fk_id,fk_file,line)"
                                              "VALUES (?,?,?,?);",256,&_locStmt,0) ) {
    _state = false;
    return;
  }
}

SqliteAdapter::~SqliteAdapter()
{
  sqlite3_close_v2(_filesDb);
  sqlite3_close_v2(_identsDb);
  sqlite3_close_v2(_locsDb);

  sqlite3_finalize(_locStmt);
  sqlite3_finalize(_idStmt);
  sqlite3_finalize(_fileStmt);
}

void
SqliteAdapter::setPragmas
(
  sqlite3    *db
)
{
  sqlite3_exec(db, "PRAGMA page_size = 65536", NULL, NULL, NULL);
  sqlite3_exec(db, "PRAGMA default_cache_size=10000", NULL, NULL, NULL);
  sqlite3_exec(db, "PRAGMA synchronous = OFF", NULL, NULL, NULL);
  sqlite3_exec(db, "PRAGMA journal_mode = MEMORY", NULL, NULL, NULL);
}

uint32_t
SqliteAdapter::storeFile
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
SqliteAdapter::storeIdentifier
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
SqliteAdapter::storeLocation
(
  const Location  &l
)
{
  sqlite3_bind_int(_locStmt,2,l.fk_id);
  sqlite3_bind_int(_locStmt,3,l.fk_file);
  sqlite3_bind_int(_locStmt,4,l.line);
  sqlite3_step(_locStmt);
  sqlite3_reset(_locStmt);
}

void
SqliteAdapter::indexLocations()
{
  sqlite3_exec(_locsDb,"CREATE INDEX fk_id_sort ON Locations(fk_id);",0,0,0);
}

void
SqliteAdapter::startBulk
(
  const uint8_t    baseID
)
{
  sqlite3_exec(getDataBase(baseID),"BEGIN TRANSACTION",0,0,0);
}

void
SqliteAdapter::endBulk
(
  const uint8_t    baseID
)
{
  sqlite3_exec(getDataBase(baseID),"END TRANSACTION",0,0,0);
}

sqlite3*
SqliteAdapter::getDataBase
(
  const uint8_t    baseID
)
{
  if( baseID == LBASE ) return _locsDb;
  if( baseID == IBASE ) return _identsDb;
  if( baseID == FBASE ) return _filesDb;

  return nullptr;
}
