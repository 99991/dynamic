import json

text = "/\\\"\b\f\n\r\t$£ह€한𐍈🐈"

for c in text:
    print("char:", c)
    print("json:", json.dumps(c))
    print("utf-8:", c.encode("utf-8"))
    print()

"""
UTF-8 encoding

up to  7 bit or 0x00007f: 0xxxxxxx
up to 11 bit or 0x0007ff: 110xxxxx 10xxxxxx
up to 16 bit or 0x00ffff: 1110xxxx 10xxxxxx 10xxxxxx
up to 21 bit or 0x1fffff: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
"""

def codepoint_to_utf8(x):
    if x <= 0x7f:
        yield x
    elif x <= 0x7ff:
        yield (0x03 << 6) | ((x >> 1 * 6) & 0x1f)
        yield (0x01 << 7) | ((x >> 0 * 6) & 0x3f)
    elif x <= 0x00ffff:
        yield (0x07 << 5) | ((x >> 2 * 6) & 0x0f)
        yield (0x01 << 7) | ((x >> 1 * 6) & 0x3f)
        yield (0x01 << 7) | ((x >> 0 * 6) & 0x3f)
    elif x <= 0x1fffff:
        yield (0x0f << 4) | ((x >> 3 * 6) & 0x07)
        yield (0x01 << 7) | ((x >> 2 * 6) & 0x3f)
        yield (0x01 << 7) | ((x >> 1 * 6) & 0x3f)
        yield (0x01 << 7) | ((x >> 0 * 6) & 0x3f)
    else:
        raise ValueError(f"Codepoint {hex(x)} out of range")

def utf8_to_codepoints(values):
    i = 0
    n = len(values)
    while i < n:
        a = values[i]
        if (a >> 7) == 0:
            i += 1
            yield a
        elif (a >> 5) == (0x03 << 1):
            assert i + 2 <= n
            b = values[i + 1]
            yield ((a & 0x1f) << 6) | (b & 0x3f)
            i += 2
        elif (a >> 4) == (0x07 << 1):
            assert i + 3 <= n
            b = values[i + 1]
            c = values[i + 2]
            yield  ((a & 0x0f) << 12) | ((b & 0x3f) << 6) | (c & 0x3f)
            i += 3
        elif (a >> 3) == (0x0f << 1):
            assert i + 4 <= n
            b = values[i + 1]
            c = values[i + 2]
            d = values[i + 3]
            yield  ((a & 0x07) << 18) | ((b & 0x3f) << 12) | ((c & 0x3f) << 6) | (d & 0x3f)
            i += 4
        else:
            raise NotImplementedError()

utf8_bytes = list(text.encode("utf-8"))

codepoints = list(utf8_to_codepoints(utf8_bytes))

utf8_again = list(x for codepoint in codepoints for x in codepoint_to_utf8(codepoint))

print("utf8_bytes:", utf8_bytes)
print("utf8_again:", utf8_again)

assert utf8_bytes == utf8_again

def json_encode_codepoint(codepoint):
    mapping = {
        ord("\b"): "b",
        ord("\f"): "f",
        ord("\n"): "n",
        ord("\r"): "r",
        ord("\t"): "t",
        ord('"'): '"',
        ord('\\'): '\\',
    }
    if codepoint in mapping:
        return "\\" + mapping[codepoint]
    elif codepoint <= 0x20:
        return "\\u%04x" % codepoint
    elif codepoint <= 0x7f:
        return chr(codepoint)
    elif codepoint <= 0xffff:
        return "\\u%04x" % codepoint
    elif codepoint <= 0x10ffff:
        x = codepoint - 0x10000
        hi = (x >> 10) | 0xd800
        lo = (x & 0x3ff) | 0xdc00
        return "\\u%04x\\u%04x" % (hi, lo)
    else:
        raise ValueError("Codepoint is 0x%x, but must not be larger than 0x10ffff" % codepoint)

def json_decode(s):
    assert s[0] == '"'
    assert s[-1] == '"'
    s = s[1:-1]

    print("s:", s)

    values = s.encode("ascii")

    result = []

    hex2int = dict(zip(map(ord, "0123456789abcdefABCDEF"), [
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
        10, 11, 12, 13, 14, 15,
        10, 11, 12, 13, 14, 15,
    ]))

    i = 0
    n = len(s)
    while i < n:
        c = values[i]
        i += 1
        if c == ord("\\"):
            c = values[i]
            i += 1
            if   c == ord('b'): result.append(ord('\b'))
            elif c == ord('f'): result.append(ord('\f'))
            elif c == ord('n'): result.append(ord('\n'))
            elif c == ord('r'): result.append(ord('\r'))
            elif c == ord('t'): result.append(ord('\t'))
            elif c == ord('"'): result.append(ord('"'))
            elif c == ord('\\'): result.append(ord('\\'))
            elif c == ord('u'):

                i -= 2
                codepoint = 0

                for _ in range(2):
                    if i + 6 > n: raise ValueError("Incomplete codepoint escape sequence")

                    assert values[i] == ord('\\')
                    i += 1
                    assert values[i] == ord('u')
                    i += 1

                    for _ in range(4):
                        assert values[i] in hex2int
                        codepoint = (codepoint << 4) | hex2int[values[i]]
                        i += 1

                    if codepoint < 0xd800 or 0xdfff < codepoint: break

                if codepoint > 0xffff:
                    hi = codepoint >> 16
                    lo = codepoint & 0xffff

                    assert (hi & (0x3f << 10)) == 0xd800
                    assert (lo & (0x3f << 10)) == 0xdc00

                    codepoint = ((hi & 0x3ff) << 10) | (lo & 0x3ff)
                    codepoint += 0x10000

                result.extend(codepoint_to_utf8(codepoint))
            else:
                raise ValueError(f"Invalid escape sequence {c}")
        elif 0x20 <= c <= 0x7f:
            result.append(c)
        else:
            raise ValueError(f"Byte out of range: {c}")

    return bytes(result).decode("utf-8")

print("codepoints:", list(map(hex, codepoints)))

json_encoded = '"' + "".join(json_encode_codepoint(codepoint) for codepoint in codepoints) + '"'
print(json_encoded)
print(json.dumps(text))

assert json_encoded == json.dumps(text)

decoded = json_decode(json.dumps(text))

print("original:", repr(text))
print("decoded :", repr(decoded))
assert decoded == text
