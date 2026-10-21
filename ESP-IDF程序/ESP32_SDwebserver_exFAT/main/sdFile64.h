#ifndef SDFILE64_H
#define SDFILE64_H

#include <Arduino.h>

extern "C" {
#include "ff.h"
}

/*
 * 64 位文件访问层（exFAT / >4GB）
 *
 * Arduino-esp32 的 fs::File 使用 size_t / uint32_t 保存文件大小和偏移量，
 * 在 ESP32（32 位）上最大只能表示 4GB-1，因此无法读取或续传大于 4GB 的文件。
 *
 * FatFs 在 FF_FS_EXFAT == 1 时会把 FSIZE_t 定义为 64 位（QWORD），
 * f_size() / f_lseek() / f_tell() 都能处理超过 4GB 的文件。
 */

#if !defined(FF_FS_EXFAT) || (FF_FS_EXFAT == 0)
#error "FatFs exFAT support is disabled. Set FF_FS_EXFAT to 1 in $IDF_PATH/components/fatfs/src/ffconf.h and do a full clean build."
#endif

#if !defined(FF_USE_LFN) || (FF_USE_LFN < 1)
#error "FatFs exFAT support requires FF_USE_LFN >= 1 (CONFIG_FATFS_LFN_HEAP)."
#endif

/* FF_FS_EXFAT == 1 时 FSIZE_t 必须是 64 位，否则 >4GB 仍会被截断 */
static_assert(sizeof(FSIZE_t) == 8, "FSIZE_t is not 64-bit: exFAT/64-bit file support is not active");

namespace sd64 {

/* 选择 SD_MMC 所在的 FatFs 逻辑驱动器并缓存，例如返回 "1:"。
 * 应在 SD 卡挂载完成后调用；重复调用会重新探测（用于热插拔/重新挂载）。 */
void init();

/* 返回 SD 卡所在的驱动器前缀（"0:" / "1:" ...）。
 * 未初始化时自动探测；探测失败时回退到挂载顺序推断值。 */
String drivePrefix();

/* 将 Arduino 路径（"/test.txt"）转换为 FatFs 路径（"1:/test.txt"） */
String toFatPath(const String &path);

/* 查询文件信息。文件/目录存在返回 true。
 * size 为 64 位文件大小；目录时 size 为 0。 */
bool stat(const String &path, uint64_t &size, bool &isDir);

/* 仅查询 64 位文件大小，文件不存在或为目录时返回 0。
 * 通过 exists() 区分为空文件和不存在。 */
uint64_t fileSize(const String &path);

/* 文件/目录是否存在 */
bool exists(const String &path);

}  // namespace sd64


/* RAII 封装的 64 位 FatFs 文件对象 */
class SdFile64 {
public:
  SdFile64() = default;
  ~SdFile64() {
    close();
  }

  SdFile64(const SdFile64 &) = delete;
  SdFile64 &operator=(const SdFile64 &) = delete;

  /* 以只读方式打开已存在的文件 */
  bool openRead(const String &path);

  /* 以读写方式打开已存在的文件（不截断） */
  bool openExisting(const String &path);

  /* 以读写方式创建/清空文件 */
  bool create(const String &path);

  /* 从指定 64 位偏移继续写入（调用前应确认文件当前大小 == offset） */
  bool seek(uint64_t pos);

  size_t read(uint8_t *buf, size_t len);
  size_t write(const uint8_t *buf, size_t len);

  bool sync();
  void close();

  bool isOpen() const {
    return _open;
  }
  uint64_t position() const;
  uint64_t size() const;

  FRESULT lastResult() const {
    return _last;
  }
  const char *lastResultName() const;

private:
  static constexpr size_t kReadAheadSize = 16 * 1024;

  bool openWith(const String &path, BYTE mode);
  bool ensureReadBuffer();
  void resetReadBuffer();

  FIL _fil{};
  bool _open = false;
  FRESULT _last = FR_OK;

  uint8_t *_readBuf = nullptr;
  size_t _readBufPos = 0;
  size_t _readBufLen = 0;
  bool _readBufUnavailable = false;
};

#endif
