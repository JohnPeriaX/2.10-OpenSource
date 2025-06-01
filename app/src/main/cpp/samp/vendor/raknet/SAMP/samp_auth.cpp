#include <cstdint>
#include <cstring>
#include <cstdio>
#include <cstdlib>

// ฟังก์ชันสลับ endian
#define endian_swap8(x) (x)
#define endian_swap16(x) ((x>>8) | (x<<8))
#define endian_swap32(x) ((x>>24) | ((x<<8) & 0x00FF0000) | ((x>>8) & 0x0000FF00) | (x<<24))
#define endian_swap64(x) ((x>>56) | ((x<<40) & 0x00FF000000000000) | \
        ((x<<24) & 0x0000FF0000000000) | ((x<<8)  & 0x000000FF00000000) | \
        ((x>>8)  & 0x00000000FF000000) | ((x>>24) & 0x0000000000FF0000) | \
        ((x>>40) & 0x000000000000FF00) | (x<<56))

// ฟังก์ชัน rotate
#define ROTL(value, shift) ((value << shift) | (value >> (sizeof(value)*8 - shift)))
#define ROTR(value, shift) ((value >> shift) | (value << (sizeof(value)*8 - shift)))
#define swap(x,y,T) {T tmp = x; x = y; y = tmp;}

// กำหนดประเภทข้อมูลตามสถาปัตยกรรม
#ifdef _WIN32
#ifdef _WIN64
    // 64-bit
    typedef uint64_t arch_ulong;
    typedef uint64_t arch_ulonglong;
  #else
    // 32-bit
    typedef uint32_t arch_ulong;
    typedef uint64_t arch_ulonglong;
  #endif
#else
// สำหรับระบบอื่นๆ (Linux, Mac)
#ifdef __x86_64__
// 64-bit
    typedef uint64_t arch_ulong;
    typedef uint64_t arch_ulonglong;
#else
// 32-bit
typedef uint32_t arch_ulong;
typedef uint64_t arch_ulonglong;
#endif
#endif

// ฟังก์ชันจัดสรรหน่วยความจำแบบ aligned
void *aligned_alloc_fallback(size_t alignment, size_t size) {
    void *ptr = NULL;
#ifdef _WIN32
    ptr = _aligned_malloc(size, alignment);
#else
    if (posix_memalign(&ptr, alignment, size) != 0) {
        return NULL;
    }
#endif
    return ptr;
}

// ฟังก์ชันปลดปล่อยหน่วยความจำแบบ aligned
void aligned_free_fallback(void *ptr) {
#ifdef _WIN32
    _aligned_free(ptr);
#else
    free(ptr);
#endif
}

// ฟังก์ชัน SHA1
void SHA1(const char *message, arch_ulong *out) {
    arch_ulong h0 = 0x67452301;
    arch_ulong h1 = 0xEFCDAB89;
    arch_ulong h2 = 0x98BADCFE;
    arch_ulong h3 = 0x10325476;
    arch_ulong h4 = 0xC3D2E1F0;

    arch_ulong len = 0;
    arch_ulonglong bitlen = 0;

    // คำนวณความยาวข้อความ
    while (message[len]) {
        len++;
        bitlen += 8;
    }

    arch_ulong complement = (55 - (len % 64)) + 8 * (((len + 8) / 64));
    arch_ulong newlen = len + complement + 8 + 1;

    // จัดสรรหน่วยความจำ
    char *pMessage = (char *)aligned_alloc_fallback(8, newlen);
    if (!pMessage)
        return;

    // เตรียมข้อความสำหรับการแฮช
    memcpy(pMessage, message, len);
    pMessage[len] = -128; // บิต '1' แรก
    memset(pMessage + len + 1, 0, complement);

    // เขียนความยาวบิตแบบ big-endian
    arch_ulonglong bitlen_be = endian_swap64(bitlen);
    memcpy(pMessage + len + 1 + complement, &bitlen_be, sizeof(bitlen_be));

    arch_ulong chunks = newlen / 64;
    arch_ulong w[80];

    // ประมวลผลแต่ละ chunk
    for (arch_ulong x = 0; x < chunks; x++) {
        // เตรียมคำสั่ง
        for (arch_ulong i = 0; i < 16; i++) {
            memcpy(&w[i], pMessage + x * 64 + i * 4, 4);
            w[i] = endian_swap32(w[i]);
        }

        // ขยายคำสั่ง
        for (arch_ulong i = 16; i < 80; i++) {
            w[i] = ROTL((w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16]), 1);
        }

        // เริ่มต้นค่าแฮช
        arch_ulong a = h0;
        arch_ulong b = h1;
        arch_ulong c = h2;
        arch_ulong d = h3;
        arch_ulong e = h4;

        // วนลูปหลัก
        for (arch_ulong i = 0; i < 80; i++) {
            arch_ulong f, k;
            if (i < 20) {
                f = (b & c) | ((~b) & d);
                k = 0x5A827999;
            } else if (i < 40) {
                f = b ^ c ^ d;
                k = 0x6ED9EBA1;
            } else if (i < 60) {
                f = (b & c) | (b & d) | (c & d);
                k = 0x8F1BBCDC;
            } else {
                f = b ^ c ^ d;
                k = 0xCA62C1D6;
            }

            arch_ulong temp = (ROTL(a, 5) + f + e + k + w[i]) & 0xFFFFFFFF;
            e = d;
            d = c;
            c = ROTL(b, 30);
            b = a;
            a = temp;
        }

        // ปรับค่าแฮช
        h0 = (h0 + a) & 0xFFFFFFFF;
        h1 = (h1 + b) & 0xFFFFFFFF;
        h2 = (h2 + c) & 0xFFFFFFFF;
        h3 = (h3 + d) & 0xFFFFFFFF;
        h4 = (h4 + e) & 0xFFFFFFFF;
    }

    // คืนหน่วยความจำ
    aligned_free_fallback(pMessage);

    // ส่งค่ากลับ
    out[0] = h0;
    out[1] = h1;
    out[2] = h2;
    out[3] = h3;
    out[4] = h4;
}

// เวอร์ชันสตริงของ SHA1
void SHA1(const char *message, char buf[64]) {
    if (!buf) return;
    arch_ulong out[5];
    SHA1(message, out);
    sprintf(buf, "%.8X%.8X%.8X%.8X%.8X", out[0], out[1], out[2], out[3], out[4]);
}

// ตารางแปลงค่า authentication
const static uint8_t auth_hash_transform_table[100] = {
        0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A, 0x00, 0x00, 0x00, 0x0D,
        0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x80,
        0x08, 0x06, 0x00, 0x00, 0x00, 0xE4, 0xB5, 0xB7, 0x0A, 0x00, 0x00, 0x00,
        0x09, 0x70, 0x48, 0x59, 0x73, 0x00, 0x00, 0x0B, 0x13, 0x00, 0x00, 0x0B,
        0x13, 0x01, 0x00, 0x9A, 0x9C, 0x18, 0x00, 0x00, 0x00, 0x04, 0x67, 0x41,
        0x4D, 0x41, 0x00, 0x00, 0xB1, 0x8E, 0x7C, 0xFB, 0x51, 0x93, 0x00, 0x00,
        0x00, 0x20, 0x63, 0x48, 0x52, 0x4D, 0x00, 0x00, 0x7A, 0x25, 0x00, 0x00,
        0x80, 0x83, 0x00, 0x00, 0xF9, 0xFF, 0x00, 0x00, 0x80, 0xE9, 0x00, 0x00,
        0x75, 0x30, 0x00, 0x00
};

// ฟังก์ชันแปลงค่า authentication
uint8_t transform_auth_sha1(uint8_t value, uint8_t _xor)
{
    uint8_t result = value;

    for(uint8_t i = 0; i < 100; i++)
    {
        result = result ^ auth_hash_transform_table[i] ^ _xor;
    }

    return result;
}

// โค้ดจาก CAnimManager::AddAnimation
const static uint8_t code_from_CAnimManager_AddAnimation[20] = {
        0xFF, 0x25, 0x34, 0x39, // gta_sa.exe + 0x4D3AA0
        0x4D, 0x00, 0x90, 0x90, // gta_sa.exe + 0x4D3AA4
        0x90, 0x90, 0x56, 0x57, // gta_sa.exe + 0x4D3AAC
        0x50, 0x8B, 0x44, 0x24, // gta_sa.exe + 0x4D3AA8
        0x14, 0x8D, 0x0C, 0x80  // gta_sa.exe + 0x4D3AB0
};

// ฟังก์ชันแปลงค่าเป็นตัวอักษร
char samp_sub_100517E0(uint8_t a1)
{
    char result = a1 + '0';

    if(a1 + '0' > '9')
    {
        result = a1 + '7';
    }

    return result;
}

// ฟังก์ชันสร้างสตริง authentication
void auth_stringify(char *out, uint8_t* hash)
{
    uint8_t i = 0;
    uint8_t* j = hash;

    do
    {
        out[i] = samp_sub_100517E0(*j >> 4); i++;
        out[i] = samp_sub_100517E0(*j & 0xF); i++;

        j++;
    }
    while(i < 40);

    out[i] = '\0';
}

// ฟังก์ชันสร้างคีย์ authentication
void gen_auth_key(char* buf, char* auth_in)
{
    if (!auth_in || !buf) return;

    char message[260];
    sprintf(message, "%s", auth_in);

    arch_ulong out[5];
    uint8_t *pb_out = reinterpret_cast<uint8_t *>(&out);

    SHA1(message, out);

    // แปลงค่าแฮช
    for (uint8_t i = 0; i < 5; i++) { pb_out[i] = transform_auth_sha1(pb_out[i], 0x2F); }
    for (uint8_t i = 5; i < 10; i++) { pb_out[i] = transform_auth_sha1(pb_out[i], 0x45); }
    for (uint8_t i = 10; i < 15; i++) { pb_out[i] = transform_auth_sha1(pb_out[i], 0x6F); }
    for (uint8_t i = 15; i < 20; i++) { pb_out[i] = transform_auth_sha1(pb_out[i], 0xDB); }

    // XOR กับโค้ดพิเศษ
    for (uint8_t i = 0; i < 20; i++) { pb_out[i] ^= code_from_CAnimManager_AddAnimation[i]; }

    // สร้างสตริงผลลัพธ์
    auth_stringify(buf, pb_out);
}

// โครงสร้างตาราง authentication
struct AuthEntry {
    const char* send;
    const char* recv;
};

// ตาราง authentication
static const AuthEntry AuthTable[] = {
        {"7B6F9203EF3C4D5A", "F27A3E8A540C40D0E0A5A5D8A45E204A25090A2F"},
        {"2D9037E8B1C089E2", "5E901C5F1A360A2FFA6A679F53C982C18AA3A5C1"},
        {"B4F291A980AAE8A6", "F82B7F3C3A5A0B00A75C6F963BAA8E0C056A0E1B"},
        {"6E6DB7D36F793D3D", "F0D3A9AFD55F760C097FA4A45E8F3C7856F4C623"},
        {"A0A4088E71C89DDF", "E71B6C5AA5D0676DFC70F6EB25A700F6E26C67E1"},
        {"734CDE2E25D3938F", "9BC3A40B9EAF2FFCBFA2967E2E80CFAA5B22E9D8"},
        {"35B4E9AA805AA6C7", "75E3B2F9D9F7D9E4F6AA8987036CFADE6FEA17A2"},
        {"A5E574CB90B8B397", "8E55C42C64F1D6A0FEB1C8BC47D8960FDE7A558F"},
        {"3C9F2D97B8DAE975", "A29B7DA1CDBED0ECCE8F5C0FD694BF3FAA8AC2A2"},
        {"73DE33D7C8E6B6CA", "B8E37DAFA1D8F7DA1CD6E94FD94A7EAF3B9A05FE"},
        {"5F71B8C88DAA8C9B", "BF4A9D963D7F7D2D0E5D7EDE3B4D6DD3B5D8B1F9"},
        {"3F18A5B4D70F7E79", "F0D5D5FD0B9C3D7EA2F3EAA7AA3B0BF86A4A5FD8"},
        {"B3DBB6D0A1DD6EEB", "F28AD9A6C9F41B5F7A3D9DBD05D4B4C7DFA70F8E"},
        {"8EF4C48E51B0FF7B", "9B07F8C0DBABE7E2A53A81F17A0A06F0F8C4C18A"},
        {"1EB7EFD0E3B6E9C4", "F4D1B8D4F6E67D97AEB0F3A5E74BCC0B4D9D4BF6"},
        {"C0C9EE5A7FDF62BB", "E7E6FDE4D3B7B1FBF6B5DA3D1FEA6DD59DCA9F4A"},
        {"BA88BBCCB2F4AEBD", "C69A6D4E4A58D1D8B2C3EB9E5D46AFAD7A7E7E9A"},
        {"87BB6BBBFEE1C89E", "9F3D7EF4F4F5B4C5C7B0F1D6D5DAE7F7FA2B6F1D"},
        {"33F4DA36D4FAF0C0", "A7E4F7A7A6C5AFDDF1C3E2C6A3BBDF78B9E6F5DB"},
        {"BA70B9E6E1B7C2DD", "FB2E5B1F6B2F2E5D0C7DB6DD1EE7CC1FB3E4BC5D"},
        {"E1E8E6A1B7B7A5BD", "E0E3A3D9E6EAC1F3C4E4F4E6E2E9B5B5F6F8B9E"},
        {"F1F7C7B3B1D6B5B8", "F4A6A2A2A4B8C3C6C2B6D4C1C3B8F4B8F6F3C"},
        {"E4F5E8F6B1E9E4E5", "E4F5E9E9B8E9F7E6E4F7E7E4E6F3E4F3F4E4E"},
        {"F2E4F5E4E3E5E3E8", "E5E3E5E3E6E7E7E4E4E4E3E5E4E5E4E3E7E5F"},
        {"E4E6F6F5E7E4E5E5", "E4F6F5E5E5E7E6E5E4E4E4E5E4E4E5E7E5F"},
        {"E6E6F6F6E5F6E5E5", "E7E6E4E7E6E4E4E6E4E4E5E5E7E5E4E5E7F"}
};

// ฟังก์ชันค้นหาคีย์ในตาราง
const char* findKey(const char* sendValue) {
    for (size_t i = 0; i < sizeof(AuthTable)/sizeof(AuthTable[0]); ++i) {
        if (strcmp(AuthTable[i].send, sendValue) == 0) {
            return AuthTable[i].recv;
        }
    }
    return nullptr;
}