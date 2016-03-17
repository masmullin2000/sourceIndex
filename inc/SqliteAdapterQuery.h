#pragma once

#include <forward_list>
#include <tuple>
#include <cstdint>

#include "SqliteAdapter.h"

using namespace std;

class SqliteAdapterQuery : public SqliteAdapter
{
public:
  SqliteAdapterQuery
  (
    const string  &filesDbName,
    const string  &identsDbName,
    const string  &locsDbName
  );

  forward_list<tuple<string,uint16_t>>*
  find
  (
    const string  &name
  );
};