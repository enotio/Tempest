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
  sz -= c;

  return c;
  }

void MemReader::skip(size_t count) {
  size_t c = std::min(count, sz);
  vec+= c;
  sz -= c;
  }

size_t MemReader::peek(size_t skip, char* dest, size_t count ) const {
  if( skip>=sz )
    return 0;

  size_t c = std::min(count, sz-skip);

  memcpy(dest, vec+skip, c);

  return c;
  }

bool MemReader::eof() const {
  return sz==0;
  }

size_t MemReader::size() const {
  return sz;
  }

MemWriter::MemWriter(char *vec, size_t sz):vec(vec), sz(sz){
  }

MemWriter::MemWriter(unsigned char *vec, size_t sz):vec((char*)vec), sz(sz) {
  }

size_t MemWriter::writeData(char *src, size_t count) {
  size_t c = std::min(count, sz);

  memcpy(vec, src, c);
  vec += c;
  sz  -= c;
  return c;
  }

char IDevice::peek() const{
  char x = 0;
  peek(0, &x, 1);
  return x;
  }

void ODevice::flush() {
  }


PeekReader::PeekReader(IDevice &dev):dev(dev), offset(0){
  }

size_t PeekReader::readData(char *dest, size_t count) {
  size_t c = dev.peek(offset, dest, count);
  offset += c;
  return c;
  }

void PeekReader::skip(size_t c) {
  offset += c;
  }

size_t PeekReader::peek(size_t skip, char *dest, size_t maxSize) const {
  size_t c = dev.peek(offset+skip, dest, maxSize);
  return c;
  }

void PeekReader::commit() {
  dev.skip( offset );
  }

