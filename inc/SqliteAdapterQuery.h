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
  
  ~SqliteAdapterQuery();

  forward_list<tuple<string,uint32_t>>*
  findId
  (
    const string  &name
  );

  forward_list<string>*
  findFile
  (
    const string  &name
  );
private:
  sqlite3_stmt  *_fLook;
};