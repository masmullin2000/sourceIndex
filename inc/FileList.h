#pragma once

#include <fstream>

using namespace std;

class FileList
{
public:
  FileList();

  FileList
  (
    string  &file
  );

  void setFile
  (
    string  &file
  );

  string
  getNextFile();
private:
  string    _fileName;
  fstream   _fs;
};
