
namespace XML {
    class Base : public NoCopy {
    public:
        char* Name() const { return name ? name : nullptr; }
        unsigned NameSize() const { return name ? name_size : 0; }
        char* Value() const { return value ? value : nullptr; }
        unsigned Valuesize() const { return value ? value_size : 0; }

        void SetName(const char* name, unsigned size) {
            this->name = const_cast<char*>(name);
            name_size = size;
        }

        void SetValue(const char* value, unsigned size) {
            this->value = const_cast<char*>(value);
            value_size = size;
        }

    private:
        char* name = nullptr;
        char* value = nullptr;
        unsigned name_size = 0;
        unsigned value_size = 0;
    };

    struct Attribute : public Base {
    public:
        Attribute* prev_attribute = nullptr;
        Attribute* next_attribute = nullptr;
    };

    struct Node : public Base {
    public:
        Node* NextSibling(const char* name = 0, unsigned name_size = 0) const {
            if (name) {
                if (name_size == 0)
                    name_size = Measure(name);
                for (Node* sibling = next_sibling; sibling; sibling = sibling->next_sibling)
                    if (Compare(sibling->Name(), sibling->NameSize(), name, name_size))
                        return sibling;
                return 0;
            } else
                return next_sibling;
        }

        Node* FirstNode(const char* name = 0, unsigned name_size = 0) const {
            if (name) {
                if (name_size == 0)
                    name_size = Measure(name);
                for (Node* child = first_node; child; child = child->NextSibling())
                    if (Compare(child->Name(), child->NameSize(), name, name_size))
                        return child;
                return 0;
            } else
                return first_node;
        }

        Attribute* FirstAttribute(const char* name = 0, unsigned name_size = 0) const {
            if (name) {
                if (name_size == 0)
                    name_size = Measure(name);
                for (Attribute* attribute = first_attribute; attribute; attribute = attribute->next_attribute)
                    if (Compare(attribute->Name(), attribute->NameSize(), name, name_size))
                        return attribute;
                return 0;
            } else
                return first_attribute;
        }

        void AppendNode(Node* child) {
            if (FirstNode()) {
                child->prev_sibling = last_node;
                last_node->next_sibling = child;
            } else {
                child->prev_sibling = 0;
                first_node = child;
            }
            last_node = child;
            child->next_sibling = 0;
        }

        void AppendAttribute(Attribute* attribute) {
            if (FirstAttribute()) {
                attribute->prev_attribute = last_attribute;
                last_attribute->next_attribute = attribute;
            } else {
                attribute->prev_attribute = 0;
                first_attribute = attribute;
            }
            last_attribute = attribute;
            attribute->next_attribute = 0;
        }

        const String Text(const char* attribute) const {
            unsigned length = 0;
            const char* s = Text(attribute, &length);
            return String(s, length);
        }

        bool Bool(const char* attribute, bool b) const {
            unsigned length = 0;
            const char* name = Text(attribute, &length);
            return (name != nullptr) ? (String(name, length) == "true") : b;
        }

        int Int(const char* attribute, int i) const {
            int val = i;
            unsigned length = 0;
            const char* name = Text(attribute, &length);
            if (name)
                Scan::Int(name, length, &val);
            return val;
        }

        float Float(const char* attribute, float f) const {
            float val = f;
            unsigned length = 0;
            const char* name = Text(attribute, &length);
            if (name)
                Scan::Float(name, length, &val);
            return val;
        }

        uint32 Hash32(const char* attribute) const {
            unsigned length = 0;
            const char* name = Text(attribute, &length);
            return (name != nullptr) ? Hash::Fnv32((char*)name, (int)length) : (uint32)-1;
        }

        uint64 Hash64(const char* attribute) const {
            unsigned length = 0;
            const char* name = Text(attribute, &length);
            return (name != nullptr) ? Hash::Fnv64((char*)name, (int)length) : (uint64)-1;
        }

        bool Color(uint32& color, const char* attribute, uint32 c) const {
            color = c;
            float a[4];
            a[0] = a[1] = a[2] = a[3] = 0.f;
            unsigned length = 0;
            const char* name = Text(attribute, &length);
            if (name)
                Scan::Vector4(name, length, a);
            color = Math::RGBA(
                (unsigned)(a[0] * 255.f),
                (unsigned)(a[1] * 255.f),
                (unsigned)(a[2] * 255.f),
                (unsigned)(a[3] * 255.f));
            return name != nullptr;
        }

        bool Vec2(float a[2], const char* attribute, float f) const {
            a[0] = a[1] = f;
            unsigned length = 0;
            const char* name = Text(attribute, &length);
            if (name)
                Scan::Vector2(name, length, a);
            return name != nullptr;
        }

        bool Vec3(float a[3], const char* attribute, float f) const {
            a[0] = a[1] = a[2] = f;
            unsigned length = 0;
            const char* name = Text(attribute, &length);
            if (name)
                Scan::Vector3(name, length, a);
            return name != nullptr;
        }

        bool Vec4(float a[4], const char* attribute, float f) const {
            a[0] = a[1] = a[2] = a[3] = f;
            unsigned length = 0;
            const char* name = Text(attribute, &length);
            if (name)
                Scan::Vector4(name, length, a);
            return name != nullptr;
        }

        bool Quat(float a[4], const char* attribute) const {
            a[0] = a[1] = a[2] = 0.f; a[3] = 1.f;
            unsigned length = 0;
            const char* name = Text(attribute, &length);
            if (name)
                Scan::Vector4(name, length, a);
            return name != nullptr;
        }

    private:
        const char* Text(const char* attribute, unsigned* out_length) const {
            auto attr = FirstAttribute(attribute);
            if (attr && out_length)
                *out_length = attr->Valuesize();
            return attr ? attr->Value() : nullptr;
        }

        static inline unsigned Measure(const char* p) {
            const char* tmp = p;
            while (*tmp)
                ++tmp;
            return (unsigned)(tmp - p);
        }

        static inline bool Compare(const char* p1, unsigned size1, const char* p2, unsigned size2) {
            if (size1 != size2)
                return false;

            for (const char* end = p1 + size1; p1 < end; ++p1, ++p2)
                if (*p1 != *p2)
                    return false;
            return true;
        }

        Node* first_node = nullptr;
        Node* last_node = nullptr;
        Attribute* first_attribute = nullptr;
        Attribute* last_attribute = nullptr;
        Node* prev_sibling = nullptr;
        Node* next_sibling = nullptr;
    };

    struct Doc : public Node {
        static const unsigned NodeMaxCount = 512;
        static const unsigned AttributeMaxCount = 1024;

        Array<Node, NodeMaxCount> instance_list;
        Array<Attribute, AttributeMaxCount> attribute_list;

        static void ParseBom(char*& text) {
            if (static_cast<unsigned char>(text[0]) == 0xEF &&
                static_cast<unsigned char>(text[1]) == 0xBB &&
                static_cast<unsigned char>(text[2]) == 0xBF) {
                text += 3;
            }
        }

        static Node* ParseXMLDeclaration(char*& text) {
            while (text[0] != char('?') || text[1] != char('>')) {
                if (!text[0]) throw Exception("Unexpected end of data");
                ++text;
            }
            text += 2;
            return 0;
        }

        static Node* ParseComment(char*& text) {
            while (text[0] != char('-') || text[1] != char('-') || text[2] != char('>')) {
                if (!text[0]) throw Exception("Unexpected end of data");
                ++text;
            }
            text += 3;
            return 0;
        }

        static Node* ParseDoctype(char*& text) {
            while (*text != char('>')) {
                switch (*text) {
                case char('[') : {
                    ++text;
                    int depth = 1;
                    while (depth > 0) {
                        switch (*text) {
                        case char('[') : ++depth; break;
                        case char(']') : --depth; break;
                        case 0: throw Exception("Unexpected end of data");
                        }
                        ++text;
                    }
                    break;
                }
                case char('\0') : throw Exception("Unexpected end of data");
                default:
                    ++text;
                }
            }
            text += 1;
            return 0;
        }

        static Node* ParsePi(char*& text) {
            while (text[0] != char('?') || text[1] != char('>')) {
                if (*text == char('\0')) throw Exception("Unexpected end of data");
                ++text;
            }
            text += 2;
            return 0;
        }

        static char ParseAndAppendData(Node* node, char*& text, char* contents_start) {
            text = contents_start;

            char* value = text;
            Skip<TextPred>(text);

            if (*node->Value() == char('\0'))
                node->SetValue(value, (unsigned)(text - value));

            return *text;
        }

        static Node* ParseCData(char*& text) {
            while (text[0] != char(']') || text[1] != char(']') || text[2] != char('>')) {
                if (!text[0]) throw Exception("Unexpected end of data");
                ++text;
            }
            text += 3;
            return 0;
        }

        void ParseNodeAttributes(char*& text, Node* node) {
            while (AttributeNamePred::Test(*text)) {
                char* name = text;
                ++text;
                Skip<AttributeNamePred>(text);
                if (text == name) throw Exception("Expected attribute name");

                auto& attribute = attribute_list.Add();
                attribute.SetName(name, (unsigned)(text - name));
                node->AppendAttribute(&attribute);

                Skip<WhitespacePred>(text);

                if (*text != char('=')) throw Exception("Expected =");
                ++text;

                Skip<WhitespacePred>(text);

                char quote = *text;
                if (quote != char('\'') && quote != char('"')) throw Exception("Expected ' or \"");
                ++text;

                char* value = text;
                if (quote == char('\''))
                    Skip<AttributeValueQuotePred>(text);
                else
                    Skip<AttributeValueDoubleQuotePred>(text);

                attribute.SetValue(value, (unsigned)(text - value));

                if (*text != quote) throw Exception("Expected ' or \"");
                ++text;

                Skip<WhitespacePred>(text);
            }
        }

        void ParseNodeContents(char*& text, Node* node) {
            while (1) {
                char* contents_start = text;
                Skip<WhitespacePred>(text);
                char next_char = *text;

            after_data_node:
                switch (next_char) {
                case char('<') :
                    if (text[1] == char('/')) {
                        text += 2;

                        Skip<NodeNamePred>(text);

                        Skip<WhitespacePred>(text);
                        if (*text != char('>')) throw Exception("Expected >");
                        ++text;
                        return;
                    } else {
                        ++text;
                        if (Node* child = ParseNode(text))
                            node->AppendNode(child);
                    }
                               break;
                case char('\0') : throw Exception("Unexpected end of data");
                default:
                    next_char = ParseAndAppendData(node, text, contents_start);
                    goto after_data_node;
                }
            }
        }

        Node* ParseElement(char*& text) {
            auto& node = instance_list.Add();

            char* name = text;
            Skip<NodeNamePred>(text);
            if (text == name) throw Exception("Expected element name");
            node.SetName(name, (unsigned)(text - name));

            Skip<WhitespacePred>(text);

            ParseNodeAttributes(text, &node);

            if (*text == char('>')) {
                ++text;
                ParseNodeContents(text, &node);
            } else if (*text == char('/')) {
                ++text;
                if (*text != char('>')) throw Exception("Expected >");
                ++text;
            } 
            else throw Exception("Expected >");

            return &node;
        }

        Node* ParseNode(char*& text) {
            switch (text[0]) {
            default:
                return ParseElement(text);
            case char('?') :
                ++text;
                if ((Scan::Compare(text, "xml", 3) || Scan::Compare(text, "XML", 3)) && WhitespacePred::Test(text[3])) {
                    text += 4;
                    return ParseXMLDeclaration(text);
                } else {
                    return ParsePi(text);
                }
            case char('!') :
                switch (text[1]) {
                case char('-') :
                    if (text[2] == char('-')) {
                        text += 3;
                        return ParseComment(text);
                    }
                    break;
                case char('[') :
                    if (Scan::Compare(&text[2], "CDATA[", 6)) {
                        text += 8;
                        return ParseCData(text);
                    }
                    break;
                case char('D') :
                    if (Scan::Compare(&text[2], "OCTYPE", 6) && WhitespacePred::Test(text[8])) {
                        text += 9;
                        return ParseDoctype(text);
                    }
                }

                ++text;
                while (*text != char('>')) {
                    if (*text == 0) throw Exception("Unexpected end of data");
                    ++text;
                }
                ++text;
                return 0;
            }
        }

        void Parse(char* text) {
            ParseBom(text);

            while (1) {
                Skip<WhitespacePred>(text);
                if (*text == 0)
                    break;

                if (*text == char('<')) {
                    ++text;
                    if (Node* node = ParseNode(text))
                        AppendNode(node);
                }
                else throw Exception("Expected <");
            }
        }

        struct WhitespacePred {
            static unsigned char Test(char ch) {
                switch (ch) {
                case 0x09:
                case 0x0A:
                case 0x0D:
                case 0x20: return 1;
                default: return 0;
                }
            }
        };

        struct NodeNamePred {
            static unsigned char Test(char ch) {
                switch (ch) {
                case 0x00:
                case 0x09:
                case 0x0A:
                case 0x0D:
                case 0x20:
                case 0x2F:
                case 0x3E:
                case 0x3F: return 0;
                default: return 1;
                }
            }
        };

        struct AttributeNamePred {
            static unsigned char Test(char ch) {
                switch (ch) {
                case 0x00:
                case 0x09:
                case 0x0A:
                case 0x0D:
                case 0x20:
                case 0x21:
                case 0x2F:
                case 0x3C:
                case 0x3D:
                case 0x3E:
                case 0x3F: return 0;
                default: return 1;
                }
            }
        };

        struct TextPred {
            static unsigned char Test(char ch) {
                switch (ch) {
                case 0x00:
                case 0x3C: return 0;
                default: return 1;
                }
            }
        };

        struct AttributeValueQuotePred {
            static unsigned char Test(char ch) {
                switch (ch) {
                case 0x00:
                case 0x27: return 0;
                default: return 1;
                }
            }
        };

        struct AttributeValueDoubleQuotePred {
            static unsigned char Test(char ch) {
                switch (ch) {
                case 0x00:
                case 0x22: return 0;
                default: return 1;
                }
            }
        };

        template<class StopPred> static void Skip(char*& text) {
            char* tmp = text;
            while (StopPred::Test(*tmp))
                ++tmp;
            text = tmp;
        }

    public:
        Doc(char* text) {
            Parse(text);
        }
    };
}

class PLY : public NoCopy {
public:
    PLY(const char* text) {
        ParseHeader(text);
        ParseElements(text);
    }

    class Value : public NoCopy {
    public:
        Value() {}
        Value(const char* p, size size) : value(p), value_size(size) {}

        void SetNext(Value* next) {
            this->next = next;
        }

        Value* Next() {
            return next;
        }

        int Int() {
            int i = 0;
            Scan::Int(value, value_size, &i);
            return i;
        }

        float Float() {
            float f = 0;
            Scan::Float(value, value_size, &f);
            return f;
        }

    private:
        Value* next = nullptr;
        const char* value = nullptr;
        size value_size = 0;
    };

    Value* First(const char* name) {
        Value* result = nullptr;
        elements.Find([&](auto& element) {
            return element.properties.Find([&](auto& property) {
                if (strncmp(name, property.name, property.name_size) == 0) {
                    result = property.first;
                    return true;
                }
                return false;
            });
        });
        return result;
    }

    unsigned Count(const char* name) {
        unsigned result = 0;
        elements.Find([&](auto& element) {
            if (strncmp(name, element.name, element.name_size) == 0) {
                result = element.count;
                return true;
            }
            return false;
        });
        return result;
    }

private:
    static const unsigned ValueMaxCount = 36 * 1024;
    static const unsigned PropertyMaxCount = 16;
    static const unsigned ElementMaxCount = 8;

    enum class Type : uint8 {
        NONE,
        LIST,
        UCHAR,
        UINT,
        FLOAT,
    };

    class Property : public NoCopy {
    public:
        Type type = Type::NONE;
        Type list_count_type = Type::NONE;
        Type list_value_type = Type::NONE;
        const char* name = nullptr;
        size name_size = 0;
        Value* first = nullptr;
    };

    class Element : public NoCopy {
    public:
        Array<Property, PropertyMaxCount> properties;
        const char* name = nullptr;
        size name_size = 0;
        unsigned count = 0;
    };

    Array<Value, ValueMaxCount> value_list;
    Array<Element, ElementMaxCount> elements;

    void ParseHeaderComment(const char*& text) {
        Skip<NonEOLPred>(text);
        Skip<WhitespacePred>(text);
    }

    void ParseHeaderElement(const char*& text) {
        auto& element = elements.Add();

        element.name = text;
        Skip<NonWhitespacePred>(text);
        element.name_size = text - element.name;
        Skip<WhitespacePred>(text);

        const char* count = text;
        Skip<NonWhitespacePred>(text);
        size count_size = text - count;
        Scan::Int(count, count_size, (int*)&element.count);
        Skip<WhitespacePred>(text);
    }

    void ParseHeaderFormat(const char*& text) {
        const bool is_ascii = Scan::Compare(text, "ascii", 5);
        if (!is_ascii) throw Exception("Non ascii PLY format not supported");
        text += 6;
        const bool version_1_0 = Scan::Compare(text, "1.0", 3);
        if (!version_1_0) throw Exception("PLY version 1.0 required");
        text += 4;
    }

    void ParseHeaderObjInfo(const char*& text) {
        Skip<NonEOLPred>(text);
        Skip<WhitespacePred>(text);
    }

    void ParseHeaderProperty(const char*& text) {
        auto& element = elements[elements.UsedCount() - 1];
        auto& property = element.properties.Add();
        property.type = ParseType(text);
        property.list_count_type = (property.type == Type::LIST) ? ParseType(text) : Type::NONE;
        property.list_value_type = (property.type == Type::LIST) ? ParseType(text) : Type::NONE;
        property.name = text;
        Skip<NonWhitespacePred>(text);
        property.name_size = text - property.name;
        Skip<WhitespacePred>(text);
        if ((property.type != Type::FLOAT) && (property.type != Type::UCHAR) && (property.type != Type::LIST)) throw Exception("PLY property must be 'float' or 'uchar' or 'list'");
        if ((property.type == Type::LIST) && (property.list_count_type != Type::UCHAR)) throw Exception("PLY property list count type must be 'uchar'");
        if ((property.type == Type::LIST) && (property.list_value_type != Type::UINT)) throw Exception("PLY property list value type must be 'uint'");
    }

    void ParseHeader(const char*& text) {
        const bool has_magic = Scan::Compare(text, "ply", 3) || Scan::Compare(text, "PLY", 3);
        if (!has_magic) throw Exception("Missing PLY header magic number");
        text += 4;
        while (1) {
            switch (text[0]) {
            case char('c') :
                if (Scan::Compare(&text[1], "omment", 6)) {
                    text += 8;
                    ParseHeaderComment(text);
                }
                break;
            case char('e') :
                if (Scan::Compare(&text[1], "lement", 6)) {
                    text += 8;
                    ParseHeaderElement(text);
                } else if (Scan::Compare(&text[1], "nd_header", 9)) {
                    text += 11;
                    return;
                }
                break;
            case char('f') :
                if (Scan::Compare(&text[1], "ormat", 5)) {
                    text += 7;
                    ParseHeaderFormat(text);
                }
                break;
            case char('p') :
                if (Scan::Compare(&text[1], "roperty", 7)) {
                    text += 9;
                    ParseHeaderProperty(text);
                }
                break;
            case char('o') :
                if (Scan::Compare(&text[1], "bj_info", 7)) {
                    text += 9;
                    ParseHeaderObjInfo(text);
                }
                break;
            default: throw Exception("Unknow PLY header token");
            }
        }
    }

    Value* ParseValue(const char*& text) {
        const char* p = text;
        Skip<NonWhitespacePred>(text);
        const size size = text - p;
        Skip<WhitespacePred>(text);
        return &value_list.Add(p, size);
    }

    void ParseElements(const char*& text) {
        elements.Process([&](auto& element) {
            FixedArray<Value*, PropertyMaxCount> previous;
            element.properties.ProcessIndex([&](auto& property, unsigned index) {
                if (property.type == Type::LIST) {
                    unsigned list_size = ParseValue(text)->Int();
                    if (list_size != 3) throw Exception("PLY face must be triangle");
                    property.first = ParseValue(text);
                    previous[index] = property.first;
                    for (unsigned j = 0; j < list_size - 1; ++j) {
                        previous[index]->SetNext(ParseValue(text));
                        previous[index] = previous[index]->Next();
                    }
                } else {
                    property.first = ParseValue(text);
                    previous[index] = property.first;
                }
            });
            const unsigned count = element.count - 1;
            for (unsigned j = 0; j < count; ++j) {
                element.properties.ProcessIndex([&](auto& property, unsigned index) {
                    if (property.type == Type::LIST) {
                        unsigned list_size = ParseValue(text)->Int();
                        if (list_size != 3) throw Exception("PLY face must be triangle");
                        for (unsigned l = 0; l < list_size; ++l) {
                            previous[index]->SetNext(ParseValue(text));
                            previous[index] = previous[index]->Next();
                        }
                    } else {
                        previous[index]->SetNext(ParseValue(text));
                        previous[index] = previous[index]->Next();
                    }
                });
            }
        });
    }

    Type ParseType(const char*& text) {
        Type type = Type::NONE;

        switch (text[0]) {
        case char('l') :
            if (Scan::Compare(&text[1], "ist", 3)) {
                text += 4;
                type = Type::LIST;
            }
            break;
        case char('u') :
            if (Scan::Compare(&text[1], "char", 4)) {
                text += 5;
                type = Type::UCHAR;
            } else if (Scan::Compare(&text[1], "int", 3)) {
                text += 4;
                type = Type::UINT;
            }
            break;
        case char('f') :
            if (Scan::Compare(&text[1], "loat", 4)) {
                text += 5;
                type = Type::FLOAT;
            }
            break;
        default: throw Exception("Unknow PLY type");
        }
        Skip<WhitespacePred>(text);
        return type;
    }

    struct NonEOLPred {
        static unsigned char Test(char ch) {
            switch (ch) {
            case 0x0A:
            case 0x0D: return 0;
            default: return 1;
            }
        }
    };

    struct NonWhitespacePred {
        static unsigned char Test(char ch) {
            switch (ch) {
            case 0x09:
            case 0x0A:
            case 0x0D:
            case 0x20: return 0;
            default: return 1;
            }
        }
    };

    struct WhitespacePred {
        static unsigned char Test(char ch) {
            switch (ch) {
            case 0x09:
            case 0x0A:
            case 0x0D:
            case 0x20: return 1;
            default: return 0;
            }
        }
    };

    template<class StopPred> static void Skip(const char*& text) {
        const char* tmp = text;
        while (StopPred::Test(*tmp))
            ++tmp;
        text = tmp;
    }
};

class TGA {
    #pragma pack(push, 1)
    struct Header {
        uint8 idlength;
        uint8 colourmaptype;
        uint8 datatypecode;
        uint16 colourmaporigin;
        uint16 colourmaplength;
        uint8 colourmapdepth;
        uint16 x_origin;
        uint16 y_origin;
        uint16 width;
        uint16 height;
        uint8 bitsperpixel;
        uint8 imagedescriptor;
    };
    #pragma pack(pop)

    static PixelFormat DeterminePixelFormat(unsigned bits_per_pixel) {
        switch (bits_per_pixel) {
        case 8: return PixelFormat::R8;
        case 16: return PixelFormat::RG8;
        case 24: return PixelFormat::RGB8;
        case 32: return PixelFormat::BGRA8;
        default: throw Exception("TGA must be either 8/16/24/32 bits");
        }
    }

public:
    TGA(const uint8* data, const size data_size, size& offset) {
        offset += sizeof(Header);

        auto& header = *reinterpret_cast<const Header*>(data);
        if (header.colourmaptype != 0)
            throw Exception("TGA shouldn't use a color-map");
        if (header.datatypecode != 2)
            throw Exception("TGA must must be true-color");

        width = header.width;
        height = header.height;
        bits_per_pixel = header.bitsperpixel;
        pixel_format = DeterminePixelFormat(bits_per_pixel);
    }

    unsigned width = 1;
    unsigned height = 1;
    unsigned bits_per_pixel = 32;
    Dimension dimension = Dimension::Tex2D;
    PixelFormat pixel_format = PixelFormat::RGBA8;
};

struct WAV : public NoCopy {
    enum WAVE_FORMAT
    {
        PCM = 0x0001,
        ADPCM = 0x0002,
        IEEE_FLOAT = 0x0003,
        EXTENSIBLE = 0xFFFE,
    };

#pragma pack(push, 1)
    struct GUID {
        unsigned long  Data1;
        unsigned short Data2;
        unsigned short Data3;
        unsigned char  Data4[8];
    };

    struct WAVEFORMAT {
        uint16 wFormatTag;
        uint16 nChannels;
        uint32 nSamplesPerSec;
        uint32 nAvgBytesPerSec;
        uint16 nBlockAlign;
    };

    struct WAVEFORMATEX
    {
        uint16 wFormatTag;
        uint16 nChannels;
        uint32 nSamplesPerSec;
        uint32 nAvgBytesPerSec;
        uint16 nBlockAlign;
        uint16 wBitsPerSample;
        uint16 cbSize;
    };

    struct PCMWAVEFORMAT {
        WAVEFORMAT wf;
        uint16 wBitsPerSample;
    };

    struct WAVEFORMATEXTENSIBLE {
        WAVEFORMATEX Format;
        union {
            uint16 wValidBitsPerSample;
            uint16 wSamplesPerBlock;
            uint16 wReserved;
        } Samples;
        uint32 dwChannelMask;
        GUID SubFormat;
    };
#pragma pack(pop)

    WAVEFORMATEXTENSIBLE wfxt;
    size offset = 0;
    size length = 0;

    static const uint32 FOURCC_RIFF_TAG = 'FFIR';
    static const uint32 FOURCC_FORMAT_TAG = ' tmf';
    static const uint32 FOURCC_DATA_TAG = 'atad';
    static const uint32 FOURCC_WAVE_FILE_TAG = 'EVAW';

#pragma pack(push,1)
    struct RIFFChunk {
        uint32 tag;
        uint32 size;
    };

    struct RIFFChunkHeader {
        uint32 tag;
        uint32 size;
        uint32 riff;
    };
#pragma pack(pop)

    static_assert(sizeof(RIFFChunk) == 8);
    static_assert(sizeof(RIFFChunkHeader) == 12);

    static const RIFFChunk* FindChunk(const uint8* data, size sizeBytes, uint32 tag) {
        const uint8* ptr = data;
        const uint8* end = data + sizeBytes;

        while (end > (ptr + sizeof(RIFFChunk))) {
            auto header = reinterpret_cast<const RIFFChunk*>(ptr);
            if (header->tag == tag)
                return header;

            ptrdiff_t offset = header->size + sizeof(RIFFChunk);
            ptr += offset;
        }

        return nullptr;
    }

public:
    WAV() {}
    WAV(const uint8* data, const size data_size) {
        if (data_size < (sizeof(RIFFChunk) * 2 + sizeof(uint32) + sizeof(WAVEFORMAT))) throw Exception("Size too small");

        const uint8* data_end = data + data_size;

        auto riff_chunk = FindChunk(data, data_size, FOURCC_RIFF_TAG);
        if (!riff_chunk || riff_chunk->size < 4)
            throw Exception("Invalid RIFF chunk");

        auto riff_header = reinterpret_cast<const RIFFChunkHeader*>(riff_chunk);
        if (riff_header->riff != FOURCC_WAVE_FILE_TAG)
            throw Exception("Invalid file tag");

        auto ptr = reinterpret_cast<const uint8*>(riff_header) + sizeof(RIFFChunkHeader);
        if ((ptr + sizeof(RIFFChunk)) > data_end)
            throw Exception("End of file");

        auto fmt_chunk = FindChunk(ptr, riff_header->size, FOURCC_FORMAT_TAG);
        if (!fmt_chunk || fmt_chunk->size < sizeof(PCMWAVEFORMAT))
            throw Exception("Invalid FORMAT chunk");

        ptr = reinterpret_cast<const uint8*>(fmt_chunk) + sizeof(RIFFChunk);
        if (ptr + fmt_chunk->size > data_end)
            throw Exception("End of file");

        auto wf = reinterpret_cast<const WAVEFORMAT*>(ptr);

        switch (wf->wFormatTag) {
        case WAVE_FORMAT::PCM:
        case WAVE_FORMAT::IEEE_FLOAT:
            // Can be a PCMWAVEFORMAT (8 bytes) or WAVEFORMATEX (10 bytes)
            break;

        default: {
            if (fmt_chunk->size < sizeof(WAVEFORMATEX))
                throw Exception("Size too small");

            auto wfx = reinterpret_cast<const WAVEFORMATEX*>(ptr);

            if (fmt_chunk->size < (sizeof(WAVEFORMATEX) + wfx->cbSize))
                throw Exception("Size too small");

            switch (wfx->wFormatTag) {
            case WAVE_FORMAT::ADPCM:
                if ((fmt_chunk->size < (sizeof(WAVEFORMATEX) + 32)) || (wfx->cbSize < 32 /*MSADPCM_FORMAT_EXTRA_BYTES*/))
                    throw Exception("Invalid ADPCM chunk");
                break;

            case WAVE_FORMAT::EXTENSIBLE:
                if ((fmt_chunk->size < sizeof(WAVEFORMATEXTENSIBLE)) || (wfx->cbSize < (sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX))))
                    throw Exception("Invalid EXTENSIBLE chunk");
                else {
                    static const GUID s_wfexBase = { 0x00000000, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71 };
                    auto wfex = reinterpret_cast<const WAVEFORMATEXTENSIBLE*>(ptr);
                    if (memcmp(reinterpret_cast<const char*>(&wfex->SubFormat) + sizeof(unsigned), reinterpret_cast<const char*>(&s_wfexBase) + sizeof(unsigned), sizeof(GUID) - sizeof(unsigned)) != 0)
                        throw Exception("Invalid EXTENSIBLE chunk");
                }
                break;
            default:
                throw Exception("Unknown WAVE format");
            }
        }
        }

        ptr = reinterpret_cast<const uint8*>(riff_header) + sizeof(RIFFChunkHeader);
        if ((ptr + sizeof(RIFFChunk)) > data_end) throw Exception("End of file");

        auto data_chunk = FindChunk(ptr, riff_chunk->size, FOURCC_DATA_TAG);
        if (!data_chunk || !data_chunk->size) throw Exception("Invalid DATA chunk");

        ptr = reinterpret_cast<const uint8*>(data_chunk) + sizeof(RIFFChunk);
        if (ptr + data_chunk->size > data_end) throw Exception("End of file");

        wfxt = *reinterpret_cast<const WAVEFORMATEXTENSIBLE*>(wf);
        offset = size(ptr - data);
        length = data_chunk->size;
    }

    size Offset() const { return offset; }
    size Length() const { return length; }
};

class DDS : public NoCopy {

    const uint32 Magic = 0x20534444; // "DDS "

#define MAKEFOURCC(ch0, ch1, ch2, ch3) ((uint32)(uint8)(ch0) | ((uint32)(uint8)(ch1) << 8) | ((uint32)(uint8)(ch2) << 16) | ((uint32)(uint8)(ch3) << 24 ))

#define ISBITMASK( r,g,b,a ) ( ddpf.RBitMask == r && ddpf.GBitMask == g && ddpf.BBitMask == b && ddpf.ABitMask == a )

#define DDS_FOURCC      0x00000004  // DDPF_FOURCC
#define DDS_RGB         0x00000040  // DDPF_RGB
#define DDS_LUMINANCE   0x00020000  // DDPF_LUMINANCE
#define DDS_ALPHA       0x00000002  // DDPF_ALPHA
#define DDS_BUMPDUDV    0x00080000  // DDPF_BUMPDUDV

#define DDS_HEADER_FLAGS_VOLUME 0x00800000  // DDSD_DEPTH

#define DDS_HEIGHT 0x00000002 // DDSD_HEIGHT
#define DDS_WIDTH  0x00000004 // DDSD_WIDTH

#define DDS_CUBEMAP_POSITIVEX 0x00000600 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEX
#define DDS_CUBEMAP_NEGATIVEX 0x00000a00 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEX
#define DDS_CUBEMAP_POSITIVEY 0x00001200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEY
#define DDS_CUBEMAP_NEGATIVEY 0x00002200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEY
#define DDS_CUBEMAP_POSITIVEZ 0x00004200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEZ
#define DDS_CUBEMAP_NEGATIVEZ 0x00008200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEZ

#define DDS_CUBEMAP_ALLFACES ( DDS_CUBEMAP_POSITIVEX | DDS_CUBEMAP_NEGATIVEX | DDS_CUBEMAP_POSITIVEY | DDS_CUBEMAP_NEGATIVEY | DDS_CUBEMAP_POSITIVEZ | DDS_CUBEMAP_NEGATIVEZ )

#define DDS_CUBEMAP 0x00000200 // DDSCAPS2_CUBEMAP

    enum DDS_MISC_FLAGS2 {
        DDS_MISC_FLAGS2_ALPHA_MODE_MASK = 0x7L,
    };

    enum D3D12_RESOURCE_DIMENSION
    {
        D3D12_RESOURCE_DIMENSION_UNKNOWN = 0,
        D3D12_RESOURCE_DIMENSION_BUFFER = 1,
        D3D12_RESOURCE_DIMENSION_TEXTURE1D = 2,
        D3D12_RESOURCE_DIMENSION_TEXTURE2D = 3,
        D3D12_RESOURCE_DIMENSION_TEXTURE3D = 4
    };

    enum D3D11_RESOURCE_MISC_FLAG {
        D3D11_RESOURCE_MISC_GENERATE_MIPS = 0x1L,
        D3D11_RESOURCE_MISC_SHARED = 0x2L,
        D3D11_RESOURCE_MISC_TEXTURECUBE = 0x4L,
        D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS = 0x10L,
        D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS = 0x20L,
        D3D11_RESOURCE_MISC_BUFFER_STRUCTURED = 0x40L,
        D3D11_RESOURCE_MISC_RESOURCE_CLAMP = 0x80L,
        D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX = 0x100L,
        D3D11_RESOURCE_MISC_GDI_COMPATIBLE = 0x200L,
        D3D11_RESOURCE_MISC_SHARED_NTHANDLE = 0x800L,
        D3D11_RESOURCE_MISC_RESTRICTED_CONTENT = 0x1000L,
        D3D11_RESOURCE_MISC_RESTRICT_SHARED_RESOURCE = 0x2000L,
        D3D11_RESOURCE_MISC_RESTRICT_SHARED_RESOURCE_DRIVER = 0x4000L,
        D3D11_RESOURCE_MISC_GUARDED = 0x8000L,
        D3D11_RESOURCE_MISC_TILE_POOL = 0x20000L,
        D3D11_RESOURCE_MISC_TILED = 0x40000L
    };

    typedef enum DXGI_FORMAT
    {
        DXGI_FORMAT_UNKNOWN = 0,
        DXGI_FORMAT_R32G32B32A32_TYPELESS = 1,
        DXGI_FORMAT_R32G32B32A32_FLOAT = 2,
        DXGI_FORMAT_R32G32B32A32_UINT = 3,
        DXGI_FORMAT_R32G32B32A32_SINT = 4,
        DXGI_FORMAT_R32G32B32_TYPELESS = 5,
        DXGI_FORMAT_R32G32B32_FLOAT = 6,
        DXGI_FORMAT_R32G32B32_UINT = 7,
        DXGI_FORMAT_R32G32B32_SINT = 8,
        DXGI_FORMAT_R16G16B16A16_TYPELESS = 9,
        DXGI_FORMAT_R16G16B16A16_FLOAT = 10,
        DXGI_FORMAT_R16G16B16A16_UNORM = 11,
        DXGI_FORMAT_R16G16B16A16_UINT = 12,
        DXGI_FORMAT_R16G16B16A16_SNORM = 13,
        DXGI_FORMAT_R16G16B16A16_SINT = 14,
        DXGI_FORMAT_R32G32_TYPELESS = 15,
        DXGI_FORMAT_R32G32_FLOAT = 16,
        DXGI_FORMAT_R32G32_UINT = 17,
        DXGI_FORMAT_R32G32_SINT = 18,
        DXGI_FORMAT_R32G8X24_TYPELESS = 19,
        DXGI_FORMAT_D32_FLOAT_S8X24_UINT = 20,
        DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS = 21,
        DXGI_FORMAT_X32_TYPELESS_G8X24_UINT = 22,
        DXGI_FORMAT_R10G10B10A2_TYPELESS = 23,
        DXGI_FORMAT_R10G10B10A2_UNORM = 24,
        DXGI_FORMAT_R10G10B10A2_UINT = 25,
        DXGI_FORMAT_R11G11B10_FLOAT = 26,
        DXGI_FORMAT_R8G8B8A8_TYPELESS = 27,
        DXGI_FORMAT_R8G8B8A8_UNORM = 28,
        DXGI_FORMAT_R8G8B8A8_UNORM_SRGB = 29,
        DXGI_FORMAT_R8G8B8A8_UINT = 30,
        DXGI_FORMAT_R8G8B8A8_SNORM = 31,
        DXGI_FORMAT_R8G8B8A8_SINT = 32,
        DXGI_FORMAT_R16G16_TYPELESS = 33,
        DXGI_FORMAT_R16G16_FLOAT = 34,
        DXGI_FORMAT_R16G16_UNORM = 35,
        DXGI_FORMAT_R16G16_UINT = 36,
        DXGI_FORMAT_R16G16_SNORM = 37,
        DXGI_FORMAT_R16G16_SINT = 38,
        DXGI_FORMAT_R32_TYPELESS = 39,
        DXGI_FORMAT_D32_FLOAT = 40,
        DXGI_FORMAT_R32_FLOAT = 41,
        DXGI_FORMAT_R32_UINT = 42,
        DXGI_FORMAT_R32_SINT = 43,
        DXGI_FORMAT_R24G8_TYPELESS = 44,
        DXGI_FORMAT_D24_UNORM_S8_UINT = 45,
        DXGI_FORMAT_R24_UNORM_X8_TYPELESS = 46,
        DXGI_FORMAT_X24_TYPELESS_G8_UINT = 47,
        DXGI_FORMAT_R8G8_TYPELESS = 48,
        DXGI_FORMAT_R8G8_UNORM = 49,
        DXGI_FORMAT_R8G8_UINT = 50,
        DXGI_FORMAT_R8G8_SNORM = 51,
        DXGI_FORMAT_R8G8_SINT = 52,
        DXGI_FORMAT_R16_TYPELESS = 53,
        DXGI_FORMAT_R16_FLOAT = 54,
        DXGI_FORMAT_D16_UNORM = 55,
        DXGI_FORMAT_R16_UNORM = 56,
        DXGI_FORMAT_R16_UINT = 57,
        DXGI_FORMAT_R16_SNORM = 58,
        DXGI_FORMAT_R16_SINT = 59,
        DXGI_FORMAT_R8_TYPELESS = 60,
        DXGI_FORMAT_R8_UNORM = 61,
        DXGI_FORMAT_R8_UINT = 62,
        DXGI_FORMAT_R8_SNORM = 63,
        DXGI_FORMAT_R8_SINT = 64,
        DXGI_FORMAT_A8_UNORM = 65,
        DXGI_FORMAT_R1_UNORM = 66,
        DXGI_FORMAT_R9G9B9E5_SHAREDEXP = 67,
        DXGI_FORMAT_R8G8_B8G8_UNORM = 68,
        DXGI_FORMAT_G8R8_G8B8_UNORM = 69,
        DXGI_FORMAT_BC1_TYPELESS = 70,
        DXGI_FORMAT_BC1_UNORM = 71,
        DXGI_FORMAT_BC1_UNORM_SRGB = 72,
        DXGI_FORMAT_BC2_TYPELESS = 73,
        DXGI_FORMAT_BC2_UNORM = 74,
        DXGI_FORMAT_BC2_UNORM_SRGB = 75,
        DXGI_FORMAT_BC3_TYPELESS = 76,
        DXGI_FORMAT_BC3_UNORM = 77,
        DXGI_FORMAT_BC3_UNORM_SRGB = 78,
        DXGI_FORMAT_BC4_TYPELESS = 79,
        DXGI_FORMAT_BC4_UNORM = 80,
        DXGI_FORMAT_BC4_SNORM = 81,
        DXGI_FORMAT_BC5_TYPELESS = 82,
        DXGI_FORMAT_BC5_UNORM = 83,
        DXGI_FORMAT_BC5_SNORM = 84,
        DXGI_FORMAT_B5G6R5_UNORM = 85,
        DXGI_FORMAT_B5G5R5A1_UNORM = 86,
        DXGI_FORMAT_B8G8R8A8_UNORM = 87,
        DXGI_FORMAT_B8G8R8X8_UNORM = 88,
        DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM = 89,
        DXGI_FORMAT_B8G8R8A8_TYPELESS = 90,
        DXGI_FORMAT_B8G8R8A8_UNORM_SRGB = 91,
        DXGI_FORMAT_B8G8R8X8_TYPELESS = 92,
        DXGI_FORMAT_B8G8R8X8_UNORM_SRGB = 93,
        DXGI_FORMAT_BC6H_TYPELESS = 94,
        DXGI_FORMAT_BC6H_UF16 = 95,
        DXGI_FORMAT_BC6H_SF16 = 96,
        DXGI_FORMAT_BC7_TYPELESS = 97,
        DXGI_FORMAT_BC7_UNORM = 98,
        DXGI_FORMAT_BC7_UNORM_SRGB = 99,
        DXGI_FORMAT_AYUV = 100,
        DXGI_FORMAT_Y410 = 101,
        DXGI_FORMAT_Y416 = 102,
        DXGI_FORMAT_NV12 = 103,
        DXGI_FORMAT_P010 = 104,
        DXGI_FORMAT_P016 = 105,
        DXGI_FORMAT_420_OPAQUE = 106,
        DXGI_FORMAT_YUY2 = 107,
        DXGI_FORMAT_Y210 = 108,
        DXGI_FORMAT_Y216 = 109,
        DXGI_FORMAT_NV11 = 110,
        DXGI_FORMAT_AI44 = 111,
        DXGI_FORMAT_IA44 = 112,
        DXGI_FORMAT_P8 = 113,
        DXGI_FORMAT_A8P8 = 114,
        DXGI_FORMAT_B4G4R4A4_UNORM = 115,

        DXGI_FORMAT_P208 = 130,
        DXGI_FORMAT_V208 = 131,
        DXGI_FORMAT_V408 = 132,


        DXGI_FORMAT_FORCE_UINT = 0xffffffff
    } DXGI_FORMAT;

#pragma pack(push, 1)
    struct Format {
        uint32 size;
        uint32 flags;
        uint32 fourCC;
        uint32 RGBBitCount;
        uint32 RBitMask;
        uint32 GBitMask;
        uint32 BBitMask;
        uint32 ABitMask;
    };

    struct Header {
        uint32 size;
        uint32 flags;
        uint32 height;
        uint32 width;
        uint32 pitchOrLinearSize;
        uint32 depth; // only if DDS_HEADER_FLAGS_VOLUME is set in flags
        uint32 mipMapCount;
        uint32 reserved1[11];
        Format ddspf;
        uint32 caps;
        uint32 caps2;
        uint32 caps3;
        uint32 caps4;
        uint32 reserved2;
    };

    struct Header_DXT10 {
        DXGI_FORMAT dxgiFormat;
        uint32 resourceDimension;
        uint32 miscFlag; // see D3D11_RESOURCE_MISC_FLAG
        uint32 arraySize;
        uint32 miscFlags2;
    };
#pragma pack(pop)

    static unsigned BitsPerPixel(DXGI_FORMAT fmt) {
        switch (fmt) {
        case DXGI_FORMAT_R32G32B32A32_TYPELESS:
        case DXGI_FORMAT_R32G32B32A32_FLOAT:
        case DXGI_FORMAT_R32G32B32A32_UINT:
        case DXGI_FORMAT_R32G32B32A32_SINT:
            return 128;

        case DXGI_FORMAT_R32G32B32_TYPELESS:
        case DXGI_FORMAT_R32G32B32_FLOAT:
        case DXGI_FORMAT_R32G32B32_UINT:
        case DXGI_FORMAT_R32G32B32_SINT:
            return 96;

        case DXGI_FORMAT_R16G16B16A16_TYPELESS:
        case DXGI_FORMAT_R16G16B16A16_FLOAT:
        case DXGI_FORMAT_R16G16B16A16_UNORM:
        case DXGI_FORMAT_R16G16B16A16_UINT:
        case DXGI_FORMAT_R16G16B16A16_SNORM:
        case DXGI_FORMAT_R16G16B16A16_SINT:
        case DXGI_FORMAT_R32G32_TYPELESS:
        case DXGI_FORMAT_R32G32_FLOAT:
        case DXGI_FORMAT_R32G32_UINT:
        case DXGI_FORMAT_R32G32_SINT:
        case DXGI_FORMAT_R32G8X24_TYPELESS:
        case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
        case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
        case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
        case DXGI_FORMAT_Y416:
        case DXGI_FORMAT_Y210:
        case DXGI_FORMAT_Y216:
            return 64;

        case DXGI_FORMAT_R10G10B10A2_TYPELESS:
        case DXGI_FORMAT_R10G10B10A2_UNORM:
        case DXGI_FORMAT_R10G10B10A2_UINT:
        case DXGI_FORMAT_R11G11B10_FLOAT:
        case DXGI_FORMAT_R8G8B8A8_TYPELESS:
        case DXGI_FORMAT_R8G8B8A8_UNORM:
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
        case DXGI_FORMAT_R8G8B8A8_UINT:
        case DXGI_FORMAT_R8G8B8A8_SNORM:
        case DXGI_FORMAT_R8G8B8A8_SINT:
        case DXGI_FORMAT_R16G16_TYPELESS:
        case DXGI_FORMAT_R16G16_FLOAT:
        case DXGI_FORMAT_R16G16_UNORM:
        case DXGI_FORMAT_R16G16_UINT:
        case DXGI_FORMAT_R16G16_SNORM:
        case DXGI_FORMAT_R16G16_SINT:
        case DXGI_FORMAT_R32_TYPELESS:
        case DXGI_FORMAT_D32_FLOAT:
        case DXGI_FORMAT_R32_FLOAT:
        case DXGI_FORMAT_R32_UINT:
        case DXGI_FORMAT_R32_SINT:
        case DXGI_FORMAT_R24G8_TYPELESS:
        case DXGI_FORMAT_D24_UNORM_S8_UINT:
        case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
        case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
        case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
        case DXGI_FORMAT_R8G8_B8G8_UNORM:
        case DXGI_FORMAT_G8R8_G8B8_UNORM:
        case DXGI_FORMAT_B8G8R8A8_UNORM:
        case DXGI_FORMAT_B8G8R8X8_UNORM:
        case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
        case DXGI_FORMAT_B8G8R8A8_TYPELESS:
        case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
        case DXGI_FORMAT_B8G8R8X8_TYPELESS:
        case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
        case DXGI_FORMAT_AYUV:
        case DXGI_FORMAT_Y410:
        case DXGI_FORMAT_YUY2:
            return 32;

        case DXGI_FORMAT_P010:
        case DXGI_FORMAT_P016:
            return 24;

        case DXGI_FORMAT_R8G8_TYPELESS:
        case DXGI_FORMAT_R8G8_UNORM:
        case DXGI_FORMAT_R8G8_UINT:
        case DXGI_FORMAT_R8G8_SNORM:
        case DXGI_FORMAT_R8G8_SINT:
        case DXGI_FORMAT_R16_TYPELESS:
        case DXGI_FORMAT_R16_FLOAT:
        case DXGI_FORMAT_D16_UNORM:
        case DXGI_FORMAT_R16_UNORM:
        case DXGI_FORMAT_R16_UINT:
        case DXGI_FORMAT_R16_SNORM:
        case DXGI_FORMAT_R16_SINT:
        case DXGI_FORMAT_B5G6R5_UNORM:
        case DXGI_FORMAT_B5G5R5A1_UNORM:
        case DXGI_FORMAT_A8P8:
        case DXGI_FORMAT_B4G4R4A4_UNORM:
            return 16;

        case DXGI_FORMAT_NV12:
        case DXGI_FORMAT_420_OPAQUE:
        case DXGI_FORMAT_NV11:
            return 12;

        case DXGI_FORMAT_R8_TYPELESS:
        case DXGI_FORMAT_R8_UNORM:
        case DXGI_FORMAT_R8_UINT:
        case DXGI_FORMAT_R8_SNORM:
        case DXGI_FORMAT_R8_SINT:
        case DXGI_FORMAT_A8_UNORM:
        case DXGI_FORMAT_AI44:
        case DXGI_FORMAT_IA44:
        case DXGI_FORMAT_P8:
            return 8;

        case DXGI_FORMAT_R1_UNORM:
            return 1;

        case DXGI_FORMAT_BC1_TYPELESS:
        case DXGI_FORMAT_BC1_UNORM:
        case DXGI_FORMAT_BC1_UNORM_SRGB:
        case DXGI_FORMAT_BC4_TYPELESS:
        case DXGI_FORMAT_BC4_UNORM:
        case DXGI_FORMAT_BC4_SNORM:
            return 4;

        case DXGI_FORMAT_BC2_TYPELESS:
        case DXGI_FORMAT_BC2_UNORM:
        case DXGI_FORMAT_BC2_UNORM_SRGB:
        case DXGI_FORMAT_BC3_TYPELESS:
        case DXGI_FORMAT_BC3_UNORM:
        case DXGI_FORMAT_BC3_UNORM_SRGB:
        case DXGI_FORMAT_BC5_TYPELESS:
        case DXGI_FORMAT_BC5_UNORM:
        case DXGI_FORMAT_BC5_SNORM:
        case DXGI_FORMAT_BC6H_TYPELESS:
        case DXGI_FORMAT_BC6H_UF16:
        case DXGI_FORMAT_BC6H_SF16:
        case DXGI_FORMAT_BC7_TYPELESS:
        case DXGI_FORMAT_BC7_UNORM:
        case DXGI_FORMAT_BC7_UNORM_SRGB:
            return 8;

        default:
            return 0;
        }
    }

    DXGI_FORMAT GetDXGIFormat(const Format& ddpf) {
        if (ddpf.flags & DDS_RGB) {
            // Note that sRGB formats are written using the "DX10" extended header
            switch (ddpf.RGBBitCount) {
            case 32:
                if (ISBITMASK(0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000)) return DXGI_FORMAT_R8G8B8A8_UNORM;
                if (ISBITMASK(0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000)) return DXGI_FORMAT_B8G8R8A8_UNORM;
                if (ISBITMASK(0x00ff0000, 0x0000ff00, 0x000000ff, 0x00000000)) return DXGI_FORMAT_B8G8R8X8_UNORM;
                // No DXGI format maps to ISBITMASK(0x000000ff,0x0000ff00,0x00ff0000,0x00000000) aka D3DFMT_X8B8G8R8
                // Note that many common DDS reader/writers (including D3DX) swap the
                // the RED/BLUE masks for 10:10:10:2 formats. We assume
                // below that the 'backwards' header mask is being used since it is most
                // likely written by D3DX. The more robust solution is to use the 'DX10'
                // header extension and specify the DXGI_FORMAT_R10G10B10A2_UNORM format directly
                // For 'correct' writers, this should be 0x000003ff,0x000ffc00,0x3ff00000 for RGB data
                if (ISBITMASK(0x3ff00000, 0x000ffc00, 0x000003ff, 0xc0000000)) return DXGI_FORMAT_R10G10B10A2_UNORM;
                // No DXGI format maps to ISBITMASK(0x000003ff,0x000ffc00,0x3ff00000,0xc0000000) aka D3DFMT_A2R10G10B10
                if (ISBITMASK(0x0000ffff, 0xffff0000, 0x00000000, 0x00000000)) return DXGI_FORMAT_R16G16_UNORM;
                if (ISBITMASK(0xffffffff, 0x00000000, 0x00000000, 0x00000000)) return DXGI_FORMAT_R32_FLOAT; // D3DX writes this out as a FourCC of 114 // Only 32-bit color channel format in D3D9 was R32F
                break;
            case 24:
                // No 24bpp DXGI formats aka D3DFMT_R8G8B8
                break;
            case 16:
                if (ISBITMASK(0x7c00, 0x03e0, 0x001f, 0x8000)) return DXGI_FORMAT_B5G5R5A1_UNORM;
                if (ISBITMASK(0xf800, 0x07e0, 0x001f, 0x0000)) return DXGI_FORMAT_B5G6R5_UNORM;
                // No DXGI format maps to ISBITMASK(0x7c00,0x03e0,0x001f,0x0000) aka D3DFMT_X1R5G5B5
                if (ISBITMASK(0x0f00, 0x00f0, 0x000f, 0xf000)) return DXGI_FORMAT_B4G4R4A4_UNORM;
                // No DXGI format maps to ISBITMASK(0x0f00,0x00f0,0x000f,0x0000) aka D3DFMT_X4R4G4B4
                // No 3:3:2, 3:3:2:8, or paletted DXGI formats aka D3DFMT_A8R3G3B2, D3DFMT_R3G3B2, D3DFMT_P8, D3DFMT_A8P8, etc.
                break;
            }
        }
        else if (ddpf.flags & DDS_LUMINANCE) {
            if (8 == ddpf.RGBBitCount) {
                if (ISBITMASK(0x000000ff, 0x00000000, 0x00000000, 0x00000000)) return DXGI_FORMAT_R8_UNORM; // D3DX10/11 writes this out as DX10 extension
                // No DXGI format maps to ISBITMASK(0x0f,0x00,0x00,0xf0) aka D3DFMT_A4L4
                if (ISBITMASK(0x000000ff, 0x00000000, 0x00000000, 0x0000ff00)) return DXGI_FORMAT_R8G8_UNORM; // Some DDS writers assume the bitcount should be 8 instead of 16
            }
            if (16 == ddpf.RGBBitCount) {
                if (ISBITMASK(0x0000ffff, 0x00000000, 0x00000000, 0x00000000)) return DXGI_FORMAT_R16_UNORM; // D3DX10/11 writes this out as DX10 extension
                if (ISBITMASK(0x000000ff, 0x00000000, 0x00000000, 0x0000ff00)) return DXGI_FORMAT_R8G8_UNORM; // D3DX10/11 writes this out as DX10 extension
            }
        }
        else if (ddpf.flags & DDS_ALPHA) {
            if (8 == ddpf.RGBBitCount)
                return DXGI_FORMAT_A8_UNORM;
        }
        else if (ddpf.flags & DDS_BUMPDUDV) {
            if (16 == ddpf.RGBBitCount) {
                if (ISBITMASK(0x00ff, 0xff00, 0x0000, 0x0000)) return DXGI_FORMAT_R8G8_SNORM; // D3DX10/11 writes this out as DX10 extension
            }
            if (32 == ddpf.RGBBitCount) {
                if (ISBITMASK(0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000)) return DXGI_FORMAT_R8G8B8A8_SNORM; // D3DX10/11 writes this out as DX10 extension
                if (ISBITMASK(0x0000ffff, 0xffff0000, 0x00000000, 0x00000000)) return DXGI_FORMAT_R16G16_SNORM; // D3DX10/11 writes this out as DX10 extension
                // No DXGI format maps to ISBITMASK(0x3ff00000, 0x000ffc00, 0x000003ff, 0xc0000000) aka D3DFMT_A2W10V10U10
            }
        }
        else if (ddpf.flags & DDS_FOURCC) {
            if (MAKEFOURCC('D', 'X', 'T', '1') == ddpf.fourCC) return DXGI_FORMAT_BC1_UNORM;
            if (MAKEFOURCC('D', 'X', 'T', '3') == ddpf.fourCC) return DXGI_FORMAT_BC2_UNORM;
            if (MAKEFOURCC('D', 'X', 'T', '5') == ddpf.fourCC) return DXGI_FORMAT_BC3_UNORM;
            // While pre-multiplied alpha isn't directly supported by the DXGI formats,
            // they are basically the same as these BC formats so they can be mapped
            if (MAKEFOURCC('D', 'X', 'T', '2') == ddpf.fourCC) return DXGI_FORMAT_BC2_UNORM;
            if (MAKEFOURCC('D', 'X', 'T', '4') == ddpf.fourCC) return DXGI_FORMAT_BC3_UNORM;
            if (MAKEFOURCC('A', 'T', 'I', '1') == ddpf.fourCC) return DXGI_FORMAT_BC4_UNORM;
            if (MAKEFOURCC('B', 'C', '4', 'U') == ddpf.fourCC) return DXGI_FORMAT_BC4_UNORM;
            if (MAKEFOURCC('B', 'C', '4', 'S') == ddpf.fourCC) return DXGI_FORMAT_BC4_SNORM;
            if (MAKEFOURCC('A', 'T', 'I', '2') == ddpf.fourCC) return DXGI_FORMAT_BC5_UNORM;
            if (MAKEFOURCC('B', 'C', '5', 'U') == ddpf.fourCC) return DXGI_FORMAT_BC5_UNORM;
            if (MAKEFOURCC('B', 'C', '5', 'S') == ddpf.fourCC) return DXGI_FORMAT_BC5_SNORM;
            // BC6H and BC7 are written using the "DX10" extended header
            if (MAKEFOURCC('R', 'G', 'B', 'G') == ddpf.fourCC) return DXGI_FORMAT_R8G8_B8G8_UNORM;
            if (MAKEFOURCC('G', 'R', 'G', 'B') == ddpf.fourCC) return DXGI_FORMAT_G8R8_G8B8_UNORM;
            if (MAKEFOURCC('Y', 'U', 'Y', '2') == ddpf.fourCC) return DXGI_FORMAT_YUY2;

            switch (ddpf.fourCC)
            {
            case 36: return DXGI_FORMAT_R16G16B16A16_UNORM; // D3DFMT_A16B16G16R16
            case 110: return DXGI_FORMAT_R16G16B16A16_SNORM; // D3DFMT_Q16W16V16U16
            case 111: return DXGI_FORMAT_R16_FLOAT; // D3DFMT_R16F
            case 112: return DXGI_FORMAT_R16G16_FLOAT; // D3DFMT_G16R16F
            case 113: return DXGI_FORMAT_R16G16B16A16_FLOAT; // D3DFMT_A16B16G16R16F
            case 114: return DXGI_FORMAT_R32_FLOAT; // D3DFMT_R32F
            case 115: return DXGI_FORMAT_R32G32_FLOAT; // D3DFMT_G32R32F
            case 116: return DXGI_FORMAT_R32G32B32A32_FLOAT; // D3DFMT_A32B32G32R32F
            }
        }
        return DXGI_FORMAT_UNKNOWN;
    }

    static PixelFormat ConvertFormat(DXGI_FORMAT dxgi_format) {
        switch (dxgi_format) {
        case DXGI_FORMAT_D32_FLOAT: return PixelFormat::D32F;
        case DXGI_FORMAT_B8G8R8A8_UNORM: return PixelFormat::BGRA8;
        case DXGI_FORMAT_R8G8B8A8_UNORM: return PixelFormat::RGBA8;
        case DXGI_FORMAT_R16G16B16A16_FLOAT: return PixelFormat::RGBA16F;
        case DXGI_FORMAT_R16G16_FLOAT: return PixelFormat::RG16F;
        case DXGI_FORMAT_BC1_UNORM: return PixelFormat::BC1;
        case DXGI_FORMAT_BC2_UNORM: return PixelFormat::BC2;
        case DXGI_FORMAT_BC3_UNORM: return PixelFormat::BC3;
        default: throw Exception("Unsupported DDS pixel format");
        };
    }

public:
    DDS(const uint8* data, const size data_size, size& offset) {
        uint32 dwMagicNumber = *(const uint32*)(data);
        if (dwMagicNumber != Magic) throw Exception("Invalid DDS magic number");

        if (data_size < (sizeof(uint32) + sizeof(Header))) throw Exception("Data size is smaller than DDS header size");
        offset += sizeof(uint32) + sizeof(Header);

        auto& header = *reinterpret_cast<const Header*>(data + sizeof(uint32));
        if (header.size != sizeof(Header) || header.ddspf.size != sizeof(Format)) throw Exception("Invalid DDS header size");

        bool dxt10 = (header.ddspf.flags & DDS_FOURCC) && (MAKEFOURCC('D', 'X', '1', '0') == header.ddspf.fourCC);
        if (dxt10) {
            if (data_size < (sizeof(Header) + sizeof(uint32) + sizeof(Header_DXT10))) throw Exception("Invalid DDS DXT10 header size");
            offset += sizeof(Header_DXT10);
        }

        width = header.width;
        height = header.height;
        depth = header.depth;
        mip_count = Math::Max(1u, header.mipMapCount);

        auto dxgi_format = DXGI_FORMAT_UNKNOWN;

        if (dxt10) {
            auto& header_dxt10 = *reinterpret_cast<const Header_DXT10*>((const char*)&header + sizeof(Header));

            dxgi_format = header_dxt10.dxgiFormat;

            slice_count = header_dxt10.arraySize;
            if (slice_count == 0) throw Exception("Invalid DDS array size");

            switch (header_dxt10.resourceDimension)
            {
            case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
                if ((header.flags & DDS_HEIGHT) && height != 1) throw Exception("DDS Invalid 1D height");
                height = depth = 1;
                dimension = Dimension::Tex1D;
                break;
            case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
                if (header_dxt10.miscFlag & D3D11_RESOURCE_MISC_TEXTURECUBE) {
                    slice_count *= 6;
                    cube_map = true;
                }
                depth = 1;
                dimension = Dimension::Tex2D;
                break;
            case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
                if (!(header.flags & DDS_HEADER_FLAGS_VOLUME)) throw Exception("DDS Missing volume flag");
                if (slice_count > 1) throw Exception("DDS Invalid array size");
                dimension = Dimension::Tex3D;
                break;
            default: throw Exception("DDS Invalid dimension");
            }
        }
        else {
            dxgi_format = GetDXGIFormat(header.ddspf);
            if (dxgi_format == DXGI_FORMAT_UNKNOWN) throw Exception("DDS Invalid pixel format");
            if (header.flags & DDS_HEADER_FLAGS_VOLUME) {
                dimension = Dimension::Tex3D;
            }
            else {
                if (header.caps2 & DDS_CUBEMAP) {
                    if ((header.caps2 & DDS_CUBEMAP_ALLFACES) != DDS_CUBEMAP_ALLFACES) throw Exception("DDS Invalid cubemap faces flag");
                    slice_count = 6;
                    cube_map = true;
                }
                depth = 1;
                dimension = Dimension::Tex2D;
            }
        }

        bits_per_pixel = BitsPerPixel(dxgi_format);
        pixel_format = ConvertFormat(dxgi_format);
    }

    unsigned width = 1;
    unsigned height = 1;
    unsigned depth = 1;
    unsigned mip_count = 1;
    unsigned slice_count = 1;
    unsigned bits_per_pixel = 32;
    Dimension dimension = Dimension::Tex1D;
    PixelFormat pixel_format = PixelFormat::BGRA8;
    bool cube_map = false;
};
