#pragma once

#include <cstdint>
#include <string>
#include <sqlite3.h>

using namespace std;

class Location;

class SqliteAdapter
{
public:
  static const uint8_t FBASE     = 0;
  static const uint8_t IBASE     = 1;
  static const uint8_t LBASE     = 2;

  SqliteAdapter
  (
    string  filesDbName,
    string  identsDbName,
    string  locsDbName
  );

  ~SqliteAdapter();

  uint32_t
  storeFile
  (
    const string    &file
  );

  void
  storeIdentifier
  (
    const string    &name,
    const uint32_t  &key
  );

  void
  storeLocation
  (
    const Location  &l
  );
  
  void
  indexLocations();

  void
  startBulk
  (
    const uint8_t    baseID
  );

  void
  endBulk
  (
    const uint8_t    baseID
  );

  SqliteAdapter() = delete;
private:
  bool       _state;
  sqlite3   *_filesDb;
  sqlite3   *_identsDb;
  sqlite3   *_locsDb;

  sqlite3*
  getDataBase
  (
    const uint8_t   baseID
  );

  sqlite3_stmt  *_fileStmt;
  sqlite3_stmt  *_locStmt;
  sqlite3_stmt  *_idStmt;

  void setPragmas
  (
    sqlite3   *db
  );
};