
template<typename T, size SIZE> class FixedArray {
    T values[SIZE];

public:
    template<typename F> void Process(F func) {
        for (unsigned i = 0; i < SIZE; ++i) {
            func(values[i]);
        }
    }

    template<typename F> void ConstProcess(F func) const {
        for (unsigned i = 0; i < SIZE; ++i) {
            func(values[i]);
        }
    }

    template<typename F> void ProcessIndex(F func) {
        for (unsigned i = 0; i < SIZE; ++i) {
            func(values[i], i);
        }
    }

    template<typename F> void ConstProcessIndex(F func) const {
        for (unsigned i = 0; i < SIZE; ++i) {
            func(values[i], i);
        }
    }

    const T* Values() const { return values; }
    T* Values() { return values; }
    unsigned Count() const { return SIZE; }

    const T& operator[](size i) const {
        DEBUG_ONLY(if (i >= SIZE) throw Exception("Out-of-bounds");)
        return values[i];
    }

    T& operator[](size i) {
        DEBUG_ONLY(if (i >= SIZE) throw Exception("Out-of-bounds");)
        return values[i];
    }
};

template<typename T, size SIZE> class Array {
    unsigned used_count = 0;
    T values[SIZE];

public:
    template<typename... ARGS> T& Add(ARGS... args) {
        DEBUG_ONLY(if (used_count >= SIZE) throw Exception("Array is full");)
        return *new(&values[used_count++]) T(args...);
    }

    template<typename F> void Process(F func) {
        for (unsigned i = 0; i < used_count; ++i) {
            func(values[i]);
        }
    }

    template<typename F> void ConstProcess(F func) const {
        for (unsigned i = 0; i < used_count; ++i) {
            func(values[i]);
        }
    }

    template<typename F> void ProcessIndex(F func) {
        for (unsigned i = 0; i < used_count; ++i) {
            func(values[i], i);
        }
    }

    template<typename F> void ConstProcessIndex(F func) const {
        for (unsigned i = 0; i < used_count; ++i) {
            func(values[i], i);
        }
    }

    template<typename F> bool Find(F func) {
        for (unsigned i = 0; i < used_count; ++i) {
            if (func(values[i]))
                return true;
        }
        return false;
    }

    template<typename F> bool ConstFind(F func) const {
        for (unsigned i = 0; i < used_count; ++i) {
            if (func(values[i]))
                return true;
        }
        return false;
    }

    T* BinaryFind(const T& goal) {
        auto begin = 0u;
        auto end = used_count;
        while (begin != end) {
            const auto mid = (begin + end) / 2;
            auto& value = values[mid];
            if (value == goal) return &value;
            else if (value < goal) begin = mid < used_count - 1 ? mid + 1 : used_count;
            else end = mid;
        }
        return nullptr;
    }

    const T* ConstBinaryFind(const T& goal) const {
        auto begin = 0u;
        auto end = used_count;
        while (begin != end) {
            const auto mid = (begin + end) / 2;
            const auto& value = values[mid];
            if (value == goal) return &value;
            else if (value < goal) begin = mid < used_count - 1 ? mid + 1 : used_count;
            else end = mid;
        }
        return nullptr;
    }

    void Sort() {
        if (used_count == 0)
            return;
        int exchanges;
        do {
            exchanges = 0;
            for (unsigned i = 0; i < used_count - 1; ++i) {
                if (values[i] > values[i + 1]) {
                    auto tmp = values[i];
                    values[i] = values[i + 1];
                    values[i + 1] = tmp;
                    exchanges++;
                }
            }
        }
        while (exchanges > 0);
    }

    T* Values() { return (T*)values; }
    const T* Values() const { return (T*)values; }
    unsigned UsedCount() const { return used_count; }

    bool IsFull() const { return used_count == SIZE; }

    const T& Back() const {
        DEBUG_ONLY(if (used_count == 0) throw Exception("Out-of-bounds");)
        return values[used_count-1];
    }

    T& Back() {
        DEBUG_ONLY(if (used_count == 0) throw Exception("Out-of-bounds");)
            return values[used_count - 1];
    }

    const T& operator[](size i) const {
        DEBUG_ONLY(if (i >= used_count) throw Exception("Out-of-bounds");)
        return values[i];
    }

    T& operator[](size i) {
        DEBUG_ONLY(if (i >= used_count) throw Exception("Out-of-bounds");)
        return values[i];
    }
};

template<typename T> class ProxyArray : public NoCopy {
    unsigned count = 0;
    T* values = nullptr;

public:
    ProxyArray() {}
    ProxyArray(T* values, unsigned count) : values(values), count(count) {}
    ~ProxyArray() {
        for (unsigned i = 0; i < count; ++i) {
            values[i].~T();
        }
    }

    template<typename F> void Process(F func) {
        for (unsigned i = 0; i < count; ++i) {
            func(values[i]);
        }
    }

    template<typename F> void ConstProcess(F func) const {
        for (unsigned i = 0; i < count; ++i) {
            func(values[i]);
        }
    }

    template<typename F> void ProcessIndex(F func) {
        for (unsigned i = 0; i < count; ++i) {
            func(values[i], i);
        }
    }

    template<typename F> void ConstProcessIndex(F func) const {
        for (unsigned i = 0; i < count; ++i) {
            func(values[i], i);
        }
    }

    T* BinaryFind(const T& goal) {
        auto begin = 0u;
        auto end = count;
        while (begin != end) {
            const auto mid = (begin + end) / 2;
            auto& value = values[mid];
            if (value == goal) return &value;
            else if (value < goal) begin = mid < count-1 ? mid+1 : count;
            else end = mid;
        }
        return nullptr;
    }

    const T* ConstBinaryFind(const T& goal) const {
        auto begin = 0u;
        auto end = count;
        while (begin != end) {
            const auto mid = (begin + end) / 2;
            const auto& value = values[mid];
            if (value == goal) return &value;
            else if (value < goal) begin = mid < count - 1 ? mid + 1 : count;
            else end = mid;
        }
        return nullptr;
    }

    T* Values() { return values; }
    const T* Values() const { return values; }
    unsigned Count() const { return count; }

    const T& operator[](size i) const {
        DEBUG_ONLY(if (i >= count) throw Exception("Out-of-bounds");) return values[i];
    }

    T& operator[](size i) {
        DEBUG_ONLY(if (i >= count) throw Exception("Out-of-bounds");)
        return values[i];
    }
};

template<typename T> class BitFlags {
    T flags;

public:
    BitFlags() {}
    BitFlags(const T& flags) : flags(flags) {}

    BitFlags operator|(const BitFlags& o) const { return (T)((unsigned)flags | (unsigned)o.flags); }
    BitFlags operator&(const BitFlags& o) const { return (T)((unsigned)flags & (unsigned)o.flags); }

    BitFlags& operator|=(const BitFlags& o) { flags = (T)((unsigned)flags | (unsigned)o.flags); return *this; }

    bool operator==(const BitFlags &o) const { return (unsigned)flags == (unsigned)o.flags; }

    operator bool() const { return (unsigned)flags > 0; }
};

template<unsigned N> class BitArray : public NoCopy {
    static_assert(Math::AlignSize(N, (unsigned)64) == N);
    static const unsigned BucketCount = N / 64;
    FixedArray<uint64, BucketCount> values;

    static constexpr uint64 Shift(unsigned i) { return (uint64)1 << i; }
    static constexpr uint64 Mask(unsigned i) { return Shift(i % 64); }

public:
    BitArray() {
        for (unsigned i = 0; i < BucketCount; ++i) {
            values[i] = 0;
        }
    }

    void Set(unsigned i) { values[i / 64] |= Mask(i); }
    void Unset(unsigned i) { values[i / 64] &= ~Mask(i); }
    bool IsSet(unsigned i) const { return (values[i / 64] & Mask(i)) > 0; }

    template<typename F> void Process(F func) {
        values.ProcessIndex([&](auto& value, unsigned index) {
            if (value) {
                for (unsigned i = 0; i < 64; ++i) {
                    if (value & Shift(i)) {
                        func(index * 64 + i);
                    }
                }
            }
        });
    }

    template<typename F> void ConstProcess(F func) const {
        values.ConstProcessIndex([&](auto& value, unsigned index) {
            if (value) {
                for (unsigned i = 0; i < 64; ++i) {
                    if (value & Shift(i)) {
                        func(index * 64 + i);
                    }
                }
            }
        });
    }
};

template <unsigned LENGTH> class FixedString {
public:
    static const unsigned MaxSize = LENGTH;

private:
    char data[LENGTH];
    unsigned length = 0; // Excluding EOS.

public:
    static const int InvalidPos = -1;

    FixedString() {}
    FixedString(const FixedString& s) : FixedString(s.Data(), s.Size()) {}
    FixedString(const char* s) : FixedString(s, Math::Length(s)) {}
    FixedString(const char* s, unsigned n) {
        DEBUG_ONLY(if (n >= LENGTH) throw Exception("String text too long");)
        memcpy(data, s, n);
        data[n] = 0; //EOS
        length = n;
    }

    FixedString operator+(const FixedString& other) const {
        FixedString s = *this;
        const unsigned new_length = s.length + other.length;
        DEBUG_ONLY(if (new_length >= LENGTH) throw Exception("String text too long");)
        memcpy(s.data + s.length, other.Data(), other.length);
        s.data[new_length] = 0; //EOS
        s.length = new_length;
        return s;
    }

    bool operator==(const FixedString& other) const {
        return (length == other.length) && (memcmp(data, other.data, length) == 0);
    }

    bool operator!=(const FixedString& other) const {
        return (length != other.length) || (memcmp(data, other.data, length) != 0);
    }

    bool operator>(const FixedString& other) const {
        for (unsigned i = 0; i < length; ++i) {
            if (data[i] != other.data[i])
                return data[i] > other.data[i];
        }
        return false;
    }

    bool StartsWith(const FixedString& other) const {
        if (length < other.length)
            return false;
        for (unsigned i = 0; i < other.length; ++i) {
            if (data[i] != other.data[i])
                return false;
        }
        return true;
    }

    bool EndsWith(const FixedString& other) const {
        if (length < other.length)
            return false;
        const unsigned start = length - other.length;
        for (unsigned i = 0; i < other.length; ++i) {
            if (data[start + i] != other.data[i])
                return false;
        }
        return true;
    }

    int FindFirst(char c, unsigned offset = 0) const {
        for (unsigned i = offset; i < length; ++i) {
            if (data[i] == c)
                return i;
        }
        return InvalidPos;
    }

    int FindLast(char c, unsigned offset = (unsigned)-1) const {
        for (int i = Math::Min(offset, length) - 1; i >= 0; --i) {
            if (data[i] == c)
                return i;
        }
        return InvalidPos;
    }

    FixedString SubString(unsigned start, unsigned count = (unsigned)-1) const {
        return FixedString(data + start, Math::Min(count, length - start));
    }

    void Replace(char old_c, char new_c) {
        for (unsigned i = 0; i < length; ++i) {
            if (data[i] == old_c)
                data[i] = new_c;
        }
    }

    const char* Data() const { return data; }
    char* Data() { return data; }
    unsigned Size() const { return length; }
};

typedef FixedString<256> String;
typedef FixedString<2048> LongString;

namespace Log {
    template<typename... ARGS> static void Put(const char* format, ARGS... args) {
        char buffer[LongString::MaxSize];
        Text::Format(buffer, LongString::MaxSize, format, args...);
        Text::Print(buffer);
    }
}

class File : public NoCopy {
    String filename;
    Descriptor descriptor;

protected:
    enum class Flag : uint8 {
        None = 0,
        Map = 1 << 0,
        ReadOnly = 1 << 1,
        CopyOnWrite = 1 << 2,
        Create = 1 << 3,
    };

    File(const String& filename, const BitFlags<Flag>& flags, size file_size)
        : filename(filename), descriptor(filename.Data(), flags & Flag::Map, flags & Flag::ReadOnly, flags & Flag::CopyOnWrite, flags & Flag::Create, file_size) {}

public:
    File() {}

    const String& FileName() const { return filename; }
    size Size() const { return descriptor.Size(); }
    void* Pointer() const { return descriptor.Pointer(); }

    bool IsNewer(const File& file) {
        return Descriptor::Newer(descriptor, file.descriptor);
    }

    static String ExtractPath(const String& filename) {
        const int i = filename.FindLast('/');
        if (i != String::InvalidPos) {
            return filename.SubString(0, i+1);
        }
        return "";
    }

    static String RemoveExtension(const String& filename) {
        const int i = filename.FindLast('.');
        if (i != String::InvalidPos) {
            return filename.SubString(0, i);
        }
        return "";
    }

    static String GetExtension(const String& filename) {
        const int i = filename.FindLast('.');
        if (i != String::InvalidPos) {
            return filename.SubString(i);
        }
        return "";
    }

    static bool Exist(const String& filename) {
        return Descriptor::Exist(filename.Data());
    }

    static void CreatePath(const String& path) {
        int offset = 0;
        while ((offset = path.FindFirst('/', offset)) != String::InvalidPos) {
            Directory::Create(path.SubString(0, offset).Data());
            offset++;
        }
    }

    static void Copy(const String& src_filename, const String& dst_filename) {
        Descriptor::Copy(src_filename.Data(), dst_filename.Data());
    }

    static void Move(const String& src_filename, const String& dst_filename) {
        Descriptor::Move(src_filename.Data(), dst_filename.Data());
    }

    static void Delete(const String& filename) {
        Descriptor::Delete(filename.Data());
    }
};

class ReadOnlyFile : public File {
public:
    ReadOnlyFile() {}
    ReadOnlyFile(const String& filename)
        : File(filename, BitFlags<Flag>(Flag::Map) | Flag::ReadOnly, 0) {}
};

class ReadCopyFile : public File {
public:
    ReadCopyFile() {}
    ReadCopyFile(const String& filename)
        : File(filename, BitFlags<Flag>(Flag::Map) | Flag::CopyOnWrite, 0) {}
};

class WriteOnlyFile : public File {
public:
    WriteOnlyFile(const String& filename, size file_size)
        : File(filename, BitFlags<Flag>(Flag::Map) | Flag::Create, file_size) {}
};

class ReadQueryFile : public File {
public:
    ReadQueryFile(const String& filename)
        : File(filename, BitFlags<Flag>(Flag::ReadOnly), 0) {}
};

enum class Color : unsigned {
    White = Math::RGBA(255, 255, 255, 255),
    Silver = Math::RGBA(192, 192, 192, 255),
    Gray = Math::RGBA(128, 128, 128, 255),
    Black = Math::RGBA(0, 0, 0, 255),
    Red = Math::RGBA(255, 0, 0, 255),
    Maroon = Math::RGBA(128, 0, 0, 255),
    Yellow = Math::RGBA(255, 255, 0, 255),
    Olive = Math::RGBA(128, 128, 0, 255),
    Lime = Math::RGBA(0, 255, 0, 255),
    Green = Math::RGBA(0, 128, 0, 255),
    Aqua = Math::RGBA(0, 255, 255, 255),
    Teal = Math::RGBA(0, 128, 128, 255),
    Blue = Math::RGBA(0, 0, 255, 255),
    Navy = Math::RGBA(0, 0, 128, 255),
    Fuschia = Math::RGBA(255, 0, 255, 255),
    Purple = Math::RGBA(128, 0, 128, 255),
    Orange = Math::RGBA(255, 128, 0, 255),
};

struct Capture {
    Capture() {}
    Capture(uint64 begin_time, Color color)
        : begin_time(begin_time), color(color) {}
    Capture(uint64 begin_time, uint64 end_time, Color color)
        : begin_time(begin_time), end_time(end_time), color(color) {}

    void End(uint64 time) { end_time = time; }

    uint64 begin_time = 0;
    uint64 end_time = 0;
    Color color;
};

class Track : public NoCopy {
    static const unsigned CaptureMaxCount = 2048;

    FixedArray<Capture, CaptureMaxCount> captures;
    unsigned start = 0;
    unsigned end = 0;

    static unsigned Next(unsigned i) { return (i + 1) % CaptureMaxCount; }

public:
    void Clear() {
        start = 0;
        end = 0;
    }

    void Begin(uint64 time, Color color) {
        if (Next(end) != start) {
            captures[end] = Capture(time, color);
        }
    }

    void End(uint64 time) {
        if (Next(end) != start) {
            captures[end].End(time);
            end = Next(end);
        }
    }

    void BeginEnd(uint64 begin_time, uint64 end_time, Color color) {
        if (Next(end) != start) {
            captures[end] = Capture(begin_time, end_time, color);
            end = Next(end);
        }
    }

    template<typename F> void Process(F func) {
        bool stop = false;
        while ((end != start) && !stop) {
            if (Next(end) != start) {
                if (func(captures[start], stop)) {
                    start = Next(start);
                }
            }
        }
    }
};

class Profile : public NoCopy {
    static const unsigned TrackMaxCount = 2;

    FixedArray<Track, TrackMaxCount> tracks;

public:
    void BeginCPU(uint64 time, Color color) { tracks[1].Begin(time, color); }
    void EndCPU(uint64 time) { tracks[1].End(time); }
    void BeginEndGPU(uint64 begin_time, uint64 end_time, Color color) { tracks[0].BeginEnd(begin_time, end_time, color); }

    FixedArray<Track, TrackMaxCount>& Tracks() { return tracks; }
};

