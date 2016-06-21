#include "CParser.h"
#include <iostream>
#include <cstring>
#include "utils.h"
#include <sys/mman.h>

using namespace std;

CKeywords* CKeywords::_instance = nullptr;
JavaKeywords* JavaKeywords::_instance = nullptr;

#define CKeywords_solo \
  "auto break case char const continue default do double "\
  "else enum extern float for goto if int long register return short signed " \
  "sizeof static struct switch typedef union unsigned void volatile while "

#define JavaKeywords_solo \
  "abstract assert boolean break byte case catch " \
  "char class const continue default do double else enum extends final " \
  "finally float for goto if implements import instanceof int interface long " \
  "native new package private protected public return short static strictfp " \
  "super switch synchronized this throw throws transient try void volatile while"

/* note the words from bool to wchar_t are ANSI-C++ additions */
#define CppKeywords_solo \
  "asm catch class delete friend inline new operator " \
  "private protected public template this throw try virtual " \
  /* ANSI-C++ additions */ \
  "bool const_cast dynamic_cast explicit export false mutable " \
  "namespace reinterpret_cast static_cast true typeid typename using wchar_t "
  
shared_ptr<forward_list<Token>>
CParser::parse()
{
  shared_ptr<forward_list<Token>> toks = nullptr;
  
  if( _fName.length() < 1 ) {
    return toks;
  }

  char* file = nullptr;
  uint64_t sz = 0;
  getFileContents(&file,&sz);
  
  toks = make_shared<forward_list<Token>>();

  char word[MAX_WORD_SZ];
  word[0] = '\0';
  int j = 0;
  int line = 1;
#define c file[i]
  for( uint64_t i = 0; i < sz; i++ ) {
    if( (IsIdentifierNonDigit(c) || IsDigit(c)) && j < MAX_WORD_SZ-1 ) {
      word[j++] = c;
    } else {
      word[j] = '\0';
      j = 0;
      if( !_kw->isKeyword(word) ) {
        if( word[0] != '\0' && !IsDigit(word[0]) ) {
          Token t;
          t.word = string(word);
          t.line = line;
          toks->emplace_front(t);
        }
      }
      if( c == '\n' ) line++;
    }
  }
#undef c
  munmap(file,sz);

  return toks;
}

void
CKeywords::setKeywords
(
  char* keywords
)
{
  int len = strlen(keywords);
  int i,j;
  i = j = 0;
  char kw[MAX_WORD_SZ];
  kw[0] = '\0';
#define c keywords[i]
  while( i < len ) {
    if( c == ' ' ) {
      kw[j] = '\0';
      _kwList.emplace_front(_hash(string(kw)));
      j = 0;
    } else {
      kw[j++] = c;
    };
    i++;
  }
#undef c
}

CKeywords::CKeywords
(
  char* keywords
)
{
  setKeywords(keywords);
}

CKeywords*
CKeywords::getInstance()
{
  if( !_instance ) {
    _instance = new CKeywords(CKeywords_solo CppKeywords_solo);
  }
  
  return _instance;
}

bool
CKeywords::isKeyword
(
  const string& word
)
{
  size_t id = _hash(word);
  for( size_t kw: _kwList ) {
    if( kw == id ) return true;
  }
  
  return false;
}

JavaKeywords*
JavaKeywords::getInstance()
{
  if( !_instance ) {
    _instance = new JavaKeywords(JavaKeywords_solo);
  }
  
  return _instance;
} 