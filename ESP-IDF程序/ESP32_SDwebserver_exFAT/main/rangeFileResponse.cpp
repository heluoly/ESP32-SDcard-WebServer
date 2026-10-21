#include "rangeFileResponse.h"
#include "sdFile64.h"

static bool parseU64(const String &s, uint64_t &out) {
  if (s.length() == 0 || s[0] < '0' || s[0] > '9') {
    return false;
  }
  errno = 0;
  char *end = nullptr;
  unsigned long long v = strtoull(s.c_str(), &end, 10);
  if (errno == ERANGE || end == s.c_str() || *end != '\0') {
    return false;
  }
  out = (uint64_t)v;
  return true;
}

static String contentTypeForPath(const String &path) {
  if (path.endsWith(".html")) return "text/html";
  if (path.endsWith(".mp4")) return "video/mp4";
  if (path.endsWith(".vtt")) return "text/vtt";
  if (path.endsWith(".css")) return "text/css";
  if (path.endsWith(".js")) return "application/javascript";
  if (path.endsWith(".json")) return "application/json";
  if (path.endsWith(".jpg") || path.endsWith(".jpeg")) return "image/jpeg";
  if (path.endsWith(".png")) return "image/png";
  if (path.endsWith(".gif")) return "image/gif";
  if (path.endsWith(".ico")) return "image/x-icon";
  if (path.endsWith(".txt")) return "text/plain; charset=utf-8";
  if (path.endsWith(".mkv")) return "video/x-matroska";
  if (path.endsWith(".mp3")) return "audio/mpeg";
  if (path.endsWith(".m4a")) return "audio/mp4";
  return "application/octet-stream";
}

class RangeFileResponse : public AsyncAbstractResponse {
private:
  SdFile64 _file;
  uint64_t _remaining;  // 本次响应还需发送的字节数（64 位）

public:
  RangeFileResponse(
    const String &path, const String &contentType, uint64_t start, uint64_t end, uint64_t total, int statusCode, bool addContentRange) : _remaining(0) {
    if (!_file.openRead(path)) {
      _code = 404;
      return;
    }
    if (!_file.seek(start)) {
      _file.close();
      _code = 500;
      return;
    }

    _code = statusCode;
    _contentType = contentType;
    _remaining = end - start + 1;

    addHeader("Accept-Ranges", "bytes");
    if (addContentRange) {
      char contentRange[96];
      snprintf(
        contentRange, sizeof(contentRange), "bytes %llu-%llu/%llu", (unsigned long long)start, (unsigned long long)end,
        (unsigned long long)total
      );
      addHeader("Content-Range", contentRange);
    }

    if (_remaining <= 0xFFFFFFFFULL) {
      /* <4GB：使用 32 位 Content-Length */
      _contentLength = (size_t)_remaining;
      _sendContentLength = true;
      _chunked = false;
    } else {
      /* >4GB：chunked 传输，避免 32 位 Content-Length 截断 */
      _contentLength = 0;   // 避免库做 32 位比较/截断
      _sendContentLength = false;
      _chunked = false;

      char sizeBuf[24];
      snprintf(sizeBuf, sizeof(sizeBuf), "%llu", (unsigned long long)_remaining);
      addHeader("Content-Length", sizeBuf);
    }
  }

  ~RangeFileResponse() override {
    _file.close();
  }

  bool _sourceValid() const override {
    return _file.isOpen();
  }

  size_t _fillBuffer(uint8_t *buf, size_t maxLen) override {
    if (!_file.isOpen() || _remaining == 0) {
      return 0;
    }
    size_t len = maxLen;
    if ((uint64_t)len > _remaining) {
      len = (size_t)_remaining;
    }
    size_t bytesRead = _file.read(buf, len);
    _remaining -= bytesRead;
    return bytesRead;
  }
};

static void sendRangeNotSatisfiable(AsyncWebServerRequest *request, uint64_t total) {
  AsyncWebServerResponse *response = request->beginResponse(416, "text/plain", "Range Not Satisfiable");
  char contentRange[64];
  snprintf(contentRange, sizeof(contentRange), "bytes */%llu", (unsigned long long)total);
  response->addHeader("Content-Range", contentRange);
  response->addHeader("Accept-Ranges", "bytes");
  request->send(response);
}

static void serveFile(AsyncWebServerRequest *request, const String &path, const String &contentType, const String &downloadName) {
  uint64_t fileSize = 0;
  bool isDir = false;
  if (!sd64::stat(path, fileSize, isDir) || isDir) {
    request->send(404, "text/plain", "File not found");
    return;
  }

  bool hasRange = request->hasHeader("Range");
  uint64_t start = 0;
  uint64_t end = (fileSize > 0) ? (fileSize - 1) : 0;
  bool partial = false;

  if (hasRange) {
    String rangeHeader = request->getHeader("Range")->value();
    rangeHeader.trim();
    if (!rangeHeader.startsWith("bytes=")) {
      request->send(400, "text/plain", "Bad Request");
      return;
    }

    String spec = rangeHeader.substring(6);
    spec.trim();
    if (spec.length() == 0 || spec.indexOf(',') >= 0) {
      /* 只支持单个 Range */
      sendRangeNotSatisfiable(request, fileSize);
      return;
    }

    int dashIndex = spec.indexOf('-');
    if (dashIndex < 0) {
      request->send(400, "text/plain", "Bad Request");
      return;
    }

    String startStr = spec.substring(0, dashIndex);
    String endStr = spec.substring(dashIndex + 1);
    startStr.trim();
    endStr.trim();

    if (startStr.length() == 0) {
      /* bytes=-suffix */
      uint64_t suffix = 0;
      if (!parseU64(endStr, suffix)) {
        request->send(400, "text/plain", "Bad Request");
        return;
      }
      if (fileSize == 0 || suffix == 0) {
        sendRangeNotSatisfiable(request, fileSize);
        return;
      }
      if (suffix > fileSize) {
        suffix = fileSize;
      }
      start = fileSize - suffix;
      end = fileSize - 1;
    } else {
      if (!parseU64(startStr, start)) {
        request->send(400, "text/plain", "Bad Request");
        return;
      }
      if (endStr.length() == 0) {
        end = (fileSize > 0) ? (fileSize - 1) : 0;
      } else {
        if (!parseU64(endStr, end)) {
          request->send(400, "text/plain", "Bad Request");
          return;
        }
        if (fileSize > 0 && end >= fileSize) {
          end = fileSize - 1;
        }
      }
    }

    if (fileSize == 0 || start >= fileSize || start > end) {
      sendRangeNotSatisfiable(request, fileSize);
      return;
    }
    partial = true;
  }

  if (fileSize == 0) {
    AsyncWebServerResponse *response = request->beginResponse(200, contentType, "");
    if (downloadName.length()) {
      response->addHeader("Content-Disposition", "attachment; filename=\"" + downloadName + "\"");
    }
    request->send(response);
    return;
  }

  RangeFileResponse *response = new (std::nothrow) RangeFileResponse(path, contentType, start, end, fileSize, partial ? 206 : 200, partial);
  if (response == nullptr) {
    request->send(500, "text/plain", "Internal Server Error");
    return;
  }
  if (!response->_sourceValid()) {
    delete response;
    request->send(500, "text/plain", "Internal Server Error");
    return;
  }
  if (downloadName.length()) {
    response->addHeader("Content-Disposition", "attachment; filename=\"" + downloadName + "\"");
  }
  request->send(response);
}

void rangeFileServe(AsyncWebServerRequest *request, const String &path) {
  serveFile(request, path, contentTypeForPath(path), "");
}

void rangeFileServeDownload(AsyncWebServerRequest *request, const String &path, const String &downloadName) {
  serveFile(request, path, "application/octet-stream", downloadName);
}
