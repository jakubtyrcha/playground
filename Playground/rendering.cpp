#include "pch.h"

#include "rendering.h"



namespace Playground {

using namespace Gfx;

f32 Viewport::GetAspectRatio() const
{
    return resolution.x() / static_cast<f32>(resolution.y());
}

Vector3 Viewport::GetLookToVector() const
{
    return (camera_look_at - camera_position).normalized();
}

Vector3 Viewport::GetRightVector() const
{
    Vector3 z = (camera_look_at - camera_position).normalized();
    return Math::cross(camera_up, z).normalized();
}

void Viewport::TranslateCamera(Vector3 v)
{
    camera_position += v;
    camera_look_at += v;
}

Matrix4x4 Viewport::GetInvViewProjectionMatrix() const
{
    return InverseLookAtLh(camera_look_at, camera_position, camera_up) * InversePerspectiveFovLh(GetAspectRatio(), fov_y, near_plane, far_plane);
}

Matrix4 LookAtLh(Vector3 look_at, Vector3 eye, Vector3 up)
{
    Vector3 z = (look_at - eye).normalized();
    Vector3 x = Math::cross(up, z).normalized();
    Vector3 y = Math::cross(z, x);

    return Matrix4 { Vector4 { x, -Math::dot(x, eye) }, Vector4 { y, -Math::dot(y, eye) }, Vector4 { z, -Math::dot(z, eye) }, Vector4 { 0, 0, 0, 1 } }.transposed();
}

Matrix4 InverseLookAtLh(Vector3 look_at, Vector3 eye, Vector3 up)
{
    Matrix4 view = LookAtLh(look_at, eye, up);
    return Matrix4::from(view.rotation().transposed(), eye);
}

Matrix4 PerspectiveFovLh(f32 aspect_ratio, f32 fov_y, f32 near_plane, f32 far_plane)
{
    f32 yscale = 1.f / tanf(fov_y * 0.5f);
    f32 xscale = yscale / aspect_ratio;

    return {
        Vector4 { xscale, 0, 0, 0 },
        Vector4 { 0, yscale, 0, 0 },
        Vector4 { 0, 0, far_plane / (far_plane - near_plane), 1 },
        Vector4 { 0, 0, -near_plane * far_plane / (far_plane - near_plane), 0 }
    };
}

Matrix4 InversePerspectiveFovLh(f32 aspect_ratio, f32 fov_y, f32 near_plane, f32 far_plane)
{
    f32 yscale = tanf(fov_y * 0.5f);
    f32 xscale = yscale * aspect_ratio;
    f32 ab = -1.f / near_plane;

    return {
        Vector4 { xscale, 0, 0, 0 },
        Vector4 { 0, yscale, 0, 0 },
        Vector4 { 0, 0, 0, ab },
        Vector4 { 0, 0, 1, -ab }
    };
}

Vector2i ViewportRenderContext::GetRes() const
{
    return viewport->resolution;
}

void ViewportRenderContext::SetViewportAndScissorRect(Gfx::Encoder * encoder)
{
    D3D12_VIEWPORT vp {
        .Width = static_cast<float>(viewport->resolution.x()),
        .Height = static_cast<float>(viewport->resolution.y()),
        .MinDepth = 0.f,
        .MaxDepth = 1.f
    };
    encoder->GetCmdList()->RSSetViewports(1, &vp);

    const D3D12_RECT r = { 0, 0, viewport->resolution.x(), viewport->resolution.y() };
    encoder->GetCmdList()->RSSetScissorRects(1, &r);
}

ViewportRenderContext BuildViewportRenderContext(Gfx::Device * device, Viewport * viewport)
{
    ViewportRenderContext result {
        .viewport = viewport,
        .view_matrix = LookAtLh(viewport->camera_look_at, viewport->camera_position, viewport->camera_up),
        .inv_view_matrix = InverseLookAtLh(viewport->camera_look_at, viewport->camera_position, viewport->camera_up),
        .unjittered_projection_matrix = PerspectiveFovLh(viewport->GetAspectRatio(), viewport->fov_y, viewport->near_plane, viewport->far_plane),
        .inv_projection_matrix = InversePerspectiveFovLh(viewport->GetAspectRatio(), viewport->fov_y, viewport->near_plane, viewport->far_plane),
        .prev_view_projection_matrix = viewport->saved_view_projection_matrix,
    };

    result.projection_matrix = result.unjittered_projection_matrix;
    if (viewport->taa_offsets.Size()) {
        result.projection_jitter = viewport->taa_offsets[viewport->taa_index] / Vector2 { viewport->resolution };
    }

    result.projection_matrix[2][0] = result.projection_jitter.x();
    result.projection_matrix[2][1] = result.projection_jitter.y();

    result.view_projection_matrix = result.projection_matrix * result.view_matrix;
    result.inv_view_projection_matrix = result.inv_view_matrix * result.inv_projection_matrix;

    if (viewport->taa_offsets.Size()) {
        viewport->taa_index = (viewport->taa_index + 1) % viewport->taa_offsets.Size();
    }

    viewport->saved_view_projection_matrix = result.view_projection_matrix;

    struct FrameConstants {
        Vector2i resolution;
        Vector2 inv_resolution;
        Vector2 near_far_planes;
        Vector2 clipspace_jitter;
        Matrix4 view_matrix;
        Matrix4 projection_matrix;
        Matrix4 view_projection_matrix;
        Matrix4 inv_view_projection_matrix;
        Matrix4 inv_view_matrix;
        Matrix4 prev_view_projection_matrix;
        float taa_history_decay;
    };
    FrameConstants constants;
    constants.resolution = viewport->resolution;
    constants.inv_resolution = 1.f / Vector2 { constants.resolution };
    constants.near_far_planes = Vector2 { viewport->near_plane, viewport->far_plane };
    if (viewport->taa_offsets.Size()) {
        constants.clipspace_jitter = result.projection_jitter;
    } else {
        constants.clipspace_jitter = {};
    }
    constants.view_matrix = result.view_matrix;
    constants.projection_matrix = result.projection_matrix;
    constants.view_projection_matrix = result.view_projection_matrix;
    constants.inv_view_projection_matrix = result.inv_view_projection_matrix;
    constants.inv_view_matrix = result.inv_view_matrix;
    constants.prev_view_projection_matrix = result.prev_view_projection_matrix;
    constants.taa_history_decay = viewport->history_decay;

    //

    result.frame_payload.frame_resources_.PushBackRvalueRef(std::move(device->CreateBuffer(D3D12_HEAP_TYPE_UPLOAD, AlignedForward(static_cast<i32>(sizeof(FrameConstants)), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT), DXGI_FORMAT_UNKNOWN, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ)));
    void* cb_dst = nullptr;
    verify_hr(result.frame_payload.frame_resources_.Last().resource_->Map(0, nullptr, &cb_dst));
    memcpy(cb_dst, &constants, sizeof(FrameConstants));
    result.frame_payload.frame_resources_.Last().resource_->Unmap(0, nullptr);

    D3D12_CONSTANT_BUFFER_VIEW_DESC cbv_desc {
        .BufferLocation = result.frame_payload.frame_resources_.Last().resource_->GetGPUVirtualAddress(),
        .SizeInBytes = static_cast<u32>(AlignedForward(static_cast<i32>(sizeof(FrameConstants)), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT))
    };

    result.frame_cbv_handle = device->CreateDescriptor(cbv_desc, Lifetime::Frame);

    return result;
}

}