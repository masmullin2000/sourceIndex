#pragma once

#include <string>
#include <mutex>
#include <unordered_map>
#include <forward_list>

#include <cstdint>

#include <sqlite3.h>

using namespace std;

class Location
{
public:
  uint32_t fk_id;
  uint16_t fk_file;
  uint16_t line;
};

class Token
{
public:
  string    word;
  uint16_t  line;
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
  static FileProcessorErrors
  run
  (
    sqlite3                          *db,
    string                            fName,
    unordered_map<string,uint32_t>   &ids,
    forward_list<Location>           &locs,
    uint32_t                         &id_key
  );

  static
  FileProcessorErrors
  storeIdentifiers
  (
    sqlite3                         *database,
    unordered_map<string,uint32_t>  &ids
  );

  static
  FileProcessorErrors
  storeLocations
  (
    sqlite3                         *database,
    forward_list<Location>           &locs
  );
protected:

private:
};
