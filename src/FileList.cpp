#include "FileList.h"

FileList::FileList()
{
  _fileName = nullptr;
}

FileList::FileList
(
  char* fileName
)
{
  setFile( fileName );
}

void
FileList::setFile
(
  char* fileName
)
{
  _fileName = fileName;
  _fs.close();
  _fs.open( _fileName, fstream::in );
}

string
FileList::getNextFile()
{
  string str;
  getline(_fs, str);

  return str;
}

