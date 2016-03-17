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

  SqliteAdapter();

  virtual ~SqliteAdapter();
protected:
  bool       _state;
  sqlite3   *_filesDb;
  sqlite3   *_identsDb;
  sqlite3   *_locsDb;

  sqlite3_stmt  *_fileStmt;
  sqlite3_stmt  *_locStmt;
  sqlite3_stmt  *_idStmt;

  sqlite3*
  getDataBase
  (
    const uint8_t   baseID
  );

  void
  openDatabases
  (
    const string   &filesDbName,
    const string   &identsDbName,
    const string   &locsDbName,
    const bool      readOnly
  );
private:
  void
  openDatabase
  (
    int               flags,
    const string     &dbname,
    sqlite3         **db
  );

  void
  setPragmas
  (
    sqlite3   *db
  );
};