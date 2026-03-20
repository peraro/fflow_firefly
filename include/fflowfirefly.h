
#pragma once

#include <fflow/capi.h>

#ifdef __cplusplus
extern "C" {
#endif

  /* API begin */

  typedef enum {
    FF_FIREFLY_SILENT,
    FF_FIREFLY_IMPORTANT,
    FF_FIREFLY_CHATTY
  } FFFireflyVerbosity;

  typedef struct {
    unsigned n_threads; // automatically chosen by default
    unsigned bunch_size; // 1 by default
    FFFireflyVerbosity verbosity; // SILENT by default
  } FFFireflyRecOptions;

  typedef struct FFFireflyRatFunList FFFireflyRatFunList;

  FFFireflyRatFunList *
  ffFireflyReconstructFunction(FFGraph graph, FFFireflyRecOptions opt);

  size_t ffFireflyRatFunListLen(const FFFireflyRatFunList * list);
  unsigned ffFireflyRatFunListNVars(const FFFireflyRatFunList * list);
  void ffFireflyRatFunListFree(FFFireflyRatFunList * list);

  // The returned string must be freed using ffFreeCStr
  char * ffFireflyRatFunToStr(const FFFireflyRatFunList * rf, unsigned idx,
                              const FFCStr * vars);

  /* API end */

#ifdef __cplusplus
} // extern "C"
#endif
