#include "FileList.h"

FileList::FileList()
{
}

FileList::FileList
(
  string &file
)
{
  setFile( file );
}

void
FileList::setFile
(
  string  &file
)
{
  _fileName = file;
  _fs.close();
  _fs.open( _fileName.c_str(), fstream::in );
}

string
FileList::getNextFile()
{
  string str;
  getline(_fs, str);

  return str;
}

