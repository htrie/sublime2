
void ScriptBuild::Compile(const LongString& name, const LongString& optimization) {
    const auto asset_name = LongString(AssetPath().Data()) + name + ".cpp";
    const auto cache_name = LongString(CachePath().Data()) + name;

    const auto lib = cache_name + ".lib";
    const auto dll = cache_name + ".data";
    const auto obj = cache_name + ".obj";
    const auto pdb = cache_name + ".pdb";
    const auto vc_pdb = cache_name + "_vc.pdb";

    LongString O = "";
    if (optimization == "none") O = "/Od";
    else if (optimization == "full") O = "/Ox";

    const auto sdk = LongString("C:\\Program Files (x86)\\Microsoft Visual Studio\\2017\\BuildTools\\VC\\Tools\\MSVC\\14.13.26128"); // TODO: Avoid hard-coding path.
    const auto wk = LongString("C:\\Program Files (x86)\\Windows Kits\\10\\Include\\10.0.16299.0");
    const auto wk_lib = LongString("C:\\Program Files (x86)\\Windows Kits\\10\\Lib\\10.0.16299.0");

    const auto quote = LongString("\"");
    const auto space = LongString(" ");

    const auto exe = quote + sdk + "\\bin\\HostX64\\x64\\cl.exe" + quote;
    const auto sdk_include = quote + sdk + "\\include" + quote;
    const auto sdk_lib = quote + sdk + "\\lib\\x64" + quote;
    const auto wk_um = quote + wk + "\\um" + quote;
    const auto wk_um_lib = quote + wk_lib + "\\um\\x64" + quote;
    const auto wk_ucrt = quote + wk + "\\ucrt" + quote;
    const auto wk_ucrt_lib = quote + wk_lib + "\\ucrt\\x64" + quote;
    const auto wk_winrt = quote + wk + "\\winrt" + quote;
    const auto wk_shared = quote + wk + "\\shared" + quote;

    const auto compile_options = LongString(" /FS /MP /GS- /GL /W3 /Gy /Zi /Zc:wchar_t /Gm- /Zc:inline /fp:fast /fp:except- /errorReport:prompt /GF /GT /WX /Zc:forScope /GR- /arch:AVX2 /Gd /Oy /Oi /MD /std:c++latest /nologo ") + O;
    const auto includes = LongString(" /I ") + sdk_include + " /I " + wk_um + " /I " + wk_ucrt + " /I " + wk_winrt + " /I " + wk_shared;
    const auto defines = LongString(" /D \"NDEBUG\" /D \"_WINDOWS\" /D \"_WINDLL\"");
    const auto tmp = LongString(" /Fo") + obj;
    const auto vc = LongString(" /Fd") + vc_pdb;
    const auto src = space + asset_name;
    const auto link = LongString(" /link ");
    const auto out = LongString(" /out:") + dll;
    const auto debug = LongString(" /PDB:") + pdb;
    const auto import = LongString(" /IMPLIB:") + lib;
    const auto libraries = LongString(" /LIBPATH:") + sdk_lib + " /LIBPATH:" + wk_um_lib + " /LIBPATH:" + wk_ucrt_lib;
    const auto link_options = LongString(" /MANIFEST:NO /LTCG /NXCOMPAT /DYNAMICBASE /DEBUG:Full /DLL /MACHINE:X64 /OPT:REF /INCREMENTAL:NO /SUBSYSTEM:WINDOWS /OPT:ICF /ERRORREPORT:PROMPT /NOLOGO");

    const auto command = exe + compile_options + includes + defines + tmp + vc + src + link + out + debug + import + libraries + link_options;

    LongString output;
    size size = 0;
    Process::Execute(command.Data(), output.Data(), size, LongString::MaxSize);
    for (unsigned i = 0; i < size; ++i) {
        if (strncmp(&output.Data()[i], "error", 5) == 0) {
            throw Exception(output.Data());
        }
    }
}

void ShaderBuild::CompileTechnique(Technique& technique, const char* source, size source_size, const String& filename, Array<Alloc, TechniqueMaxCount>& bytecodes, size& total_bytecode_size) {
    if (technique.vertex_function_name.Size()) bytecodes.Add(Compile(source, source_size, filename, technique.vertex_function_name, "vs_5_0", technique.vertex_binary, total_bytecode_size));
    if (technique.pixel_function_name.Size()) bytecodes.Add(Compile(source, source_size, filename, technique.pixel_function_name, "ps_5_0", technique.pixel_binary, total_bytecode_size));
}

Alloc ShaderBuild::Compile(const char* source, size source_size, const String& filename, const String& entry_point, const char* target, Binary& binary, size& total_bytecode_size) {
    Microsoft::WRL::ComPtr<ID3DBlob> error;
    Microsoft::WRL::ComPtr<ID3DBlob> code;
    D3DCompile(source, source_size, filename.Data(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entry_point.Data(), target, D3DCOMPILE_ALL_RESOURCES_BOUND, 0, &code, &error);
    if (error) throw Exception((LongString("Shader compilation error: ") + (char*)error->GetBufferPointer()).Data());
    Alloc bytecode(code->GetBufferPointer(), code->GetBufferSize());
    binary.offset_from_base = (uint32)total_bytecode_size;
    binary.size = (uint32)code->GetBufferSize();
    total_bytecode_size += code->GetBufferSize();
    return bytecode;
}

void SurfaceBuild::Convert(bool mips) {
    const auto tga_filename = AssetPath() + Name() + ".tga";
    const auto dds_filename = AssetPath() + Name() + ".dds";

    LongString exe = "Tools\\Crunch\\bin\\crunch_x64.exe";
    LongString in = LongString(" -file ") + tga_filename.Data();
    LongString out = LongString(" -out ") + dds_filename.Data();
    LongString options = " -outsamedir -quiet -fileformat dds -DXT5 -dxtQuality fast -yflip";
    LongString generate = LongString(" -mipMode ") + (mips ? "Generate" : "None");
    LongString command = exe + in + out + options + generate;

    if (!File::Exist(tga_filename))
        throw Exception("Missing input TGA file");

    LongString output;
    size out_size = 0;
    Process::Execute(command.Data(), output.Data(), out_size, LongString::MaxSize);

    if (!File::Exist(dds_filename))
        throw Exception("Failed to crunch TGA dile to DDS");

    {
        ReadOnlyFile file(dds_filename);
        auto* data = (uint8*)file.Pointer();
        const auto file_size = file.Size();
        size offset = 0;

        DDS dds(data, file_size, offset);

        width = dds.width;
        height = dds.height;
        depth = dds.depth;
        pixel_format = dds.pixel_format;
        bits_per_pixel = dds.bits_per_pixel;
        slice_count = dds.slice_count;
        mip_count = dds.mip_count;
        is_cube_map = (uint8)(dds.cube_map ? 1 : 0);
        dimension = dds.dimension;

        const size data_size = file_size - offset;
        WriteOnlyFile data_file(CacheDataFilename(Name()), data_size);
        memcpy((uint8*)data_file.Pointer(), data + offset, data_size);
    }

    File::Delete(dds_filename);

}