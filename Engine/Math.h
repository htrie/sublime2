
typedef signed char int8;
typedef short int16;
typedef int int32;
typedef long long int64;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned long long uint64;
typedef unsigned long long size;

static_assert(sizeof(int8) == 1);
static_assert(sizeof(int16) == 2);
static_assert(sizeof(int32) == 4);
static_assert(sizeof(int64) == 8);
static_assert(sizeof(uint8) == 1);
static_assert(sizeof(uint16) == 2);
static_assert(sizeof(uint32) == 4);
static_assert(sizeof(uint64) == 8);
static_assert(sizeof(size) == 8);

class NoCopy {
public:
    NoCopy() = default;

private:
    NoCopy(const NoCopy&) = delete;
    NoCopy& operator=(const NoCopy&) = delete;
};

namespace Math {
    const float Epsilon = 2.2204460492503131e-016;
    const float Large = 3.402823466e+38F;
    const float Pi = 3.141593f;
    const float TwoPi = 6.283185f;
    const float InvTwoPi = 0.1591549f;
    const float ThreeHalfPi = 4.7123889f;
    const float HalfPi = 1.570796f;
    const float QuarterPi = 0.7853982f;

    static constexpr uint32 RGBA(unsigned r, unsigned g, unsigned b, unsigned a) {
        return (uint32)((a << 24) | (b << 16) | (g << 8) | r);
    }

    static unsigned Length(const char* s) {
        unsigned length = 0;
        while (s[length] != 0) { length++; }
        return length;
    }

    template<typename T> static constexpr T Abs(T a) { return (a < 0) ? -a : a; }
    static constexpr float Abs(float a) { return (a < 0.f) ? -a : a; }

    static float CopySign(float x, float s) {
        const uint32 sign = *((uint32*)&s) & 0x80000000;
        const uint32 value = *((uint32*)&x) ^ sign;
        return *(float*)&value;
    }

    static float Floor(float x) { return floorf(x); }
    static float InvSqrt(float x) { return 1.f / sqrtf(x); }
    static float Sqrt(float x) { return sqrtf(x); }
    
    template<typename T> static constexpr T Min(T a, T b) { return (a < b) ? a : b; }
    template<typename T> static constexpr T Max(T a, T b) { return (a > b) ? a : b; }
    template<typename T> static constexpr T Clamp(T x, T a, T b) { return Math::Min(Math::Max(x, a), b); }
    template<typename T> static constexpr T AlignSize(T x, T n) { return ((x + n - 1) & ~(n - 1)); }
    template<typename T> static constexpr T Lerp(T x, T y, T s) { return x + s * (y - x); }

    static constexpr unsigned Log2(unsigned x) {
        unsigned n = 0;
        unsigned tx= (unsigned)x >> 1;
        for (; tx; tx = tx >> 1, ++n);
        return n;
    }

    static constexpr bool IsPowerOf2(unsigned x) { return (( x & (x-1)) == 0); }

    static constexpr uint32 NextPowerOf2(uint32 x) {
        unsigned n = x;
        n--;
        n |= n >> 1;
        n |= n >> 2;
        n |= n >> 4;
        n |= n >> 8;
        n |= n >> 16;
        return ++n;
    }

    
    union Bits {
        float f;
        int32 si;
        uint32 ui;
    };
    
    static const int Shift = 13;
    static const int ShiftSign = 16;
    
    static const int32 InfN = 0x7F800000; // flt32 infinity
    static const int32 MaxN = 0x477FE000; // max flt16 normal as a flt32
    static const int32 MinN = 0x38800000; // min flt16 normal as a flt32
    static const int32 SignN = 0x80000000; // flt32 sign bit
    
    static const int32 InfC = InfN >> Shift;
    static const int32 NanN = (InfC + 1) << Shift; // minimum flt16 nan as a flt32
    static const int32 MaxC = MaxN >> Shift;
    static const int32 MinC = MinN >> Shift;
    static const int32 SignC = SignN >> ShiftSign; // flt16 sign bit
    
    static const int32 MulN = 0x52000000; // (1 << 23) / minN
    static const int32 MulC = 0x33800000; // minN / (1 << (23 - shift))
    
    static const int32 SubC = 0x003FF; // max flt32 subnormal down shifted
    static const int32 NorC = 0x00400; // min flt32 normal down shifted
    
    static const int32 MaxD = InfC - MaxC - 1;
    static const int32 MinD = MinC - SubC - 1;

    static uint16 HalfCompress(float x) {
        Bits v, s;
        v.f = x;
        uint32 sign = v.si & SignN;
        v.si ^= sign;
        sign >>= ShiftSign; // logical shift
        s.si = MulN;
        s.si = (int32)(s.f * v.f); // correct subnormals
        v.si ^= (s.si ^ v.si) & -(MinN > v.si);
        v.si ^= (InfN ^ v.si) & -((InfN > v.si) & (v.si > MaxN));
        v.si ^= (NanN ^ v.si) & -((NanN > v.si) & (v.si > InfN));
        v.ui >>= Shift; // logical shift
        v.si ^= ((v.si - MaxD) ^ v.si) & -(v.si > MaxC);
        v.si ^= ((v.si - MinD) ^ v.si) & -(v.si > SubC);
        return v.ui | sign;
    }

    static float HalfDecompress(uint16 x) {
        Bits v;
        v.ui = x;
        int32 sign = v.si & SignC;
        v.si ^= sign;
        sign <<= ShiftSign;
        v.si ^= ((v.si + MinD) ^ v.si) & -(v.si > SubC);
        v.si ^= ((v.si + MaxD) ^ v.si) & -(v.si > MaxC);
        Bits s;
        s.si = MulC;
        s.f *= v.si;
        int32 mask = -(NorC > v.si);
        v.si <<= Shift;
        v.si ^= (s.si ^ v.si) & mask;
        v.si |= sign;
        return v.f;
    }

    static float Cos32s(float x) {
        const float c1 = 0.99940307f;
        const float c2 = -0.49558072f;
        const float c3 = 0.03679168f;
        const float x2 = x * x;
        return (c1 + x2 * (c2 + c3 * x2));
    }

    static void SinCos(float angle, float& sin, float& cos) {
        angle = angle - Floor(angle * InvTwoPi) * TwoPi;
        const float sin_multiplier = angle > 0.f && angle < Pi ? 1.f : -1.f;
        angle = angle > 0.f ? angle : -angle;
        if (angle < HalfPi) {
            cos = Cos32s(angle);
            sin = sin_multiplier * Sqrt(1.f - cos*cos);
            return;
        }
        if (angle < Pi) {
            cos = -Cos32s(Pi - angle);
            sin = sin_multiplier * Sqrt(1.f - cos*cos);
            return;
        }
        if (angle < ThreeHalfPi) {
            cos = -Cos32s(angle - Pi);
            sin = sin_multiplier * Sqrt(1.f - cos*cos);
            return;
        }
        cos = Cos32s(TwoPi - angle);
        sin = sin_multiplier * Sqrt(1.f - cos*cos);
        return;
    }

    static float Tan(float x) {
        float sin, cos;
        SinCos(x, sin, cos);
        return sin / cos;
    }

    static float ATan(float x) {
        return QuarterPi * x - x * (Abs(x) - 1)*(0.2447f + 0.0663f * Abs(x));
    }

    static float ATan2(float y, float x) {
        if (Abs(x) > Abs(y)) {
            const float atan = ATan(y / x);
            if (x > 0.f) return atan;
            else return y>0.f ? atan + Pi : atan - Pi;
        } else {
            const float atan = ATan(x / y);
            if (x > 0.f) return y > 0.f ? HalfPi - atan : -HalfPi - atan;
            else return y > 0.f ? HalfPi + atan : -HalfPi + atan;
        }
    }

    constexpr float Pow(float x, float y) {
        union {
            double d;
            int x[2];
        } u = { x };
        u.x[1] = (int)(y * (u.x[1] - 1072632447) + 1072632447);
        u.x[0] = 0;
        return (float)u.d;
    }
};

namespace Hash {
    static const uint64 FnvBasisU64 = 14695981039346656037u;
    static const uint64 FnvPrimeU64 = 1099511628211u;

    static const uint32 FnvBasisU32 = 2166136261u;
    static const uint32 FnvPrimeU32 = 16777619u;

    static uint64 Fnv64(const char* s, size count) {
        uint64 hash = FnvBasisU64;
        for (int i = 0; i < count; ++i)
            hash = (hash ^ s[i]) * FnvPrimeU64;
        hash = (hash ^ '+') * FnvPrimeU64;
        hash = (hash ^ '+') * FnvPrimeU64;
        return hash;
    }

    static uint32 Fnv32(const char* s, size count) {
        uint32 hash = FnvBasisU32;
        for (int i = 0; i < count; ++i)
            hash = (hash ^ s[i]) * FnvPrimeU32;
        hash = (hash ^ '+') * FnvPrimeU32;
        hash = (hash ^ '+') * FnvPrimeU32;
        return hash;
    }

    static uint64 Fnv64(const char* s) { return Fnv64(s, Math::Length(s)); }
    static uint32 Fnv32(const char* s) { return Fnv32(s, Math::Length(s)); }
};

namespace Scan {
    static char* IntToString(long long i, char* p, unsigned max_size) {
        char buf[256];
        buf[0] = 0;
        char* t = buf;
        if (i < 0) {
            *t = '-';
            ++t;
        }
        long long r = Math::Abs(i);
        while (r > 0) {
            *t = '0' + r % 10;
            ++t;
            r = r / 10;
        }
        char* f = p;
        while (t != buf) {
            --t;
            *f = *t;
            f++;
        }
        *f = 0; // EOS
        return p;
    }

    static const char* StringToInt(const char* p, int* i) {
        int r = 0;
        bool neg = false;
        if (*p == '-') {
            neg = true;
            ++p;
        }
        while (*p >= '0' && *p <= '9') {
            r = (r * 10) + (*p - '0');
            ++p;
        }
        if (neg) {
            r = -r;
        }
        *i = r;
        return p;
    }

    static const char* StringToFloat(const char* p, float* f) {
        float r = 0.f;
        bool neg = false;
        if (*p == '-') {
            neg = true;
            ++p;
        }
        while (*p >= '0' && *p <= '9') {
            r = (r*10.f) + (*p - '0');
            ++p;
        }
        if (*p == '.') {
            float val = 0.f;
            float mul = 1.f;
            ++p;
            while (*p >= '0' && *p <= '9') {
                val = (val*10.f) + (*p - '0');
                ++p;
                mul *= 0.1f;
            }
            r += val * mul;
        }
        if (neg) {
            r = -r;
        }
        *f = r;
        return p;
    }

    static bool Compare(const char* text, const char* other, const size length) {
        unsigned index = 0;
        while ((index < length) && (text[index] == other[index])) { index++; }
        return index == length;
    }

    static void Int(const char* s, size n, int* i) { StringToInt(s, i); }
    static void Float(const char* s, size n, float* f) { StringToFloat(s, f); }

    static void Vector2(const char* s, size n, float* f) {
        const char* s0 = StringToFloat(s, &f[0]); s0++;
        StringToFloat(s0, &f[1]);
    }

    static void Vector3(const char* s, size n, float* f) {
        const char* s0 = StringToFloat(s, &f[0]); s0++;
        const char* s1 = StringToFloat(s0, &f[1]); s1++;
        StringToFloat(s1, &f[2]);
    }

    static void Vector4(const char* s, size n, float* f) {
        const char* s0 = StringToFloat(s, &f[0]); s0++;
        const char* s1 = StringToFloat(s0, &f[1]); s1++;
        const char* s2 = StringToFloat(s1, &f[2]); s2++;
        StringToFloat(s2, &f[3]);
    }
}

class Vector2 {
public:
    float x = 0.f, y = 0.f;

    Vector2() {}
    Vector2(float x, float y) : x(x), y(y) {}
    Vector2(float f) : x(f), y(f) {}

    Vector2 operator+(const Vector2& o) const { return Vector2(x + o.x, y + o.y); }
    Vector2 operator-(const Vector2& o) const { return Vector2(x - o.x, y - o.y); }
    Vector2 operator*(const Vector2& o) const { return Vector2(x * o.x, y * o.y); }
    Vector2 operator/(const Vector2& o) const { return Vector2(x / o.x, y / o.y); }
};

class Vector3 {
public:
    float x = 0.f, y = 0.f, z = 0.f;

    Vector3() {}
    Vector3(float x, float y, float z) : x(x), y(y), z(z) {}
    Vector3(const Vector2& v, float z) : x(v.x), y(v.y), z(z) {}
    Vector3(float f) : x(f), y(f), z(f) {}

    Vector3 operator-() const { return Vector3(-x, -y, -z); }

    Vector3 operator+(const Vector3& o) const { return Vector3(x + o.x, y + o.y, z + o.z); }
    Vector3 operator-(const Vector3& o) const { return Vector3(x - o.x, y - o.y, z - o.z); }
    Vector3 operator*(const Vector3& o) const { return Vector3(x * o.x, y * o.y, z * o.z); }
    Vector3 operator/(const Vector3& o) const { return Vector3(x / o.x, y / o.y, z / o.z); }

    Vector3& operator+=(const Vector3& o) { x += o.x; y += o.y; z += o.z; return *this; }
    Vector3& operator-=(const Vector3& o) { x -= o.x; y -= o.y; z -= o.z; return *this; }
    Vector3& operator*=(const Vector3& o) { x *= o.x; y *= o.y; z *= o.z; return *this; }
    Vector3& operator/=(const Vector3& o) { x /= o.x; y /= o.y; z /= o.z; return *this; }

    bool operator==(const Vector3& o) const { return x == o.x && y == o.y && z == o.z; }

    Vector3 Cross(const Vector3& o) const { return Vector3(y * o.z - z * o.y, z * o.x - x * o.z, x * o.y - y * o.x); }
    float Dot(const Vector3& o) const { return x * o.x + y * o.y + z * o.z; }

    float Length() const { return Math::Sqrt(Dot(*this)); }
    float SquareLength() const { return Dot(*this); }

    float Distance(const Vector3& o) const { return (o - *this).Length(); }
    float SquareDistance(const Vector3& o) const { return (o - *this).SquareLength(); }

    Vector3 Normalize() const { return Vector3(1.f / Length()) * *this; }

    Vector3 Absolute() const { return Vector3(Math::Abs(x), Math::Abs(y), Math::Abs(z)); }

    Vector3 Minimum(const Vector3& o) const { return Vector3(Math::Min(x, o.x), Math::Min(y, o.y), Math::Min(z, o.z)); }
    Vector3 Maximum(const Vector3& o) const { return Vector3(Math::Max(x, o.x), Math::Max(y, o.y), Math::Max(z, o.z)); }

    Vector3 Lerp(const Vector3& o, float s) const { return *this + (o - *this) * s; }

    static Vector3 CubicHermite(const Vector3& va, const Vector3& vb, const Vector3& ta, const Vector3& tb, float s) {
        const auto s2 = s * s;
        const auto s3 = s2 * s;
        return va * (2.f * s3 - 3.f * s2 + 1.f) + vb * (-2.f * s3 + 3.f * s2) + ta * (s3 - 2.f * s2 + s) + tb * (s3 - s2);
    } 
};

class Vector4 {
public:
    float x = 0.f, y = 0.f, z = 0.f, w = 0.f;

    Vector4() {}
    Vector4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
    Vector4(const Vector3& v, float w) : x(v.x), y(v.y), z(v.z), w(w) {}
    Vector4(float f) : x(f), y(f), z(f), w(f) {}

    Vector4 operator+(const Vector4& o) const { return Vector4(x + o.x, y + o.y, z + o.z, w + o.w); }
    Vector4 operator-(const Vector4& o) const { return Vector4(x - o.x, y - o.y, z - o.z, w - o.w); }
    Vector4 operator*(const Vector4& o) const { return Vector4(x * o.x, y * o.y, z * o.z, w * o.w); }
    Vector4 operator/(const Vector4& o) const { return Vector4(x / o.x, y / o.y, z / o.z, w / o.w); }

    float Dot(const Vector4& o) const { return x * o.x + y * o.y + z * o.z + w * o.w; }

    float Length() const { return Math::Sqrt(Dot(*this)); }
    float SquareLength() const { return Dot(*this); }

    Vector4 Normalize() const { return Vector4(1.f / Length()) * *this; }

    Vector4 Absolute() const { return Vector4(Math::Abs(x), Math::Abs(y), Math::Abs(z), Math::Abs(w)); }
};

class Quaternion {
public:
    float x = 0.f, y = 0.f, z = 0.f, w = 1.f;

    Quaternion() {}
    Quaternion(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
    Quaternion(float f) : x(f), y(f), z(f), w(f) {}
    Quaternion(const Vector3& axis, float angle) {
        const float sin = sinf(angle * 0.5f);
        const float cos = cosf(angle * 0.5f);
        *this = Quaternion(axis.x * sin, axis.y * sin, axis.z * sin, cos).Normalize();
    }

    Quaternion operator+(const Quaternion& o) const { return Quaternion(x + o.x, y + o.y, z + o.z, w + o.w); }
    Quaternion operator-(const Quaternion& o) const { return Quaternion(x - o.x, y - o.y, z - o.z, w - o.w); }
    Quaternion& operator+=(const Quaternion& o) { x += o.x; y += o.y; z += o.z; w += o.w; return *this; }

    Quaternion operator*(const float& f) const { return Quaternion(x * f, y * f, z * f, w * f); }
    Quaternion operator*(const Vector3& v) const {
        return Quaternion(
             v.x * w + v.y * z - v.z * y,
             v.y * w + v.z * x - v.x * z,
             v.z * w + v.x * y - v.y * x,
            -v.x * x - v.y * y - v.z * z);
    }
    Quaternion operator*(const Quaternion& o) const {
        return Quaternion(
            o.w * x + o.x * w + o.y * z - o.z * y,
            o.w * y + o.y * w + o.z * x - o.x * z,
            o.w * z + o.z * w + o.x * y - o.y * x,
            o.w * w - o.x * x - o.y * y - o.z * z);
    }

    float Dot(const Quaternion& o) const { return x * o.x + y * o.y + z * o.z + w * o.w; }

    float Length() const { return Math::Sqrt(Dot(*this)); }
    float SquareLength() const { return Dot(*this); }

    float Distance(const Quaternion& o) const { return (o - *this).Length(); }
    float SquareDistance(const Quaternion& o) const { return (o - *this).SquareLength(); }

    Quaternion Normalize() const { return *this * (1.f / Length()); }

    Quaternion Conjugate() const { return Quaternion(-x, -y, -z, w); }

    Quaternion Slerp(const Quaternion& o, float t) const {
        const float cosTheta = Dot(o);
        float alpha = cosTheta >= 0 ? 1.f : -1.f;
        const float halfY = 1.f + alpha * cosTheta;

        float f2b = t - 0.5f;
        float u = f2b >= 0 ? f2b : -f2b;
        float f2a = u - f2b;
        f2b += u;
        u += u;
        float f1 = 1.f - u;

        float halfSecHalfTheta = 1.09f - (0.476537f - 0.0903321f * halfY) * halfY;
        halfSecHalfTheta *= 1.5f - halfY * halfSecHalfTheta * halfSecHalfTheta;
        const float versHalfTheta = 1.f - halfY * halfSecHalfTheta;

        const float sqNotU = f1 * f1;
        float ratio2 = 0.0000440917108f * versHalfTheta;
        float ratio1 = -0.00158730159f + (sqNotU - 16.f) * ratio2;
        ratio1 = 0.0333333333f + ratio1 * (sqNotU - 9.f) * versHalfTheta;
        ratio1 = -0.333333333f + ratio1 * (sqNotU - 4.f) * versHalfTheta;
        ratio1 = 1.f + ratio1 * (sqNotU - 1.f) * versHalfTheta;

        const float sqU = u * u;
        ratio2 = -0.00158730159f + (sqU - 16.f) * ratio2;
        ratio2 = 0.0333333333f + ratio2 * (sqU - 9.f) * versHalfTheta;
        ratio2 = -0.333333333f + ratio2 * (sqU - 4.f) * versHalfTheta;
        ratio2 = 1.f + ratio2 * (sqU - 1.f) * versHalfTheta;

        f1 *= ratio1 * halfSecHalfTheta;
        f2a *= ratio2;
        f2b *= ratio2;
        alpha *= f1 + f2a;
        const float beta = f1 + f2b;

        return *this * alpha + o * beta;
    }

    Vector3 Right() const {
        return Vector3(
            1.0f - 2.0f * (z * z + y * y),
            2.0f * (x * y - w * z),
            2.0f * (x * z + w * y));
    }

    Vector3 Up() const {
        return Vector3(
            2.0f * (x * y + w * z),
            1.0f - 2.0f * (x * x + z * z),
            2.0f * (y * z - w * x));
    }

    Vector3 At() const {
        return Vector3(
            2.0f * (x * z - w * y),
            2.0f * (y * z + w * x),
            1.0f - 2.0f * (x * x + y * y));
    }

    Vector3 Transform(const Vector3& v) const {
        const Vector3 u(x, y , z);
        const Vector3 t = u.Cross(v) * 2.f;
        return v + t * w + u.Cross(t);
    }
};

class Matrix {
public:
    Vector4 row0 = Vector4(1.f, 0.f, 0.f, 0.f);
    Vector4 row1 = Vector4(0.f, 1.f, 0.f, 0.f);
    Vector4 row2 = Vector4(0.f, 0.f, 1.f, 0.f);
    Vector4 row3 = Vector4(0.f, 0.f, 0.f, 1.f);

    Matrix() {}
    Matrix(const Vector3& translation) : row3(translation, 1.0f) {}
    Matrix(const Vector4& row0, const Vector4& row1, const Vector4& row2, const Vector4& row3)
        : row0(row0), row1(row1), row2(row2), row3(row3) {}
    
    Matrix Transpose() const {
        return Matrix(
            Vector4(row0.x, row1.x, row2.x, row3.x),
            Vector4(row0.y, row1.y, row2.y, row3.y),
            Vector4(row0.z, row1.z, row2.z, row3.z),
            Vector4(row0.w, row1.w, row2.w, row3.w));
    }

    Matrix operator*(const Matrix& o) const {
        const auto t = o.Transpose();
        return Matrix(
            Vector4(row0.Dot(t.row0), row0.Dot(t.row1), row0.Dot(t.row2), row0.Dot(t.row3)),
            Vector4(row1.Dot(t.row0), row1.Dot(t.row1), row1.Dot(t.row2), row1.Dot(t.row3)),
            Vector4(row2.Dot(t.row0), row2.Dot(t.row1), row2.Dot(t.row2), row2.Dot(t.row3)),
            Vector4(row3.Dot(t.row0), row3.Dot(t.row1), row3.Dot(t.row2), row3.Dot(t.row3)));
    }

    Matrix operator+(const Matrix& o) const {
        return Matrix(row0 + o.row0, row1 + o.row1, row2 + o.row2, row3 + o.row3);
    }

    Matrix operator-(const Matrix& o) const {
        return Matrix(row0 - o.row0, row1 - o.row1, row2 - o.row2, row3 - o.row3);
    }

    Matrix operator*(const float& f) const {
        return Matrix(row0 * f, row1 * f, row2 * f, row3 * f);
    }

    Matrix operator/(const float& f) const {
        return Matrix(row0 / f, row1 / f, row2 / f, row3 / f);
    }

    Vector3 Translation() const {
        return Vector3(row3.x, row3.y, row3.z);
    }
    
    Quaternion Rotation() const {
        return Quaternion(
            Math::Sqrt(Math::Max(row0.x - row1.y - row2.z + row3.w, 0.0f)) * 0.5f,
            Math::Sqrt(Math::Max(-row0.x + row1.y - row2.z + row3.w, 0.0f)) * 0.5f,
            Math::Sqrt(Math::Max(-row0.x - row1.y + row2.z + row3.w, 0.0f)) * 0.5f,
            Math::Sqrt(Math::Max(row0.x + row1.y + row2.z + row3.w, 0.0f)) * 0.5f);
    }

    float Trace() const {
        return row0.x + row1.y + row2.z + row3.w;
    }

    float Determinant() const {
        return
            row0.x * row1.y * row2.z * row3.w + row0.x * row1.z * row2.w * row3.y + row0.x * row1.w * row2.y * row3.z +
            row0.y * row1.x * row2.w * row3.z + row0.y * row1.z * row2.x * row3.w + row0.y * row1.w * row2.z * row3.x +
            row0.z * row1.x * row2.y * row3.w + row0.z * row1.y * row2.w * row3.x + row0.z * row1.w * row2.x * row3.y +
            row0.w * row1.x * row2.z * row3.y + row0.w * row1.y * row2.x * row3.z + row0.w * row1.z * row2.y * row3.x - (
            row0.x * row1.y * row2.w * row3.w + row0.x * row1.z * row2.y * row3.w + row0.x * row1.w * row2.z * row3.y +
            row0.y * row1.x * row2.z * row3.w + row0.y * row1.z * row2.w * row3.x + row0.y * row1.w * row2.y * row3.x +
            row0.z * row1.x * row2.w * row3.y + row0.z * row1.y * row2.x * row3.w + row0.z * row1.w * row2.y * row3.x +
            row0.w * row1.x * row2.y * row3.z + row0.w * row1.y * row2.z * row3.x + row0.w * row1.z * row2.x * row3.y);
    }

    Matrix Inverse() const
    {
        const float det = Determinant();
        if (det <= 1e-6f)
            return *this;

        const Matrix id;
        const Matrix m2 = *this * *this;
        const Matrix m3 = m2 * *this;

        const float tr = Trace();
        const float tr2 = m2.Trace();
        const float tr3 = m3.Trace();
        
        const float tmp1 = (tr * tr * tr - 3.0f * tr * tr2 + 2.0f * tr3) / 6.0f;
        const float tmp2 = -(tr * tr - tr2) / 2.0f;
        
        return Matrix(id * tmp1 + *this * tmp2 + m2 * tr - m3) / det;
    }

    static void OrthoRH(float w, float h, float n, float f, Matrix& out, Matrix& out_inverse) {
        const float x = 2.f / w;
        const float y = 2.f / h;
        const float z = 1.f / (n - f);
        out = Matrix(
            Vector4(x, 0.f, 0.f, 0.f),
            Vector4(0.f, y, 0.f, 0.f),
            Vector4(0.f, 0.f, z, 0.f),
            Vector4(0.f, 0.f, n * z, 1.f));
        out_inverse = Matrix(
            Vector4(1.f / x, 0.f, 0.f, 0.f),
            Vector4(0.f, 1.f / y, 0.f, 0.f),
            Vector4(0.f, 0.f, 1.f / z, 0.f),
            Vector4(0.f, 0.f, -n, 1.f));
    }

    static void PerspectiveFovRH(float fov, float aspect, float n, float f, Matrix& out, Matrix& out_inverse) {
        const float y = 1.f / Math::Tan(0.5f * fov * Math::Pi / 180.f);
        const float x = y / aspect;
        const float z = f / (n - f);
        const float w = n * z;
        out = Matrix(
            Vector4(x, 0.f, 0.f, 0.f),
            Vector4(0.f, y, 0.f, 0.f),
            Vector4(0.f, 0.f, z, -1.f),
            Vector4(0.f, 0.f, w, 0.f));
        out_inverse = Matrix(
            Vector4(1.f / x, 0.f, 0.f, 0.f),
            Vector4(0.f, 1.f / y, 0.f, 0.f),
            Vector4(0.f, 0.f, 0.f, 1.f / w),
            Vector4(0.f, 0.f, -1.f, 1.f / n));
    }

    static void LookAtRH(const Vector3& eye, const Vector3& at, const Vector3& up, Matrix& out, Matrix& out_inverse) {
        const Vector3 z_axis = at.Normalize();
        const Vector3 x_axis = z_axis.Cross(up).Normalize();
        const Vector3 y_axis = x_axis.Cross(z_axis);
        out = Matrix(
            Vector4(x_axis.x, y_axis.x, z_axis.x, 0.f),
            Vector4(x_axis.y, y_axis.y, z_axis.y, 0.f),
            Vector4(x_axis.z, y_axis.z, z_axis.z, 0.f),
            Vector4(x_axis.Dot(eye), y_axis.Dot(eye), z_axis.Dot(eye), 1.f));
        out_inverse = out.Inverse();
    }

    static void OrthoLH(float w, float h, float n, float f, Matrix& out, Matrix& out_inverse) {
        const float x = 2.f / w;
        const float y = 2.f / h;
        const float z = 1.f / (f - n);
        out = Matrix(
            Vector4(x, 0.f, 0.f, 0.f),
            Vector4(0.f, y, 0.f, 0.f),
            Vector4(0.f, 0.f, z, 0.f),
            Vector4(0.f, 0.f, -n * z, 1.f));
        out_inverse = Matrix(
            Vector4(1.f / x, 0.f, 0.f, 0.f),
            Vector4(0.f, 1.f / y, 0.f, 0.f),
            Vector4(0.f, 0.f, 1.f / z, 0.f),
            Vector4(0.f, 0.f, n, 1.f));
    }

    static void PerspectiveFovLH(float fov, float aspect, float n, float f, Matrix& out, Matrix& out_inverse) {
        const float y = 1.f / Math::Tan(0.5f * fov * Math::Pi / 180.f);
        const float x = y / aspect;
        const float z = f / (f - n);
        const float w = -n * z;
        out = Matrix(
            Vector4(x, 0.f, 0.f, 0.f),
            Vector4(0.f, y, 0.f, 0.f),
            Vector4(0.f, 0.f, z, 1.f),
            Vector4(0.f, 0.f, w, 0.f));
        out_inverse = Matrix(
            Vector4(1.f / x, 0.f, 0.f, 0.f),
            Vector4(0.f, 1.f / y, 0.f, 0.f),
            Vector4(0.f, 0.f, 0.f, 1.f / w),
            Vector4(0.f, 0.f, 1.f, -z / w));
    }

    static void LookAtLH(const Vector3& eye, const Vector3& at, const Vector3& up, Matrix& out, Matrix& out_inverse) {
        const Vector3 z_axis = at.Normalize();
        const Vector3 x_axis = up.Cross(z_axis).Normalize();
        const Vector3 y_axis = z_axis.Cross(x_axis);
        out = Matrix(
            Vector4(x_axis.x, y_axis.x, z_axis.x, 0.f),
            Vector4(x_axis.y, y_axis.y, z_axis.y, 0.f),
            Vector4(x_axis.z, y_axis.z, z_axis.z, 0.f),
            Vector4(-x_axis.Dot(eye), -y_axis.Dot(eye), -z_axis.Dot(eye), 1.f));
        out_inverse = out.Inverse();
    }
};

struct Ray {
    Vector3 from;
    Vector3 to;
    Vector3 direction;
    Vector3 hit_position;
    float hit_fraction = 1.f;
    bool hit = false;

    Ray() {}
    Ray(const Vector3& from, const Vector3& to)
        : from(from), to(to), direction((to - from).Normalize()) {}

    bool ClosestHit(const Vector3& position) {
        const float fraction = from.Distance(position) / from.Distance(to);
        if (fraction < hit_fraction) {
            hit_fraction = fraction;
            hit_position = position;
            hit = true;
            return true;
        }
        return false;
    }
};

struct Sphere {
    Vector3 center;
    float radius = 0.f;

    Sphere() {}
    Sphere(const Vector3& center, float radius) :
        center(center), radius(radius) {}
    Sphere(const Vector3& min, const Vector3& max) :
        center((min + max) * 0.5f), radius((max - min).Length() * 0.5f) {}

    bool Intersect(const Ray& ray) const {
        const auto p = center - ray.from;
        const auto d = p.Dot(ray.direction);
        const auto pc = ray.from + ray.direction * d;
        const auto square_dist = (pc - center).SquareLength();
        return square_dist < radius * radius;
    }

    bool Intersect(const Sphere& other) const {
        const auto square_dist = (other.center - center).SquareLength();
        const auto total_radius = (radius + other.radius);
        const auto square_total_radius = total_radius * total_radius;
        return square_dist < square_total_radius;
    }
};

struct Box {
    Vector3 extents;

    Box() {}
    Box(const Vector3& extents) : extents(extents) {}

    bool Intersect(Ray& ray, const Quaternion& rotation, const Vector3& position) const {
        const auto ray_diff = ray.from - position;
        const Vector3 diff = rotation.Transform(ray_diff);
        const Vector3 direction = rotation.Transform(ray.direction);
        float t0 = -Math::Large;
        float t1 = Math::Large;
        if (Clip(+direction.x, -diff.x - extents.x, t0, t1) &&
            Clip(-direction.x, +diff.x - extents.x, t0, t1) &&
            Clip(+direction.y, -diff.y - extents.y, t0, t1) &&
            Clip(-direction.y, +diff.y - extents.y, t0, t1) &&
            Clip(+direction.z, -diff.z - extents.z, t0, t1) &&
            Clip(-direction.z, +diff.z - extents.z, t0, t1)) {
            if (t0 > 0.f)
                return ray.ClosestHit(ray.from + ray.direction * t0);
        }
        return false;
    }

private:
    static bool Clip(float denom, float numer, float& t0, float& t1) {
        if (denom > 0.f) {
            if (numer > denom * t1) return false;
            if (numer > denom * t0) t0 = numer / denom;
            return true;
        } else if (denom < 0.f) {
            if (numer > denom * t0) return false;
            if (numer > denom * t1) t1 = numer / denom;
            return true;
        } else
            return numer <= 0.f;
    }
};
