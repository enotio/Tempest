#include "iodevice.h"

#include <algorithm>
#include <cstring>

using namespace Tempest;

MemReader::MemReader(const char *vec, size_t sz):vec(vec), sz(sz){
  }

MemReader::MemReader(const unsigned char *vec, size_t sz):vec((const char*)vec), sz(sz) {
  }

size_t MemReader::readData(char *dest, size_t count) {
  size_t c = std::min(count, sz);

  memcpy(dest, vec, c);
  vec+=c;

  return c;
  }

size_t MemReader::skip(size_t count) {
  size_t c = std::min(count, sz);
  vec+=c;
  return c;
  }

char MemReader::peek() const {
  if( sz )
    return *vec; else
    return 0;
  }

bool MemReader::eof() const {
  return sz==0;
  }

MemWriter::MemWriter(char *vec, size_t sz):vec(vec), sz(sz){
  }

MemWriter::MemWriter(unsigned char *vec, size_t sz):vec((char*)vec), sz(sz) {
  }

size_t MemWriter::writeData(char *src, size_t count) {
  size_t c = std::min(count, sz);

  memcpy(vec, src, c);
  vec += c;
  return c;
  }

void ODevice::flush() {
  }
