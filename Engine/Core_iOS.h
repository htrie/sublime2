
namespace Text {
    static inline void Format(char* buffer, size max_size, const char* format, ...) {
        va_list args;
        va_start(args, format);
        vsnprintf(buffer, max_size, format, args);
        va_end(args);
    }
    
    static inline void Print(const char* text) {
        printf("%s", text);
    }
}

class Exception {
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
        snprintf(this->text, TextMaxSize, "errno: %u", errno);
    }
    
    const char* Text() const { return text; }
};

class Timer : public NoCopy {
    uint64 last_time = 0;
    float elapsed_time = 0.f;
    float modifier = 1.f;
    
public:
    uint64 Now() {
        timeval t;
        gettimeofday(&t, NULL);
        return t.tv_sec * 1000000.0 + t.tv_usec;
    }
    
    void SetModifier(float factor) {
        modifier = factor;
    }
    
    void Tick() {
        const uint64 now = Now();
        const float real_elapsed_time = Math::Min((float)((double)(now - last_time) * 0.000001), 1.f);
        elapsed_time = Math::Clamp(real_elapsed_time, 1.f / 200.f, 1.f / 30.f);
        elapsed_time *= modifier;
        last_time = now;
    }
    
    float ElapsedTime() const { return elapsed_time; }
};

class Thread {
    pthread_t thread;
    
public:
    Thread() { }
    Thread(void*(*func)(void*), void* data) {
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setstacksize(&attr, 2 * 1024 * 1024);
        pthread_create(&thread, &attr, func, data);
        pthread_attr_destroy(&attr);
    }
    ~Thread() { pthread_join(thread, nullptr); }
    
    static void Sleep(unsigned milliseconds) { usleep(milliseconds * 1000); }
};

namespace Memory {
    static const size PageSize = 64 * 1024;

    void* Malloc(size size) {
        void* mem = nullptr;
        posix_memalign(&mem, PageSize , size);
        return mem;
    }
    
    void Free(void* mem, size size) {
        free(mem);
    }
    
    void* AllocatePage() {
        void* mem = nullptr;
        posix_memalign(&mem, PageSize, PageSize);
        return mem;
    }
    
    void FreePage(void* mem) {
        free(mem);
    }

    class Alloc { // TODO: Remove.
        void* mem = nullptr;
        size size = 0;
        
    public:
        Alloc() {}
        Alloc(void* data, ::size size) : mem(Malloc(size)), size(size) { memcpy(mem, data, size); }
        Alloc(Alloc& other) : mem(other.mem), size(other.size) { other.mem = nullptr; other.size = 0; }
        ~Alloc() { Free(mem, size); }
        
        void* Pointer() const { return mem; }
        ::size Size() const { return size; }
    };
};

class Descriptor {
    int desc = -1;
    void* mem = nullptr;
    size mem_size = 0;
    
    void Open(const char* path, bool read_only, bool create) {
        desc = open(path, read_only ? O_RDONLY : create ? O_CREAT | O_RDWR : O_WRONLY, S_IRWXU);
        DEBUG_ONLY(if (desc == -1) throw Exception();)
    }
    
    void Close() {
        if (desc != -1)
            close(desc);
    }
    
    void Resize(size file_size) {
        if (ftruncate(desc, file_size) == -1) {
            DEBUG_ONLY(throw Exception();)
        }
    }
    
    void Map(bool read_only, bool copy_on_write) {
        struct stat st;
        fstat(desc, &st);
        mem_size = st.st_size;
        DEBUG_ONLY(if (mem_size == 0) throw Exception();)
        mem = mmap(nullptr, mem_size, read_only ? PROT_READ : PROT_READ | PROT_WRITE, copy_on_write ? MAP_PRIVATE : MAP_SHARED, desc, 0);
        DEBUG_ONLY(if (mem == MAP_FAILED) throw Exception();)
    }
    
    void Unmap() {
        munmap(mem, mem_size);
    }
    
    uint64 ModifiedTime() const {
        struct stat st;
        fstat(desc, &st);
        return st.st_mtimespec.tv_sec * 1000000 + st.st_mtimespec.tv_nsec / 1000;
    }
    
public:
    Descriptor() {}
    Descriptor(const char* path, bool map, bool read_only, bool copy_on_write, bool create, size file_size) {
        Open(path, read_only, create); // TODO: Copy on write.
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
    
    void* Pointer() const { return mem; }
    size Size() const { return mem_size; }
    
    static bool Exist(const char* path) {
        return access(path, F_OK) != -1;
    }
    
    static bool Newer(const Descriptor& descriptor0, const Descriptor& descriptor1) {
        const uint64 time0 = descriptor0.ModifiedTime();
        const uint64 time1 = descriptor1.ModifiedTime();
        return time0 > time1;
    }
    
    static void Copy(const char* src_filename, const char* dst_filename) { // TODO: Remove.
        // NOT IMPLEMENTED
    }
    
    static void Move(const char* src_filename, const char* dst_filename) { // TODO: Remove.
        // NOT IMPLEMENTED
    }
    
    static void Delete(const char* path) {
        remove(path);
    }
};

class Directory {
public:
    enum class Action {
        None = 0,
        Added,
        Modified,
        RenamedNew,
    };
    
    template<typename F> void Watch(const char* path, F func) {
        // NOT IMPLEMENTED
    }
    
    static void Create(const char* path) {
        const auto res = mkdir(path, 0777);
        if ((res == -1) && (errno == EEXIST)) return;
        DEBUG_ONLY(if (res == -1) throw Exception();)
    }
    
    template<typename F> static void ProcessFiles(const char* path, F func) {
        struct dirent* entry;
        DIR* dir = opendir(path);
        if (dir == nullptr)
            return;
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_type != DT_DIR) {
                func(entry->d_name);
            }
        }
        closedir(dir);
    }
    
    template<typename F> static void ProcessFolders(const char* path, F func) {
        struct dirent* entry;
        DIR* dir = opendir(path);
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_type == DT_DIR) {
                char sub[1024];
                if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                    continue;
                snprintf(sub, sizeof(sub), "%s%s/", path, entry->d_name);
                func(sub);
                ProcessFolders(sub, func);
            }
        }
        closedir(dir);
    }
};

class Library {
    bool EndsWith(const char* name, const char* cmp) {
        const size name_length = strlen(name);
        const size cmp_length = strlen(cmp);
        if (name_length < cmp_length) return false;
        const size offset = name_length - cmp_length;
        return strcmp(&name[offset], cmp) == 0;
    }
    
public:
    Library() {}
    Library(const char* name) {
    }
    
    ~Library() {
    }
    
    template <typename T> T Address(const char* name) {
        return (T)nullptr;
    }
};

class Process {
    static const int ArgsMaxLength = 4096;
    static const int ArgsMaxCount = 32;
    
    static unsigned CopyArgs(const char* command, char* args) {
        const unsigned length = (unsigned)strlen(command);
        DEBUG_ONLY(if (length > ArgsMaxLength) throw Exception("Command too long");)
        strncpy(args, command, ArgsMaxLength);
        args[length] = 0; // EOS
        return length;
    }
    
    static void ParseArgs(char* args, const unsigned length, char** argv) {
        argv[0] = args;
        unsigned count = 1;
        unsigned index = 0;
        while (index < length) {
            if (args[index] == ' ') {
                args[index] = 0; // EOS
                DEBUG_ONLY(if (count == ArgsMaxCount) throw Exception("Too many args");)
                argv[count++] = &args[index+1];
            }
            index++;
        }
        argv[count++] = nullptr;
    }
    
    static int ChildExecute(char** argv) {
        int out_pipe[2];
        pipe(out_pipe);
        pid_t pid = fork();
        if (pid == 0) {
            close(out_pipe[0]);
            dup2(out_pipe[1], STDOUT_FILENO);
            dup2(out_pipe[1], STDERR_FILENO);
            execv(argv[0], (char**)argv);
        }
        close(out_pipe[1]);
        int status = 0;
        waitpid(pid, &status, 0);
        DEBUG_ONLY(if (status == -1) throw Exception();)
        return out_pipe[0];
    }
    
    static void ReadOutput(int read_pipe, char* out_buffer, size& out_size, size out_max_size) {
        struct stat out_stat;
        if (fstat(read_pipe, &out_stat) == -1) {
            DEBUG_ONLY(throw Exception();)
        }
        const auto out_length = (size)out_stat.st_size;
        out_size = out_length < out_max_size ? out_length : out_max_size-1;
        const auto offset = read(read_pipe, out_buffer, out_size);
        DEBUG_ONLY(if (offset  == -1) throw Exception();)
        out_buffer[offset] = 0;
    }
    
public:
    static void Execute(const char* command, char* out_buffer, size& out_size, size out_max_size) {
        char args[ArgsMaxLength];
        const auto length = CopyArgs(command, args);
        const char* argv[ArgsMaxCount];
        ParseArgs(args, length, (char**)argv);
        ReadOutput(ChildExecute((char**)argv), out_buffer, out_size, out_max_size);
    }
};
