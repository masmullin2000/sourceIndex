#pragma once

#include "SqliteAdapter.h"

class SqliteAdapterInsert : public SqliteAdapter
{
public:
  SqliteAdapterInsert
  (
    const string  &filesDbName,
    const string  &identsDbName,
    const string  &locsDbName
  ) noexcept;

  uint32_t
  storeFile
  (
    const string    &file
  ) noexcept;

  void
  storeIdentifier
  (
    const string    &name,
    const uint32_t  &key
  ) noexcept;

  void
  storeLocation
  (
    const Location  &l
  ) noexcept;

  void
  indexLocations() noexcept;

  void
  startBulk
  (
    const uint8_t    baseID
  ) noexcept;

  void
  endBulk
  (
    const uint8_t    baseID
  ) noexcept;

  SqliteAdapterInsert() = delete;
private:

};
