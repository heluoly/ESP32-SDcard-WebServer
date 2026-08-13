#include "WebDAV.h"

AsyncWebServer *esp32_server_WebDAV = nullptr;
extern char webDavState;
extern char webDavMode;
extern int webDavPort;

const char *wdays[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
const char *months[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

// time_t → HTTP 日期字符串
String date2date(time_t date) {
  tm *gTm = gmtime(&date);
  char buf[40];
  snprintf(buf, sizeof(buf), "%s, %02d %s %04d %02d:%02d:%02d GMT",
           wdays[gTm->tm_wday],
           gTm->tm_mday,
           months[gTm->tm_mon],
           gTm->tm_year + 1900,
           gTm->tm_hour,
           gTm->tm_min,
           gTm->tm_sec);
  return buf;
}

bool notEncodable (char c) {
    return (c >= 'A' && c <= 'Z') ||
           (c >= 'a' && c <= 'z') ||
           (c >= '0' && c <= '9') ||
           c == '-' || c == '_' || c == '.' || c == '~' || c == '/';
}

char itoH(int c)
{
    return c <= 9 ? c + '0' : c - 10 + 'A';
}

String c2enc(const String& decoded)
{
    size_t l = decoded.length();
    for (size_t i = 0; i < decoded.length(); i++)
        if (!notEncodable(decoded[i]))
            l += 2;

    String ret;
    ret.reserve(l);
    for (size_t i = 0; i < decoded.length(); i++)
    {
        char c = decoded[i];
        if (notEncodable(c))
            ret += c;
        else
        {
            ret += '%';
            ret += itoH(c >> 4);
            ret += itoH(c & 0xf);
        }
    }
    return ret;
}

void handleGet(AsyncWebServerRequest *request) {
  rangeFileServe(request, request->url());
}

// 构建 PROPFIND 的单条响应 XML
String buildPropResponse(const String &href, File file, bool isDir) {
  String xml = "<D:response>\n";
  xml += "<D:href>" + c2enc(href) + "</D:href>\n";
  xml += "<D:propstat>\n";
  xml += "<D:prop>\n";

  time_t lastWrite = file.getLastWrite();
  String dateStr = date2date(lastWrite);
  xml += "<D:getlastmodified>" + dateStr + "</D:getlastmodified>\n";
  // xml += "<D:creationdate>" + dateStr + "</D:creationdate>\n";

  if (isDir) {
    xml += "<D:resourcetype><D:collection/></D:resourcetype>\n";
  } else {
    xml += "<D:resourcetype/>\n";
    xml += "<D:getcontentlength>" + String(file.size()) + "</D:getcontentlength>\n";
  }
  xml += "<D:displayname>" + String(file.name()) + "</D:displayname>\n";

  xml += "</D:prop>\n";
  xml += "<D:status>HTTP/1.1 200 OK</D:status>\n";
  xml += "</D:propstat>\n";
  xml += "</D:response>\n";
  return xml;
}

// 处理 PROPFIND 请求
void handlePropfind(AsyncWebServerRequest *request) {
  String path = request->url();
  if (path.indexOf("..") >= 0) {
    request->send(400);
    return;
  }

  String openPath = path;
  if (openPath.length() > 1 && openPath.endsWith("/")) {
    openPath.remove(openPath.length() - 1);
  }

  File entry = my_fs.open(openPath);
  if (!entry) {
    request->send(404);
    return;
  }
  bool isDir = entry.isDirectory();

  // 读取 Depth 头，默认为 1
  String depthStr = "1";
  if (request->hasHeader("Depth")) {
    depthStr = request->getHeader("Depth")->value();
    depthStr.trim();
  }
  int depth = (depthStr == "0") ? 0 : 1;  // 仅支持 0 或 1，infinity 暂降级为 1

  String xml = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
  xml += "<D:multistatus xmlns:D=\"DAV:\">\n";

  // 自身属性
  xml += buildPropResponse(path, entry, isDir);

  // Depth 1 且为目录时，列出直接子项
  if (depth == 1 && isDir) {
    File child = entry.openNextFile();
    while (child) {
      String childHref = child.path();
      xml += buildPropResponse(childHref, child, child.isDirectory());
      child = entry.openNextFile();
    }
  }
  entry.close();

  xml += "</D:multistatus>";
  request->send(207, "application/xml; charset=utf-8", xml);
}


// 处理 OPTIONS
void handleOptions(AsyncWebServerRequest *request) {
  AsyncWebServerResponse *response = request->beginResponse(200);
  response->addHeader("Allow", "GET, PROPFIND, OPTIONS");
  response->addHeader("DAV", "1");
  request->send(response);
}

// 主分发器
void handleWebDAV(AsyncWebServerRequest *request) {
  if (request->method() == HTTP_OPTIONS) {
    handleOptions(request);
  } else if (request->method() == HTTP_PROPFIND) {
    handlePropfind(request);
  } else if (request->method() == HTTP_GET) {
    handleGet(request);
  } else {
    request->send(405, "text/plain", "Method Not Allowed");
  }
}


void startWebDavService() {
  if (esp32_server_WebDAV != nullptr) {
    esp32_server_WebDAV->end();
    delete esp32_server_WebDAV;
    esp32_server_WebDAV = nullptr;
  }
  esp32_server_WebDAV = new AsyncWebServer(webDavPort);
  esp32_server_WebDAV->onNotFound(handleWebDAV);
  esp32_server_WebDAV->begin();
  webDavState = 1;
}

void stopWebDavService() {
  if (esp32_server_WebDAV != nullptr) {
    esp32_server_WebDAV->end();
    delete esp32_server_WebDAV;
    esp32_server_WebDAV = nullptr;
    webDavState = 0;
  }
}
