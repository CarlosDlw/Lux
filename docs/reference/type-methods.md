# Type Methods

All methods available on each built-in type. Methods are called with dot syntax: `value.method()`.

---

## Integer Methods

Available on all signed (`int8`, `int16`, `int32`, `int64`, `int128`, `isize`) and unsigned (`uint8`, `uint16`, `uint32`, `uint64`, `uint128`, `usize`) integer types.

### Arithmetic

| Method | Signature | Description |
|--------|-----------|-------------|
| `abs()` | `→ T` | Absolute value |
| `sign()` | `→ T` | Returns -1, 0, or 1 |
| `clamp(min, max)` | `(T, T) → T` | Clamp to range |
| `min(other)` | `(T) → T` | Minimum of two values |
| `max(other)` | `(T) → T` | Maximum of two values |
| `pow(exp)` | `(uint32) → T` | Raise to power |

### Wrapping Arithmetic

| Method | Signature | Description |
|--------|-----------|-------------|
| `wrappingAdd(other)` | `(T) → T` | Addition with wrapping on overflow |
| `wrappingSub(other)` | `(T) → T` | Subtraction with wrapping |
| `wrappingMul(other)` | `(T) → T` | Multiplication with wrapping |
| `saturatingAdd(other)` | `(T) → T` | Addition clamped to max/min |
| `saturatingSub(other)` | `(T) → T` | Subtraction clamped to max/min |

### Bit Operations

| Method | Signature | Description |
|--------|-----------|-------------|
| `leadingZeros()` | `→ uint32` | Count leading zero bits |
| `trailingZeros()` | `→ uint32` | Count trailing zero bits |
| `countOnes()` | `→ uint32` | Count set bits (popcount) |
| `rotateLeft(n)` | `(uint32) → T` | Bit rotate left |
| `rotateRight(n)` | `(uint32) → T` | Bit rotate right |
| `toBigEndian()` | `→ T` | Convert to big endian |
| `toLittleEndian()` | `→ T` | Convert to little endian |
| `byteSwap()` | `→ T` | Swap byte order |
| `isPowerOfTwo()` | `→ bool` | Check if power of two |
| `nextPowerOfTwo()` | `→ T` | Next power of two |
| `log2()` | `→ uint32` | Floor of log base 2 |

### Conversion

| Method | Signature | Description |
|--------|-----------|-------------|
| `toString()` | `→ string` | Convert to decimal string |
| `toStringRadix(base)` | `(uint32) → string` | Convert to string in given base |
| `toFloat()` | `→ float64` | Convert to float |
| `toChar()` | `→ char` | Convert to character |

---

## Float Methods

Available on `float32`, `float64`, `float80`, `float128`, and `double`.

### Math

| Method | Signature | Description |
|--------|-----------|-------------|
| `abs()` | `→ T` | Absolute value |
| `ceil()` | `→ T` | Round up |
| `floor()` | `→ T` | Round down |
| `round()` | `→ T` | Round to nearest |
| `trunc()` | `→ T` | Truncate toward zero |
| `fract()` | `→ T` | Fractional part |
| `sqrt()` | `→ T` | Square root |
| `cbrt()` | `→ T` | Cube root |
| `pow(exp)` | `(T) → T` | Raise to power |
| `exp()` | `→ T` | e^x |
| `exp2()` | `→ T` | 2^x |
| `ln()` | `→ T` | Natural logarithm |
| `log2()` | `→ T` | Log base 2 |
| `log10()` | `→ T` | Log base 10 |

### Trigonometry

| Method | Signature | Description |
|--------|-----------|-------------|
| `sin()` | `→ T` | Sine |
| `cos()` | `→ T` | Cosine |
| `tan()` | `→ T` | Tangent |
| `asin()` | `→ T` | Arc sine |
| `acos()` | `→ T` | Arc cosine |
| `atan()` | `→ T` | Arc tangent |
| `atan2(x)` | `(T) → T` | Two-argument arc tangent |
| `sinh()` | `→ T` | Hyperbolic sine |
| `cosh()` | `→ T` | Hyperbolic cosine |
| `tanh()` | `→ T` | Hyperbolic tangent |
| `hypot(other)` | `(T) → T` | Hypotenuse |

### Utility

| Method | Signature | Description |
|--------|-----------|-------------|
| `min(other)` | `(T) → T` | Minimum |
| `max(other)` | `(T) → T` | Maximum |
| `clamp(min, max)` | `(T, T) → T` | Clamp to range |
| `lerp(other, t)` | `(T, T) → T` | Linear interpolation |
| `sign()` | `→ T` | Sign (-1, 0, or 1) |
| `copySign(other)` | `(T) → T` | Copy sign from other |

### Checks

| Method | Signature | Description |
|--------|-----------|-------------|
| `isNaN()` | `→ bool` | Is not a number |
| `isInf()` | `→ bool` | Is infinite |
| `isFinite()` | `→ bool` | Is finite |
| `isNormal()` | `→ bool` | Is normal (not zero, subnormal, inf, or NaN) |
| `isNegative()` | `→ bool` | Is negative |

### Conversion

| Method | Signature | Description |
|--------|-----------|-------------|
| `toRadians()` | `→ T` | Degrees to radians |
| `toDegrees()` | `→ T` | Radians to degrees |
| `toString()` | `→ string` | Convert to string |
| `toStringPrecision(n)` | `(uint32) → string` | Convert with N decimal places |
| `toInt()` | `→ int64` | Truncate to integer |
| `toBits()` | `→ uint64` | Bit representation |

---

## Bool Methods

| Method | Signature | Description |
|--------|-----------|-------------|
| `toString()` | `→ string` | `"true"` or `"false"` |
| `toInt()` | `→ int32` | `1` or `0` |
| `toggle()` | `→ bool` | Returns opposite value |

---

## Char Methods

### Classification

| Method | Signature | Description |
|--------|-----------|-------------|
| `isAlpha()` | `→ bool` | Is alphabetic |
| `isDigit()` | `→ bool` | Is digit (0-9) |
| `isHexDigit()` | `→ bool` | Is hex digit |
| `isAlphaNum()` | `→ bool` | Is alphanumeric |
| `isUpper()` | `→ bool` | Is uppercase |
| `isLower()` | `→ bool` | Is lowercase |
| `isSpace()` | `→ bool` | Is whitespace |
| `isPrintable()` | `→ bool` | Is printable |
| `isControl()` | `→ bool` | Is control character |
| `isPunct()` | `→ bool` | Is punctuation |
| `isAscii()` | `→ bool` | Is ASCII (0-127) |

### Conversion

| Method | Signature | Description |
|--------|-----------|-------------|
| `toUpper()` | `→ char` | To uppercase |
| `toLower()` | `→ char` | To lowercase |
| `toInt()` | `→ int32` | ASCII code |
| `toString()` | `→ string` | Single-character string |
| `repeat(n)` | `(usize) → string` | Repeat character N times |
| `digitToInt()` | `→ int32` | Digit value (0-9) |

---

## String Methods

### Access

| Method | Signature | Description |
|--------|-----------|-------------|
| `len()` | `→ usize` | Length in bytes |
| `isEmpty()` | `→ bool` | True if length is 0 |
| `at(index)` | `(usize) → char` | Character at index |
| `front()` | `→ char` | First character |
| `back()` | `→ char` | Last character |

### Search

| Method | Signature | Description |
|--------|-----------|-------------|
| `contains(sub)` | `(string) → bool` | Contains substring |
| `startsWith(prefix)` | `(string) → bool` | Starts with prefix |
| `endsWith(suffix)` | `(string) → bool` | Ends with suffix |
| `indexOf(sub)` | `(string) → int64` | First occurrence (-1 if not found) |
| `lastIndexOf(sub)` | `(string) → int64` | Last occurrence (-1 if not found) |
| `count(sub)` | `(string) → usize` | Count occurrences |

### Transform

| Method | Signature | Description |
|--------|-----------|-------------|
| `substring(start, len)` | `(usize, usize) → string` | Extract substring |
| `slice(start, end)` | `(int64, int64) → string` | Slice (supports negative indices) |
| `trim()` | `→ string` | Remove leading/trailing whitespace |
| `trimLeft()` | `→ string` | Remove leading whitespace |
| `trimRight()` | `→ string` | Remove trailing whitespace |
| `trimChar(c)` | `(char) → string` | Trim specific character |
| `toUpper()` | `→ string` | To uppercase |
| `toLower()` | `→ string` | To lowercase |
| `capitalize()` | `→ string` | Capitalize first character |
| `reverse()` | `→ string` | Reverse string |
| `repeat(n)` | `(usize) → string` | Repeat N times |
| `concat(other)` | `(string) → string` | Concatenate two strings |

### Padding

| Method | Signature | Description |
|--------|-----------|-------------|
| `padLeft(width, fill)` | `(usize, char) → string` | Pad on the left |
| `padRight(width, fill)` | `(usize, char) → string` | Pad on the right |

### Replace and Remove

| Method | Signature | Description |
|--------|-----------|-------------|
| `replace(old, new)` | `(string, string) → string` | Replace all occurrences |
| `replaceFirst(old, new)` | `(string, string) → string` | Replace first occurrence |
| `removePrefix(prefix)` | `(string) → string` | Remove prefix if present |
| `removeSuffix(suffix)` | `(string) → string` | Remove suffix if present |
| `remove(start, len)` | `(usize, usize) → string` | Remove substring |
| `insert(pos, sub)` | `(usize, string) → string` | Insert at position |

### Split and Decompose

| Method | Signature | Description |
|--------|-----------|-------------|
| `split(delim)` | `(string) → vec<string>` | Split by delimiter |
| `join(sep)` | `(string) → string` | Join (only on vec\<string\>) |
| `chars()` | `→ vec<char>` | Decompose into characters |
| `bytes()` | `→ vec<uint8>` | Decompose into bytes |
| `lines()` | `→ vec<string>` | Split by newlines |
| `words()` | `→ vec<string>` | Split by whitespace |

### Comparison

| Method | Signature | Description |
|--------|-----------|-------------|
| `compareTo(other)` | `(string) → int32` | Lexicographic comparison |
| `equalsIgnoreCase(other)` | `(string) → bool` | Case-insensitive equality |

### Classification

| Method | Signature | Description |
|--------|-----------|-------------|
| `isNumeric()` | `→ bool` | All characters are digits |
| `isAlpha()` | `→ bool` | All characters are letters |
| `isAlphaNum()` | `→ bool` | All characters are alphanumeric |
| `isUpper()` | `→ bool` | All letters are uppercase |
| `isLower()` | `→ bool` | All letters are lowercase |
| `isBlank()` | `→ bool` | Empty or only whitespace |

### Conversion

| Method | Signature | Description |
|--------|-----------|-------------|
| `toInt()` | `→ int64` | Parse as integer |
| `toFloat()` | `→ float64` | Parse as float |
| `toBool()` | `→ bool` | `"true"` → true, else false |
| `hash()` | `→ uint64` | Hash value |

---

## Array Methods

Available on fixed-size arrays (`[N]T`) and dynamic arrays (`[]T`).

### Access

| Method | Signature | Description |
|--------|-----------|-------------|
| `len()` | `→ usize` | Number of elements |
| `isEmpty()` | `→ bool` | True if length is 0 |
| `at(index)` | `(usize) → T` | Element at index |
| `first()` | `→ T` | First element |
| `last()` | `→ T` | Last element |

### Search

| Method | Signature | Description |
|--------|-----------|-------------|
| `contains(val)` | `(T) → bool` | Contains element |
| `indexOf(val)` | `(T) → int64` | First index (-1 if not found) |
| `lastIndexOf(val)` | `(T) → int64` | Last index (-1 if not found) |
| `count(val)` | `(T) → usize` | Count occurrences |

### Modify

| Method | Signature | Description |
|--------|-----------|-------------|
| `fill(val)` | `(T) → void` | Fill all elements |
| `swap(i, j)` | `(usize, usize) → void` | Swap two elements |
| `reverse()` | `→ void` | Reverse in place |
| `slice(start, len)` | `(usize, usize) → []T` | Extract slice |

### Aggregation (numeric arrays only)

| Method | Signature | Description |
|--------|-----------|-------------|
| `sum()` | `→ T` | Sum of elements |
| `product()` | `→ T` | Product of elements |
| `min()` | `→ T` | Minimum element |
| `max()` | `→ T` | Maximum element |
| `minIndex()` | `→ usize` | Index of minimum |
| `maxIndex()` | `→ usize` | Index of maximum |
| `average()` | `→ float64` | Average value |

### Sorting (numeric arrays only)

| Method | Signature | Description |
|--------|-----------|-------------|
| `sort()` | `→ void` | Sort ascending |
| `sortDesc()` | `→ void` | Sort descending |
| `isSorted()` | `→ bool` | Is sorted ascending |

### Other

| Method | Signature | Description |
|--------|-----------|-------------|
| `copy()` | `→ []T` | Shallow copy |
| `rotate(n)` | `(int32) → void` | Rotate elements |
| `equals(other)` | `([]T) → bool` | Element-wise equality |
| `toString()` | `→ string` | String representation |
| `join(sep)` | `(string) → string` | Join with separator |

---

## vec\<T\> Methods

### Capacity

| Method | Signature | Description |
|--------|-----------|-------------|
| `len()` | `→ usize` | Number of elements |
| `capacity()` | `→ usize` | Allocated capacity |
| `isEmpty()` | `→ bool` | True if length is 0 |
| `reserve(n)` | `(usize) → void` | Ensure capacity for N elements |
| `shrink()` | `→ void` | Shrink capacity to length |
| `resize(n, val)` | `(usize, T) → void` | Resize with default value |
| `truncate(n)` | `(usize) → void` | Truncate to N elements |

### Element Operations

| Method | Signature | Description |
|--------|-----------|-------------|
| `at(index)` | `(usize) → T` | Element at index |
| `first()` | `→ T` | First element |
| `last()` | `→ T` | Last element |
| `push(val)` | `(T) → void` | Append element |
| `pop()` | `→ T` | Remove and return last |
| `insert(index, val)` | `(usize, T) → void` | Insert at position |
| `removeAt(index)` | `(usize) → T` | Remove at position (shift) |
| `removeSwap(index)` | `(usize) → T` | Remove by swapping with last |
| `clear()` | `→ void` | Remove all elements |
| `fill(val)` | `(T) → void` | Fill all elements |
| `swap(i, j)` | `(usize, usize) → void` | Swap two elements |

### Search

| Method | Signature | Description |
|--------|-----------|-------------|
| `contains(val)` | `(T) → bool` | Contains element |
| `indexOf(val)` | `(T) → int64` | First index (-1 if not found) |
| `lastIndexOf(val)` | `(T) → int64` | Last index (-1 if not found) |
| `count(val)` | `(T) → usize` | Count occurrences |

### Reorder

| Method | Signature | Description |
|--------|-----------|-------------|
| `reverse()` | `→ void` | Reverse in place |
| `sort()` | `→ void` | Sort ascending (numeric only) |
| `sortDesc()` | `→ void` | Sort descending (numeric only) |
| `rotate(n)` | `(int32) → void` | Rotate elements |

### Aggregation (numeric only)

| Method | Signature | Description |
|--------|-----------|-------------|
| `sum()` | `→ T` | Sum of elements |
| `product()` | `→ T` | Product of elements |
| `min()` | `→ T` | Minimum element |
| `max()` | `→ T` | Maximum element |
| `average()` | `→ float64` | Average value |
| `isSorted()` | `→ bool` | Is sorted ascending |

### Other

| Method | Signature | Description |
|--------|-----------|-------------|
| `equals(other)` | `(vec<T>) → bool` | Element-wise equality |
| `toString()` | `→ string` | String representation |
| `join(sep)` | `(string) → string` | Join with separator |
| `clone()` | `→ vec<T>` | Deep copy |
| `free()` | `→ void` | Release heap memory |

---

## map\<K, V\> Methods

| Method | Signature | Description |
|--------|-----------|-------------|
| `len()` | `→ usize` | Number of entries |
| `isEmpty()` | `→ bool` | True if empty |
| `get(key)` | `(K) → V` | Get value by key |
| `getOrDefault(key, def)` | `(K, V) → V` | Get value or default |
| `has(key)` | `(K) → bool` | Check if key exists |
| `insert(key, val)` | `(K, V) → void` | Insert or update entry |
| `remove(key)` | `(K) → void` | Remove entry |
| `clear()` | `→ void` | Remove all entries |
| `keys()` | `→ vec<K>` | All keys as vector |
| `values()` | `→ vec<V>` | All values as vector |
| `free()` | `→ void` | Release heap memory |

---

## set\<T\> Methods

| Method | Signature | Description |
|--------|-----------|-------------|
| `len()` | `→ usize` | Number of elements |
| `isEmpty()` | `→ bool` | True if empty |
| `add(val)` | `(T) → void` | Add element |
| `has(val)` | `(T) → bool` | Check membership |
| `remove(val)` | `(T) → void` | Remove element |
| `clear()` | `→ void` | Remove all elements |
| `values()` | `→ vec<T>` | All elements as vector |
| `free()` | `→ void` | Release heap memory |
