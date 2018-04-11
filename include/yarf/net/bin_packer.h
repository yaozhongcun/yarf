/*
 * bin_packer.h
 *
 *  Created on: 2016-05-19
 *      Author: yaozhongcun
 */

#ifndef YARF_NET_BIN_PACKER_H_
#define YARF_NET_BIN_PACKER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sstream>
#include <iomanip>
#include <string>
#include <iostream>
#include <exception>
#include <stddef.h>
#include <arpa/inet.h>

namespace yarf {
namespace net {

class BinEncoder {
 public:
  BinEncoder()
  : write_pos_(0), buffer_(nullptr), len_(0) {
  }
  BinEncoder(char * buffer, size_t len)
  : write_pos_(0), buffer_(buffer), len_(len)  {
  }

  ~BinEncoder() {}

  inline void Reset(char * buffer, size_t len) {
    write_pos_ = 0; buffer_ = buffer; len_ = len;
  }

  inline size_t Len() const { return len_; }

  inline const char * InnerBuffer() const { return buffer_; }

  inline size_t WritePos() const { return write_pos_; }
  inline void SetWritePos(size_t pos) { write_pos_ = pos; }
  inline bool SkipWritePos(size_t len) {
    if (write_pos_ + len > len_) return false;
    write_pos_ += len;
    return true;
  }

  bool Write(int8_t value) {
    if (write_pos_ + sizeof(value) > len_) return false;
    memcpy(buffer_ + write_pos_, &value, sizeof(value));
    write_pos_ += sizeof(value);
    return true;
  }

  bool Write(uint8_t value) {
    if (write_pos_ + sizeof(value) > len_) return false;
    memcpy(buffer_ + write_pos_, &value, sizeof(value));
    write_pos_ += sizeof(value);
    return true;
  }

  bool Write(int16_t value, char hton = 1) {
    if (write_pos_ + sizeof(value) > len_) return false;
    if (hton) value = htons(value);
    memcpy(buffer_ + write_pos_, &value, sizeof(value));
    write_pos_ += sizeof(value);
    return true;
  }

  bool Write(uint16_t value, char hton = 1) {
    if (write_pos_ + sizeof(value) > len_) return false;
    if (hton) value = htons(value);
    memcpy(buffer_ + write_pos_, &value, sizeof(value));
    write_pos_ += sizeof(value);
    return true;
  }

  bool Write(int32_t value, char hton = 1) {
    if (write_pos_ + sizeof(value) > len_) return false;
    if (hton) value = htonl(value);
    memcpy(buffer_ + write_pos_, &value, sizeof(value));
    write_pos_ += sizeof(value);
    return true;
  }

  bool Write(uint32_t value, char hton = 1) {
    if (write_pos_ + sizeof(value) > len_) return false;
    if (hton) value = htonl(value);
    memcpy(buffer_ + write_pos_, &value, sizeof(value));
    write_pos_ += sizeof(value);
    return true;
  }

  inline uint64_t htonll(uint64_t host) {
    uint64_t net = (uint64_t)ntohl((int32_t)host) << 32;
    net += ntohl((int32_t)(host >> 32));
    return net;
  }

  bool Write(int64_t value, char hton = 1) {
    if (write_pos_ + sizeof(value) > len_) return false;
    if (hton) value = htonll(value);
    memcpy(buffer_ + write_pos_, &value, sizeof(value));
    write_pos_ += sizeof(value);
    return true;
  }

  bool Write(uint64_t value, char hton = 1) {
    if (write_pos_ + sizeof(value) > len_) return false;
    if (hton) value = htonll(value);
    memcpy(buffer_ + write_pos_, &value, sizeof(value));
    write_pos_ += sizeof(value);
    return true;
  }

  bool Write(const void * value, size_t len) {
    if (write_pos_ + len > len_) return false;
    memcpy(buffer_ + write_pos_, value, len);
    write_pos_ += len;
    return true;
  }

 private:
  size_t write_pos_;
  char * buffer_;
  size_t len_;
};  // class BinEncoder


class BinDecoder {
 public:
  BinDecoder()
  : read_pos_(0), buffer_(NULL), len_(0) {
  }
  BinDecoder(const char * buffer, size_t len)
  : read_pos_(0), buffer_(buffer), len_(len) {
  }

  ~BinDecoder() {}

  inline void Reset(const char * buffer, size_t len) {
    read_pos_ = 0; buffer_ = buffer; len_ = len;
  }

  inline size_t Len() const { return len_; }

  inline const char * InnerBuffer() const { return buffer_; }

  inline size_t ReadPos() const { return read_pos_; }
  inline void SetReadPos(size_t pos) { read_pos_ = pos; }
  inline bool SkipReadPos(size_t len) {
    if (read_pos_ + len > len_) return false;
    read_pos_ += len;
    return true;
  }
    //inline void ReadPos(size_t pos) { read_pos_ = pos; }

  bool Read(int8_t & value) {
    if (read_pos_ + sizeof(value) > len_) return false;
    memcpy(&value, buffer_ + read_pos_, sizeof(value));
    read_pos_ += sizeof(value);
    return true;
  }

  bool Read(uint8_t & value) {
    if (read_pos_ + sizeof(value) > len_) return false;
    memcpy(&value, buffer_ + read_pos_, sizeof(value));
    read_pos_ += sizeof(value);
    return true;
  }

  bool Read(int16_t & value, char ntoh = 1) {
    if (read_pos_ + sizeof(value) > len_) return false;
    memcpy(&value, buffer_ + read_pos_, sizeof(value));
    if (ntoh) value = ntohs(value);
    read_pos_ += sizeof(value);
    return true;
  }

  bool Read(uint16_t & value, char ntoh = 1) {
    if (read_pos_ + sizeof(value) > len_) return false;
    memcpy(&value, buffer_ + read_pos_, sizeof(value));
    if (ntoh) value = ntohs(value);
    read_pos_ += sizeof(value);
    return true;
  }

  bool Read(int32_t & value, char ntoh = 1) {
    if (read_pos_ + sizeof(value) > len_) return false;
    memcpy(&value, buffer_ + read_pos_, sizeof(value));
    if (ntoh) value = ntohl(value);
    read_pos_ += sizeof(value);
    return true;
  }

  bool Read(uint32_t & value, char ntoh = 1) {
    if (read_pos_ + sizeof(value) > len_) return false;
    memcpy(&value, buffer_ + read_pos_, sizeof(value));
    if (ntoh) value = ntohl(value);
    read_pos_ += sizeof(value);
    return true;
  }

  inline uint64_t ntohll(uint64_t net) {
    uint64_t host = (uint64_t)ntohl((int32_t)net) << 32;
    host += ntohl((int32_t)(net >> 32));
    return host;
  }

  bool Read(int64_t & value, char ntoh = 1) {
    if (read_pos_ + sizeof(value) > len_) return false;
    memcpy(&value, buffer_ + read_pos_, sizeof(value));
    if (ntoh) value = ntohll(value);
    read_pos_ += sizeof(value);
    return true;
  }

  bool Read(uint64_t & value, char ntoh = 1) {
    if (read_pos_ + sizeof(value) > len_) return false;
    memcpy(&value, buffer_ + read_pos_, sizeof(value));
    if (ntoh) value = ntohll(value);
    read_pos_ += sizeof(value);
    return true;
  }

  bool Read(void * value, size_t len) {
    if (read_pos_ + len > len_) return false;
    memcpy(value, buffer_ + read_pos_, len);
    read_pos_ += len;
    return true;
  }

  bool Read(const void ** value, size_t len) {
    if (read_pos_ + len > len_) return false;
    *value = buffer_ + read_pos_;
    read_pos_ += len;
    return true;
  }

 private:
  size_t read_pos_;
  const char * buffer_;
  size_t len_;
};  // class BinDecoder 


}  // namespace net
}  // namespace yarf

#endif  // YARF_NET_BIN_PACKER_H_

