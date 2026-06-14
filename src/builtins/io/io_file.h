#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// ── Low-level file I/O (cross-platform) ──────────────────────────────

// Open: path, flags, mode → fd (-1 on error)
int lucis_open(const char* path, int flags, int mode);

// Read: fd, buf, count → bytes read (-1 on error)
int lucis_read(int fd, void* buf, int count);

// Write: fd, buf, count → bytes written (-1 on error)
int lucis_write(int fd, const void* buf, int count);

// Close: fd → 0 on success, -1 on error
int lucis_close(int fd);

// Seek: fd, offset, whence → new offset (-1 on error)
int64_t lucis_lseek(int fd, int64_t offset, int whence);

#ifdef __cplusplus
}
#endif
