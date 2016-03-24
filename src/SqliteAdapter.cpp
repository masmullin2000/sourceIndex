#include "SqliteAdapter.h"

SqliteAdapter::SqliteAdapter() noexcept
{
  _state = true;

  // --Init the databases--
  sqlite3_enable_shared_cache(1);
}

SqliteAdapter::~SqliteAdapter() noexcept
{
  sqlite3_finalize(_locStmt);
  sqlite3_finalize(_idStmt);
  sqlite3_finalize(_fileStmt);

  sqlite3_close_v2(_filesDb);
  sqlite3_close_v2(_identsDb);
  sqlite3_close_v2(_locsDb);
}

void
SqliteAdapter::openDatabases
(
  const string     &filesDbName,
  const string     &identsDbName,
  const string     &locsDbName,
  const bool        readOnly
) noexcept
{
  int flags = SQLITE_OPEN_NOMUTEX;

  if( readOnly )  flags |= SQLITE_OPEN_READONLY;
  else            flags |= SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE;

  openDatabase(flags,filesDbName,&_filesDb);
  openDatabase(flags,identsDbName,&_identsDb);
  openDatabase(flags,locsDbName,&_locsDb);
}

void
SqliteAdapter::openDatabase
(
  int             flags,
  const string   &dbName,
  sqlite3       **db
) noexcept
{
  if( sqlite3_open_v2(dbName.c_str(),db,flags,nullptr) ) {
    _state = false;
  }
  setPragmas(*db);
}

void
SqliteAdapter::setPragmas
(
  sqlite3    *db
) noexcept
{
  sqlite3_exec(db, "PRAGMA page_size = 65536", NULL, NULL, NULL);
  sqlite3_exec(db, "PRAGMA default_cache_size=10000", NULL, NULL, NULL);
  sqlite3_exec(db, "PRAGMA synchronous = OFF", NULL, NULL, NULL);
  sqlite3_exec(db, "PRAGMA journal_mode = MEMORY", NULL, NULL, NULL);
}

sqlite3*
SqliteAdapter::getDataBase
(
  const uint8_t    baseID
) noexcept
{
  if( baseID == LBASE ) return _locsDb;
  if( baseID == IBASE ) return _identsDb;
  if( baseID == FBASE ) return _filesDb;

  return nullptr;
}
