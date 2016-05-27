#pragma once

#include "SqliteAdapterInsert.h"

#include <string>
#include <mutex>
#include <unordered_map>
#include <forward_list>

#include <cstdint>

#include <sqlite3.h>

#include <iostream>

using namespace std;

class Location
{
public:
  uint32_t      fk_id;
  uint32_t      fk_file;
  uint32_t      line;
  
  Location
  (
    uint32_t    id,
    uint32_t    file,
    uint32_t    l
  )
  {
    fk_id       = id;
    fk_file     = file;
    line        = l;
  }
};

class Token
{
public:
  string    word;
  uint32_t  line;
};

enum class FileProcessorErrors
{
  SUCCESS = 0,
  FILE_NOT_FOUND,
  FILE_NO_STATUS,
  FILE_MMAP_ERROR,
  SQL_CREATE,
  SQL_SELECT,
  SQL_INSERT,
  SQL_PREPARE
};

static mutex fileMutex;
static mutex inMutex;
class FileProcessor
{
public:
  FileProcessor() noexcept;
  ~FileProcessor() noexcept;

  FileProcessorErrors
  run
  (
    string                           &fList,
    uint8_t                           threads,
    bool                              idx = true
  ) noexcept;
private:
  FileProcessorErrors
  processFile
  (
    string                            fName,
    unordered_map<string,uint32_t>   &ids,
    forward_list<Location>           &locs,
    uint32_t                         &id_key
  ) noexcept;

  FileProcessorErrors
  storeIdentifiers
  (
    unordered_map<string,uint32_t>  &ids
  ) noexcept;

  FileProcessorErrors
  storeLocations
  (
    forward_list<Location>           &locs
  ) noexcept;

  SqliteAdapterInsert               *sql;
};
