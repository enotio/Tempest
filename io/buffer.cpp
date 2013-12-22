#include "buffer.h"

#include <algorithm>
#include <cstring>

using namespace Tempest;

BufferReader::BufferReader(const std::vector<char> &vec):vec(vec), pos(0){
  }

size_t BufferReader::readData(char *dest, size_t count) {
  size_t c = std::min(count, vec.size()-pos);

  memcpy(dest, &vec[pos], c);
  pos+=c;

  return c;
  }

size_t BufferReader::skip(size_t count) {
  size_t c = std::min(count, vec.size()-pos);
  pos+=c;
  return c;
  }

char BufferReader::peek() const {
  if( pos<vec.size() )
    return vec[pos];

  return 0;
  }

bool BufferReader::eof() const {
  return vec.size()==pos;
  }


BufferWriter::BufferWriter(std::vector<char> &vec):vec(vec){
  }

size_t BufferWriter::writeData(const char *src, size_t count) {
  vec.resize( vec.size()+count );
  memcpy( &vec[vec.size()-count-1], src, count );
  return count;
  }
