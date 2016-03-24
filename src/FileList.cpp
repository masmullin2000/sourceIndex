#include "FileList.h"

FileList::FileList() noexcept
{
}

FileList::FileList
(
  string &file
) noexcept
{
  setFile( file );
}

void
FileList::setFile
(
  string  &file
) noexcept
{
  _fileName = file;
  _fs.close();
  _fs.open( _fileName.c_str(), fstream::in );
}

string
FileList::getNextFile() noexcept
{
  string str;
  getline(_fs, str);

  return str;
}

