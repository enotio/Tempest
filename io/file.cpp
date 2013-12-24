#include "file.h"

#include <cstdint>

using namespace Tempest;

static void makeMode( char* out, uint16_t m ){
  static const std::pair<char, uint16_t> mdef[] = {
    {'r', 1},
    {'w', 2},
    {'+', 4},
    {'b', 8}
    };

  for( int i=0; i<4; ++i )
    if( m & mdef[i].second ){
      *out = mdef[i].first;
      ++out;
      }

  *out = 0;
  }

RFile::RFile(const char *name, RFile::Mode m) {
  char md[4];
  makeMode(md, m|1);

  impl = SystemAPI::fopen(name, md);
  }

RFile::RFile( RFile && mov) {
  impl     = mov.impl;
  mov.impl = 0;
  }

RFile &RFile::operator =( RFile && mov) {
  std::swap( impl, mov.impl );
  return *this;
  }

RFile::RFile(const char16_t *name, RFile::Mode m) {
  char md[4];
  makeMode(md, m|1);

  impl = SystemAPI::fopen(name, md);
  }

RFile::~RFile() {
  SystemAPI::fclose(impl);
  }

size_t RFile::readData(char *dest, size_t count) {
  return SystemAPI::readData(impl, dest, count);
  }

size_t RFile::skip(size_t count) {
  return SystemAPI::skip(impl, count);
  }

bool RFile::eof() const{
  return SystemAPI::eof(impl);
  }


WFile::WFile(const char *name, WFile::Mode m) {
  char md[4];
  makeMode(md, m|2);

  impl = SystemAPI::fopen(name, md);
  }

WFile::WFile(const char16_t *name, WFile::Mode m) {
  char md[4];
  makeMode(md, m|2);

  impl = SystemAPI::fopen(name, md);
  }

WFile::WFile( WFile && mov) {
  impl     = mov.impl;
  mov.impl = 0;
  }

WFile &WFile::operator =( WFile && mov ) {
  std::swap( impl, mov.impl );
  return *this;
  }

WFile::~WFile() {
  SystemAPI::fclose(impl);
  }

size_t WFile::writeData(const char *src, size_t count) {
  return SystemAPI::writeData(impl, src, count);
  }

void WFile::flush() {
  SystemAPI::flush(impl);
  }
