#pragma once

#include "FileParser.h"
#include <forward_list>

class CKeywords
{
public:
  static CKeywords*
  getInstance();
  
  bool isKeyword
  (
    const string& word
  );
protected:
  void setKeywords( char* keywords );
  CKeywords( char* keywords );
private:
  static CKeywords*           _instance;
  std::forward_list<size_t>   _kwList;
  std::hash<std::string>      _hash;
};

class JavaKeywords : public CKeywords
{
public:
  static JavaKeywords*
  getInstance();

private:
  JavaKeywords( char* keywords ) : CKeywords(keywords){}
  static JavaKeywords*        _instance;
};

class CParser: public FileParser
{
public:
  CParser( const string& fileName ) : FileParser(fileName)
  {
    _kw = CKeywords::getInstance();
  };
  virtual     ~CParser(){}
  virtual      shared_ptr<forward_list<Token>> parse();
protected:
  CKeywords*  _kw;
};

class JavaParser: public CParser
{
public:
  JavaParser( const string& fileName ) : CParser(fileName)
  {
    _kw = JavaKeywords::getInstance();
  }
};