
namespace Text {
    static void Format(char* buffer, size max_size, const char* format, ...) {
        va_list args;
        va_start(args, format);
        vsnprintf(buffer, max_size, format, args);
        va_end(args);
    }

    static void Print(const char* text) {
        printf("%s", text);
        OutputDebugStringA(text);
    }
}

class Exception : public NoCopy {
    static const size TextMaxSize = 4096;
    char text[TextMaxSize];

public:
    Exception(const char* text) {
        const size length = Math::Length(text);
        const size size = length < TextMaxSize ? length : TextMaxSize - 1;
        memcpy(this->text, text, size);
        this->text[size] = 0;
    }

    Exception() {
        PVOID msg = nullptr;
        const DWORD dw = GetLastError();
        FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&msg, 0, NULL);
        new (this) Exception((char*)msg);
        LocalFree(msg);
    }

    const char* Text() const { return text; }
};

class Timer : public NoCopy {
    LARGE_INTEGER frequency;
    uint64 last_time = 0;
    float elapsed_time = 0.f;

public:
    Timer() {
        QueryPerformanceFrequency(&frequency);
    }

    uint64 Now() const {
        LARGE_INTEGER counter;
        QueryPerformanceCounter(&counter);
        return 1000000 * counter.QuadPart / frequency.QuadPart;
    }

    void Tick() {
        const uint64 now = Now();
        const float real_elapsed_time = Math::Min((float)((double)(now - last_time) * 0.000001), 1.f);
        elapsed_time = Math::Clamp(real_elapsed_time, 1.f / 200.f, 1.f / 30.f);
        last_time = now;
    }

    float ElapsedTime() const { return elapsed_time; }
};

class Handle : public NoCopy {
    HANDLE handle = INVALID_HANDLE_VALUE;

    Handle(Handle& other) = delete;
    Handle& operator=(const Handle& other) = delete;

public:
    Handle() {}
    Handle(HANDLE handle) : handle(handle) {}
    ~Handle() {
        if (handle != INVALID_HANDLE_VALUE) {
            CloseHandle(handle);
        }
    }

    Handle& operator=(HANDLE other) {
        handle = other;
        return *this;
    }

    operator bool() const { return handle != INVALID_HANDLE_VALUE; }
    const HANDLE& Native() const { return handle; }
    HANDLE& Native() { return handle; }
};

class Thread : public NoCopy {
    Handle thread;

public:
    Thread() {}
    Thread(void*(*func)(void*), void* data) { thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)func, data, 0, NULL); }
    ~Thread() { if (thread) { WaitForSingleObject(thread.Native(), INFINITE); } }

    static void Sleep(unsigned milliseconds) { ::Sleep(milliseconds); }
};

namespace Memory {
    void* Malloc(size size) {
        void* mem = malloc(size);
        memset(mem, 0, size);
        return mem;
    }
    void Free(void* mem, size size) {
        if (mem)
            free(mem);
    }
};

class Descriptor : public NoCopy {
    Handle file;
    WIN32_MEMORY_RANGE_ENTRY range;

    void Open(const char* path, bool read_only, bool create) {
        do { file = CreateFileA(path, GenericFlags(read_only), ShareFlags(read_only), nullptr, OpenFlags(create), AttributeFlags(read_only), nullptr);
        } while (GetLastError() == ERROR_SHARING_VIOLATION);
        DEBUG_ONLY(if (!file) throw Exception();)
    }

    void Resize(size file_size) {
        BOOL b = SetFilePointerEx(file.Native(), (LARGE_INTEGER&)file_size, NULL, FILE_BEGIN);
        DEBUG_ONLY(if (b == FALSE) throw Exception();)
        b = SetEndOfFile(file.Native());
        DEBUG_ONLY(if (b == FALSE) throw Exception();)
    }

    void Map(bool read_only, bool copy_on_write) {
        size file_size = 0;
        const BOOL b = GetFileSizeEx(file.Native(), (PLARGE_INTEGER)&file_size);
        DEBUG_ONLY(if (b == FALSE) throw Exception();)
        Handle map = CreateFileMapping(file.Native(), nullptr, PageFlags(read_only, copy_on_write), 0, 0, nullptr);
        DEBUG_ONLY(if (!map) throw Exception();)
        void* mem = MapViewOfFile(map.Native(), MapFlags(read_only, copy_on_write), 0, (DWORD)0, file_size);
        DEBUG_ONLY(if (mem == nullptr) throw Exception();)
        range.VirtualAddress = mem;
        range.NumberOfBytes = file_size;
        PrefetchVirtualMemory(GetCurrentProcess(), 1, &range, 0);
    }

    void Unmap() {
        if (file && range.VirtualAddress)
            UnmapViewOfFile(range.VirtualAddress);
    }

    uint64 ModifiedTime() const {
        FILE_BASIC_INFO info;
        const BOOL b = GetFileInformationByHandleEx(file.Native(), FileBasicInfo, &info, sizeof(FILE_BASIC_INFO));
        DEBUG_ONLY(if (b == FALSE) throw Exception();)
        return info.LastWriteTime.QuadPart;
    }

    static DWORD GenericFlags(bool read_only) { return read_only ? GENERIC_READ : GENERIC_READ | GENERIC_WRITE; }
    static DWORD ShareFlags(bool read_only) { return read_only ? FILE_SHARE_READ : 0; }
    static DWORD OpenFlags(bool create) { return create ? OPEN_ALWAYS : OPEN_EXISTING; }
    static DWORD AttributeFlags(bool read_only) { return read_only ? FILE_ATTRIBUTE_READONLY : FILE_ATTRIBUTE_NORMAL; }
    static DWORD PageFlags(bool read_only, bool copy_on_write) { return read_only ? PAGE_READONLY : copy_on_write ? PAGE_WRITECOPY : PAGE_READWRITE; }
    static DWORD MapFlags(bool read_only, bool copy_on_write) { return read_only ? FILE_MAP_READ : copy_on_write ? FILE_MAP_COPY : FILE_MAP_WRITE; }

public:
    Descriptor() {}
    Descriptor(const char* path, bool map, bool read_only, bool copy_on_write, bool create, size file_size) {
        Open(path, read_only, create);
        if (file_size) {
            Resize(file_size);
        }
        if (map) {
            Map(read_only, copy_on_write);
        }
    }

    ~Descriptor() {
        Unmap();
    }

    void* Pointer() const { return range.VirtualAddress; }
    size Size() const { return range.NumberOfBytes; }

    static bool Exist(const char* path) {
        const DWORD dwAttrib = GetFileAttributesA(path);
        return (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
    }

    static bool Newer(const Descriptor& descriptor0, const Descriptor& descriptor1) {
        const uint64 time0 = descriptor0.ModifiedTime();
        const uint64 time1 = descriptor1.ModifiedTime();
        return CompareFileTime((FILETIME*)&time0, (FILETIME*)&time1) >= 0;
    }

    static void Copy(const char* src_filename, const char* dst_filename) {
        do { CopyFile(src_filename, dst_filename, false);
        } while (GetLastError() == ERROR_SHARING_VIOLATION);
    }

    static void Move(const char* src_filename, const char* dst_filename) {
        do { MoveFileExA(src_filename, dst_filename, MOVEFILE_REPLACE_EXISTING);
        } while (GetLastError() == ERROR_SHARING_VIOLATION);
    }

    static void Delete(const char* path) {
        do { DeleteFileA(path);
        } while (GetLastError() == ERROR_SHARING_VIOLATION);
    }
};

class Directory : public NoCopy {
public:
    enum class Action {
        None = 0,
        Added,
        Modified,
        RenamedNew,
    };

private:
    Handle handle;

    HANDLE OpenExisting(const char* path) {
        return CreateFileA(path, FILE_LIST_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
    }

    bool ReadChanges(char* buffer, DWORD max_size, DWORD& read_count) {
        memset(buffer, 0, max_size);
        read_count = 0;
        const BOOL success = ReadDirectoryChangesW(handle.Native(), buffer, max_size, TRUE, FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION, &read_count, NULL, NULL);
        if (GetLastError() == ERROR_OPERATION_ABORTED)
            return false;
        DEBUG_ONLY(if (!success) throw Exception();)
        return true;
    }

    template<typename F> void ProcessChanges(char* buffer, F func) {
        auto info = (FILE_NOTIFY_INFORMATION*)buffer;
        size offset = 0;
        do {
            size count = 0;
            char filename[MAX_PATH];
            wcstombs_s(&count, filename, info->FileName, info->FileNameLength);
            func(filename, ConvertAction(info->Action));
            offset = info->NextEntryOffset;
            info = (FILE_NOTIFY_INFORMATION*)&buffer[offset];
        } while (offset > 0);
    }

    Action ConvertAction(DWORD action) {
        switch (action) {
        case FILE_ACTION_ADDED: return Action::Added;
        case FILE_ACTION_MODIFIED: return Action::Modified;
        case FILE_ACTION_RENAMED_NEW_NAME: return Action::RenamedNew;
        default: return Action::None;
        }
    }

public:
    template<typename F> void Watch(const char* path, F func) {
        handle = OpenExisting(path);
        DEBUG_ONLY(if (!handle) throw Exception();)
        while (TRUE) {
            const DWORD max_size = 16 * 1024;
            char buffer[max_size];
            DWORD read_count = 0;
            if (!ReadChanges(buffer, max_size, read_count))
                return;
            ProcessChanges(buffer, func);
        }
    }

    ~Directory() {
        CancelIoEx(handle.Native(), NULL);
    }

    static void Create(const char* path) {
        const BOOL b = CreateDirectoryA(path, NULL);
        if (GetLastError() == ERROR_ALREADY_EXISTS)
            return;
        DEBUG_ONLY(if (b == FALSE) throw Exception();)
    }

    template<typename F> static void ProcessFiles(const char* path, F func) {
        TCHAR filter[MAX_PATH];
        strncpy_s(filter, MAX_PATH, path, MAX_PATH);
        strncpy_s(filter + strlen(filter), MAX_PATH, "/*", MAX_PATH);
        WIN32_FIND_DATA find_data;
        HANDLE handle = FindFirstFile(filter, &find_data);
        do {
            if (handle != INVALID_HANDLE_VALUE) {
                if ((find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
                    func(find_data.cFileName);
            }
        } while (FindNextFile(handle, &find_data) != 0);
        FindClose(handle);
    }

    template<typename F> static void ProcessFolders(const char* path, F func) {
        TCHAR dir[MAX_PATH];
        strncpy_s(dir, MAX_PATH, path, MAX_PATH);
        strncpy_s(dir + strlen(dir), MAX_PATH, "/*", MAX_PATH);
        WIN32_FIND_DATA find_data;
        HANDLE handle = FindFirstFile(dir, &find_data);
        do {
            if (handle != INVALID_HANDLE_VALUE) {
                if ((find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) > 0) {
                    if (find_data.cFileName[0] != '.') {
                        strncpy_s(dir, MAX_PATH, path, MAX_PATH);
                        strncpy_s(dir + strlen(dir), MAX_PATH, find_data.cFileName, MAX_PATH);
                        strncpy_s(dir + strlen(dir), MAX_PATH, "/", MAX_PATH);
                        func(dir);
                        ProcessFolders(dir, func);
                    }
                }
            }
        } while (FindNextFile(handle, &find_data) != 0);
        FindClose(handle);
    }
};

class Library : public NoCopy {
    HMODULE module = NULL;

public:
    Library() {}
    Library(const char* name) {
        module = LoadLibrary(name);
        DEBUG_ONLY(if (module == NULL) throw Exception();)
    }

    ~Library() {
        if (module) {
            FreeLibrary(module);
        }
    }

    template <typename T> T Address(const char* name) {
        FARPROC address = GetProcAddress(module, name);
        return (T)address;
    }
};

class Process : public NoCopy {
    static bool CreateHandles(Handle& out_read, Handle& out_write) {
        SECURITY_ATTRIBUTES security_attributes;
        security_attributes.nLength = sizeof(SECURITY_ATTRIBUTES);
        security_attributes.bInheritHandle = TRUE;
        security_attributes.lpSecurityDescriptor = NULL;
        if (!CreatePipe(&out_read.Native(), &out_write.Native(), &security_attributes, 0))
            return false;
        if (!SetHandleInformation(out_read.Native(), HANDLE_FLAG_INHERIT, 0))
            return false;
        return true;
    }

    static bool Start(const char* command, HANDLE write) {
        PROCESS_INFORMATION process_info;
        ZeroMemory(&process_info, sizeof(PROCESS_INFORMATION));
        STARTUPINFO startup_info;
        ZeroMemory(&startup_info, sizeof(STARTUPINFO));
        startup_info.cb = sizeof(STARTUPINFO);
        startup_info.hStdError = write;
        startup_info.hStdOutput = write;
        startup_info.dwFlags |= STARTF_USESTDHANDLES;
        if (!CreateProcess(NULL, (LPSTR)command, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &startup_info, &process_info))
            return false;
        WaitForSingleObject(process_info.hProcess, INFINITE);
        return true;
    }

    static void ReadPipe(char* out, size& out_size, size out_max_size, HANDLE read) {
        out_size = 0;
        for (;;) {
            const unsigned size = 1024;
            DWORD read_size;
            CHAR buffer[size];
            if (!ReadFile(read, buffer, size, &read_size, NULL))
                break;
            if (out_size + read_size < out_max_size) {
                memcpy(&out[out_size], buffer, read_size);
                out_size += read_size;
            }
        }
    }

public:
    static void Execute(const char* command, char* out, size& out_size, size out_max_size) {
        Handle out_read;
        {
            Handle out_write;
            if (!CreateHandles(out_read, out_write))
                return;
            if (!Start(command, out_write.Native()))
                return;
        }
        ReadPipe(out, out_size, out_max_size, out_read.Native());
    }
};
