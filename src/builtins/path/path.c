#define _POSIX_C_SOURCE 200809L

#include "path.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

/* ── helper: null-terminate a (ptr, len) pair ────────────────────────── */
static char* make_cstr(const char* s, size_t len) {
    char* buf = (char*)malloc(len + 1);
    if (!buf) return NULL;
    memcpy(buf, s, len);
    buf[len] = '\0';
    return buf;
}

/* ── helper: build result from buffer + length ───────────────────────── */
static lux_path_str_result make_result(const char* src, size_t len) {
    lux_path_str_result res = {NULL, 0};
    if (len == 0) return res;
    char* buf = (char*)malloc(len);
    if (!buf) return res;
    memcpy(buf, src, len);
    res.ptr = buf;
    res.len = len;
    return res;
}

/* ── helper: find last separator in (ptr, len) ───────────────────────── */
static size_t find_last_sep(const char* p, size_t len) {
    for (size_t i = len; i > 0; --i) {
        if (p[i - 1] == '/') return i - 1;
    }
    return (size_t)-1;
}

/* ── join ─────────────────────────────────────────────────────────────── */
lux_path_str_result lux_pathJoin(const char* a, size_t a_len,
                                       const char* b, size_t b_len) {
    lux_path_str_result res = {NULL, 0};

    /* If b is absolute, return b */
    if (b_len > 0 && b[0] == '/') return make_result(b, b_len);

    /* If a is empty, return b */
    if (a_len == 0) return make_result(b, b_len);

    /* If b is empty, return a */
    if (b_len == 0) return make_result(a, a_len);

    /* Strip trailing slash from a */
    size_t alen = a_len;
    while (alen > 1 && a[alen - 1] == '/') --alen;

    /* Strip leading slash from b */
    size_t boff = 0;
    while (boff < b_len && b[boff] == '/') ++boff;

    size_t total = alen + 1 + (b_len - boff);
    char* buf = (char*)malloc(total);
    if (!buf) return res;

    memcpy(buf, a, alen);
    buf[alen] = '/';
    memcpy(buf + alen + 1, b + boff, b_len - boff);

    res.ptr = buf;
    res.len = total;
    return res;
}

/* ── parent ───────────────────────────────────────────────────────────── */
lux_path_str_result lux_parent(const char* p, size_t len) {
    /* Strip trailing slashes */
    size_t end = len;
    while (end > 1 && p[end - 1] == '/') --end;

    size_t pos = find_last_sep(p, end);
    if (pos == (size_t)-1) {
        /* No separator — return "." */
        return make_result(".", 1);
    }
    if (pos == 0) {
        /* Root — return "/" */
        return make_result("/", 1);
    }
    return make_result(p, pos);
}

/* ── fileName ─────────────────────────────────────────────────────────── */
lux_path_str_result lux_fileName(const char* p, size_t len) {
    /* Strip trailing slashes */
    size_t end = len;
    while (end > 1 && p[end - 1] == '/') --end;

    size_t pos = find_last_sep(p, end);
    if (pos == (size_t)-1) {
        return make_result(p, end);
    }
    return make_result(p + pos + 1, end - pos - 1);
}

/* ── stem ─────────────────────────────────────────────────────────────── */
lux_path_str_result lux_stem(const char* p, size_t len) {
    /* Get fileName first */
    size_t end = len;
    while (end > 1 && p[end - 1] == '/') --end;

    size_t sep = find_last_sep(p, end);
    const char* name;
    size_t name_len;
    if (sep == (size_t)-1) {
        name = p;
        name_len = end;
    } else {
        name = p + sep + 1;
        name_len = end - sep - 1;
    }

    /* Find last dot in name (but not at position 0) */
    for (size_t i = name_len; i > 1; --i) {
        if (name[i - 1] == '.') {
            return make_result(name, i - 1);
        }
    }
    return make_result(name, name_len);
}

/* ── extension ────────────────────────────────────────────────────────── */
lux_path_str_result lux_extension(const char* p, size_t len) {
    lux_path_str_result res = {NULL, 0};

    /* Get fileName first */
    size_t end = len;
    while (end > 1 && p[end - 1] == '/') --end;

    size_t sep = find_last_sep(p, end);
    const char* name;
    size_t name_len;
    if (sep == (size_t)-1) {
        name = p;
        name_len = end;
    } else {
        name = p + sep + 1;
        name_len = end - sep - 1;
    }

    /* Find last dot (but not at position 0) */
    for (size_t i = name_len; i > 1; --i) {
        if (name[i - 1] == '.') {
            return make_result(name + i, name_len - i);
        }
    }
    /* No extension */
    return res;
}

/* ── isAbsolute ───────────────────────────────────────────────────────── */
int32_t lux_isAbsolute(const char* p, size_t len) {
    return (len > 0 && p[0] == '/') ? 1 : 0;
}

/* ── isRelative ───────────────────────────────────────────────────────── */
int32_t lux_isRelative(const char* p, size_t len) {
    return (len == 0 || p[0] != '/') ? 1 : 0;
}

/* ── normalize ────────────────────────────────────────────────────────── */
lux_path_str_result lux_normalize(const char* p, size_t len) {
    lux_path_str_result res = {NULL, 0};
    if (len == 0) return make_result(".", 1);

    int absolute = (p[0] == '/');

    /* Split into segments, resolve . and .. */
    /* Stack of segment (offset, length) pairs */
    size_t* seg_off = (size_t*)malloc(len * sizeof(size_t));
    size_t* seg_len = (size_t*)malloc(len * sizeof(size_t));
    if (!seg_off || !seg_len) {
        free(seg_off);
        free(seg_len);
        return res;
    }
    size_t nseg = 0;

    size_t i = 0;
    while (i < len) {
        /* Skip slashes */
        while (i < len && p[i] == '/') ++i;
        if (i >= len) break;

        /* Find end of segment */
        size_t start = i;
        while (i < len && p[i] != '/') ++i;
        size_t slen = i - start;

        if (slen == 1 && p[start] == '.') {
            /* Current dir — skip */
            continue;
        }
        if (slen == 2 && p[start] == '.' && p[start + 1] == '.') {
            /* Parent dir — pop if possible */
            if (nseg > 0 && !(seg_len[nseg - 1] == 2 &&
                              p[seg_off[nseg - 1]] == '.' &&
                              p[seg_off[nseg - 1] + 1] == '.')) {
                --nseg;
            } else if (!absolute) {
                seg_off[nseg] = start;
                seg_len[nseg] = slen;
                ++nseg;
            }
            continue;
        }

        seg_off[nseg] = start;
        seg_len[nseg] = slen;
        ++nseg;
    }

    /* Reconstruct path */
    if (nseg == 0) {
        free(seg_off);
        free(seg_len);
        return make_result(absolute ? "/" : ".", absolute ? 1 : 1);
    }

    /* Calculate total length */
    size_t total = absolute ? 1 : 0;
    for (size_t j = 0; j < nseg; ++j) {
        if (j > 0 || absolute) {
            if (j > 0) total += 1; /* '/' between segments */
        }
        total += seg_len[j];
    }

    char* buf = (char*)malloc(total);
    if (!buf) {
        free(seg_off);
        free(seg_len);
        return res;
    }

    size_t pos = 0;
    if (absolute) {
        buf[pos++] = '/';
    }
    for (size_t j = 0; j < nseg; ++j) {
        if (j > 0) buf[pos++] = '/';
        memcpy(buf + pos, p + seg_off[j], seg_len[j]);
        pos += seg_len[j];
    }

    free(seg_off);
    free(seg_len);

    res.ptr = buf;
    res.len = pos;
    return res;
}

/* ── toAbsolute ───────────────────────────────────────────────────────── */
lux_path_str_result lux_toAbsolute(const char* p, size_t len) {
    if (len > 0 && p[0] == '/') {
        return lux_normalize(p, len);
    }

    char cwdbuf[PATH_MAX];
    if (!getcwd(cwdbuf, sizeof(cwdbuf))) {
        return make_result(p, len);
    }
    size_t cwd_len = strlen(cwdbuf);

    lux_path_str_result joined = lux_pathJoin(cwdbuf, cwd_len, p, len);
    if (!joined.ptr) return make_result(p, len);

    lux_path_str_result normalized = lux_normalize(joined.ptr, joined.len);
    free((void*)joined.ptr);
    return normalized;
}

/* ── separator ────────────────────────────────────────────────────────── */
uint8_t lux_separator(void) {
    return (uint8_t)'/';
}

/* ── withExtension ────────────────────────────────────────────────────── */
lux_path_str_result lux_withExtension(const char* p, size_t p_len,
                                             const char* ext, size_t ext_len) {
    /* Find parent + stem part */
    size_t end = p_len;
    while (end > 1 && p[end - 1] == '/') --end;

    /* Find last separator */
    size_t sep = find_last_sep(p, end);
    const char* name;
    size_t name_start;
    size_t name_len;
    if (sep == (size_t)-1) {
        name = p;
        name_start = 0;
        name_len = end;
    } else {
        name = p + sep + 1;
        name_start = sep + 1;
        name_len = end - sep - 1;
    }

    /* Find last dot in name (not at position 0) */
    size_t dot_pos = name_len;
    for (size_t i = name_len; i > 1; --i) {
        if (name[i - 1] == '.') {
            dot_pos = i - 1;
            break;
        }
    }

    /* base = path up to and including the stem part */
    size_t base_len = name_start + dot_pos;

    if (ext_len == 0) {
        /* Remove extension */
        return make_result(p, base_len);
    }

    /* Check if ext starts with dot */
    int has_dot = (ext_len > 0 && ext[0] == '.');

    size_t total = base_len + (has_dot ? 0 : 1) + ext_len;
    char* buf = (char*)malloc(total);
    if (!buf) {
        lux_path_str_result res = {NULL, 0};
        return res;
    }

    memcpy(buf, p, base_len);
    size_t pos = base_len;
    if (!has_dot) buf[pos++] = '.';
    memcpy(buf + pos, ext, ext_len);

    lux_path_str_result res;
    res.ptr = buf;
    res.len = total;
    return res;
}

/* ── withFileName ─────────────────────────────────────────────────────── */
lux_path_str_result lux_withFileName(const char* p, size_t p_len,
                                            const char* name, size_t name_len) {
    /* Find parent part */
    size_t end = p_len;
    while (end > 1 && p[end - 1] == '/') --end;

    size_t sep = find_last_sep(p, end);
    if (sep == (size_t)-1) {
        /* No directory part — just return the new name */
        return make_result(name, name_len);
    }

    /* parent + '/' + name */
    size_t parent_len = sep + 1; /* include the separator */
    size_t total = parent_len + name_len;
    char* buf = (char*)malloc(total);
    if (!buf) {
        lux_path_str_result res = {NULL, 0};
        return res;
    }

    memcpy(buf, p, parent_len);
    memcpy(buf + parent_len, name, name_len);

    lux_path_str_result res;
    res.ptr = buf;
    res.len = total;
    return res;
}

/* ── joinAll: join Vec<string> of path components ──────────────── */

lux_path_str_result lux_joinAllVec(const lux_path_vec_header* parts) {
    typedef struct { const char* ptr; size_t len; } str_elem;
    str_elem* arr = (str_elem*)parts->ptr;
    size_t count = parts->len;

    if (count == 0) {
        char* r = (char*)malloc(1);
        r[0] = '\0';
        return (lux_path_str_result){ r, 0 };
    }

    // Join sequentially using existing pathJoin
    char* cur = (char*)malloc(arr[0].len + 1);
    memcpy(cur, arr[0].ptr, arr[0].len);
    cur[arr[0].len] = '\0';
    size_t curLen = arr[0].len;

    for (size_t i = 1; i < count; i++) {
        lux_path_str_result joined = lux_pathJoin(cur, curLen,
                                                        arr[i].ptr, arr[i].len);
        free(cur);
        cur = (char*)joined.ptr;
        curLen = joined.len;
    }

    lux_path_str_result result;
    result.ptr = cur;
    result.len = curLen;
    return result;
}
