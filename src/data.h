#ifndef OVERLAP_DATA_H_
#define OVERLAP_DATA_H_

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <vector>

#include "util.h"


namespace overlap {


const uint8_t* ReverseComplement(const uint8_t* data, size_t size);

class String {
public:
  String(const uint8_t* data, size_t size);
  virtual ~String();

  const uint8_t* data() const;
  size_t size() const;

  const uint8_t& operator[](const uint32_t idx) const;
  bool operator< (const String& other) const;

private:
  const uint8_t* data_;
  const size_t size_;
};

class Read : public String {
public:
  Read(const uint8_t* data, size_t size);
  ~Read();

  const uint8_t* data_rc() const;

private:
  const uint8_t* data_rc_;
};

class ReadSet {
public:
  ReadSet(size_t capacity);
  ~ReadSet();

  void Add(Read* read);
  const Read* Get(uint32_t read_idx) const;

  size_t size() const;

  Read* const& operator[](const uint32_t idx) const;
  Read*& operator[](const uint32_t idx);

private:
  std::vector<Read*> reads_;
};

ReadSet* ReadFasta(FILE* fd);

}  // namespace overlap

#endif  // OVERLAP_DATA_H_
