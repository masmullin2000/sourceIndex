#pragma once

#include <string>
#include <mutex>
#include <sstream>

#include <cstdint>

using namespace std;

static mutex primaryKeyMutex;
static mutex setInsertMutex;


class Identifier
{
public:
  string    word;
  uint32_t  file_key;
  uint16_t  line_num;

  Identifier
  (
    string  w,
    int     file,
    int     line
  )
  {
    word = w;
    file_key = file;
    line_num = line;
  }

  const string
  str() const
  {
    stringstream ss;
    ss << word << ":" << file_key << ":" << line_num;

    return ss.str();
  }
};

enum class FileProcessorErrors
{
  SUCCESS = 0,
  FILE_NOT_FOUND,
  FILE_NO_STATUS,
  FILE_MMAP_ERROR,
  SQL_SELECT,
  SQL_INSERT
};

class FileProcessor
{
public:
  FileProcessor();
  FileProcessor
  (
    string fileName
  );

  void setFile
  (
    string fileName
  );

  FileProcessorErrors
  run();

protected:

private:
  string _fileName;
};
