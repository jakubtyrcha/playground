#include "pch.h"

#include "array.h"
#include "assertions.h"
#include "files.h"
#include "gfx.h"
#include "os.h"
#include <fmt/format.h>

#include "shader.h"

#include "imgui_impl_win32.h"
#include <imgui/imgui.h>

#include "Copy.h"
#include "Heightfield.h"
#include "MotionVectorDebug.h"
#include "Pointset.h"
#include "Rendering.h"
#include "Shapes.h"
#include "Taa.h"
#include "imgui_backend.h"
#include <fastnoise/FastNoise.h>

#include <math.h>

using namespace Playground;

void InitImgui()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    //io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }
}

namespace Playground {

enum class TAAPattern {
    Standard4x,
    Standard8x
};

void SetPattern(Viewport& viewport, TAAPattern pattern)
{
    viewport.taa_offsets.Clear();

    switch (pattern) {
    case TAAPattern::Standard4x: {
        // msaa 4x
        viewport.taa_offsets.PushBack({ -2, -6 });
        viewport.taa_offsets.PushBack({ 6, -2 });
        viewport.taa_offsets.PushBack({ -6, 2 });
        viewport.taa_offsets.PushBack({ 2, 6 });
    } break;
    case TAAPattern::Standard8x: {
        // msaa 8x
        viewport.taa_offsets.PushBack({ 1, -3 });
        viewport.taa_offsets.PushBack({ -1, 3 });
        viewport.taa_offsets.PushBack({ 5, 1 });
        viewport.taa_offsets.PushBack({ -3, -5 });
        viewport.taa_offsets.PushBack({ -5, 5 });
        viewport.taa_offsets.PushBack({ -7, -1 });
        viewport.taa_offsets.PushBack({ 3, 7 });
        viewport.taa_offsets.PushBack({ 7, -7 });
    } break;
    }

    for (Vector2& sample : viewport.taa_offsets) {
        sample /= 8.f;
    }

    viewport.taa_index = 0;
}

Array<Vector2> Generate2DGridSamples(i32 N, Vector2 v0, Vector2 v1)
{
    Array<Vector2> output;
    output.Reserve(N);

    Rng gen {};

    for (i32 i = 0; i < N; i++) {
        output.PushBack({ gen.F32UniformInRange(v0.x(), v1.x()), gen.F32UniformInRange(v0.y(), v1.y()) });
    }

    return output;
}

Vector2 RandomPointInAnnulus(f32 r0, f32 r1, Vector2 z)
{
    assert(r0 < r1);

    f32 theta = 2.f * Magnum::Math::Constants<f32>::pi() * z.x();
    f32 d = sqrtf(z.y() * (r1 * r1 - r0 * r0) + r0 * r0);

    return d * Vector2 { cosf(theta), sinf(theta) };
}

// Robert Bridson algorithm
Array<Vector2> Generate2DGridSamplesPoissonDisk(Vector2 v0, Vector2 v1, float min_distance, i32 samples_before_rejection)
{
    Array<Vector2> output;
    Rng gen {};

    constexpr i32 DIM = 2;
    const i32 k = samples_before_rejection;
    const f32 r = min_distance;
    const f32 r2 = r * r;

    const f32 cell_size = r / sqrtf(DIM);

    const Vector2 span = v1 - v0;
    const Vector2i grid_cells = Vector2i { Math::ceil((span / cell_size)) };
    const f32 inv_cell_size = 1.f / cell_size;

    Array<i32> grid;
    grid.ResizeUninitialised(grid_cells.x() * grid_cells.y());

    output.Reserve(grid.Size());

    for (i32 i = 0; i < grid_cells.x() * grid_cells.y(); i++) {
        grid[i] = -1;
    }

    Vector2 x0 { gen.F32UniformInRange(0.f, span.x()), gen.F32UniformInRange(0.f, span.y()) };
    Vector2i grid_coord = Vector2i { Math::floor(x0 * inv_cell_size) };

    output.PushBack(x0);
    grid[grid_coord.x() + grid_coord.y() * grid_cells.x()] = 0;

    Array<i32> active_list;
    active_list.PushBack(0);

    while (active_list.Size()) {
        i32 index = gen.I32UniformInRange(0, static_cast<i32>(active_list.Size()));
        Vector2 xi = output[active_list[index]];

        int tries = k;
        while (tries) {
            Vector2 offset = RandomPointInAnnulus(r, 2.f * r, { gen.F32Uniform(), gen.F32Uniform() });
            Vector2 s = xi + offset;

            bool rejected = false;
            Vector2i grid_coord = Vector2i { Math::floor(s * inv_cell_size) };

            assert(grid_coord != Vector2i { Math::floor(xi * inv_cell_size) });

            if (((s < Vector2 { 0.f }) || (span <= s)).any()) {
                rejected = true;
                goto break_double_loop;
            }

            assert((Vector2i { 0, 0 } <= grid_coord && grid_coord < grid_cells).all());

            for (i32 x = Max(0, grid_coord.x() - 2); x < Min(grid_cells.x(), grid_coord.x() + 3); x++) {
                for (i32 y = Max(0, grid_coord.y() - 2); y < Min(grid_cells.y(), grid_coord.y() + 3); y++) {
                    i32 tested_index = x + y * grid_cells.x();

                    if (grid[tested_index] == -1) {
                        continue;
                    }

                    if ((s - output[grid[tested_index]]).dot() < r2) {
                        rejected = true;
                        goto break_double_loop;
                    }
                }
            }

        break_double_loop:
            if (rejected) {
                tries--;
                continue;
            }

            i32 next_sample_index = static_cast<i32>(output.Size());
            assert(grid[grid_coord.x() + grid_coord.y() * grid_cells.x()] == -1);
            grid[grid_coord.x() + grid_coord.y() * grid_cells.x()] = next_sample_index;
            active_list.PushBack(next_sample_index);
            output.PushBack(s);
            assert(tries);
            break;
        }

        if (tries == 0) {
            active_list.RemoveAtAndSwapWithLast(index);
        }
    }

    for (Vector2& p : output) {
        p += v0;
    }

    return output;
}

namespace Gfx {
    void UpdateTexture2DSubresource(Device* device, Resource* resource, i32 subresource, Vector2i resource_size, DXGI_FORMAT fmt, const void* src, i32 src_pitch, i32 rows)
    {
        i32 upload_pitch = AlignedForward(src_pitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
        i64 upload_size = upload_pitch * rows;

        Gfx::Resource upload_buffer = device->CreateBuffer(
            D3D12_HEAP_TYPE_UPLOAD,
            upload_size,
            DXGI_FORMAT_UNKNOWN,
            D3D12_RESOURCE_FLAG_NONE,
            D3D12_RESOURCE_STATE_GENERIC_READ);

        u8* mapped_memory = nullptr;
        verify_hr(upload_buffer.resource_->Map(0, nullptr, reinterpret_cast<void**>(&mapped_memory)));
        for (i64 r = 0; r < resource_size.y(); r++) {
            memcpy(mapped_memory + r * upload_pitch, reinterpret_cast<const u8*>(src) + r * src_pitch, src_pitch);
        }

        upload_buffer.resource_->Unmap(0, nullptr);

        Gfx::Encoder encoder = device->CreateEncoder();
        D3D12_TEXTURE_COPY_LOCATION copy_src {
            .pResource = *upload_buffer.resource_,
            .Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT,
            .PlacedFootprint = {
                .Footprint = {
                    .Format = fmt,
                    .Width = static_cast<u32>(resource_size.x()),
                    .Height = static_cast<u32>(resource_size.y()),
                    .Depth = 1,
                    .RowPitch = static_cast<u32>(upload_pitch) } }
        };
        D3D12_TEXTURE_COPY_LOCATION copy_dst {
            .pResource = *resource->resource_,
            .Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX,
            .SubresourceIndex = As<u32>(subresource)
        };

        Gfx::Pass* copy_to_pass = device->graph_.AddSubsequentPass(Gfx::PassAttachments {}.Attach({ .resource = *resource->resource_, .subresource = subresource }, D3D12_RESOURCE_STATE_COPY_DEST));
        encoder.SetPass(copy_to_pass);
        encoder.GetCmdList()->CopyTextureRegion(&copy_dst, 0, 0, 0, &copy_src, nullptr);

        Gfx::Pass* transition_to_readable_pass = device->graph_.AddSubsequentPass(Gfx::PassAttachments {}.Attach({ .resource = *resource->resource_, .subresource = subresource }, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
        encoder.SetPass(transition_to_readable_pass);
        encoder.Submit();

        device->ReleaseWhenCurrentFrameIsDone(std::move(upload_buffer));
    }
}

Optional<f32> RayQuadIntersection2D(Vector2 origin, Vector2 direction, Vector2 v0, Vector2 v1, Vector2 v2, Vector2 v3)
{
    auto RaySegmentIntersection = [](Vector2 o, Vector2 d, Vector2 a, Vector2 b) -> Optional<f32> {
        Vector2 v1 = o - a;
        Vector2 v2 = b - a;
        Vector2 v3 { -d.y(), d.x() };

        f32 t1 = cross(v2, v1) / dot(v2, v3);
        f32 t2 = dot(v1, v3) / dot(v2, v3);

        if (t1 > 0.f && t2 >= 0.f && t2 <= 1.f) {
            return t1;
        }

        return NullOpt;
    };

    Optional<f32> t = NullOpt;
    if (Optional<f32> te = RaySegmentIntersection(origin, direction, v0, v1); te) {
        t = t ? Min(*t, *te) : *te;
    }
    if (Optional<f32> te = RaySegmentIntersection(origin, direction, v1, v2); te) {
        t = t ? Min(*t, *te) : *te;
    }
    if (Optional<f32> te = RaySegmentIntersection(origin, direction, v2, v3); te) {
        t = t ? Min(*t, *te) : *te;
    }
    if (Optional<f32> te = RaySegmentIntersection(origin, direction, v0, v3); te) {
        t = t ? Min(*t, *te) : *te;
    }

    return t;
};

}

namespace Playground {

f32 Frac(f32 x)
{
    return x - As<i32>(x);
}

// http://ns1.wili.cc/research/hfshadow/hfshadow.pdf
void UpdateHorizonTexture(f32 sun_azimuth,
    i32 heightfield_tex_res,
    Array<f32> const& heightfield_data,
    Vector3 span,
    Gfx::Device* device,
    Gfx::Resource* texture)
{
    i32 tex_res = heightfield_tex_res;
    Array<f32> const& data = heightfield_data;
    struct HorizonPoint {
        f32 h;
        f32 d;
    };

    i32 horizon_tex_res = As<i32>(ceilf(sqrtf(2.f) * tex_res));
    horizon_tex_res = AlignedForward(horizon_tex_res, 2);
    Array<f32> horizon_angle;
    horizon_angle.Resize(horizon_tex_res * horizon_tex_res);

    f32 quad_rotation = sun_azimuth - Math::Constants<f32>::piHalf();
    // find box edges (rotated positions)
    f32 tex_res_f = As<f32>(tex_res);
    const Vector2 original_v0 = { 0, 0 };
    const Vector2 original_v1 = { tex_res_f - 1.f, 0 };
    const Vector2 original_v2 = { tex_res_f - 1.f, tex_res_f - 1.f };
    const Vector2 original_v3 = { 0, tex_res_f - 1.f };

    Matrix3 A = Matrix3::translation(-original_v2 * 0.5f);
    Matrix3 B = Matrix3::rotation(Rad(quad_rotation));
    Matrix3 C = Matrix3::translation(Vector2 { As<f32>(horizon_tex_res - 1) * 0.5f });
    Matrix3 hm_texel_to_horizon_texel = C * B * A;
    const Vector2 v0 = hm_texel_to_horizon_texel.transformPoint(original_v0);
    const Vector2 v1 = hm_texel_to_horizon_texel.transformPoint(original_v1);
    const Vector2 v2 = hm_texel_to_horizon_texel.transformPoint(original_v2);
    const Vector2 v3 = hm_texel_to_horizon_texel.transformPoint(original_v3);
    // for every pixel, find collisions with the planes, adjust to pixel center
    // voila, run the sweep algorithm (adjust h extraction, bilinear tap)

    auto SampleHmBilinear = [&data, tex_res](Vector2 texel) -> f32 {
        texel.x() = Min(texel.x(), As<f32>(tex_res - 1) - 0.0001f);
        texel.y() = Min(texel.y(), As<f32>(tex_res - 1) - 0.0001f);
        i32 x = As<i32>(texel.x());
        i32 y = As<i32>(texel.y());

        f32 fracx = texel.x() - x;
        f32 fracy = texel.y() - y;

        f32 tap[4];
        tap[0] = data.At(x + y * tex_res);
        tap[1] = data.At(x + 1 + y * tex_res);
        tap[2] = data.At(x + (y + 1) * tex_res);
        tap[3] = data.At(x + 1 + (y + 1) * tex_res);

        return tap[0] * (1 - fracx) * (1 - fracy) + tap[1] * fracx * (1 - fracy) + tap[2] * (1 - fracx) * fracy + tap[3] * fracx * fracy;
    };

    // -quad_offset * neg rotation * 0.5f / res(tex_res)
    A = Matrix3::translation(original_v2 * 0.5f);
    B = Matrix3::rotation(-Rad(quad_rotation));
    C = Matrix3::translation(-Vector2 { As<f32>(horizon_tex_res - 1) * 0.5f });
    Matrix3 horizon_texel_to_hm_texel = A * B * C;

    f32 inv_tex_res = 1.f / tex_res_f;

    //f32 step_x = 1.f / As<f32>(horizon_tex_res);
    for (i32 a = 0; a < horizon_tex_res; a++) {
        f32 x = As<f32>(a);
        Vector2 origin { x, 0.f };
        Vector2 sweep { 0.f, 1.f };

        Optional<f32> t = RayQuadIntersection2D(origin, sweep, v0, v1, v2, v3);

        if (!t) {
            continue;
        }
        f32 y = origin.y() + *t;
        // snap to the next pixel
        y = ceilf(y) + 0.001f;

        Vector2 hm_texel = horizon_texel_to_hm_texel.transformPoint(Vector2 { x, y });
        Array<HorizonPoint> stack;
        f32 d = 0.f;

        if (((Vector2 { 0.f } <= hm_texel) && (hm_texel <= Vector2 { As<f32>(tex_res - 1) })).all()) {
            Vector2i hm_texeli = Vector2i { hm_texel };
            horizon_angle.At(As<i32>(x) + As<i32>(y) * horizon_tex_res) = 0.f;
            f32 h = SampleHmBilinear(hm_texel) * span.y();
            stack.PushBack({ .h = h, .d = d });

            y += 1.f;
            d += 1.f * span.x() * inv_tex_res;
            hm_texel = horizon_texel_to_hm_texel.transformPoint(Vector2 { x, y });
        }

        while (((Vector2 { 0.f } <= hm_texel) && (hm_texel <= Vector2 { As<f32>(tex_res - 1) })).all()) {
            f32 h = SampleHmBilinear(hm_texel) * span.y();

            auto [h1, d1] = stack.Last();
            f32 dh = h1 - h;
            f32 dd = d1 - d;
            f32 ha = dh / sqrt(dh * dh + dd * dd);
            stack.PushBack({ .h = h, .d = d });

            while (stack.Size() > 2) {
                auto [h2, d2] = stack.At(stack.Size() - 3);

                dh = h2 - h;
                dd = d2 - d;
                f32 ha2 = dh / sqrt(dh * dh + dd * dd);

                if (ha2 < ha) {
                    break;
                }

                ha = ha2;
                stack.RemoveAt(stack.Size() - 2);
            }

            horizon_angle.At(As<i32>(x) + As<i32>(y) * horizon_tex_res) = ha;

            y += 1.f;
            d += 1.f * span.x() * inv_tex_res;
            hm_texel = horizon_texel_to_hm_texel.transformPoint(Vector2 { x, y });
        }
    }

    auto SampleHorizonBilinear = [&horizon_angle, horizon_tex_res](Vector2 texel) -> f32 {
        texel.x() = Min(texel.x(), As<f32>(horizon_tex_res - 1) - 0.0001f);
        texel.y() = Min(texel.y(), As<f32>(horizon_tex_res - 1) - 0.0001f);
        i32 x = As<i32>(texel.x());
        i32 y = As<i32>(texel.y());

        f32 fracx = texel.x() - x;
        f32 fracy = texel.y() - y;

        f32 tap[4];
        tap[0] = horizon_angle.At(x + y * horizon_tex_res);
        tap[1] = horizon_angle.At(x + 1 + y * horizon_tex_res);
        tap[2] = horizon_angle.At(x + (y + 1) * horizon_tex_res);
        tap[3] = horizon_angle.At(x + 1 + (y + 1) * horizon_tex_res);

        return tap[0] * (1 - fracx) * (1 - fracy) + tap[1] * fracx * (1 - fracy) + tap[2] * (1 - fracx) * fracy + tap[3] * fracx * fracy;
    };

    Array<f32> data2;
    data2.ResizeUninitialised(tex_res * tex_res);

    for (i32 y = 0; y < tex_res; y++) {
        for (i32 x = 0; x < tex_res; x++) {
            data2.At(x + y * tex_res) = SampleHorizonBilinear(hm_texel_to_horizon_texel.transformPoint(Vector2 { As<f32>(x), As<f32>(y) }));
        }
    }

    Gfx::UpdateTexture2DSubresource(device, texture, 0, { tex_res, tex_res }, DXGI_FORMAT_R32_FLOAT, data2.Data(), sizeof(f32) * tex_res, tex_res);
}

struct TextureView {
    Array<f32> & texels;
    i32 width;
};

f32 SampleBilinear(TextureView texture, Vector2 texel)
{
    texel.x() = Min(texel.x(), As<f32>(texture.width - 1) - 0.0001f);
    texel.y() = Min(texel.y(), As<f32>(texture.width - 1) - 0.0001f);
    i32 x = As<i32>(texel.x());
    i32 y = As<i32>(texel.y());

    f32 fracx = texel.x() - x;
    f32 fracy = texel.y() - y;

    f32 tap[4];
    tap[0] = texture.texels.At(x + y * texture.width);
    tap[1] = texture.texels.At(x + 1 + y * texture.width);
    tap[2] = texture.texels.At(x + (y + 1) * texture.width);
    tap[3] = texture.texels.At(x + 1 + (y + 1) * texture.width);

    return tap[0] * (1 - fracx) * (1 - fracy) + tap[1] * fracx * (1 - fracy) + tap[2] * (1 - fracx) * fracy + tap[3] * fracx * fracy;
};

// https://weigert.vsos.ethz.ch/2020/04/10/simple-particle-based-hydraulic-erosion/
struct ErosionParticles {
    struct Particle {
        Vector2 position;
        Vector2 velocity;
        float volume;
        float sediment;
    };

    AABox2D area_;
    Array<f32> * heightfield_ = nullptr;
    i32 heightfield_res_;

    f32 density_;
    f32 friction_;
    f32 deposition_rate_;
    f32 evap_rate_;

    Vector2 world_to_texcoord_;
    Vector3 span_;
    Vector2 sample_mult_;

    Array<Particle> particles_;
    Rng generator_;

    f32 SampleHeight(Vector2 texcoord) const
    {
        return SampleBilinear({ .texels = *heightfield_,
                           .width = heightfield_res_ },
            texcoord * sample_mult_) * span_.y();
    }

    f32& GetHeight(Vector2i position)
    {
        return heightfield_->At(position.x() + position.y() * heightfield_res_);
    }

    Vector3 SurfaceNormal(Vector2 texcoord) const
    {
        f32 delta = 0.01f / As<f32>(heightfield_res_ - 1);

        f32 h10 = SampleHeight(texcoord + Vector2 { delta, 0.f });
        f32 nh10 = SampleHeight(texcoord + Vector2 { -delta, 0.f });
        f32 h01 = SampleHeight(texcoord + Vector2 { 0.f, delta });
        f32 nh01 = SampleHeight(texcoord + Vector2 { 0.f, -delta });

        delta *= 2.f * span_.x();
        return (Vector3 { (nh10 - h10) / delta, 1.f, (nh01 - h01) / delta }).normalized();
    }

    void Spawn()
    {
        Vector2 span = area_.Span();
        Vector2 pos { generator_.F32UniformInRange(0.f, span.x()), generator_.F32UniformInRange(0.f, span.y()) };
        particles_.PushBack({ .position = pos, .velocity = {} , .volume = 1.f, .sediment = 0.f});
    }

    void Tick(f32 dt)
    {
        for(i32 i=0; i<particles_.Size(); i++) 
        {
            Particle & p = particles_[i];
            Vector3 n = SurfaceNormal(p.position * world_to_texcoord_);
            Vector2 prev_position = p.position;

            p.velocity += dt * Vector2 { n.x(), n.z() } / (p.volume * density_);
            p.position += dt * p.velocity;
            p.velocity *= (1.f - dt * friction_);

            f32 c_eq = p.volume * p.velocity.length() * (SampleHeight(p.position * world_to_texcoord_) - SampleHeight(prev_position * world_to_texcoord_)); // * difference in height?
            c_eq = Max(0.f, c_eq);
            f32 c_diff = c_eq - p.sediment;
            p.sediment += dt * c_diff * deposition_rate_;

            GetHeight(Vector2i { p.position }) -= dt * c_diff * deposition_rate_ * p.volume;

            p.volume *= (1.f - dt * evap_rate_);

            if ((p.position < Vector2 { 0.f } || Vector2 { span_.x(), span_.z() } <= p.position).any() || p.volume < 0.01f) 
            {
                particles_.RemoveAtAndSwapWithLast(i);
            }
        }
    }
};

}

int main(int argc, char** argv)
{
    Gfx::Device device;

    InitImgui();

    ImGuiRenderer imgui_renderer;
    imgui_renderer.Init(&device);

    ImmediateModeShapeRenderer shape_renderer;
    shape_renderer.Init(&device);

    Pointset pointset;

    i32 N = 1000;
    Vector2 v0 { -50.f, -50.f };
    Vector2 v1 { 50.f, 50.f };

    f32 span = Min((v1 - v0).x(), (v1 - v0).y());
    f32 pointsize = 0.125f * span / sqrtf(static_cast<f32>(N));
    //for (Vector2 p : Generate2DGridSamples(N, v0, v1))
    FastNoise noise_gen;
    noise_gen.SetNoiseType(FastNoise::SimplexFractal);
    noise_gen.SetFrequency(10.f);

    FastNoise noise_gen_hf;
    noise_gen_hf.SetNoiseType(FastNoise::SimplexFractal);
    noise_gen_hf.SetSeed(17);
    noise_gen_hf.SetFrequency(0.1f);

    for (Vector2 p : Generate2DGridSamplesPoissonDisk(v0, v1, 3.f * pointsize, 30)) {
        Vector2 c = ((p - v0) / (v1 - v0));
        f32 y = noise_gen.GetNoise(p.x(), p.y());
        y += noise_gen_hf.GetNoise(p.x(), p.y()) * 0.05f;
        pointset.Add({ p.x(), y, p.y() }, pointsize, Color4 { 0.5f, y * 0.5f + 0.5f, 0.5f, 1.f });
    }

    PointsetRenderer pointset_renderer;
    pointset_renderer.Init(&device);

    pointset_renderer.pointset_ = &pointset;

    HeightfieldRenderer heightfield;
    heightfield.Init(&device);
    heightfield.bounding_box_.vec_min = { -50.f, -20.f, -50.f };
    heightfield.bounding_box_.vec_max = { 50.f, 0.f, 50.f };
    heightfield.resolution_ = { 128, 128 };
    heightfield.CreateIB();
    heightfield.light_dir_ = Vector3 { 0, 1, 0 }.normalized();

    f32 sun_azimuth = 0.f;
    f32 sun_altitude = Math::Constants<f32>::pi() * 0.5f * 1.f / 3.f;

    Array<f32> data;
    i32 tex_res;
    Vector3 hf_span = heightfield.bounding_box_.vec_max - heightfield.bounding_box_.vec_min;
    {
        FastNoise fn;
        fn.SetFrequency(5.f / As<f32>(heightfield.resolution_.x()));
        noise_gen_hf.SetFrequency(200.f / As<f32>(heightfield.resolution_.x()));
        tex_res = heightfield.resolution_.x();

        heightfield.heightfield_texture_ = device.CreateTexture2D(D3D12_HEAP_TYPE_DEFAULT, { tex_res, tex_res }, DXGI_FORMAT_R32_FLOAT, 1, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_COPY_DEST);
        heightfield.heightfield_srv_ = device.CreateDescriptor(&*heightfield.heightfield_texture_, D3D12_SHADER_RESOURCE_VIEW_DESC { .Format = DXGI_FORMAT_R32_FLOAT, .ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D, .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING, .Texture2D = { .MipLevels = 1 } },
            Gfx::Lifetime::Manual);

        data.Reserve(tex_res * tex_res);
        for (i32 y = 0; y < tex_res; y++) {
            for (i32 x = 0; x < tex_res; x++) {
                f32 h = fn.GetNoise(static_cast<f32>(x), static_cast<f32>(y));
                h += noise_gen_hf.GetNoise(static_cast<f32>(x), static_cast<f32>(y)) * 0.05f;
                data.PushBack(h * 0.5f + 0.5f);
            }
        }

        Gfx::UpdateTexture2DSubresource(&device, &*heightfield.heightfield_texture_, 0, { tex_res, tex_res }, DXGI_FORMAT_R32_FLOAT, data.Data(), sizeof(f32) * tex_res, tex_res);

        f32 phi = sun_azimuth;
        f32 theta = sun_altitude;
        heightfield.light_dir_ = Vector3 { sin(theta) * cos(phi), cos(theta), sin(theta) * sin(phi) };

        heightfield.horizon_angle_texture_ = device.CreateTexture2D(D3D12_HEAP_TYPE_DEFAULT, { tex_res, tex_res }, DXGI_FORMAT_R32_FLOAT, 1, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_COPY_DEST);
        heightfield.horizon_angle_srv_ = device.CreateDescriptor(&*heightfield.horizon_angle_texture_, D3D12_SHADER_RESOURCE_VIEW_DESC { .Format = DXGI_FORMAT_R32_FLOAT, .ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D, .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING, .Texture2D = { .MipLevels = 1 } },
            Gfx::Lifetime::Manual);

        //Gfx::UpdateTexture2DSubresource(&device, &*heightfield.horizon_angle_texture_, 0, { horizon_tex_res, horizon_tex_res }, DXGI_FORMAT_R32_FLOAT, horizon_angle.Data(), sizeof(f32) * horizon_tex_res, horizon_tex_res);

        UpdateHorizonTexture(sun_azimuth, tex_res, data, hf_span, &device, &*heightfield.horizon_angle_texture_);
    }

    ErosionParticles particles;
    particles.area_.vec_min = heightfield.bounding_box_.vec_min.xz();
    particles.area_.vec_max = heightfield.bounding_box_.vec_max.xz();
    particles.heightfield_ = &data;
    particles.heightfield_res_ = tex_res;
    
    particles.density_ = 1.f;
    particles.friction_ = 0.05f;
    particles.deposition_rate_ = 0.1f;
    particles.evap_rate_ = 0.001f;
    //
    particles.span_ = hf_span;
    particles.sample_mult_ = Vector2{ As<f32>(particles.heightfield_res_ - 1) };
    particles.world_to_texcoord_ = Vector2 { 1.f } / hf_span.xz();


    ImTextureID hm_debug_handle = imgui_renderer.RegisterHandle(heightfield.heightfield_srv_);
    ImTextureID debug_handle = imgui_renderer.RegisterHandle(heightfield.horizon_angle_srv_);

    TAA taa;
    taa.Init(&device);

    MotionVectorDebug motion_debug;
    motion_debug.Init(&device);

    //Rendering::SphereTracer sphere_tracer;
    //sphere_tracer.Init(&device);

    // TODO: who should own backbuffers num?
    // Device, swapchain?
    i32 backbuffers_num = 3;

    // TODO: this should be one call creating (device, window, swapchain) tuple?
    // one device can have many (window, swapchain) pairs
    Os::Window* window = Os::CreateOsWindow({ 1920, 1080 });
    Gfx::Swapchain* window_swapchain = device.CreateSwapchain(window, backbuffers_num);
    // this is here, because WndProc needs the mappings ready
    window->Init();
    ImGui_ImplWin32_Init(window->hwnd_);

    Array<FrameData> frame_data_queue;

    i32 max_frames_queued_ = 3;

    struct ScreenResources {
        Gfx::Device* device = nullptr;
        Gfx::Resource final_texture;
        Gfx::Resource colour_texture;
        Gfx::Resource prev_colour_texture;
        Gfx::Resource motion_vectors_texture;
        Gfx::Resource depth_buffer;
        Vector2i resolution;

        DXGI_FORMAT colour_texture_format = DXGI_FORMAT_R16G16B16A16_FLOAT;

        void Resize(Vector2i in_resolution)
        {
            if (resolution == in_resolution) {
                return;
            }

            resolution = in_resolution;

            final_texture = device->CreateTexture2D(D3D12_HEAP_TYPE_DEFAULT, resolution, colour_texture_format, 1, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_RESOURCE_STATE_RENDER_TARGET);
            colour_texture = device->CreateTexture2D(D3D12_HEAP_TYPE_DEFAULT, resolution, colour_texture_format, 1, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_RESOURCE_STATE_RENDER_TARGET);
            prev_colour_texture = device->CreateTexture2D(D3D12_HEAP_TYPE_DEFAULT, resolution, colour_texture_format, 1, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_RESOURCE_STATE_RENDER_TARGET);
            motion_vectors_texture = device->CreateTexture2D(D3D12_HEAP_TYPE_DEFAULT, resolution, DXGI_FORMAT_R16G16_FLOAT, 1, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_RESOURCE_STATE_RENDER_TARGET);
            depth_buffer = device->CreateTexture2D(D3D12_HEAP_TYPE_DEFAULT, resolution, DXGI_FORMAT_D24_UNORM_S8_UINT, 1, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL, D3D12_RESOURCE_STATE_DEPTH_WRITE);
        }
    };

    ScreenResources screen_resources { .device = &device };
    screen_resources.Resize(window->resolution_);

    int frames_ctr = 3;

    Array<Gfx::Waitable> frame_waitables;

    Viewport main_viewport {};

    main_viewport.camera_look_at = { 0, 0, 0 };
    main_viewport.camera_position = { 5, 5, -5 };
    main_viewport.camera_up = { 0, 1, 0 };
    main_viewport.resolution = window->resolution_;
    main_viewport.fov_y = Magnum::Math::Constants<float>::piHalf();
    main_viewport.near_plane = 0.1f;
    main_viewport.far_plane = 1000.f;

    struct NoiseWidget {
        Optional<Gfx::Resource> noise_texture;
        Gfx::DescriptorHandle noise_texture_srv_handle;
        ImTextureID im_tex_handle;
    };

    NoiseWidget noise_widget;

    i32 taa_pattern = 0;
    SetPattern(main_viewport, static_cast<TAAPattern>(taa_pattern));

    while (window->PumpMessages()) {
        i32 current_backbuffer_index = window_swapchain->swapchain_->GetCurrentBackBufferIndex();

        screen_resources.Resize(window->resolution_);
        main_viewport.resolution = window->resolution_;

        f32 camera_sensivity = 0.1f;
        if (window->user_input_.keys_down_['W']) {
            main_viewport.TranslateCamera(main_viewport.GetLookToVector() * camera_sensivity);
        }
        if (window->user_input_.keys_down_['S']) {
            main_viewport.TranslateCamera(-main_viewport.GetLookToVector() * camera_sensivity);
        }
        if (window->user_input_.keys_down_['A']) {
            main_viewport.TranslateCamera(-main_viewport.GetRightVector() * camera_sensivity);
        }
        if (window->user_input_.keys_down_['D']) {
            main_viewport.TranslateCamera(main_viewport.GetRightVector() * camera_sensivity);
        }

        if (window->user_input_.keys_down_['R'] && window->user_input_.keys_down_[0x10]) {
            while (frame_waitables.Size()) {
                frame_waitables.RemoveAt(0).Wait();
            }

            Gfx::ReloadShaders();
        }

        ImGuiIO& io = ImGui::GetIO();

        if (!io.WantCaptureMouse && window->user_input_.mouse_down_[0]) {
            f32 delta_x = io.MouseDelta.x / static_cast<f32>(window->resolution_.x());
            f32 delta_y = -io.MouseDelta.y / static_cast<f32>(window->resolution_.y());

            //
            Vector4 clip_space_point_to { delta_x * 2.f, delta_y * 2.f, 1.f, 1.f };

            Vector4 world_point_to = main_viewport.GetInvViewProjectionMatrix() * clip_space_point_to;

            main_viewport.camera_look_at = main_viewport.camera_position + world_point_to.xyz().normalized();
        }

        static bool show_motionvectors = false;
        static bool show_axes_helper = true;
        static f32 erosion_step = 0.01f;

        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        static i32 override_taa_sample = -1;
        if (override_taa_sample != -1) {
            main_viewport.taa_index = override_taa_sample;
        }

        ViewportRenderContext viewport_render_context = BuildViewportRenderContext(&device, &main_viewport);

        if (show_axes_helper) {
            ShapeVertex vertices[] = {
                { .position = { 0, 0, 0 }, .colour = { 255, 0, 0, 1 } },
                { .position = { 1, 0, 0 }, .colour = { 255, 0, 0, 1 } },
                { .position = { 0, 0, 0 }, .colour = { 0, 255, 0, 1 } },
                { .position = { 0, 1, 0 }, .colour = { 0, 255, 0, 1 } },
                { .position = { 0, 0, 0 }, .colour = { 0, 0, 255, 1 } },
                { .position = { 0, 0, 1 }, .colour = { 0, 0, 255, 1 } },
            };
            shape_renderer.vertices_.Append(vertices, _countof(vertices));
        }

        {
            ImGui::Begin("Perf");
            ImGui::Text("Time: %f ms", io.DeltaTime * 1000.f);
            ImGui::End();

            ImGui::Begin("Controls");

            if (ImGui::CollapsingHeader("View")) {
                ImGui::SliderFloat("Near plane", &main_viewport.near_plane, 0.0001f, 1.f);
                ImGui::SliderFloat("TAA decay", &main_viewport.history_decay, 0.f, 1.f);
                ImGui::SliderInt("TAA sample", &override_taa_sample, -1, static_cast<i32>(main_viewport.taa_offsets.Size()) - 1);
                const char* items[] = { "MSAA 4x", "MSAA 8x" };
                if (ImGui::Combo("TAA pattern", &taa_pattern, items, _countof(items))) {
                    SetPattern(main_viewport, static_cast<TAAPattern>(taa_pattern));
                }
            }

            if (ImGui::CollapsingHeader("Debug")) {
                ImGui::Checkbox("Axes", &show_axes_helper);
                ImGui::Checkbox("Motion vectors", &show_motionvectors);

                ImGui::Text("Particles num: %d", particles.particles_.Size());

                ImGui::SliderFloat("Density", &particles.density_, 0.01f, 1000.f);
                ImGui::SliderFloat("Friction", &particles.friction_, 0.01f, 10.f);
                ImGui::SliderFloat("Deposition rate", &particles.deposition_rate_, 0.01f, 1000.f);
                ImGui::SliderFloat("Evap rate", &particles.evap_rate_, 0.001f, 0.1f);
                ImGui::SliderFloat("dT", &erosion_step, 0.01f, 1.f);

                static i32 num_spawned = 1000;
                ImGui::SliderInt("Spawn num", &num_spawned, 1, 10000);
                if (ImGui::Button("Spawn particle")) {
                    for (i32 i = 0; i < num_spawned; i++) {
                        particles.Spawn();
                    }
                }

                ImGui::Image(hm_debug_handle, { As<f32>(tex_res), As<f32>(tex_res) });
                ImGui::Image(debug_handle, { As<f32>(tex_res), As<f32>(tex_res) });
            }

            ImGui::End();

            ImGui::Begin("Light");
            ImGui::SliderAngle("Azimuth", &sun_azimuth, 0.f, 360.f);
            ImGui::SliderAngle("Altitude", &sun_altitude, 0.f, 90.f);
            ImGui::End();

            ImGui::Begin("Noise");
            {
                struct NoiseTypeString {
                    FastNoise::NoiseType type;
                    const char* str;
                };
                constexpr NoiseTypeString noise_type_items[] = {
                    { FastNoise::Value, "Value" },
                    { FastNoise::ValueFractal, "ValueFractal" },
                    { FastNoise::Perlin, "Perlin" },
                    { FastNoise::PerlinFractal, "PerlinFractal" },
                    { FastNoise::Simplex, "Simplex" },
                    { FastNoise::SimplexFractal, "SimplexFractal" },
                    { FastNoise::Cellular, "Cellular" },
                    { FastNoise::WhiteNoise, "WhiteNoise" },
                    { FastNoise::Cubic, "Cubic" },
                    { FastNoise::CubicFractal, "CubicFractal" },
                };

                struct FractalTypeString {
                    FastNoise::FractalType type;
                    const char* str;
                };
                constexpr FractalTypeString fractal_type_items[] = {
                    { FastNoise::FBM, "FBM" },
                    { FastNoise::Billow, "Billow" },
                    { FastNoise::RigidMulti, "RigidMulti" },
                };

                static bool dirty = true;
                static i32 noise_type_index = 0;
                static f32 noise_freq = 0.01f;
                static i32 fractal_type_index = 0;
                static i32 tex_res = 512;

                {
                    Array<const char*> combo_items;
                    for (NoiseTypeString item : noise_type_items) {
                        combo_items.PushBack(item.str);
                    }

                    if (ImGui::Combo("Noise type", &noise_type_index, combo_items.Data(), static_cast<i32>(combo_items.Size()))) {
                        dirty = true;
                    }
                }
                if (ImGui::SliderFloat("Frequency", &noise_freq, 0.01f, 1.f)) {
                    dirty = true;
                }
                {
                    Array<const char*> combo_items;
                    for (FractalTypeString item : fractal_type_items) {
                        combo_items.PushBack(item.str);
                    }

                    if (ImGui::Combo("Fractal type", &fractal_type_index, combo_items.Data(), static_cast<i32>(combo_items.Size()))) {
                        dirty = true;
                    }
                }

                if (dirty) {
                    dirty = false;

                    FastNoise fn;
                    fn.SetNoiseType(noise_type_items[noise_type_index].type);
                    fn.SetFrequency(noise_freq);
                    fn.SetFractalType(fractal_type_items[fractal_type_index].type);

                    if (!noise_widget.noise_texture) {
                        noise_widget.noise_texture = device.CreateTexture2D(D3D12_HEAP_TYPE_DEFAULT, { tex_res, tex_res }, DXGI_FORMAT_R32_FLOAT, 1, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_COPY_DEST);
                        noise_widget.noise_texture_srv_handle = device.CreateDescriptor(&*noise_widget.noise_texture, D3D12_SHADER_RESOURCE_VIEW_DESC { .Format = DXGI_FORMAT_R32_FLOAT, .ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D, .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING, .Texture2D = { .MipLevels = 1 } },
                            Gfx::Lifetime::Manual);

                        noise_widget.im_tex_handle = imgui_renderer.RegisterHandle(noise_widget.noise_texture_srv_handle);
                    }

                    Array<f32> data;
                    data.Reserve(tex_res * tex_res);
                    for (i32 y = 0; y < tex_res; y++) {
                        for (i32 x = 0; x < tex_res; x++) {
                            data.PushBack(fn.GetNoise(static_cast<f32>(x), static_cast<f32>(y)) * 0.5f + 0.5f);
                        }
                    }

                    Gfx::UpdateTexture2DSubresource(&device, &*noise_widget.noise_texture, 0, { tex_res, tex_res }, DXGI_FORMAT_R32_FLOAT, data.Data(), sizeof(f32) * tex_res, tex_res);
                }

                ImGui::Image(noise_widget.im_tex_handle, { static_cast<f32>(tex_res), static_cast<f32>(tex_res) });
                ImGui::Text("( %f, %f )", static_cast<f32>(tex_res) * noise_freq, static_cast<f32>(tex_res) * noise_freq);
            }

            ImGui::End();
        }

        f32 phi = sun_azimuth;
        f32 theta = sun_altitude;
        heightfield.light_dir_ = Vector3 { sin(theta) * cos(phi), cos(theta), sin(theta) * sin(phi) };

        if (0) {
            for (i32 y = 0; y < particles.heightfield_res_; y+=4) {
                for (i32 x = 0; x < particles.heightfield_res_; x+=4) {
                    Vector2 pos_xz = Vector2 { As<f32>(x), As<f32>(y) } + Vector2 { 0.25f };
                    Vector2 texcoord = pos_xz / Vector2 { As<f32>(particles.heightfield_res_ - 1) };
                    Vector3 normal = particles.SurfaceNormal(texcoord);

                    pos_xz = texcoord * particles.area_.Span() + particles.area_.vec_min;
                    Vector3 position = { pos_xz.x(), particles.SampleHeight(texcoord) + heightfield.bounding_box_.vec_min.y(), pos_xz.y() };

                    shape_renderer.vertices_.PushBack({ .position = position, .colour = ColourR8G8B8A8U { 255, 0, 0, 0 } });
                    shape_renderer.vertices_.PushBack({ .position = position + normal, .colour = ColourR8G8B8A8U { 255, 0, 0, 0 } });
                }
            }
        }

        for(i32 i = 0; i<particles.particles_.Size(); i++) {
            Vector2 pos_xz = particles.particles_[i].position;
            Vector2 texcoord = pos_xz / hf_span.xz();
            Vector3 position = Vector3{ pos_xz.x(), particles.SampleHeight(texcoord), pos_xz.y() } + heightfield.bounding_box_.vec_min;

            shape_renderer.vertices_.PushBack({ .position = position, .colour = ColourR8G8B8A8U { 0, 255, 0, 0 } });
            shape_renderer.vertices_.PushBack({ .position = position + Vector3 { 0, 1, 0 }, .colour = ColourR8G8B8A8U { 0, 255, 0, 0 } });
        }

        particles.Tick(erosion_step);

        UpdateTexture2DSubresource(&device, &*heightfield.heightfield_texture_, 0, { tex_res, tex_res }, DXGI_FORMAT_R32_FLOAT, data.Data(), sizeof(f32) * tex_res, tex_res);
        UpdateHorizonTexture(sun_azimuth, tex_res, data, hf_span, &device, &*heightfield.horizon_angle_texture_);

        ID3D12Resource* current_backbuffer = *window_swapchain->backbuffers_[current_backbuffer_index];

        Gfx::Pass* clear_pass = device.graph_.AddSubsequentPass(Gfx::PassAttachments {}
                                                                    .Attach({ .resource = *screen_resources.colour_texture.resource_ }, D3D12_RESOURCE_STATE_RENDER_TARGET)
                                                                    .Attach({ .resource = *screen_resources.motion_vectors_texture.resource_ }, D3D12_RESOURCE_STATE_RENDER_TARGET)
                                                                    .Attach({ .resource = *screen_resources.depth_buffer.resource_ }, D3D12_RESOURCE_STATE_DEPTH_WRITE));

        heightfield.colour_target_ = &screen_resources.colour_texture;
        heightfield.depth_buffer_ = &screen_resources.depth_buffer;
        heightfield.motionvec_target_ = &screen_resources.motion_vectors_texture;
        heightfield.AddPassesToGraph();

        pointset_renderer.colour_target_ = &screen_resources.colour_texture;
        pointset_renderer.depth_buffer_ = &screen_resources.depth_buffer;
        pointset_renderer.motionvec_target_ = &screen_resources.motion_vectors_texture;
        //pointset_renderer.AddPassesToGraph();

        taa.AddPassesToGraph(&screen_resources.final_texture, current_backbuffer, &screen_resources.colour_texture, &screen_resources.depth_buffer, &screen_resources.prev_colour_texture, &screen_resources.motion_vectors_texture);

        //copy.AddPassesToGraph(current_backbuffer, &screen_resources.final_texture);

        if (show_motionvectors) {
            motion_debug.AddPassesToGraph(current_backbuffer, &screen_resources.motion_vectors_texture);
        }

        Gfx::Pass* render_ui_pass = device.graph_.AddSubsequentPass(Gfx::PassAttachments {}
                                                                        .Attach({ .resource = current_backbuffer }, D3D12_RESOURCE_STATE_RENDER_TARGET));

        Gfx::Pass* present_pass = device.graph_.AddSubsequentPass(Gfx::PassAttachments {}
                                                                      .Attach({ .resource = current_backbuffer }, D3D12_RESOURCE_STATE_PRESENT));

        Gfx::Encoder encoder = device.CreateEncoder();

        ImGui::Render();

        encoder.SetPass(clear_pass);

        auto create_rtv_handle = [&device](Gfx::Resource* texture, DXGI_FORMAT fmt) -> D3D12_CPU_DESCRIPTOR_HANDLE {
            D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle = device.rtvs_descriptor_heap_.heap_->GetCPUDescriptorHandleForHeapStart();
            D3D12_RENDER_TARGET_VIEW_DESC rtv_desc {
                .Format = fmt,
                .ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D,
                .Texture2D = {}
            };
            rtv_handle.ptr += device.rtvs_descriptor_heap_.AllocateTable(1) * device.rtvs_descriptor_heap_.increment_;
            device.device_->CreateRenderTargetView(*texture->resource_, &rtv_desc, rtv_handle);
            return rtv_handle;
        };

        auto create_rtv_handle_ = [&device](ID3D12Resource* texture, DXGI_FORMAT fmt) -> D3D12_CPU_DESCRIPTOR_HANDLE {
            D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle = device.rtvs_descriptor_heap_.heap_->GetCPUDescriptorHandleForHeapStart();
            D3D12_RENDER_TARGET_VIEW_DESC rtv_desc {
                .Format = fmt,
                .ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D,
                .Texture2D = {}
            };
            rtv_handle.ptr += device.rtvs_descriptor_heap_.AllocateTable(1) * device.rtvs_descriptor_heap_.increment_;
            device.device_->CreateRenderTargetView(texture, &rtv_desc, rtv_handle);
            return rtv_handle;
        };

        D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle = create_rtv_handle(&screen_resources.colour_texture, DXGI_FORMAT_R16G16B16A16_FLOAT);
        D3D12_CPU_DESCRIPTOR_HANDLE mv_rtv_handle = create_rtv_handle(&screen_resources.motion_vectors_texture, DXGI_FORMAT_R16G16_FLOAT);

        f32 clear_colour[4] = {};
        encoder.GetCmdList()->ClearRenderTargetView(rtv_handle, clear_colour, 0, nullptr);
        encoder.GetCmdList()->ClearRenderTargetView(mv_rtv_handle, clear_colour, 0, nullptr);

        D3D12_DEPTH_STENCIL_VIEW_DESC dsv_desc {
            .Format = DXGI_FORMAT_D24_UNORM_S8_UINT,
            .ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D,
            .Flags = D3D12_DSV_FLAG_NONE,
            .Texture2D = {}
        };
        D3D12_CPU_DESCRIPTOR_HANDLE dsv_handle = device.dsvs_descriptor_heap_.heap_->GetCPUDescriptorHandleForHeapStart();
        dsv_handle.ptr += device.dsvs_descriptor_heap_.AllocateTable(1) * device.dsvs_descriptor_heap_.increment_;
        device.device_->CreateDepthStencilView(*screen_resources.depth_buffer.resource_, &dsv_desc, dsv_handle);

        encoder.GetCmdList()->ClearDepthStencilView(dsv_handle, D3D12_CLEAR_FLAG_DEPTH, 1.f, 0, 0, nullptr);

        //sphere_tracer.Render(&encoder, &main_viewport, rtv_handle);
        //particle_generator.Render(&encoder, &main_viewport, rtv_handle, dsv_handle, mv_rtv_handle);
        D3D12_CPU_DESCRIPTOR_HANDLE handles[] = { rtv_handle, mv_rtv_handle };
        heightfield.Render(&encoder, &viewport_render_context, handles, dsv_handle);
        //pointset_renderer.Render(&encoder, &viewport_render_context, handles, dsv_handle);

        shape_renderer.Render(&encoder, &viewport_render_context, rtv_handle, mv_rtv_handle);

        D3D12_CPU_DESCRIPTOR_HANDLE final_rtv_handle = create_rtv_handle(&screen_resources.final_texture, DXGI_FORMAT_R16G16B16A16_FLOAT);

        D3D12_CPU_DESCRIPTOR_HANDLE bb_rtv_handle = create_rtv_handle_(current_backbuffer, DXGI_FORMAT_R8G8B8A8_UNORM);

        taa.Render(&encoder, &viewport_render_context, final_rtv_handle, bb_rtv_handle);

        //copy.Render(&encoder, &main_viewport, bb_rtv_handle);

        if (show_motionvectors) {
            motion_debug.Render(&encoder, &viewport_render_context, bb_rtv_handle);
        }

        encoder.SetPass(render_ui_pass);

        imgui_renderer.RenderDrawData(ImGui::GetDrawData(), &encoder, bb_rtv_handle);

        encoder.SetPass(present_pass);

        // we should wait for the presenting being done before we access that backbuffer again?
        if (frame_waitables.Size() == backbuffers_num) {
            frame_waitables.RemoveAt(0).Wait();
        }

        encoder.Submit();

        verify_hr(window_swapchain->swapchain_->Present(1, 0));
        device.AdvanceFence();
        device.RecycleResources();

        Gfx::Waitable frame_end_fence = device.GetWaitable();
        frame_waitables.PushBack(frame_end_fence);
        viewport_render_context.frame_payload.waitable_ = frame_end_fence;
        frame_data_queue.PushBackRvalueRef(std::move(viewport_render_context.frame_payload));
        current_backbuffer_index = (current_backbuffer_index + 1) % backbuffers_num;

        std::swap(screen_resources.final_texture, screen_resources.prev_colour_texture);
    }

    device.GetWaitable().Wait();

    ImGui_ImplWin32_Shutdown();
    imgui_renderer.Shutdown();
    shape_renderer.Shutdown();
    ImGui::DestroyContext();

    return 0;
}
