
void ScriptBuild::Compile(const LongString& name, const LongString& optimization) {
}

void ShaderBuild::CompileTechnique(Technique& technique, const char* source, size source_size, const String& filename, Array<Memory::Alloc, TechniqueMaxCount>& bytecodes, size& total_bytecode_size) {
    bytecodes.Add(Compile(source, source_size, filename, "dummy", "dummy", technique.vertex_binary, total_bytecode_size));
    bytecodes.Add(Compile(source, source_size, filename, "dummy", "dummy", technique.pixel_binary, total_bytecode_size));
}

Memory::Alloc ShaderBuild::Compile(const char* source, size source_size, const String& filename, const String& entry_point, const char* target, Binary& binary, size& total_bytecode_size) {
    char buf[256];
    Memory::Alloc bytecode(buf, 256);
    binary.offset_from_base = (uint32)total_bytecode_size;
    binary.size = (uint32)128;
    total_bytecode_size += 128;
    return bytecode;
}

void SurfaceBuild::Convert(bool mips) {
}
