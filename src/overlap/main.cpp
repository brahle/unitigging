#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <memory>
#include <vector>

#include <gflags/gflags.h>

#include "fm-index.h"
#include "overlap.h"
#include "read.h"
#include "sort.h"
#include "suffix-array.h"
#include "suffix-filter.h"
#include "util.h"
#include "validate.h"

int main(int argc, char* argv[]) {
   google::ParseCommandLineFlags(&argc, &argv, true);
  if (argc < 2) {
    exit(1);
  }

  time_t start = clock(), prev, curr;

  printf("Reading genome data.\n");
  prev = clock();
  FILE* in = fopen(argv[1], "r");
  std::unique_ptr<overlap::ReadSet> reads(overlap::ReadFasta(in));
  fclose(in);
  curr = clock();
  printf("  %.2fs\n", ((double)curr - prev) / CLOCKS_PER_SEC);
  const size_t num_reads = reads->size();

  printf("Building BWT.\n");
  prev = clock();
  overlap::SaisSACA saca;
  std::unique_ptr<overlap::String> bwt(saca.BuildBWT(*reads, 4));
  if (bwt.get() == nullptr) {
    exit(1);
  }
  curr = clock();
  printf("  %.2fs\n", ((double)curr - prev) / CLOCKS_PER_SEC);
  const size_t bwt_size = bwt->size();

  printf("Building FM-index.\n");
  prev = clock();
  overlap::BucketedFmIndex fmi(*bwt, 4, 32);
  fmi.Init();
  curr = clock();
  printf("  %.2fs\n", ((double)curr - prev) / CLOCKS_PER_SEC);

  printf("Sorting reads.\n"); fflush(stdout);
  prev = clock();
  overlap::UintArray read_order = overlap::STLStringOrder(*reads);
  curr = clock();
  printf("  %.2fs\n", ((double)curr - prev) / CLOCKS_PER_SEC);

  printf("Finding candidates.\n");
  prev = clock();
  overlap::BFSSuffixFilter sufter;
  std::unique_ptr<overlap::OverlapSet> candidates(
      sufter.FindCandidates(*reads, read_order, fmi));
  curr = clock();
  printf("  %.2fs\n", ((double)curr - prev) / CLOCKS_PER_SEC);
  size_t num_raw_candidates = candidates->size();

  printf("Filtering candidates.\n");
  prev = clock();
  candidates->Sort();
  candidates.reset(sufter.FilterCandidates(*candidates));
  curr = clock();
  printf("  %.2fs\n", ((double)curr - prev) / CLOCKS_PER_SEC);
  size_t num_filtered_candidates = candidates->size();

  printf("Validating overlap candidates.\n");
  prev = clock();
  candidates.reset(overlap::ValidateCandidates(*reads, *candidates));
  curr = clock();
  printf("  %.2fs\n", ((double)curr - prev) / CLOCKS_PER_SEC);
  size_t num_overlaps = candidates->size();

  printf("\nSummary\n");
  printf(" + number of reads: %d\n", num_reads);
  printf(" + size of BWT: %d\n", bwt_size);
  printf(" + number of raw candidates: %d\n", num_raw_candidates);
  printf(" + number of filtered candidates: %d\n", num_filtered_candidates);
  printf(" + number of overlaps: %d\n", num_overlaps);
  printf(" + wall time: %.2fs\n", ((double)curr - start) / CLOCKS_PER_SEC);

  FILE* fout = fopen(argv[2], "w");
  for (uint32_t oid = 0; oid < candidates->size(); ++oid) {
    const overlap::Overlap* o = candidates->Get(oid);
    fprintf(fout, "%d %d %d %d EB %d\n",
        reads->Get(o->read_one)->orig_id(),
        reads->Get(o->read_two)->orig_id(),
        o->len_one,
        o->len_two,
        o->score);
  }
  fclose(fout);

  return 0;
}
