struct FrameConstants {
    uint2 resolution;
    float2 inv_resolution;
    float4x4 view_matrix;
    float4x4 projection_matrix;
    float4x4 view_projection_matrix;
    float4x4 inv_view_projection_matrix;
    float4x4 inv_view_matrix;
};

cbuffer CB : register(b0) {
    FrameConstants frame;
};

float3 ReadCameraPos() {
    return frame.inv_view_matrix._14_24_34;
}

struct PS_INPUT {
    linear sample float4 pos : SV_POSITION;
    uint page : TEXCOORD0;
    uint index : TEXCOORD1;
};

struct ParticleData {
    float4 wposition_size;
};

struct ParticleData_PositionSize {
    float4 wposition_size;
};

Buffer<int> ParticlePageBuffer : register(t0);
Texture2D<float4> ParticlePositionSize : register(t1);

struct AxisBounds {
	float3 L;
	float3 U;
};

float Square(float x) {
	return x * x;
}

AxisBounds GetBoundForAxis(float3 a, float3 C, float r, float near_z) {
	float2 c = float2(dot(a, C), C.z); //C in the a-z frame
	float2 bounds[2]; //In the a-z reference frame
	const float t_squared = dot(c, c) - Square(r);
	const bool camera_inside_sphere = (t_squared <= 0); 
	//(cos, sin) of angle theta between c and a tangent vector
	float2 v = camera_inside_sphere ?
		float2(0.0f, 0.0f) :
		float2(sqrt(t_squared), r) / length(c);

	//Does the near plane intersect the sphere?
	const bool clip_sphere = (c.y - r <= near_z);
	//Square root of the discriminant; NaN (and unused)
	//if the camera is in the sphere
	float k = sqrt(Square(r) - Square(c.y - near_z));
	for (int i = 0; i < 2; ++i) {
		if (! camera_inside_sphere) {
			bounds[i] = mul(float2x2( v.x, v.y, -v.y, v.x ), c) * v.x;
		}
		const bool clip_bound = camera_inside_sphere || (bounds[i].y < near_z);
		if (clip_sphere && clip_bound) {
			bounds[i] = float2(c.x + k, near_z);
		}
		//Set up for the lower bound
		v.y = -v.y;  k = -k;
	}
	AxisBounds result = { { bounds[1].x * a }, { bounds[0].x * a }};
	result.L.z = bounds[1].y;
	result.U.z = bounds[0].y;
	return result;
}

struct AABox2D {
    float2 min_max[2];
};

AABox2D Xyxy(float x0, float y0, float x1, float y1) {
    AABox2D result;
    result.min_max[0].x = min(x0, x1);
    result.min_max[1].x = max(x0, x1);
    result.min_max[0].y = min(y0, y1);
    result.min_max[1].y = max(y0, y1);
    return result;
}

float3 Project(float4x4 m, float3 p) {
    float4 pp = mul(m, float4(p, 1));
    return pp.xyz / pp.w;
}

PS_INPUT VsMain(uint VertexID: SV_VertexID, uint InstanceID: SV_InstanceID) {
    uint page_index = ParticlePageBuffer[InstanceID / 32];
    uint quad_index = VertexID % 4;

    float4 wpos_size = ParticlePositionSize[uint2(InstanceID % 32, page_index)]; // meh


    float3 center = mul(frame.view_matrix, float4(wpos_size.xyz,1)).xyz;
    float radius = wpos_size.w;
    AxisBounds axis_bounds_x = GetBoundForAxis(float3(1,0,0), center, radius, 0.1f);
    AxisBounds axis_bounds_y = GetBoundForAxis(float3(0,1,0), center, radius, 0.1f);
    AABox2D clipspace_bb = Xyxy(
        Project(frame.projection_matrix, axis_bounds_x.L).x, 
        Project(frame.projection_matrix, axis_bounds_y.L).y,
        Project(frame.projection_matrix, axis_bounds_x.U).x,
        Project(frame.projection_matrix, axis_bounds_y.U).y);
        

    PS_INPUT output;
    output.pos = float4(clipspace_bb.min_max[quad_index < 2].x, clipspace_bb.min_max[(quad_index & 1) == 0].y, 0, 1);
    output.page = page_index;
    output.index = InstanceID;
    return output;
}

struct Ray {
    float3 origin;
    float3 direction;
};

struct Sphere {
    float3 center;
    float radius;
};

float RaySphereIntersection(Ray ray, Sphere sphere) {
    float3 m = ray.origin - sphere.center;
    float b = dot(m, ray.direction);
    float c = dot(m, m) - sphere.radius * sphere.radius;

    if(c > 0 && b > 0) {
        return -1;
    }

    float discr = b*b - c;
    if(discr < 0) {
        return -1;
    }

    float t = -b - sqrt(discr);

    return max(t, 0);
}

struct PS_OUTPUT {
    float4 colour : SV_TARGET;
    float depth : SV_DepthGreaterEqual;
};

PS_OUTPUT PsMain(PS_INPUT input) {
    float4 wpos_size = ParticlePositionSize[uint2(input.index % 32, input.page)]; // meh

    Sphere sphere;
    sphere.center = wpos_size.xyz;
    sphere.radius = wpos_size.w;

    float2 clip_space_xy = input.pos.xy * frame.inv_resolution * float2(2, -2) + float2(-1, 1);
    float4 ray_dir_wh = mul(frame.inv_view_projection_matrix, float4(clip_space_xy, 1, 1));
    float3 ray_dir = normalize(ray_dir_wh.xyz);

    Ray screen_ray;
    screen_ray.origin = ReadCameraPos();
    screen_ray.direction = ray_dir;

    float t = RaySphereIntersection(screen_ray, sphere);

    if(t < 0) {
        discard;
    }

    PS_OUTPUT output;
    float3 hit = screen_ray.origin + screen_ray.direction * t;
    float3 N = normalize(hit - sphere.center);
    float3 V = -ray_dir;

    output.colour = float4(frac(sphere.center), 1.f);
    output.depth = 1;
    return output;
}