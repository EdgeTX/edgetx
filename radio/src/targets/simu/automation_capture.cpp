/*
 * Copyright (C) EdgeTX
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 */

#include "automation_capture.h"

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <cwctype>
#include <exception>
#include <limits>
#include <system_error>

#if defined(_WIN32)
#include <windows.h>
#else
#include <fcntl.h>
#include <unistd.h>
#endif

namespace edgetx
{
namespace automation
{
namespace
{

constexpr int WRITE_CANCELLABLE = 0;
constexpr int WRITE_CANCELLED = 1;
constexpr int WRITE_COMMITTING = 2;

std::atomic<bool> lcdInvalidationRequested{false};

void setError(std::string* error, const std::string& message)
{
  if (error != nullptr) *error = message;
}

void removeIfPresent(const std::filesystem::path& path)
{
  std::error_code ignored;
  (void)std::filesystem::remove(path, ignored);
}

#if defined(_WIN32)

std::string windowsError(const char* operation, DWORD error)
{
  return std::string(operation) + " (Win32 error " + std::to_string(error) +
         ")";
}

bool writeAll(HANDLE file, const void* data, std::size_t size,
              std::string* error)
{
  const auto* bytes = static_cast<const std::uint8_t*>(data);
  std::size_t offset = 0;
  while (offset < size) {
    const std::size_t remaining = size - offset;
    const DWORD requested = static_cast<DWORD>(
        std::min<std::size_t>(remaining, (std::numeric_limits<DWORD>::max)()));
    DWORD written = 0;
    if (!WriteFile(file, bytes + offset, requested, &written, nullptr)) {
      setError(error, windowsError("cannot write capture temporary file",
                                   GetLastError()));
      return false;
    }
    if (written == 0) {
      setError(error, "cannot write capture temporary file: zero-byte write");
      return false;
    }
    offset += written;
  }
  return true;
}

#else

bool writeAll(int file, const void* data, std::size_t size, std::string* error)
{
  const auto* bytes = static_cast<const std::uint8_t*>(data);
  std::size_t offset = 0;
  while (offset < size) {
    const ssize_t written = write(file, bytes + offset, size - offset);
    if (written > 0) {
      offset += static_cast<std::size_t>(written);
      continue;
    }
    if (written < 0 && errno == EINTR) continue;
    setError(error, std::string("cannot write capture temporary file: ") +
                        std::strerror(errno));
    return false;
  }
  return true;
}

#endif

bool isPathWithin(const std::filesystem::path& child,
                  const std::filesystem::path& root)
{
  auto childPart = child.begin();
  auto rootPart = root.begin();
  for (; rootPart != root.end(); ++rootPart, ++childPart) {
    if (childPart == child.end()) return false;
#if defined(_WIN32)
    std::wstring childValue = childPart->native();
    std::wstring rootValue = rootPart->native();
    std::transform(childValue.begin(), childValue.end(), childValue.begin(),
                   [](wchar_t value) { return std::towlower(value); });
    std::transform(rootValue.begin(), rootValue.end(), rootValue.begin(),
                   [](wchar_t value) { return std::towlower(value); });
    if (childValue != rootValue) return false;
#else
    if (*childPart != *rootPart) return false;
#endif
  }
  return true;
}

#if defined(_WIN32)

bool isUnsafeWin32Filename(const std::filesystem::path& filename)
{
  std::wstring value = filename.native();
  if (value.empty() || value.back() == L' ' || value.back() == L'.')
    return true;
  for (wchar_t character : value) {
    if (character > 0 && character < 32) return true;
    if (std::wstring(L"<>:\"/\\|?*").find(character) != std::wstring::npos)
      return true;
  }

  const std::size_t period = value.find(L'.');
  std::wstring base = value.substr(0, period);
  std::transform(base.begin(), base.end(), base.begin(),
                 [](wchar_t character) { return std::towupper(character); });
  if (base == L"CON" || base == L"PRN" || base == L"AUX" || base == L"NUL") {
    return true;
  }
  if (base.size() == 4 &&
      (base.compare(0, 3, L"COM") == 0 || base.compare(0, 3, L"LPT") == 0)) {
    const wchar_t suffix = base[3];
    return (suffix >= L'1' && suffix <= L'9') || suffix == L'\u00b9' ||
           suffix == L'\u00b2' || suffix == L'\u00b3';
  }
  return false;
}

#endif

CaptureOperationResult cancelledResult()
{
  return CaptureOperationResult::failure(
      ErrorCode::CaptureCancelled,
      "capture cancelled because the session is stopping");
}

}  // namespace

CaptureOperationResult CaptureOperationResult::success(std::uint64_t bytes)
{
  CaptureOperationResult result;
  result.ok = true;
  result.errorCode = ErrorCode::None;
  result.bytes = bytes;
  return result;
}

CaptureOperationResult CaptureOperationResult::failure(
    ErrorCode code, const std::string& message)
{
  CaptureOperationResult result;
  result.errorCode = code;
  result.message = message;
  return result;
}

CaptureOperationResult writeRgb565Ppm(const CaptureWriteRequest& request,
                                      std::atomic<int>& commitState)
{
  if (request.pixels == nullptr || request.width == 0 || request.height == 0 ||
      request.pixelCount !=
          static_cast<std::size_t>(request.width) * request.height) {
    return CaptureOperationResult::failure(
        ErrorCode::CaptureFailed, "capture framebuffer dimensions are invalid");
  }

  const std::string header = "P6\n" + std::to_string(request.width) + " " +
                             std::to_string(request.height) + "\n255\n";
  const std::uint64_t expectedBytes =
      static_cast<std::uint64_t>(header.size()) +
      static_cast<std::uint64_t>(request.pixelCount) * 3;
  std::string ioError;
  bool opened = false;

#if defined(_WIN32)
  HANDLE file =
      CreateFileW(request.artifactPath.temporaryPath.c_str(), GENERIC_WRITE, 0,
                  nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
  if (file == INVALID_HANDLE_VALUE) {
    return CaptureOperationResult::failure(
        ErrorCode::CaptureFailed,
        windowsError("cannot open capture temporary file", GetLastError()));
  }
  opened = true;
#else
  int openFlags = O_WRONLY | O_CREAT | O_EXCL;
#if defined(O_CLOEXEC)
  openFlags |= O_CLOEXEC;
#endif
  int file = open(request.artifactPath.temporaryPath.c_str(), openFlags, 0600);
  if (file == -1) {
    return CaptureOperationResult::failure(
        ErrorCode::CaptureFailed,
        std::string("cannot open capture temporary file: ") +
            std::strerror(errno));
  }
  opened = true;
#endif

  bool writeOk = writeAll(file, header.data(), header.size(), &ioError);
  std::vector<std::uint8_t> row(static_cast<std::size_t>(request.width) * 3);
  for (std::uint16_t y = 0; writeOk && y < request.height; ++y) {
    if (commitState.load(std::memory_order_acquire) == WRITE_CANCELLED) {
      writeOk = false;
      ioError = "capture cancelled";
      break;
    }
    const std::uint16_t* source =
        request.pixels + static_cast<std::size_t>(y) * request.width;
    for (std::uint16_t x = 0; x < request.width; ++x) {
      const std::uint16_t pixel = source[x];
      const std::uint8_t red = static_cast<std::uint8_t>((pixel >> 11) & 0x1f);
      const std::uint8_t green = static_cast<std::uint8_t>((pixel >> 5) & 0x3f);
      const std::uint8_t blue = static_cast<std::uint8_t>(pixel & 0x1f);
      row[static_cast<std::size_t>(x) * 3] =
          static_cast<std::uint8_t>((red << 3) | (red >> 2));
      row[static_cast<std::size_t>(x) * 3 + 1] =
          static_cast<std::uint8_t>((green << 2) | (green >> 4));
      row[static_cast<std::size_t>(x) * 3 + 2] =
          static_cast<std::uint8_t>((blue << 3) | (blue >> 2));
    }
    writeOk = writeAll(file, row.data(), row.size(), &ioError);
  }

  bool flushOk = writeOk;
  if (flushOk) {
#if defined(_WIN32)
    if (!FlushFileBuffers(file)) {
      ioError =
          windowsError("cannot flush capture temporary file", GetLastError());
      flushOk = false;
    }
#else
    if (fsync(file) != 0) {
      ioError = std::string("cannot flush capture temporary file: ") +
                std::strerror(errno);
      flushOk = false;
    }
#endif
  }

  bool closeOk = true;
  if (opened) {
#if defined(_WIN32)
    if (!CloseHandle(file)) {
      if (ioError.empty())
        ioError =
            windowsError("cannot close capture temporary file", GetLastError());
      closeOk = false;
    }
#else
    if (close(file) != 0) {
      if (ioError.empty())
        ioError = std::string("cannot close capture temporary file: ") +
                  std::strerror(errno);
      closeOk = false;
    }
#endif
  }

  if (!writeOk || !flushOk || !closeOk) {
    removeIfPresent(request.artifactPath.temporaryPath);
    if (commitState.load(std::memory_order_acquire) == WRITE_CANCELLED)
      return cancelledResult();
    return CaptureOperationResult::failure(ErrorCode::CaptureFailed, ioError);
  }

  std::error_code sizeError;
  const std::uintmax_t actualBytes =
      std::filesystem::file_size(request.artifactPath.temporaryPath, sizeError);
  if (sizeError || actualBytes != expectedBytes) {
    removeIfPresent(request.artifactPath.temporaryPath);
    return CaptureOperationResult::failure(
        ErrorCode::CaptureFailed,
        sizeError ? "cannot verify capture temporary file size"
                  : "capture temporary file has an unexpected size");
  }

  int expectedState = WRITE_CANCELLABLE;
  if (!commitState.compare_exchange_strong(expectedState, WRITE_COMMITTING,
                                           std::memory_order_acq_rel)) {
    removeIfPresent(request.artifactPath.temporaryPath);
    return cancelledResult();
  }

#if defined(_WIN32)
  if (!MoveFileExW(request.artifactPath.temporaryPath.c_str(),
                   request.artifactPath.finalPath.c_str(),
                   MOVEFILE_WRITE_THROUGH)) {
    const DWORD publishError = GetLastError();
    removeIfPresent(request.artifactPath.temporaryPath);
    if (publishError == ERROR_FILE_EXISTS ||
        publishError == ERROR_ALREADY_EXISTS) {
      return CaptureOperationResult::failure(ErrorCode::ArtifactExists,
                                             "capture target already exists");
    }
    return CaptureOperationResult::failure(
        ErrorCode::CaptureFailed,
        windowsError("cannot publish capture artifact", publishError));
  }
#else
  if (link(request.artifactPath.temporaryPath.c_str(),
           request.artifactPath.finalPath.c_str()) != 0) {
    const int publishError = errno;
    removeIfPresent(request.artifactPath.temporaryPath);
    if (publishError == EEXIST) {
      return CaptureOperationResult::failure(ErrorCode::ArtifactExists,
                                             "capture target already exists");
    }
    return CaptureOperationResult::failure(
        ErrorCode::CaptureFailed,
        std::string("cannot publish capture artifact: ") +
            std::strerror(publishError));
  }
  if (unlink(request.artifactPath.temporaryPath.c_str()) != 0) {
    const int cleanupError = errno;
    removeIfPresent(request.artifactPath.finalPath);
    removeIfPresent(request.artifactPath.temporaryPath);
    return CaptureOperationResult::failure(
        ErrorCode::CaptureFailed,
        std::string("cannot remove capture temporary link: ") +
            std::strerror(cleanupError));
  }
#endif

  return CaptureOperationResult::success(expectedBytes);
}

void AutomationCapture::PendingCapture::clear()
{
  id = 0;
  epoch = 0;
  armedAfter = 0;
  capturedSequence = 0;
  artifactPath = CaptureArtifactPath();
}

AutomationCapture::~AutomationCapture() { shutdown(); }

bool AutomationCapture::configure(const std::string& outputRoot,
                                  std::uint16_t captureWidth,
                                  std::uint16_t captureHeight,
                                  std::uint8_t captureDepth, std::string* error,
                                  CaptureWriteFunction captureWriteFunction)
{
  if (error != nullptr) error->clear();
  std::lock_guard<std::mutex> lock(mutex);
  if (stage != Stage::Disabled) {
    setError(error, "automation capture is already configured");
    return false;
  }

  std::error_code pathError;
  std::filesystem::path root = std::filesystem::canonical(
      std::filesystem::u8path(outputRoot), pathError);
  if (pathError || !std::filesystem::is_directory(root, pathError) ||
      pathError) {
    setError(error, "automation output root is not an existing directory");
    return false;
  }

  canonicalRoot = std::move(root);
  width = captureWidth;
  height = captureHeight;
  depth = captureDepth;
  writeFunction =
      captureWriteFunction == nullptr ? writeRgb565Ppm : captureWriteFunction;
  if (depth != 16 || width == 0 || height == 0) {
    return true;
  }

  try {
    snapshot.resize(static_cast<std::size_t>(width) * height);
    stage = Stage::Idle;
    worker = std::thread(&AutomationCapture::workerMain, this);
  } catch (const std::exception& exception) {
    snapshot.clear();
    stage = Stage::Disabled;
    setError(error,
             std::string("cannot start capture worker: ") + exception.what());
    return false;
  }
  return true;
}

bool AutomationCapture::configured() const
{
  std::lock_guard<std::mutex> lock(mutex);
  return stage != Stage::Disabled && stage != Stage::Stopping;
}

CaptureOperationResult AutomationCapture::validatePath(
    const std::string& relativePath, RequestId id, SessionEpoch epoch,
    CaptureArtifactPath* artifactPath) const
{
  std::filesystem::path root;
  std::uint8_t captureDepth = 0;
  {
    std::lock_guard<std::mutex> lock(mutex);
    root = canonicalRoot;
    captureDepth = depth;
  }
  if (captureDepth != 16) {
    return CaptureOperationResult::failure(
        ErrorCode::UnsupportedLcdDepth,
        "capture currently requires an RGB565 LCD target");
  }
  if (artifactPath == nullptr) {
    return CaptureOperationResult::failure(ErrorCode::InternalError,
                                           "capture path output is missing");
  }
  if (relativePath.empty() || relativePath.size() > MAX_CAPTURE_PATH_BYTES) {
    return CaptureOperationResult::failure(
        relativePath.size() > MAX_CAPTURE_PATH_BYTES ? ErrorCode::PathTooLong
                                                     : ErrorCode::UnsafePath,
        relativePath.size() > MAX_CAPTURE_PATH_BYTES
            ? "capture path exceeds 1024 UTF-8 bytes"
            : "capture path is empty");
  }
  if (relativePath.find('\0') != std::string::npos) {
    return CaptureOperationResult::failure(ErrorCode::UnsafePath,
                                           "capture path contains NUL");
  }

  const std::filesystem::path relative = std::filesystem::u8path(relativePath);
  if (relative.empty() || relative.is_absolute() || relative.has_root_name() ||
      relative.has_root_directory() || relative.filename().empty()) {
    return CaptureOperationResult::failure(
        ErrorCode::UnsafePath, "capture path must be a relative file path");
  }
  for (const std::filesystem::path& component : relative) {
    if (component == "." || component == "..") {
      return CaptureOperationResult::failure(
          ErrorCode::UnsafePath,
          "capture path cannot contain dot or parent components");
    }
  }
  if (relative.extension() != ".ppm") {
    return CaptureOperationResult::failure(
        ErrorCode::UnsafePath, "capture path must end in lowercase .ppm");
  }
#if defined(_WIN32)
  if (isUnsafeWin32Filename(relative.filename())) {
    return CaptureOperationResult::failure(
        ErrorCode::UnsafePath, "capture filename is reserved by Win32");
  }
#endif

  const std::filesystem::path finalPath = root / relative;
  const std::filesystem::path parent = finalPath.parent_path();
  std::error_code pathError;
  const std::filesystem::path canonicalParent =
      std::filesystem::canonical(parent, pathError);
  if (pathError || !std::filesystem::is_directory(canonicalParent, pathError) ||
      pathError || !isPathWithin(canonicalParent, root)) {
    return CaptureOperationResult::failure(
        ErrorCode::UnsafePath,
        "capture parent must be an existing directory under the output root");
  }

  const std::filesystem::file_status finalStatus =
      std::filesystem::symlink_status(finalPath, pathError);
  if (!pathError && std::filesystem::exists(finalStatus)) {
    return CaptureOperationResult::failure(ErrorCode::ArtifactExists,
                                           "capture target already exists");
  }
  if (pathError && pathError != std::errc::no_such_file_or_directory) {
    return CaptureOperationResult::failure(
        ErrorCode::UnsafePath, "cannot inspect capture target path");
  }

  std::filesystem::path temporaryName = ".";
  temporaryName += finalPath.filename().native();
  temporaryName +=
      ".tmp-v1-" + std::to_string(epoch) + "-" + std::to_string(id);

  artifactPath->relative = relativePath;
  artifactPath->finalPath = canonicalParent / finalPath.filename();
  artifactPath->temporaryPath = canonicalParent / temporaryName;
  return CaptureOperationResult::success(0);
}

CaptureOperationResult AutomationCapture::arm(
    RequestId id, SessionEpoch epoch, DisplaySequence armedAfter,
    CaptureArtifactPath&& artifactPath)
{
  std::lock_guard<std::mutex> lock(mutex);
  if (stage == Stage::Disabled) {
    return CaptureOperationResult::failure(
        ErrorCode::UnsupportedLcdDepth,
        "capture currently requires an RGB565 LCD target");
  }
  if (stage != Stage::Idle) {
    return CaptureOperationResult::failure(
        ErrorCode::OperationBusy, "another capture operation is active");
  }

  pending.id = id;
  pending.epoch = epoch;
  pending.armedAfter = armedAfter;
  pending.capturedSequence = 0;
  pending.artifactPath = std::move(artifactPath);
  completion = CaptureCompletion();
  commitState.store(WRITE_CANCELLABLE, std::memory_order_release);
  stage = Stage::Armed;
  return CaptureOperationResult::success(0);
}

void AutomationCapture::onDisplayFrame(DisplaySequence sequence,
                                       SessionEpoch epoch,
                                       const std::uint16_t* pixels,
                                       std::size_t pixelCount)
{
  std::lock_guard<std::mutex> lock(mutex);
  if (stage != Stage::Armed || pending.epoch != epoch ||
      sequence <= pending.armedAfter) {
    return;
  }

  if (pixels == nullptr || pixelCount != snapshot.size()) {
    completeLocked(CaptureOperationResult::failure(
        ErrorCode::CaptureFailed,
        "LCD notification exposed an unexpected framebuffer size"));
    condition.notify_all();
    return;
  }

  std::copy_n(pixels, pixelCount, snapshot.data());
  pending.capturedSequence = sequence;
  stage = Stage::SnapshotReady;
  condition.notify_one();
}

bool AutomationCapture::takeCompletion(CaptureCompletion* result)
{
  std::lock_guard<std::mutex> lock(mutex);
  return takeCompletionLocked(result);
}

bool AutomationCapture::cancelAndWait(CaptureCompletion* result)
{
  std::unique_lock<std::mutex> lock(mutex);
  if (stage == Stage::Disabled || stage == Stage::Idle ||
      stage == Stage::Stopping) {
    return false;
  }
  if (stage == Stage::Completed) return takeCompletionLocked(result);
  if (stage == Stage::Armed) {
    completeLocked(cancelledResult());
    return takeCompletionLocked(result);
  }

  int expectedState = WRITE_CANCELLABLE;
  (void)commitState.compare_exchange_strong(expectedState, WRITE_CANCELLED,
                                            std::memory_order_acq_rel);
  condition.notify_all();
  condition.wait(lock, [this]() { return stage == Stage::Completed; });
  return takeCompletionLocked(result);
}

void AutomationCapture::shutdown()
{
  CaptureCompletion ignored;
  (void)cancelAndWait(&ignored);

  {
    std::lock_guard<std::mutex> lock(mutex);
    if (!worker.joinable()) {
      stage = Stage::Disabled;
      snapshot.clear();
      return;
    }
    stage = Stage::Stopping;
    condition.notify_all();
  }
  worker.join();

  std::lock_guard<std::mutex> lock(mutex);
  stage = Stage::Disabled;
  pending.clear();
  completion = CaptureCompletion();
  snapshot.clear();
}

void AutomationCapture::workerMain()
{
  while (true) {
    CaptureWriteRequest request;
    {
      std::unique_lock<std::mutex> lock(mutex);
      condition.wait(lock, [this]() {
        return stage == Stage::SnapshotReady || stage == Stage::Stopping;
      });
      if (stage == Stage::Stopping) return;

      request.artifactPath = pending.artifactPath;
      request.pixels = snapshot.data();
      request.pixelCount = snapshot.size();
      request.width = width;
      request.height = height;
      stage = Stage::Writing;
    }

    CaptureOperationResult result;
    try {
      if (commitState.load(std::memory_order_acquire) == WRITE_CANCELLED) {
        result = cancelledResult();
      } else {
        result = writeFunction(request, commitState);
      }
    } catch (const std::exception& exception) {
      removeIfPresent(request.artifactPath.temporaryPath);
      result = CaptureOperationResult::failure(
          ErrorCode::CaptureFailed,
          std::string("capture writer raised an exception: ") +
              exception.what());
    } catch (...) {
      removeIfPresent(request.artifactPath.temporaryPath);
      result = CaptureOperationResult::failure(
          ErrorCode::CaptureFailed,
          "capture writer raised an unknown exception");
    }

    {
      std::lock_guard<std::mutex> lock(mutex);
      completeLocked(result);
      condition.notify_all();
    }
  }
}

void AutomationCapture::completeLocked(const CaptureOperationResult& result)
{
  completion.id = pending.id;
  completion.epoch = pending.epoch;
  completion.ok = result.ok;
  completion.errorCode = result.errorCode;
  completion.message = result.message;
  completion.artifact.displaySequence = pending.capturedSequence;
  completion.artifact.path = pending.artifactPath.relative;
  completion.artifact.width = width;
  completion.artifact.height = height;
  completion.artifact.depth = depth;
  completion.artifact.bytes = result.bytes;
  stage = Stage::Completed;
}

bool AutomationCapture::takeCompletionLocked(CaptureCompletion* result)
{
  if (stage != Stage::Completed) return false;
  if (result != nullptr) *result = completion;
  completion = CaptureCompletion();
  pending.clear();
  stage = Stage::Idle;
  return true;
}

void requestAutomationLcdInvalidation()
{
  lcdInvalidationRequested.store(true, std::memory_order_release);
}

bool consumeAutomationLcdInvalidation()
{
  return lcdInvalidationRequested.exchange(false, std::memory_order_acq_rel);
}

}  // namespace automation
}  // namespace edgetx
