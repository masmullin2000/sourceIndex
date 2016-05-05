#pragma once

#include <forward_list>
#include <tuple>
#include <cstdint>
#include <algorithm>
#include <functional>

#include "SqliteAdapter.h"

using namespace std;

class identifierCallback {
public:
   
  virtual void operator()
  ( 
    const string    file,
    const uint32_t  line
  ) const
  {
    return ;
  }
};

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
  
  void
  findId
  (
    const string                &name,
    function<void(string&,uint32_t&)>    cb
  );

  forward_list<string>*
  findFile
  (
    const string  &name
  );
private:
  sqlite3_stmt  *_fLook;
};