#include "rangeFileResponse.h"

size_t parseSize(const String& s) {
    if (s.length() == 0) return 0;
    return (size_t) strtoul(s.c_str(), nullptr, 10);
}

class RangeFileResponse : public AsyncAbstractResponse {
private:
  File _file;
  size_t _start;
  size_t _end;
  size_t _position;

public:
  RangeFileResponse(String path, String contentType, size_t start, size_t end, size_t total, int statusCode, bool addContentRange) {
    _file = my_fs.open(path, FILE_READ);
    if (!_file) {
      _code = 404;
      return;
    }
    if (!_file.seek(start)) {
      _file.close();
      _code = 500;
      return;
    }
    _code = statusCode;
    _contentLength = end - start + 1;
    _contentType = contentType;
    _start = start;
    _end = end;
    _position = 0;

    addHeader("Accept-Ranges", "bytes");
    if (addContentRange) {
      char contentRange[64];
      snprintf(contentRange, sizeof(contentRange), "bytes %zu-%zu/%zu", _start, _end, total);
      addHeader("Content-Range", contentRange);
    }
  }

  ~RangeFileResponse() {
    if (_file) _file.close();
    // Serial.println("File close");
  }

  bool _sourceValid() const override {
    return !!_file;
  }

  size_t _fillBuffer(uint8_t *buf, size_t maxLen) override {
    // Serial.printf("_fillBuffer called, position=%d, contentLength=%d\n", _position, _contentLength);
    if (!_file || _position >= _contentLength) return 0;
    size_t available = _contentLength - _position;
    size_t len = maxLen;
    if (len > available) len = available;
    size_t read = _file.read(buf, len);
    _position += read;
    return read;
  }
};


void rangeFileServe(AsyncWebServerRequest *request, const String& path) {
  String contentType = "";
  if (path.endsWith(".html")) contentType = "text/html";
  else if (path.endsWith(".mp4")) contentType = "video/mp4";
  else if (path.endsWith(".css")) contentType = "text/css";
  else if (path.endsWith(".js")) contentType = "application/javascript";
  else if (path.endsWith(".jpg")) contentType = "image/jpeg";
  else if (path.endsWith(".png")) contentType = "image/png";
  else if (path.endsWith(".gif")) contentType = "image/gif";
  else if (path.endsWith(".ico")) contentType = "image/x-icon";
  else contentType = "application/octet-stream";

  if (!my_fs.exists(path)) {
    request->send(404, "text/plain", "File not found");
    return;
  }

  File file = my_fs.open(path, FILE_READ);
  if (!file) {
    request->send(500, "text/plain", "Failed to open file");
    return;
  }
  size_t fileSize = file.size();
  file.close();

  // 检查Range头
  if (request->hasHeader("Range")) {
    String rangeHeader = request->getHeader("Range")->value();
    if (rangeHeader.startsWith("bytes=")) {
      String range = rangeHeader.substring(6);
      range.trim();
      int dashIndex = range.indexOf('-');

      if (dashIndex > 0) {
        String startStr = range.substring(0, dashIndex);
        String endStr = range.substring(dashIndex + 1);
        size_t start = 0, end = fileSize - 1;

        if (startStr.length() > 0) start = parseSize(startStr);
        if (endStr.length() > 0) end = parseSize(endStr);
        else end = fileSize - 1;  // 如果结束位置为空，则到文件尾

        // 验证范围有效性
        if (start >= fileSize || end >= fileSize || start > end) {
          request->send(416, "text/plain", "Range not satisfiable");
          return;
        }

        RangeFileResponse *response = new RangeFileResponse(path, contentType, start, end, fileSize, 206, true);
        if (!response->_sourceValid()) {
          delete response;
          request->send(500, "text/plain", "Internal Server Error");
          return;
        }
        request->send(response);
        return;
      } else if (dashIndex == 0) {
        // 后缀范围: "-suffix"
        String suffixStr = range.substring(1);
        size_t suffix = parseSize(suffixStr);
        if (suffix > fileSize) suffix = fileSize;
        size_t start = fileSize - suffix;
        size_t end = fileSize - 1;

        RangeFileResponse *response = new RangeFileResponse(path, contentType, start, end, fileSize, 206, true);
        if (!response->_sourceValid()) {
          delete response;
          request->send(500, "text/plain", "Internal Server Error");
          return;
        }
        request->send(response);
        return;
      } else {
        request->send(400, "text/plain", "Bad Request");
        return;
      }
    } else {
      request->send(400, "text/plain", "Bad Request");
      return;
    }
  }

  // 无Range：发送完整文件
  RangeFileResponse *response = new RangeFileResponse(path, contentType, 0, fileSize - 1, fileSize, 200, false);
  if (!response->_sourceValid()) {
    delete response;
    request->send(500, "text/plain", "Internal Server Error");
    return;
  }
  request->send(response);
}
