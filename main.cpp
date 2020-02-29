// Dependencies.
#include "boni.hpp"
#include <pcaudiolib/audio.h>

// Standard library.
#include <array>
#include <string>

// C standard library.
#include <cstddef>
#include <cstdio>
#include <cstdlib>

namespace pcaudiolib::impl {

/** \brief Callable releasing `audio_object*`
 *  for use with `std::unique_ptr`.
 */
using device_deleter =
    boni::handle_deleter<audio_object*, audio_object_destroy>;

} // namespace pcaudiolib::_impl

/** \brief Contains RAII wrappers for using `pcaudiolib`.
 *
 *  There are no attempts to wrap the entire `pcaudiolib` API
 *  into a C++ API.
 *  The interfaces provided are for simplifying resource management.
 *  In particular, the aim is to make it less likely to leak resources.
 */
namespace pcaudiolib {

/** \brief RAII wrapper releasing `audio_object*` when lifetime ends.
 *
 *  This class provides a implicit conversion
 *  to the underlying handle `audio_object*`,
 *  to make using the `pcaudiolib` C API easier.
 *
 *  ```
 *  {
 *    pcaudiolib::device audio_device{ create_audio_device_object(
          nullptr, "pcaudiolib-example", "raw-audio-player")};
 *    // Do things with `audio_device`.
 *    constexpr auto format = AUDIO_OBJECT_FORMAT_S16LE;
 *    constexpr auto rate = 44100u;
 *    constexpr auto channels = 1u;
 *    // There are implicit conversions to `audio_object*`.
 *    audio_object_open(audio_device, format, rate, channels);
 *    audio_object_close(audio_device);
 *    // Here, `audio_object_destroy` is called automatically.
 *  }
 *  ```
 */
using device = boni::auto_handle<impl::device_deleter>;

/** \brief Prints a descriptive error and `throw`
 * if `error_code` represents an error.
 *
 *  The provided `error_source` is necessary
 *  for obtaining the relevant error message from `pcaudiolib`.
 */
void throw_if_error(
    audio_object* const error_source, int const error_code) {
  // Zero is no error.
  if (error_code != 0) {
    std::string error_string{
        audio_object_strerror(error_source, error_code)};
    std::fprintf(stderr, "%s\n", error_string.c_str());
    std::exit(EXIT_FAILURE);
  }
}

/** \brief RAII wrapper calling `audio_object_close`.
 *
 *  An RAII wrapper usually stores a handle to the resource managed.
 *  However, the initialisation part for this RAII wrapper
 *  simply opens access to an audio device,
 *  with the lifetime of that device handle
 *  being managed in a different [class](\ref device).
 *  So this class is intended to be used to reduce the likelihood
 *  of forgetting to call `audio_object_close`,
 *  rather than directly managing the lifetime of a handle.
 *
 *  ```cpp
 *  {
 *    // This will call `audio_object_open` to open a stream.
 *    constexpr auto format = AUDIO_OBJECT_FORMAT_S16LE;
 *    constexpr auto rate = 44100u;
 *    constexpr auto channels = 1u;
 *    pcaudiolib::stream audio_sink{
 *        audio_device, format, rate, channels};
 *    // Here, `audio_object_close` will be called automatically.
 *  }
 *  ```
 */
class stream {
public:
  /** \brief The device type to close.
   *
   *  \par Design choices
   *
   *  An alternative and more general choice of `data_type`
   *  would be the underlying handle `audio_object`
   *  provided by `pcaudiolib`.
   *  However, since a reference type cannot be changed,
   *  using a reference to the \ref device class
   *  makes it clear to the compiler
   *  that the data used for closing the stream
   *  is precisely the same as those in the device object.
   *  This allows the compiler to optimise away
   *  the data members of this class entirely.
   */
  using data_type = device&;

  /** \brief Calls `audio_object_open` with the given arguments.
   *  \param target_device The device to open a stream for.
   *  The caller must guarantee that the `target_device` is not destroyed
   *  while this `stream` is alive.
   *  \param args Arguments to forward to `audio_object_open`.
   *  \invariant Unless `target_device` is closed unexpectedly,
   *  it will be opened as long as this object is alive.
   *  In particular, this allows for a `reopen` method to be added
   *  as a future possibility.
   *  Such a feature is used, for example, in
   *  [eSpeak
   * NG](https://github.com/espeak-ng/espeak-ng/blob/520a30e0b002dcaf0428e0599e8ed2f05aff6e46//src/libespeak-ng/speech.c#L122-L139).
   *  While this is easy to implement,
   *  there is no direct use for such a feature yet.
   *
   *  \par Design choices
   *  In general, there may be numerous function signatures
   *  that can produce one type of handle
   *  that an RAII wrapper may be interested in.
   *  So attempting to wrap all the "create" functions
   *  are usually not worthwhile, though doable.
   *  However, a special case is made for this
   *  because the "handle", or data necessary for clean-up,
   *  namely `target_device`,
   *  is specified as an argument to the "create" function,
   *  rather than as its returned value.
   */
  template <class... Args>
  stream(device& target_device, Args&&... args)
      : parent_device(target_device) {
    throw_if_error(parent_device, open(args...));
  }

  /** \brief Calls `audio_object_close` to release resource. */
  ~stream() { close(); }

private:
  int open(
      audio_object_format const format, uint32_t const rate,
      uint8_t const channels) {
    return audio_object_open(parent_device, format, rate, channels);
  }
  void close() { audio_object_close(parent_device); }
  device& parent_device;
};

} // namespace pcaudiolib

int main(int argc, char** argv) {
  if (argc != 2) {
    std::printf(
        "Plays an audio file.\n"
        "Usage: pcaudiolib-exaple <audio-file>\n"
        "The file must contain raw audio data:\n"
        "  * With Signed 16-bit PCM encoding,\n"
        "  * In Little-endian byte order,\n"
        "  * Has one channel, and\n"
        "  * Has sample rate of 44100 Hz.\n");
    std::exit(EXIT_SUCCESS);
  }

  const std::string audio_filename = argv[1];
  boni::file test_file{std::fopen(audio_filename.c_str(), "rb")};
  if (test_file.get() == nullptr) {
    std::fprintf(stderr, "Unable to load: %s\n", audio_filename.c_str());
    std::fflush(stderr);
    std::perror(nullptr);
    std::exit(EXIT_FAILURE);
  }

  pcaudiolib::device audio_device{create_audio_device_object(
      nullptr, "pcaudiolib-example", "raw-audio-player")};

  constexpr auto format = AUDIO_OBJECT_FORMAT_S16LE;
  constexpr auto rate = 44100u;
  constexpr auto channels = 1u;
  pcaudiolib::stream audio_sink{audio_device, format, rate, channels};

  constexpr auto buffer_size_in_bytes = rate * 2; // Two-second buffer.
  std::array<std::byte, buffer_size_in_bytes> read_buffer;
  const auto target_buffer = read_buffer.data();
  const auto source_stream = test_file.get();
  while (true) {
    using buffer_type = decltype(read_buffer);
    auto read_count = std::fread(
        /* buffer */ target_buffer,
        /* object_size */ sizeof(decltype(read_buffer)::value_type),
        /* object_count_in_buffer */ buffer_size_in_bytes,
        /* source_stream */ source_stream);
    if (read_count <= 0) {
      break;
    }
    pcaudiolib::throw_if_error(
        audio_device, audio_object_write(
                          audio_device, read_buffer.data(), read_count));
  }
  pcaudiolib::throw_if_error(
      audio_device, audio_object_drain(audio_device));
}
