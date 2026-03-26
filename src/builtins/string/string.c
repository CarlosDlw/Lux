#include "string.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

// ── Search & Match ──────────────────────────────────────────────────────────

int tollvm_contains(const char* s, size_t sLen,
                    const char* sub, size_t subLen) {
    if (subLen == 0) return 1;
    if (subLen > sLen) return 0;

    for (size_t i = 0; i <= sLen - subLen; i++) {
        if (memcmp(s + i, sub, subLen) == 0) return 1;
    }
    return 0;
}

int tollvm_startsWith(const char* s, size_t sLen,
                      const char* prefix, size_t prefixLen) {
    if (prefixLen > sLen) return 0;
    return memcmp(s, prefix, prefixLen) == 0;
}

int tollvm_endsWith(const char* s, size_t sLen,
                    const char* suffix, size_t suffixLen) {
    if (suffixLen > sLen) return 0;
    return memcmp(s + sLen - suffixLen, suffix, suffixLen) == 0;
}

int64_t tollvm_indexOf(const char* s, size_t sLen,
                       const char* sub, size_t subLen) {
    if (subLen == 0) return 0;
    if (subLen > sLen) return -1;

    for (size_t i = 0; i <= sLen - subLen; i++) {
        if (memcmp(s + i, sub, subLen) == 0)
            return (int64_t)i;
    }
    return -1;
}

int64_t tollvm_lastIndexOf(const char* s, size_t sLen,
                           const char* sub, size_t subLen) {
    if (subLen == 0) return (int64_t)sLen;
    if (subLen > sLen) return -1;

    for (size_t i = sLen - subLen; ; i--) {
        if (memcmp(s + i, sub, subLen) == 0)
            return (int64_t)i;
        if (i == 0) break;
    }
    return -1;
}

size_t tollvm_count(const char* s, size_t sLen,
                    const char* sub, size_t subLen) {
    if (subLen == 0) return 0;
    if (subLen > sLen) return 0;

    size_t count = 0;
    for (size_t i = 0; i <= sLen - subLen; i++) {
        if (memcmp(s + i, sub, subLen) == 0) {
            count++;
            i += subLen - 1; // non-overlapping
        }
    }
    return count;
}

// ── Transformation ──────────────────────────────────────────────────────────

tollvm_str_result tollvm_toUpper(const char* s, size_t sLen) {
    char* buf = (char*)malloc(sLen + 1);
    for (size_t i = 0; i < sLen; i++)
        buf[i] = (char)toupper((unsigned char)s[i]);
    buf[sLen] = '\0';
    return (tollvm_str_result){ buf, sLen };
}

tollvm_str_result tollvm_toLower(const char* s, size_t sLen) {
    char* buf = (char*)malloc(sLen + 1);
    for (size_t i = 0; i < sLen; i++)
        buf[i] = (char)tolower((unsigned char)s[i]);
    buf[sLen] = '\0';
    return (tollvm_str_result){ buf, sLen };
}

tollvm_str_result tollvm_trim(const char* s, size_t sLen) {
    size_t start = 0;
    while (start < sLen && isspace((unsigned char)s[start])) start++;
    size_t end = sLen;
    while (end > start && isspace((unsigned char)s[end - 1])) end--;

    size_t newLen = end - start;
    char* buf = (char*)malloc(newLen + 1);
    memcpy(buf, s + start, newLen);
    buf[newLen] = '\0';
    return (tollvm_str_result){ buf, newLen };
}

tollvm_str_result tollvm_trimLeft(const char* s, size_t sLen) {
    size_t start = 0;
    while (start < sLen && isspace((unsigned char)s[start])) start++;

    size_t newLen = sLen - start;
    char* buf = (char*)malloc(newLen + 1);
    memcpy(buf, s + start, newLen);
    buf[newLen] = '\0';
    return (tollvm_str_result){ buf, newLen };
}

tollvm_str_result tollvm_trimRight(const char* s, size_t sLen) {
    size_t end = sLen;
    while (end > 0 && isspace((unsigned char)s[end - 1])) end--;

    char* buf = (char*)malloc(end + 1);
    memcpy(buf, s, end);
    buf[end] = '\0';
    return (tollvm_str_result){ buf, end };
}

tollvm_str_result tollvm_replace(const char* s, size_t sLen,
                                 const char* old, size_t oldLen,
                                 const char* rep, size_t repLen) {
    if (oldLen == 0) {
        char* buf = (char*)malloc(sLen + 1);
        memcpy(buf, s, sLen);
        buf[sLen] = '\0';
        return (tollvm_str_result){ buf, sLen };
    }

    // Count occurrences first
    size_t cnt = 0;
    for (size_t i = 0; i <= sLen - oldLen; i++) {
        if (memcmp(s + i, old, oldLen) == 0) {
            cnt++;
            i += oldLen - 1;
        }
    }

    if (cnt == 0) {
        char* buf = (char*)malloc(sLen + 1);
        memcpy(buf, s, sLen);
        buf[sLen] = '\0';
        return (tollvm_str_result){ buf, sLen };
    }

    size_t newLen = sLen + cnt * (repLen - oldLen);
    char* buf = (char*)malloc(newLen + 1);
    size_t wi = 0;

    for (size_t i = 0; i < sLen; ) {
        if (i <= sLen - oldLen && memcmp(s + i, old, oldLen) == 0) {
            memcpy(buf + wi, rep, repLen);
            wi += repLen;
            i += oldLen;
        } else {
            buf[wi++] = s[i++];
        }
    }

    buf[newLen] = '\0';
    return (tollvm_str_result){ buf, newLen };
}

tollvm_str_result tollvm_replaceFirst(const char* s, size_t sLen,
                                      const char* old, size_t oldLen,
                                      const char* rep, size_t repLen) {
    if (oldLen == 0 || oldLen > sLen) {
        char* buf = (char*)malloc(sLen + 1);
        memcpy(buf, s, sLen);
        buf[sLen] = '\0';
        return (tollvm_str_result){ buf, sLen };
    }

    // Find first occurrence
    int64_t pos = -1;
    for (size_t i = 0; i <= sLen - oldLen; i++) {
        if (memcmp(s + i, old, oldLen) == 0) {
            pos = (int64_t)i;
            break;
        }
    }

    if (pos < 0) {
        char* buf = (char*)malloc(sLen + 1);
        memcpy(buf, s, sLen);
        buf[sLen] = '\0';
        return (tollvm_str_result){ buf, sLen };
    }

    size_t newLen = sLen - oldLen + repLen;
    char* buf = (char*)malloc(newLen + 1);
    memcpy(buf, s, (size_t)pos);
    memcpy(buf + pos, rep, repLen);
    memcpy(buf + pos + repLen, s + pos + oldLen, sLen - (size_t)pos - oldLen);
    buf[newLen] = '\0';
    return (tollvm_str_result){ buf, newLen };
}

tollvm_str_result tollvm_repeat(const char* s, size_t sLen, size_t n) {
    size_t newLen = sLen * n;
    char* buf = (char*)malloc(newLen + 1);
    for (size_t i = 0; i < n; i++)
        memcpy(buf + i * sLen, s, sLen);
    buf[newLen] = '\0';
    return (tollvm_str_result){ buf, newLen };
}

tollvm_str_result tollvm_reverse(const char* s, size_t sLen) {
    char* buf = (char*)malloc(sLen + 1);
    for (size_t i = 0; i < sLen; i++)
        buf[i] = s[sLen - 1 - i];
    buf[sLen] = '\0';
    return (tollvm_str_result){ buf, sLen };
}

// ── Formatting ──────────────────────────────────────────────────────────────

tollvm_str_result tollvm_padLeft(const char* s, size_t sLen,
                                 size_t width, char fill) {
    if (sLen >= width) {
        char* buf = (char*)malloc(sLen + 1);
        memcpy(buf, s, sLen);
        buf[sLen] = '\0';
        return (tollvm_str_result){ buf, sLen };
    }

    char* buf = (char*)malloc(width + 1);
    size_t pad = width - sLen;
    memset(buf, fill, pad);
    memcpy(buf + pad, s, sLen);
    buf[width] = '\0';
    return (tollvm_str_result){ buf, width };
}

tollvm_str_result tollvm_padRight(const char* s, size_t sLen,
                                  size_t width, char fill) {
    if (sLen >= width) {
        char* buf = (char*)malloc(sLen + 1);
        memcpy(buf, s, sLen);
        buf[sLen] = '\0';
        return (tollvm_str_result){ buf, sLen };
    }

    char* buf = (char*)malloc(width + 1);
    memcpy(buf, s, sLen);
    memset(buf + sLen, fill, width - sLen);
    buf[width] = '\0';
    return (tollvm_str_result){ buf, width };
}

// ── Extraction ──────────────────────────────────────────────────────────────

tollvm_str_result tollvm_substring(const char* s, size_t sLen,
                                   size_t start, size_t length) {
    if (start >= sLen) {
        char* buf = (char*)malloc(1);
        buf[0] = '\0';
        return (tollvm_str_result){ buf, 0 };
    }

    if (start + length > sLen)
        length = sLen - start;

    char* buf = (char*)malloc(length + 1);
    memcpy(buf, s + start, length);
    buf[length] = '\0';
    return (tollvm_str_result){ buf, length };
}

char tollvm_charAt(const char* s, size_t sLen, size_t index) {
    if (index >= sLen) return '\0';
    return s[index];
}

tollvm_str_result tollvm_slice(const char* s, size_t sLen,
                               int64_t start, int64_t end) {
    // Resolve negative indices
    if (start < 0) start = (int64_t)sLen + start;
    if (end < 0)   end   = (int64_t)sLen + end;

    // Clamp to bounds
    if (start < 0)          start = 0;
    if (end > (int64_t)sLen) end  = (int64_t)sLen;
    if (start >= end) {
        char* buf = (char*)malloc(1);
        buf[0] = '\0';
        return (tollvm_str_result){ buf, 0 };
    }

    size_t newLen = (size_t)(end - start);
    char* buf = (char*)malloc(newLen + 1);
    memcpy(buf, s + start, newLen);
    buf[newLen] = '\0';
    return (tollvm_str_result){ buf, newLen };
}

// ── Parsing ─────────────────────────────────────────────────────────────────

int64_t tollvm_parseInt(const char* s, size_t sLen) {
    // Create null-terminated copy for strtoll
    char* tmp = (char*)malloc(sLen + 1);
    memcpy(tmp, s, sLen);
    tmp[sLen] = '\0';
    int64_t result = strtoll(tmp, NULL, 10);
    free(tmp);
    return result;
}

int64_t tollvm_parseIntRadix(const char* s, size_t sLen, uint32_t radix) {
    char* tmp = (char*)malloc(sLen + 1);
    memcpy(tmp, s, sLen);
    tmp[sLen] = '\0';
    int64_t result = strtoll(tmp, NULL, (int)radix);
    free(tmp);
    return result;
}

double tollvm_parseFloat(const char* s, size_t sLen) {
    char* tmp = (char*)malloc(sLen + 1);
    memcpy(tmp, s, sLen);
    tmp[sLen] = '\0';
    double result = strtod(tmp, NULL);
    free(tmp);
    return result;
}

// ── Conversion ──────────────────────────────────────────────────────────────

char tollvm_fromCharCode(int32_t code) {
    if (code < 0 || code > 127) return '\0';
    return (char)code;
}

// ── Vec-returning functions ─────────────────────────────────────────────────

void tollvm_split(tollvm_str_vec_header* out,
                  const char* s, size_t sLen,
                  const char* delim, size_t delimLen) {
    typedef struct { const char* ptr; size_t len; } str_elem;

    out->ptr = NULL;
    out->len = 0;
    out->cap = 0;

    size_t cap = 8;
    str_elem* arr = (str_elem*)malloc(cap * sizeof(str_elem));
    size_t count = 0;

    if (delimLen == 0) {
        // Empty delimiter: split into individual characters
        for (size_t i = 0; i < sLen; i++) {
            char* ch = (char*)malloc(2);
            ch[0] = s[i]; ch[1] = '\0';
            if (count == cap) { cap *= 2; arr = (str_elem*)realloc(arr, cap * sizeof(str_elem)); }
            arr[count].ptr = ch;
            arr[count].len = 1;
            count++;
        }
    } else {
        size_t start = 0;
        while (start <= sLen) {
            // Find next delimiter
            const char* found = NULL;
            for (size_t i = start; i + delimLen <= sLen; i++) {
                if (memcmp(s + i, delim, delimLen) == 0) {
                    found = s + i;
                    break;
                }
            }

            size_t end = found ? (size_t)(found - s) : sLen;
            size_t partLen = end - start;

            char* part = (char*)malloc(partLen + 1);
            memcpy(part, s + start, partLen);
            part[partLen] = '\0';

            if (count == cap) { cap *= 2; arr = (str_elem*)realloc(arr, cap * sizeof(str_elem)); }
            arr[count].ptr = part;
            arr[count].len = partLen;
            count++;

            if (!found) break;
            start = end + delimLen;
        }
    }

    out->ptr = arr;
    out->len = count;
    out->cap = cap;
}

void tollvm_splitN(tollvm_str_vec_header* out,
                   const char* s, size_t sLen,
                   const char* delim, size_t delimLen, size_t maxParts) {
    typedef struct { const char* ptr; size_t len; } str_elem;

    out->ptr = NULL;
    out->len = 0;
    out->cap = 0;

    if (maxParts == 0) return;

    size_t cap = maxParts < 8 ? maxParts : 8;
    str_elem* arr = (str_elem*)malloc(cap * sizeof(str_elem));
    size_t count = 0;

    size_t start = 0;
    while (start <= sLen && count < maxParts - 1) {
        const char* found = NULL;
        for (size_t i = start; i + delimLen <= sLen; i++) {
            if (memcmp(s + i, delim, delimLen) == 0) {
                found = s + i;
                break;
            }
        }
        if (!found) break;

        size_t end = (size_t)(found - s);
        size_t partLen = end - start;
        char* part = (char*)malloc(partLen + 1);
        memcpy(part, s + start, partLen);
        part[partLen] = '\0';

        if (count == cap) { cap *= 2; arr = (str_elem*)realloc(arr, cap * sizeof(str_elem)); }
        arr[count].ptr = part;
        arr[count].len = partLen;
        count++;

        start = end + delimLen;
    }

    // Remainder
    size_t remLen = sLen - start;
    char* rem = (char*)malloc(remLen + 1);
    memcpy(rem, s + start, remLen);
    rem[remLen] = '\0';

    if (count == cap) { cap *= 2; arr = (str_elem*)realloc(arr, cap * sizeof(str_elem)); }
    arr[count].ptr = rem;
    arr[count].len = remLen;
    count++;

    out->ptr = arr;
    out->len = count;
    out->cap = cap;
}

tollvm_str_result tollvm_joinVec(const tollvm_str_vec_header* vec,
                                 const char* sep, size_t sepLen) {
    typedef struct { const char* ptr; size_t len; } str_elem;
    str_elem* arr = (str_elem*)vec->ptr;
    size_t count = vec->len;

    if (count == 0) {
        char* r = (char*)malloc(1);
        r[0] = '\0';
        return (tollvm_str_result){ r, 0 };
    }

    // Calculate total length
    size_t total = 0;
    for (size_t i = 0; i < count; i++) {
        total += arr[i].len;
        if (i > 0) total += sepLen;
    }

    char* buf = (char*)malloc(total + 1);
    size_t pos = 0;
    for (size_t i = 0; i < count; i++) {
        if (i > 0 && sepLen > 0) {
            memcpy(buf + pos, sep, sepLen);
            pos += sepLen;
        }
        memcpy(buf + pos, arr[i].ptr, arr[i].len);
        pos += arr[i].len;
    }
    buf[total] = '\0';

    return (tollvm_str_result){ buf, total };
}

void tollvm_lines(tollvm_str_vec_header* out, const char* s, size_t sLen) {
    typedef struct { const char* ptr; size_t len; } str_elem;

    out->ptr = NULL;
    out->len = 0;
    out->cap = 0;

    size_t cap = 8;
    str_elem* arr = (str_elem*)malloc(cap * sizeof(str_elem));
    size_t count = 0;

    size_t start = 0;
    for (size_t i = 0; i <= sLen; i++) {
        if (i == sLen || s[i] == '\n') {
            size_t end = i;
            if (end > start && s[end - 1] == '\r') end--;

            size_t partLen = end - start;
            char* part = (char*)malloc(partLen + 1);
            memcpy(part, s + start, partLen);
            part[partLen] = '\0';

            if (count == cap) { cap *= 2; arr = (str_elem*)realloc(arr, cap * sizeof(str_elem)); }
            arr[count].ptr = part;
            arr[count].len = partLen;
            count++;

            start = i + 1;
        }
    }

    out->ptr = arr;
    out->len = count;
    out->cap = cap;
}

void tollvm_chars(tollvm_str_vec_header* out, const char* s, size_t sLen) {
    out->ptr = NULL;
    out->len = 0;
    out->cap = 0;

    uint8_t* arr = (uint8_t*)malloc(sLen);
    for (size_t i = 0; i < sLen; i++) {
        arr[i] = (uint8_t)s[i];
    }

    out->ptr = arr;
    out->len = sLen;
    out->cap = sLen;
}

tollvm_str_result tollvm_fromCharsVec(const tollvm_str_vec_header* vec) {
    uint8_t* arr = (uint8_t*)vec->ptr;
    size_t count = vec->len;

    char* buf = (char*)malloc(count + 1);
    for (size_t i = 0; i < count; i++) {
        buf[i] = (char)arr[i];
    }
    buf[count] = '\0';

    return (tollvm_str_result){ buf, count };
}

void tollvm_toBytes(tollvm_str_vec_header* out, const char* s, size_t sLen) {
    out->ptr = NULL;
    out->len = 0;
    out->cap = 0;

    uint8_t* arr = (uint8_t*)malloc(sLen);
    memcpy(arr, s, sLen);

    out->ptr = arr;
    out->len = sLen;
    out->cap = sLen;
}

tollvm_str_result tollvm_fromBytesVec(const tollvm_str_vec_header* vec) {
    uint8_t* arr = (uint8_t*)vec->ptr;
    size_t count = vec->len;

    char* buf = (char*)malloc(count + 1);
    memcpy(buf, arr, count);
    buf[count] = '\0';

    return (tollvm_str_result){ buf, count };
}

// ── C FFI String Conversion ─────────────────────────────────────────────────

char* tollvm_cstr(const char* s, size_t sLen) {
    char* buf = (char*)malloc(sLen + 1);
    if (!buf) return NULL;
    memcpy(buf, s, sLen);
    buf[sLen] = '\0';
    return buf;
}

tollvm_str_result tollvm_fromCStr(const char* cstr) {
    if (!cstr) return (tollvm_str_result){ "", 0 };
    size_t len = strlen(cstr);
    return (tollvm_str_result){ cstr, len };
}

tollvm_str_result tollvm_fromCStrLen(const char* cstr, size_t len) {
    if (!cstr) return (tollvm_str_result){ "", 0 };
    return (tollvm_str_result){ cstr, len };
}
