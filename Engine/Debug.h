
class DebugShapes : public NoCopy {
    static const unsigned LineMaxCount = 8 * 1024;

    struct Line {
        Line() {}
        Line(const Vector3& from, const Vector3& to, const Vector3& color)
            : from(from), to(to), color(color) {}

        Vector3 from;
        Vector3 to;
        Vector3 color;
    };

    unsigned vertex_count = 0;
    size vertex_buffer_offset = 0;
    FixedArray<Line, LineMaxCount> lines;
    unsigned line_count = 0;

public:
    void Swap() {
        line_count = 0;
        vertex_count = 0;
    }

    void AddLine(const Vector3& from, const Vector3& to, const Vector3& color) {
        lines[line_count++] = Line(from, to, color);
    }

    void DrawDebugPoint(const Vector3& point, const Vector3& color) {
       const float half_size = 0.1f;
       const Vector3 xmin = point - Vector3(half_size, 0.f, 0.f);
       const Vector3 xmax = point + Vector3(half_size, 0.f, 0.f);
       const Vector3 ymin = point - Vector3(0.f, half_size, 0.f);
       const Vector3 ymax = point + Vector3(0.f, half_size, 0.f);
       const Vector3 zmin = point - Vector3(0.f, 0.f, half_size);
       const Vector3 zmax = point + Vector3(0.f, 0.f, half_size);
       AddLine(xmin, xmax, color);
       AddLine(ymin, ymax, color);
       AddLine(zmin, zmax, color);
    }

    void DrawDebugBox(const Vector3& min, const Vector3& max, const Quaternion& rotation, const Vector3& position, const Vector3& color) {
        const auto a = position + rotation.Transform(Vector3(min.x, min.y, min.z));
        const auto b = position + rotation.Transform(Vector3(max.x, min.y, min.z));
        const auto c = position + rotation.Transform(Vector3(max.x, max.y, min.z));
        const auto d = position + rotation.Transform(Vector3(min.x, max.y, min.z));
        const auto e = position + rotation.Transform(Vector3(min.x, min.y, max.z));
        const auto f = position + rotation.Transform(Vector3(max.x, min.y, max.z));
        const auto g = position + rotation.Transform(Vector3(max.x, max.y, max.z));
        const auto h = position + rotation.Transform(Vector3(min.x, max.y, max.z));
        AddLine(a, b, color);
        AddLine(b, c, color);
        AddLine(c, d, color);
        AddLine(d, a, color);
        AddLine(a, e, color);
        AddLine(b, f, color);
        AddLine(c, g, color);
        AddLine(d, h, color);
        AddLine(e, f, color);
        AddLine(f, g, color);
        AddLine(g, h, color);
        AddLine(h, e, color);
    }

    void DrawDebugAxis(const Quaternion& rotation, const Vector3& origin) {
        const float half_size = 0.25f;
        AddLine(origin, origin + rotation.Right() * half_size, Vector3(1.f, 0.f, 0.f));
        AddLine(origin, origin + rotation.Up() * half_size, Vector3(0.f, 1.f, 0.f));
        AddLine(origin, origin + rotation.At() * half_size, Vector3(0.f, 0.f, 1.f));
    }

    void Draw(DebugDraw& debug_draw, const Matrix& viewproj) {
        vertex_buffer_offset = debug_draw.AlignBufferOffset();
        lines.ProcessIndex([&](const Line& line, unsigned index) {
            if (index > line_count)
                return;
            const auto color = Math::RGBA((unsigned char)(line.color.x * 255), (unsigned char)(line.color.y * 255), (unsigned char)(line.color.z * 255), 255);
            debug_draw.PushVertex(line.from, color, vertex_count);
            debug_draw.PushVertex(line.to, color, vertex_count);
        });
        debug_draw.AlignBufferOffset();
        size uniform_buffer_offset = debug_draw.PushData(sizeof(Matrix), (uint8*)&viewproj);
        debug_draw.AlignBufferOffset();
        debug_draw.SetConstantBuffer(uniform_buffer_offset);
        debug_draw.SetVertexBuffer(vertex_buffer_offset, vertex_count);
        debug_draw.DrawPrimitives(vertex_count, false);
    }
};

class DebugProfile : public NoCopy {
public:
    DebugProfile() {
        for (unsigned i = 0; i < Timings::BufferCount; ++i)
            frame_begin_times[i] = 0;
    }

    void ToggleProfileJobs() { is_jobs_enabled = !is_jobs_enabled; }

    void Swap(uint64 now) {
        debug_index = (debug_index + 1) % Timings::BufferCount;
        frame_begin_times[debug_index] = now;
    }

    void Draw(Profile& profile, DebugDraw& debug_draw, unsigned window_witdh, unsigned window_height) {
        if (is_jobs_enabled) {
            DrawTracks(profile, debug_draw, window_witdh, window_height);
        }
    }

private:
    struct StateJobs {
    public:
        StateJobs(unsigned window_witdh, unsigned window_height, uint64 frame_begin_time, uint64 frame_end_time) {
            Matrix::OrthoLH(2.f, 2.f, -1.f, 1.f, proj, proj_inverse);
            const uint64 frame_duration = frame_end_time - frame_begin_time;
            this->frame_begin_time = frame_begin_time;
            this->frame_end_time = frame_end_time;
            frame_span = (float)((frame_duration / 15000) + 1) * 16666.f;
            frame_indicator_count = Math::Min(2 + (unsigned)(frame_duration / 15000.f), (unsigned)4);
            pixel_size_u = 1.f / window_witdh;
            pixel_size_v = 1.f / window_height;
            border = 20.f * pixel_size_u;
            bound_left = 20.f * pixel_size_u;
            bound_right = Math::Min((frame_indicator_count - 1) * 140.f, (float)window_witdh - 20.f) * pixel_size_u;
            bound_span = bound_right - bound_left;
            trame_span = bound_span / (float)(frame_indicator_count - 1);
        }

        Matrix proj;
        Matrix proj_inverse;
        uint64 frame_begin_time;
        uint64 frame_end_time;
        unsigned frame_indicator_count;
        float z_captures = -0.999f;
        float z_axis = -1.f;
        float frame_span;
        float border;
        float capture_top;
        float capture_bottom;
        float bound_span;
        float trame_span;
        float pixel_size_u;
        float pixel_size_v;
        float bound_left;
        float bound_right;
        float bound_top;
        float bound_bottom;
    };

    FixedArray<uint64, Timings::BufferCount> frame_begin_times;
    unsigned debug_index = 0;
    bool is_jobs_enabled = false;

    void DrawTracks(Profile& profile, DebugDraw& debug_draw, unsigned window_witdh, unsigned window_height) {
        if (!is_jobs_enabled) {
            profile.Tracks().Process([&](auto& track) {
                track.Clear();
            });
        }
        const unsigned begin_index = (debug_index + 1) % Timings::BufferCount;
        const unsigned end_index = (debug_index + 2) % Timings::BufferCount;
        const uint64 frame_begin_time = frame_begin_times[begin_index];
        const uint64 frame_end_time = frame_begin_times[end_index];
        StateJobs state(window_witdh, window_height, frame_begin_time, frame_end_time);
        profile.Tracks().ProcessIndex([&](auto& track, unsigned index) {
            DrawTrack(debug_draw, state, track, index);
        });
    }

    static void DrawTrack(DebugDraw& debug_draw, StateJobs& state, Track& track, unsigned i) {
        const float bar_v = 14.f * state.pixel_size_v;
        const float capture_v = 0.65f;
        const float dist_v = bar_v + 4.f * state.pixel_size_v;
        const float offset_v = i * (capture_v + 0.05f);
        state.bound_top = 1.f - bar_v * 2.f - offset_v * dist_v;
        state.bound_bottom = state.bound_top + bar_v;
        state.capture_top = state.bound_top + bar_v * 0.05f;
        state.capture_bottom = state.capture_top + bar_v * capture_v;
        DrawCaptures(debug_draw, state, track);
        DrawIndicators(debug_draw, state);
    }

    static void DrawIndicators(DebugDraw& debug_draw, const StateJobs& state) {
        unsigned vertex_count = 0;
        const size uniform_buffer_offset = debug_draw.PushData(sizeof(Matrix), (uint8*)&state.proj);
        const size vertex_buffer_offset = debug_draw.AlignBufferOffset();
        for (unsigned i = 0; i < state.frame_indicator_count; i++) {
            const float u = state.bound_left + i * state.trame_span + state.border;
            DrawLine(debug_draw, u, u, state.bound_top, state.bound_bottom, state.z_axis, Color::Gray, vertex_count);
        }
        debug_draw.AlignBufferOffset();
        debug_draw.SetConstantBuffer(uniform_buffer_offset);
        debug_draw.SetVertexBuffer(vertex_buffer_offset, vertex_count);
        debug_draw.DrawPrimitives(vertex_count, false);
    }

    static void DrawCaptures(DebugDraw& debug_draw, const StateJobs& state, Track& track) {
        unsigned vertex_count = 0;
        const size uniform_buffer_offset = debug_draw.PushData(sizeof(Matrix), (uint8*)&state.proj);
        const size vertex_buffer_offset = debug_draw.AlignBufferOffset();
        track.Process([&](auto& capture, bool& stop) {
            if (capture.begin_time >= state.frame_end_time) {
                stop = true;
                return false;
            }
            bool remove = false;
            if (capture.end_time <= state.frame_end_time)
                remove = true;
            else
                stop = true;
            const auto begin_time = Math::Clamp(capture.begin_time, state.frame_begin_time, state.frame_end_time);
            const auto end_time = Math::Clamp(capture.end_time, state.frame_begin_time, state.frame_end_time);
            DrawCapture(debug_draw, state, track, begin_time, end_time, capture.color, vertex_count);
            return remove;
        });
        debug_draw.AlignBufferOffset();
        debug_draw.SetConstantBuffer(uniform_buffer_offset);
        debug_draw.SetVertexBuffer(vertex_buffer_offset, vertex_count);
        debug_draw.DrawPrimitives(vertex_count, true);
    }

    static void DrawCapture(DebugDraw& debug_draw, const StateJobs& state, const Track& track, uint64 begin_time, uint64 end_time, Color color, unsigned& vertex_count) {
        const float capture_begin = (float)(begin_time - state.frame_begin_time) / state.frame_span;
        const float capture_end = (float)(end_time - state.frame_begin_time) / state.frame_span;
        const float capture_left = state.bound_left + capture_begin * state.bound_span;
        const float capture_right = state.bound_left + capture_end * state.bound_span;
        const float left = capture_left + state.border;
        const float right = capture_right + state.border;
        DrawQuad(debug_draw, left, right, state.capture_top, state.capture_bottom, state.z_captures, color, vertex_count);
    }

    static void DrawLine(DebugDraw& debug_draw, float left, float right, float top, float bottom, float z, Color color, unsigned& vertex_count) {
        const Vector3 v0(left * 2.f - 1.f, top * 2.f - 1.f, z);
        const Vector3 v1(right * 2.f - 1.f, bottom * 2.f - 1.f, z);
        debug_draw.PushLine(v0, v1, (uint32)color, vertex_count);
    }

    static void DrawQuad(DebugDraw& debug_draw, float left, float right, float top, float bottom, float z, Color color, unsigned& vertex_count) {
        const Vector3 v0(left * 2.f - 1.f, top * 2.f - 1.f, z);
        const Vector3 v1(right * 2.f - 1.f, top * 2.f - 1.f, z);
        const Vector3 v2(right * 2.f - 1.f, bottom * 2.f - 1.f, z);
        const Vector3 v3(left * 2.f - 1.f, bottom * 2.f - 1.f, z);
        debug_draw.PushTriangle(v0, v1, v2, (uint32)color, vertex_count);
        debug_draw.PushTriangle(v0, v2, v3, (uint32)color, vertex_count);
    }
};
