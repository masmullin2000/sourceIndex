#pragma once

#include <fstream>

using namespace std;

class FileList
{
public:
  FileList() noexcept;

  FileList
  (
    string  &file
  ) noexcept;

  void setFile
  (
    string  &file
  ) noexcept;

  string
  getNextFile() noexcept;
private:
  string    _fileName;
  fstream   _fs;
};
