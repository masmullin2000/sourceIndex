#pragma once

#include <fstream>

using namespace std;

class FileList
{
public:
  FileList();
  
  FileList
  (
    char* fileName
  );
  
  void setFile
  (
    char* fileName
  );
  
  string
  getNextFile();
private:
  char* _fileName;
  fstream _fs;
};
