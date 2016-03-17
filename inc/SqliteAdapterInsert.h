#pragma once

#include "SqliteAdapter.h"

class SqliteAdapterInsert : public SqliteAdapter
{
public:
  SqliteAdapterInsert
  (
    const string  filesDbName,
    const string  identsDbName,
    const string  locsDbName
  );

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
  
  SqliteAdapterInsert() = delete;
private:

};
