#ifndef OVERLAP_FM_INDEX_H_
#define OVERLAP_FM_INDEX_H_

#include <stdint.h>
#include <sys/types.h>
#include <memory>

namespace overlap {


class String;

class FMIndex {
 public:
  FMIndex();
  virtual ~FMIndex();

  virtual size_t size() const = 0;
  // Return count of values less than 'chr' in complete BWT.
  virtual uint32_t Less(uint8_t chr) const = 0;
  // Return count of values equal to 'chr' in first 'pos' values of BWT.
  virtual uint32_t Rank(uint8_t chr, uint32_t pos) const = 0;
};

class BucketedFMIndex : public FMIndex {
 public:
  BucketedFMIndex(const String* bwt, size_t max_val, size_t bucket_size);
  ~BucketedFMIndex();

  uint32_t Less(uint8_t chr) const;
  uint32_t Rank(uint8_t chr, uint32_t pos) const;

 private:
  void Init();


  // BWT string and it's size.
  const std::unique_ptr<const String> bwt_;
  const size_t size_;
  // Maximal value in BWT.
  const size_t max_val_;
  // Cumulative sum of total char counts.
  uint32_t* const char_counts_;
  // Bucket count data.
  const size_t bucket_size_;
  const size_t num_buckets_;
  uint32_t* const buckets_;
};


}  // namespace overlap

#endif  // OVERLAP_FM_INDEX_H_