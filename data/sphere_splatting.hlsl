#include "frame.hlsli"

#include "sphere_bounds.hlsli"

struct PS_INPUT {
    linear sample float4 pos : SV_POSITION;
    uint index : TEXCOORD0;
};

struct ParticleData {
    float3 position;
    float size;
    float3 prev_position;
    uint colour_packed;
};

float4 UnpackColourUint(uint value)
{
    return (uint4(value >> 24, value >> 16, value >> 8, value) & 0xFF) / 255.f;
}

StructuredBuffer<ParticleData> ParticlesBuffer : register(t0);

PS_INPUT VsMain(uint VertexID: SV_VertexID, uint InstanceID: SV_InstanceID) {
    uint quad_index = VertexID % 4;

    ParticleData particle = ParticlesBuffer[InstanceID];

    float3 center = mul(frame.view_matrix, float4(particle.position, 1)).xyz;
    float radius = particle.size;
    AxisBounds axis_bounds_x = GetBoundForAxis(float3(1,0,0), center, radius, frame.near_far_planes.x);
    AxisBounds axis_bounds_y = GetBoundForAxis(float3(0,1,0), center, radius, frame.near_far_planes.x);
    AABox2D clipspace_bb = AABox2D_Xyxy(
        Project(frame.projection_matrix, axis_bounds_x.L).x, 
        Project(frame.projection_matrix, axis_bounds_y.L).y,
        Project(frame.projection_matrix, axis_bounds_x.U).x,
        Project(frame.projection_matrix, axis_bounds_y.U).y);

    // why do I need this IF the bounds are working correctly? the projection includes jitter
    #if 1
    clipspace_bb.min_max[0] -= frame.inv_resolution;
    clipspace_bb.min_max[1] += frame.inv_resolution;
    #endif
        
    float z = center.z + radius;
    float z_postproj = frame.projection_matrix._33 + frame.projection_matrix._34 / z;

    PS_INPUT output;
    output.pos = float4(clipspace_bb.min_max[quad_index < 2].x, clipspace_bb.min_max[(quad_index & 1) == 0].y, z_postproj, 1);
    output.index = InstanceID;
    return output;
}

struct PS_OUTPUT {
    float4 colour : SV_TARGET0;
    float depth : SV_DepthLessEqual;
    float2 motion_vector : SV_TARGET1;
};

PS_OUTPUT PsMain(PS_INPUT input) {
    ParticleData particle = ParticlesBuffer[input.index];

    Sphere sphere;
    sphere.center = particle.position;
    sphere.radius = particle.size;

    // convert to jittered clip (clip including jitter) so we can apply inv viewproj of the unjittered view projection to recover world pos
    float2 clip_space_xy = Frame_PixelPosToJitteredClip(input.pos.xy);
    float4 ray_dir_wh = mul(frame.inv_view_projection_matrix, float4(clip_space_xy, 1, 1));
    float3 ray_dir = normalize(ray_dir_wh.xyz);

    Ray screen_ray;
    screen_ray.origin = Frame_GetCameraWPos();
    screen_ray.direction = ray_dir;

    float t = RaySphereIntersection(screen_ray, sphere);

    if(t < 0) {
        discard;
    }

    PS_OUTPUT output;
    float3 hit = screen_ray.origin + screen_ray.direction * t;
    float3 N = normalize(hit - sphere.center);
    float3 V = -ray_dir;

    float4 clip_pos = mul(frame.view_projection_matrix, float4(hit, 1.f));
    float z_postproj = clip_pos.z / clip_pos.w;

    if(z_postproj < 0.f) {
        discard;
    }

    output.colour = UnpackColourUint(particle.colour_packed);
    output.depth = z_postproj;

    float3 velocity = particle.position - particle.prev_position;
    float4 prev_clip_pos = mul(frame.prev_view_projection_matrix, float4(hit - velocity, 1.f));

    output.motion_vector = ((prev_clip_pos.xy / prev_clip_pos.w) - clip_space_xy);
    return output;
}