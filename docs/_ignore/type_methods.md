# Type Methods

Methods callable via dot syntax on built-in types: `value.method(args)`

---

## int* / uint* (all integer types)

| Method | Signature | Description |
|--------|-----------|-------------|
| `abs()` | `int* → uint*` | Absolute value (signed only) |
| `clamp(min, max)` | `int* → int*` | Clamp value between min and max |
| `pow(exp)` | `(int*, uint32) → int*` | Exponentiation |
| `min(other)` | `(int*, int*) → int*` | Minimum of two values |
| `max(other)` | `(int*, int*) → int*` | Maximum of two values |
| `wrappingAdd(other)` | `(int*, int*) → int*` | Addition with wrap-around on overflow |
| `wrappingSub(other)` | `(int*, int*) → int*` | Subtraction with wrap-around on overflow |
| `wrappingMul(other)` | `(int*, int*) → int*` | Multiplication with wrap-around on overflow |
| `saturatingAdd(other)` | `(int*, int*) → int*` | Addition clamped to type min/max |
| `saturatingSub(other)` | `(int*, int*) → int*` | Subtraction clamped to type min/max |
| `leadingZeros()` | `int* → uint32` | Count of leading zero bits |
| `trailingZeros()` | `int* → uint32` | Count of trailing zero bits |
| `countOnes()` | `int* → uint32` | Population count (number of 1-bits) |
| `rotateLeft(n)` | `(int*, uint32) → int*` | Bitwise rotate left by n positions |
| `rotateRight(n)` | `(int*, uint32) → int*` | Bitwise rotate right by n positions |
| `toBigEndian()` | `int* → int*` | Convert to big-endian byte order |
| `toLittleEndian()` | `int* → int*` | Convert to little-endian byte order |
| `byteSwap()` | `int* → int*` | Reverse byte order |
| `isPowerOfTwo()` | `int* → bool` | Check if value is a power of two |
| `nextPowerOfTwo()` | `int* → int*` | Smallest power of two >= self |
| `log2()` | `int* → uint32` | Floor of log base 2 |
| `sign()` | `int* → int32` | Returns -1, 0, or 1 (signed only) |
| `toString()` | `int* → string` | Convert to decimal string |
| `toStringRadix(base)` | `(int*, uint32) → string` | Convert to string in given base (2-36) |
| `toFloat()` | `int* → float64` | Convert to float64 |
| `toChar()` | `int* → char` | Convert low byte to char |

---

## float32 / float64 / double

| Method | Signature | Description |
|--------|-----------|-------------|
| `abs()` | `float* → float*` | Absolute value |
| `ceil()` | `float* → float*` | Round up to nearest integer |
| `floor()` | `float* → float*` | Round down to nearest integer |
| `round()` | `float* → float*` | Round to nearest integer |
| `trunc()` | `float* → float*` | Truncate decimal part |
| `fract()` | `float* → float*` | Fractional part (x - floor(x)) |
| `sqrt()` | `float* → float*` | Square root |
| `cbrt()` | `float* → float*` | Cube root |
| `pow(exp)` | `(float*, float*) → float*` | Exponentiation |
| `exp()` | `float* → float*` | e^x |
| `exp2()` | `float* → float*` | 2^x |
| `ln()` | `float* → float*` | Natural logarithm |
| `log2()` | `float* → float*` | Logarithm base 2 |
| `log10()` | `float* → float*` | Logarithm base 10 |
| `sin()` | `float* → float*` | Sine (radians) |
| `cos()` | `float* → float*` | Cosine (radians) |
| `tan()` | `float* → float*` | Tangent (radians) |
| `asin()` | `float* → float*` | Arc sine |
| `acos()` | `float* → float*` | Arc cosine |
| `atan()` | `float* → float*` | Arc tangent |
| `atan2(x)` | `(float*, float*) → float*` | Two-argument arc tangent |
| `sinh()` | `float* → float*` | Hyperbolic sine |
| `cosh()` | `float* → float*` | Hyperbolic cosine |
| `tanh()` | `float* → float*` | Hyperbolic tangent |
| `hypot(other)` | `(float*, float*) → float*` | Hypotenuse: sqrt(x² + y²) |
| `min(other)` | `(float*, float*) → float*` | Minimum of two values |
| `max(other)` | `(float*, float*) → float*` | Maximum of two values |
| `clamp(min, max)` | `float* → float*` | Clamp value between min and max |
| `lerp(target, t)` | `(float*, float*, float*) → float*` | Linear interpolation |
| `sign()` | `float* → float*` | Returns -1.0, 0.0, or 1.0 |
| `copySign(other)` | `(float*, float*) → float*` | Copy sign from other |
| `isNaN()` | `float* → bool` | Check if NaN |
| `isInf()` | `float* → bool` | Check if infinite |
| `isFinite()` | `float* → bool` | Check if not NaN and not infinite |
| `isNormal()` | `float* → bool` | Check if not zero, subnormal, infinite, or NaN |
| `isNegative()` | `float* → bool` | Check if negative (including -0.0) |
| `toRadians()` | `float* → float*` | Convert degrees to radians |
| `toDegrees()` | `float* → float*` | Convert radians to degrees |
| `toString()` | `float* → string` | Convert to string representation |
| `toStringPrecision(n)` | `(float*, uint32) → string` | Convert with n decimal places |
| `toInt()` | `float* → int64` | Truncate to integer |
| `toBits()` | `float* → uint64` | Raw IEEE 754 bit pattern |

---

## bool

| Method | Signature | Description |
|--------|-----------|-------------|
| `toString()` | `bool → string` | Returns `"true"` or `"false"` |
| `toInt()` | `bool → int32` | Returns `0` or `1` |
| `toggle()` | `bool → bool` | Returns the opposite value |

---

## char

| Method | Signature | Description |
|--------|-----------|-------------|
| `isAlpha()` | `char → bool` | Is ASCII letter (a-z, A-Z) |
| `isDigit()` | `char → bool` | Is ASCII digit (0-9) |
| `isHexDigit()` | `char → bool` | Is hex digit (0-9, a-f, A-F) |
| `isAlphaNum()` | `char → bool` | Is letter or digit |
| `isUpper()` | `char → bool` | Is uppercase letter |
| `isLower()` | `char → bool` | Is lowercase letter |
| `isSpace()` | `char → bool` | Is whitespace (space, tab, newline, etc.) |
| `isPrintable()` | `char → bool` | Is printable ASCII (0x20-0x7E) |
| `isControl()` | `char → bool` | Is control character (0x00-0x1F, 0x7F) |
| `isPunct()` | `char → bool` | Is punctuation character |
| `isAscii()` | `char → bool` | Is valid ASCII (0x00-0x7F) |
| `toUpper()` | `char → char` | Convert to uppercase |
| `toLower()` | `char → char` | Convert to lowercase |
| `toInt()` | `char → int32` | ASCII code as integer |
| `toString()` | `char → string` | Single-character string |
| `repeat(n)` | `(char, usize) → string` | Repeat char n times into a string |
| `digitToInt()` | `char → int32` | '0'-'9' → 0-9, -1 if not a digit |

---

## string

| Method | Signature | Description |
|--------|-----------|-------------|
| `len()` | `string → usize` | String length in bytes |
| `isEmpty()` | `string → bool` | Check if length is 0 |
| `at(index)` | `(string, usize) → char` | Get char at index |
| `front()` | `string → char` | First character |
| `back()` | `string → char` | Last character |
| `contains(substr)` | `(string, string) → bool` | Check if contains substring |
| `startsWith(prefix)` | `(string, string) → bool` | Check if starts with prefix |
| `endsWith(suffix)` | `(string, string) → bool` | Check if ends with suffix |
| `indexOf(substr)` | `(string, string) → int64` | First index of substring, -1 if not found |
| `lastIndexOf(substr)` | `(string, string) → int64` | Last index of substring, -1 if not found |
| `count(substr)` | `(string, string) → usize` | Count non-overlapping occurrences |
| `substring(start, len)` | `(string, usize, usize) → string` | Extract substring |
| `slice(start, end)` | `(string, int64, int64) → string` | Slice with start/end indices (negative = from end) |
| `trim()` | `string → string` | Remove leading/trailing whitespace |
| `trimLeft()` | `string → string` | Remove leading whitespace |
| `trimRight()` | `string → string` | Remove trailing whitespace |
| `trimChar(c)` | `(string, char) → string` | Remove leading/trailing occurrences of char |
| `padLeft(width, fill)` | `(string, usize, char) → string` | Pad left to width with fill char |
| `padRight(width, fill)` | `(string, usize, char) → string` | Pad right to width with fill char |
| `toUpper()` | `string → string` | Convert all chars to uppercase |
| `toLower()` | `string → string` | Convert all chars to lowercase |
| `capitalize()` | `string → string` | Uppercase first char, lowercase rest |
| `replace(old, new)` | `(string, string, string) → string` | Replace all occurrences |
| `replaceFirst(old, new)` | `(string, string, string) → string` | Replace first occurrence only |
| `removePrefix(prefix)` | `(string, string) → string` | Remove prefix if present |
| `removeSuffix(suffix)` | `(string, string) → string` | Remove suffix if present |
| `split(delim)` | `(string, string) → []string` | Split by delimiter |
| `join(arr)` | `(string, []string) → string` | Join array of strings with self as separator |
| `repeat(n)` | `(string, usize) → string` | Repeat n times |
| `reverse()` | `string → string` | Reverse the string |
| `insert(index, substr)` | `(string, usize, string) → string` | Insert substring at index |
| `remove(start, len)` | `(string, usize, usize) → string` | Remove len chars starting at index |
| `chars()` | `string → []char` | Get array of characters |
| `bytes()` | `string → []uint8` | Get array of raw bytes |
| `lines()` | `string → []string` | Split by newline |
| `words()` | `string → []string` | Split by whitespace |
| `concat(other)` | `(string, string) → string` | Concatenate two strings |
| `compareTo(other)` | `(string, string) → int32` | Lexicographic compare: -1, 0, or 1 |
| `equalsIgnoreCase(other)` | `(string, string) → bool` | Case-insensitive equality |
| `isNumeric()` | `string → bool` | All chars are digits |
| `isAlpha()` | `string → bool` | All chars are letters |
| `isAlphaNum()` | `string → bool` | All chars are letters or digits |
| `isUpper()` | `string → bool` | All letter chars are uppercase |
| `isLower()` | `string → bool` | All letter chars are lowercase |
| `isBlank()` | `string → bool` | Only whitespace or empty |
| `toInt()` | `string → int64` | Parse as integer |
| `toFloat()` | `string → float64` | Parse as float |
| `toBool()` | `string → bool` | "true" → true, "false" → false |
| `hash()` | `string → uint64` | Hash value |

---

## []T (arrays)

| Method | Signature | Description |
|--------|-----------|-------------|
| `len()` | `[]T → usize` | Number of elements |
| `isEmpty()` | `[]T → bool` | Check if length is 0 |
| `at(index)` | `([]T, usize) → T` | Get element at index |
| `first()` | `[]T → T` | Get first element |
| `last()` | `[]T → T` | Get last element |
| `contains(elem)` | `([]T, T) → bool` | Check if array contains element |
| `indexOf(elem)` | `([]T, T) → int64` | First index of element, -1 if not found |
| `lastIndexOf(elem)` | `([]T, T) → int64` | Last index of element, -1 if not found |
| `count(elem)` | `([]T, T) → usize` | Count occurrences of element |
| `fill(value)` | `([]T, T) → void` | Fill all elements with value |
| `swap(i, j)` | `([]T, usize, usize) → void` | Swap two elements in place |
| `reverse()` | `[]T → void` | Reverse in place |
| `copy()` | `[]T → []T` | Shallow copy of the array |
| `slice(start, end)` | `([]T, usize, usize) → []T` | Sub-array from start to end (exclusive) |
| `sum()` | `[]T → T` | Sum all elements (numeric T only) |
| `product()` | `[]T → T` | Product of all elements (numeric T only) |
| `min()` | `[]T → T` | Minimum element (numeric/comparable T) |
| `max()` | `[]T → T` | Maximum element (numeric/comparable T) |
| `minIndex()` | `[]T → usize` | Index of minimum element |
| `maxIndex()` | `[]T → usize` | Index of maximum element |
| `average()` | `[]T → float64` | Arithmetic mean (numeric T only) |
| `isSorted()` | `[]T → bool` | Check if sorted ascending (comparable T) |
| `sort()` | `[]T → void` | Sort in place ascending (comparable T) |
| `sortDesc()` | `[]T → void` | Sort in place descending |
| `equals(other)` | `([]T, []T) → bool` | Element-wise equality check |
| `toString()` | `[]T → string` | String representation: `[1, 2, 3]` |
| `join(sep)` | `([]T, string) → string` | Join elements as string with separator |
| `rotate(n)` | `([]T, int32) → void` | Rotate elements by n positions |

---


