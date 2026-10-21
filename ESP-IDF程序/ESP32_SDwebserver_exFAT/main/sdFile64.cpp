#include "sdFile64.h"
#include "common.h"

/* FatFs 逻辑驱动器数量（ESP-IDF 默认 2：内部 FFat + SD_MMC） */
#ifndef FF_VOLUMES
#define FF_VOLUMES 2
#endif

static int s_drive = -2;  // -2: 未探测, -1: 未找到, >=0: 驱动器号

static uint64_t volumeTotalBytes(FATFS *fs) {
  if (fs == nullptr || fs->n_fatent <= 2) {
    return 0;
  }
  uint64_t sectors = (uint64_t)(fs->n_fatent - 2) * fs->csize;
  uint32_t sectorSize = 512;
#if FF_MAX_SS != FF_MIN_SS
  sectorSize = fs->ssize;
#endif
  return sectors * sectorSize;
}

static void detectDrive() {
  int best = -1;
  uint64_t bestSize = 0;
  uint64_t bestDiff = UINT64_MAX;
  uint64_t cardSize = 0;

  /* 物理 SD 卡容量（未挂载时为 0），用于区分同一时刻挂载的 FFat 分区 */
  cardSize = SD_MMC.cardSize();

  for (int i = 0; i < FF_VOLUMES; i++) {
    char drv[3] = { (char)('0' + i), ':', '\0' };
    FATFS *fs = nullptr;
    DWORD freeClusters = 0;
    if (f_getfree(drv, &freeClusters, &fs) != FR_OK || fs == nullptr) {
      continue;
    }
    uint64_t total = volumeTotalBytes(fs);

    if (cardSize > 0) {
      /* 选择与物理卡容量最接近的卷（SD 卡分区表会占用少量空间） */
      uint64_t diff = (total > cardSize) ? (total - cardSize) : (cardSize - total);
      if (diff < bestDiff) {
        bestDiff = diff;
        best = i;
      }
    } else if (total > bestSize) {
      /* 未知物理容量时退化为“容量最大的卷” */
      bestSize = total;
      best = i;
    }
  }

  if (best < 0) {
    /* 探测失败时按工程挂载顺序回退：
     * CONFIG_SD == 0 时 FFat 先挂载占用 0:，SD_MMC 为 1:；
     * CONFIG_SD == 1 时 SD_MMC 是第一个挂载的卷，为 0:。 */
#if CONFIG_SD
    best = 0;
#else
    best = 1;
#endif
  }
  s_drive = best;
}

static bool driveError(FRESULT r) {
  return r == FR_INVALID_DRIVE || r == FR_NOT_ENABLED;
}

static void redetectDrive() {
  s_drive = -2;
  detectDrive();
}

namespace sd64 {

void init() {
  s_drive = -2;
  detectDrive();
}

String drivePrefix() {
  if (s_drive == -2) {
    detectDrive();
  }
  return String(s_drive) + ":";
}

String toFatPath(const String &path) {
  String p = path;
  if (!p.startsWith("/")) {
    p = "/" + p;
  }
  return drivePrefix() + p;
}

bool stat(const String &path, uint64_t &size, bool &isDir) {
  size = 0;
  isDir = false;

  FILINFO fno;
  FRESULT res = f_stat(toFatPath(path).c_str(), &fno);
  if (driveError(res)) {
    redetectDrive();
    res = f_stat(toFatPath(path).c_str(), &fno);
  }
  if (res != FR_OK) {
    return false;
  }

  isDir = (fno.fattrib & AM_DIR) != 0;
  size = isDir ? 0 : (uint64_t)fno.fsize;
  return true;
}

uint64_t fileSize(const String &path) {
  uint64_t size = 0;
  bool isDir = false;
  if (!stat(path, size, isDir)) {
    return 0;
  }
  return size;
}

bool exists(const String &path) {
  FILINFO fno;
  FRESULT res = f_stat(toFatPath(path).c_str(), &fno);
  if (driveError(res)) {
    redetectDrive();
    res = f_stat(toFatPath(path).c_str(), &fno);
  }
  return res == FR_OK;
}

}  // namespace sd64


bool SdFile64::openWith(const String &path, BYTE mode) {
  close();

  FRESULT res = f_open(&_fil, sd64::toFatPath(path).c_str(), mode);
  if (driveError(res)) {
    redetectDrive();
    res = f_open(&_fil, sd64::toFatPath(path).c_str(), mode);
  }

  _last = res;
  _open = (res == FR_OK);
  return _open;
}

bool SdFile64::openRead(const String &path) {
  return openWith(path, FA_READ | FA_OPEN_EXISTING);
}

bool SdFile64::openExisting(const String &path) {
  return openWith(path, FA_READ | FA_WRITE | FA_OPEN_EXISTING);
}

bool SdFile64::create(const String &path) {
  return openWith(path, FA_READ | FA_WRITE | FA_CREATE_ALWAYS);
}

bool SdFile64::seek(uint64_t pos) {
  if (!_open) {
    return false;
  }
  resetReadBuffer();
  _last = f_lseek(&_fil, (FSIZE_t)pos);
  return _last == FR_OK;
}

bool SdFile64::ensureReadBuffer() {
  if (_readBuf != nullptr) {
    return true;
  }
  if (_readBufUnavailable) {
    return false;
  }
  _readBuf = (uint8_t *)heap_caps_aligned_alloc(64, kReadAheadSize, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  if (_readBuf == nullptr) {
    _readBuf = (uint8_t *)heap_caps_aligned_alloc(64, kReadAheadSize, MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA | MALLOC_CAP_8BIT);
  }
  if (_readBuf == nullptr) {
    _readBufUnavailable = true;
    return false;
  }
  return true;
}

void SdFile64::resetReadBuffer() {
  _readBufPos = 0;
  _readBufLen = 0;
}

size_t SdFile64::read(uint8_t *buf, size_t len) {
  if (!_open || buf == nullptr || len == 0) {
    return 0;
  }

  size_t total = 0;
  while (total < len) {
    if (_readBufPos < _readBufLen) {
      size_t avail = _readBufLen - _readBufPos;
      size_t need = len - total;
      size_t n = (need < avail) ? need : avail;
      memcpy(buf + total, _readBuf + _readBufPos, n);
      _readBufPos += n;
      total += n;
      continue;
    }

    if (!ensureReadBuffer()) {
      UINT bytesRead = 0;
      _last = f_read(&_fil, buf + total, (UINT)(len - total), &bytesRead);
      if (_last != FR_OK || bytesRead == 0) {
        break;
      }
      total += bytesRead;
      continue;
    }

    UINT bytesRead = 0;
    _last = f_read(&_fil, _readBuf, (UINT)kReadAheadSize, &bytesRead);
    if (_last != FR_OK || bytesRead == 0) {
      break;
    }
    _readBufLen = bytesRead;
    _readBufPos = 0;
  }
  return total;
}

size_t SdFile64::write(const uint8_t *buf, size_t len) {
  if (!_open || buf == nullptr || len == 0) {
    return 0;
  }
  UINT bytesWritten = 0;
  _last = f_write(&_fil, buf, (UINT)len, &bytesWritten);
  return (_last == FR_OK) ? (size_t)bytesWritten : 0;
}

bool SdFile64::sync() {
  if (!_open) {
    return false;
  }
  _last = f_sync(&_fil);
  return _last == FR_OK;
}

void SdFile64::close() {
  if (_open) {
    f_close(&_fil);
    _open = false;
  }
  resetReadBuffer();
  if (_readBuf != nullptr) {
    heap_caps_free(_readBuf);
    _readBuf = nullptr;
  }
  _readBufUnavailable = false;
}

uint64_t SdFile64::position() const {
  if (!_open) {
    return 0;
  }
  return (uint64_t)f_tell(&_fil);
}

uint64_t SdFile64::size() const {
  if (!_open) {
    return 0;
  }
  return (uint64_t)f_size(&_fil);
}

const char *SdFile64::lastResultName() const {
  switch (_last) {
    case FR_OK: return "OK";
    case FR_DISK_ERR: return "DISK_ERR";
    case FR_INT_ERR: return "INT_ERR";
    case FR_NOT_READY: return "NOT_READY";
    case FR_NO_FILE: return "NO_FILE";
    case FR_NO_PATH: return "NO_PATH";
    case FR_INVALID_NAME: return "INVALID_NAME";
    case FR_DENIED: return "DENIED";
    case FR_EXIST: return "EXIST";
    case FR_INVALID_OBJECT: return "INVALID_OBJECT";
    case FR_WRITE_PROTECTED: return "WRITE_PROTECTED";
    case FR_INVALID_DRIVE: return "INVALID_DRIVE";
    case FR_NOT_ENABLED: return "NOT_ENABLED";
    case FR_NO_FILESYSTEM: return "NO_FILESYSTEM";
    case FR_MKFS_ABORTED: return "MKFS_ABORTED";
    case FR_TIMEOUT: return "TIMEOUT";
    case FR_LOCKED: return "LOCKED";
    case FR_NOT_ENOUGH_CORE: return "NOT_ENOUGH_CORE";
    case FR_TOO_MANY_OPEN_FILES: return "TOO_MANY_OPEN_FILES";
    case FR_INVALID_PARAMETER: return "INVALID_PARAMETER";
    default: return "UNKNOWN";
  }
}
