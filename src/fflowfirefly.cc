#include <atomic>
#include <thread>
#include <type_traits>
#include <string>
#include <firefly/Reconstructor.hpp>
#include <string.h>
#include <fflowfirefly.h>
#include <fflow/capi.h>
#include <fflow/capi_native_ext.h>
#include <fflow/capi_custom_threads.h>


namespace {

  // These are part of a hacky workaround to "guess" from which thread
  // the black box is being called, since this information is not
  // provided by Firefly's Reconstructor.  Apparently, there's a
  // Firefly branch where this info is provided, but it seems to be
  // obsolete.
  const unsigned INVALID_THREAD_ID = ~((unsigned)0);
  std::atomic_uint thread_id_count(0);
  thread_local unsigned current_thread_id = INVALID_THREAD_ID;

  void resetThreads()
  {
    thread_id_count = 0;
  }

  class FFBlackBox : public firefly::BlackBoxBase<FFBlackBox> {
  public:

    typedef firefly::FFInt FFInt;

    FFBlackBox() = default;

    FFBlackBox(FFBlackBox &) = delete;
    FFBlackBox(FFBlackBox &&) = delete;

    ~FFBlackBox();

    bool setup(FFGraph graph, unsigned n_threads);

    std::vector<FFInt> operator()(const std::vector<FFInt> & values);

    template <int N>
    std::vector<firefly::FFIntVec<N>>
    operator()(const std::vector<firefly::FFIntVec<N>> & values);

    void prime_changed()
    {
      // ¯\_(ツ)_/¯
    }

    std::vector<FFInt> failed();

    template<int N>
    std::vector<firefly::FFIntVec<N>> failed_n();

    FFMTGraphEvaluator * eval = nullptr;
    std::unique_ptr<std::unique_ptr<FFUInt[]>[]> inputs;
    std::unique_ptr<std::unique_ptr<FFUInt[]>[]> outputs;
    unsigned n_threads = 0;
    unsigned nparsin = 0;
    unsigned nparsout = 0;
    std::atomic_flag has_failed;
  };

  std::vector<firefly::FFInt> FFBlackBox::failed()
  {
    // I am not really sure about what I am supposed to do if anything
    // fails.  I'm just returning a list of zeroes for now and set
    // some internal flag so that at the end I can test if something
    // has failed.
    has_failed.test_and_set();
    return std::vector<FFInt>(nparsout);
  }

  template<int N>
  std::vector<firefly::FFIntVec<N>> FFBlackBox::failed_n()
  {
    has_failed.test_and_set();
    return std::vector<firefly::FFIntVec<N>>(nparsout);
  }


  std::vector<firefly::FFInt>
  FFBlackBox::operator()(const std::vector<firefly::FFInt> & values)
  {
    if (current_thread_id == INVALID_THREAD_ID)
      current_thread_id = thread_id_count++;

    const unsigned thread_id = current_thread_id;
    if (thread_id >= n_threads) {
      ffLogErr("The thread_id is larger than expected");
      return failed();
    }

    FFMod p{FFInt::p, FFInt::p_inv};

    FFUInt * xin = inputs[thread_id].get();
    FFUInt * xout = outputs[thread_id].get();

    for (unsigned j=0; j<nparsin; ++j)
      xin[j] = values[j].n;

    FFStatus ret = ffMTEvalWithId(eval, thread_id, xin, p, xout);
    if (ret == FF_ERROR)
      return failed();

    std::vector<firefly::FFInt> out(nparsout);
    for (unsigned j=0; j<nparsout; ++j)
      out[j].n = xout[j];

    return out;
  }

  template <int N>
  std::vector<firefly::FFIntVec<N>>
  FFBlackBox::operator()(const std::vector<firefly::FFIntVec<N>> & values)
  {
    if (current_thread_id == INVALID_THREAD_ID)
      current_thread_id = thread_id_count++;

    const unsigned thread_id = current_thread_id;
    if (thread_id >= n_threads) {
      ffLogErr("The thread_id is larger than expected");
      return failed_n<N>();
    }

    FFMod p{FFInt::p, FFInt::p_inv};

    FFUInt * xin = inputs[thread_id].get();
    FFUInt * xout = outputs[thread_id].get();

    std::vector<firefly::FFIntVec<N>> out(nparsout);

    for (unsigned k=0; k<N; ++k) {

      for (unsigned j=0; j<nparsin; ++j)
        xin[j] = values[j].vec[k].n;

      FFStatus ret = ffMTEvalWithId(eval, thread_id, xin, p, xout);
      if (ret == FF_ERROR)
        return failed_n<N>();

      for (unsigned j=0; j<nparsout; ++j)
        out[j].vec[k].n = xout[j];
    }

    return out;
  }


  bool FFBlackBox::setup(FFGraph graph, unsigned n_threads_in)
  {
    n_threads = n_threads_in;

    nparsout = ffGraphNParsOut(graph);
    if (nparsout == FF_ERROR)
      return false;

    nparsin = ffNodeNParsOut(graph,0);
    if (nparsin == FF_ERROR)
      nparsin = 0;

    eval = ffMTEvalBegin(graph, n_threads);
    if (eval == 0)
      return false;

    inputs.reset(new std::unique_ptr<FFUInt[]>[n_threads]());
    if (nparsin)
      for (unsigned j=0; j<n_threads; ++j)
        inputs[j].reset(new FFUInt[nparsin]);

    outputs.reset(new std::unique_ptr<FFUInt[]>[n_threads]());
    for (unsigned j=0; j<n_threads; ++j)
      outputs[j].reset(new FFUInt[nparsout]);

    has_failed.clear();

    return true;
  }

  FFBlackBox::~FFBlackBox()
  {
    ffMTEvalEnd(eval);
  }

  firefly::Reconstructor<FFBlackBox>::verbosity_levels
  convertVerbosity(FFFireflyVerbosity v)
  {
    typedef firefly::Reconstructor<FFBlackBox> Rec;
    switch (v) {
    case FF_FIREFLY_SILENT:
      return Rec::SILENT;
    case FF_FIREFLY_IMPORTANT:
      return Rec::IMPORTANT;
    case FF_FIREFLY_CHATTY:
      return Rec::CHATTY;
    }
  }

} // namespace

extern "C" {

  struct FFFireflyRatFunList {
    std::vector<firefly::RationalFunction> rfs;
    unsigned nvars;
  };

  FFFireflyRatFunList * ffFireflyReconstructFunction(FFGraph graph,
                                                     FFFireflyRecOptions opt)
  {
    typedef firefly::Reconstructor<FFBlackBox> Rec;

    resetThreads();

    if (!opt.n_threads)
      opt.n_threads = std::thread::hardware_concurrency();

    if (!opt.bunch_size)
      opt.bunch_size = 1;

    FFBlackBox bb;
    if (!bb.setup(graph, opt.n_threads))
      return 0;

    Rec reconst(bb.nparsin, bb.n_threads, opt.bunch_size,
                bb, convertVerbosity(opt.verbosity));

    reconst.enable_factor_scan();
    reconst.enable_shift_scan();

    reconst.reconstruct();
    if (bb.has_failed.test())
      return 0;

    auto * results = new FFFireflyRatFunList{reconst.get_result(), bb.nparsin};

    return results;
  }

  size_t ffFireflyRatFunListLen(const FFFireflyRatFunList * list)
  {
    if (!list)
      return 0;
    return list->rfs.size();
  }

  unsigned ffFireflyRatFunListNVars(const FFFireflyRatFunList * list)
  {
    if (!list)
      return 0;
    return list->nvars;
  }

  void ffFireflyRatFunListFree(FFFireflyRatFunList * list)
  {
    delete list;
  }

  char * ffFireflyRatFunToStr(const FFFireflyRatFunList * rf, unsigned idx,
                              const FFCStr * vars)
  {
    if (!rf || idx >= rf->rfs.size()) {
      ffLogErr("Index of FireflyRatFunToStr out of bounds");
      return 0;
    }
    std::vector<std::string> ffvars(rf->nvars);
    for (unsigned j=0; j<rf->nvars; ++j)
      ffvars[j] = vars[j];
    std::string ret = rf->rfs[idx].to_string(ffvars);
    return strdup(ret.c_str());
  }

} // extern "C"
