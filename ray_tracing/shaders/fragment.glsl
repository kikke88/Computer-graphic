#version 330

#define float2 vec2
#define float3 vec3
#define float4 vec4
#define float4x4 mat4
#define float3x3 mat3

in float2 fragmentTexCoord;
layout(location = 0) out vec4 fragColor;

uniform int g_screenWidth;
uniform int g_screenHeight;
uniform float4x4 g_rayMatrix;
uniform float g_curTime;
uniform int g_SharpSoft;
uniform samplerCube skybox;

#define BOX 1
#define TORUS 2
#define SPECIAL1 2
#define SPECIAL2 3
#define OCTAHEDRON 4

#define MAX_MARCHING_STEPS 256
#define MAX_RAY_DEPTH 50
#define MAX_REFLECTION_DEPTH 10
#define EPS 1e-3f
#define K 18.0

struct Material
{
    vec3 color;
    float reflection;
};

struct Primitive
{
    int type;
    vec3 centre;
    vec3 features;
    Material material;
};

#define PRIMITIVE_NUM 7
Primitive primitive[PRIMITIVE_NUM] = Primitive[PRIMITIVE_NUM](
    Primitive(OCTAHEDRON, vec3(-2.6, -1.0, 1.5), vec3(0.6, 0.0, 0.0), Material(vec3(0.8, 0.51, 0.09), 0.1)),
    Primitive(BOX, vec3(0.0, -3.0, -1.0), vec3(5.0, 0.0, 5.0), Material(vec3(0.87, 0.87, 0.87), 0.0)),
    Primitive(TORUS, vec3(0.0, -1.0, 0.0), vec3(1.0, 0.3, 0.0), Material(vec3(0.93, 0.3, 0.002), 0.3)),
    Primitive(SPECIAL1, vec3(0.0, -1.0, -3.0), vec3(0.61, 0.0, 0.0), Material(vec3(0.3, 0.5, 0.87), 0.0)),
    Primitive(SPECIAL1, vec3(0.0, -1.0, -3.0), vec3(0.6, 0.6, 0.6), Material(vec3(0.3, 0.5, 0.87), 0.0)),
    Primitive(SPECIAL2, vec3(2.6, -1.0, 1.5), vec3(0.8, 0.0, 0.0), Material(vec3(0.4, 0.9, 0.3), 0.0)),
    Primitive(SPECIAL2, vec3(2.6, -1.7, 1.5), vec3(1.6, 0.08, 0.0), Material(vec3(0.4, 0.9, 0.3), 0.0))
    );

float SDTorus(vec3 p, int prim_num)
{
    p = p - primitive[prim_num].centre;
    vec2 q = vec2(length(p.xz) - primitive[prim_num].features.x, p.y);
    return length(q) - primitive[prim_num].features.y;
}

float UDBox(vec3 p, int prim_num)
{
    return length(max(abs(p - primitive[prim_num].centre) - primitive[prim_num].features, 0.0));
}

float SDOctahedron(vec3 p, int prim_num)
{
    float tmp = primitive[prim_num].features[0];
    p = p - primitive[prim_num].centre;
    p = abs(p);
    float m = p.x + p.y + p.z - tmp;
    vec3 q;
    if (3.0 * p.x < m) {
        q = p.xyz;
    } else if(3.0 * p.y < m) {
        q = p.yzx;
    } else if(3.0 * p.z < m) {
        q = p.zxy;
    } else {
        return m * 0.57735027;
    }
    float k = clamp(0.5 * (q.z - q.y + tmp), 0.0, tmp);
    return length(vec3(q.x, q.y - tmp + k, q.z - k));
}

float SDSphere(vec3 p, int prim_num)
{
    if (prim_num == 3) {
        return distance(p, primitive[prim_num].centre) - (primitive[prim_num].features[0]+ 0.2 * (1 + sin(g_curTime)) / 2);
    } else {
        return distance(p, primitive[prim_num].centre) - primitive[prim_num].features[0];
    }
}

float SDVerticalCapsule(vec3 p, int prim_num)
{
    p = p - primitive[prim_num].centre;
    p.y -= clamp(p.y, 0.0, (primitive[prim_num].features[0] + 0.3 * sin(g_curTime * 2)));
    return length(p) - (primitive[prim_num].features[1] + 0.08 * (1 + sin(g_curTime)) / 2);
}

float opSubtraction(float d1, float d2)
{
    return max(-d1, d2);
}

float opUnion(float d1, float d2)
{
    return min(d1, d2);
}

struct Hit
{
    bool intersection;
    float distance;
    int prim_num;
    vec3 normal;
};

struct Hit_dist_prim
{
    float dist;
    int prim_num;
};

Hit_dist_prim sceneSDF(vec3 curPoint) {
    Hit_dist_prim cur = Hit_dist_prim(SDOctahedron(curPoint, 0), 0);
    Hit_dist_prim tmp;
    for (int i = 1; i < PRIMITIVE_NUM - 2; ++i) {
        if (i == 3) {
            tmp = Hit_dist_prim(opSubtraction(SDSphere(curPoint, 3), UDBox(curPoint, 4)), 3);
        } else if (i == 4) {
            tmp = Hit_dist_prim(opUnion(SDSphere(curPoint, 5), SDVerticalCapsule(curPoint, 6)), 5);
        } else if (primitive[i].type == BOX) {
            tmp = Hit_dist_prim(UDBox(curPoint, i), i);
        } else if (primitive[i].type == TORUS) {
            tmp = Hit_dist_prim(SDTorus(curPoint, i), i);
        }
        if (tmp.dist < cur.dist) {
            cur = tmp;
        }
    }
    return cur;
}

vec3 EstimateNormal(vec3 z)
{
  vec3 z1 = z + float3(EPS, 0, 0);
  vec3 z2 = z - float3(EPS, 0, 0);
  vec3 z3 = z + float3(0, EPS, 0);
  vec3 z4 = z - float3(0, EPS, 0);
  vec3 z5 = z + float3(0, 0, EPS);
  vec3 z6 = z - float3(0, 0, EPS);
  return normalize(vec3(sceneSDF(z1).dist - sceneSDF(z2).dist,
                        sceneSDF(z3).dist - sceneSDF(z4).dist,
                        sceneSDF(z5).dist - sceneSDF(z6).dist));
}

struct Ray
{
    vec3 dir;
    vec3 pos;
};

Hit RaySceneIntersection(Ray ray)
{
    float depth = 0;
    ray.pos = ray.pos + EPS * 2 * ray.dir;
    Hit_dist_prim curPoint;
    vec3 cur_pos;
    for (int i = 0; i < MAX_MARCHING_STEPS; ++i) {
        cur_pos = ray.pos + depth * ray.dir;
        curPoint = sceneSDF(cur_pos);
        if (curPoint.dist < EPS) {
            return Hit(true, depth, curPoint.prim_num, EstimateNormal(cur_pos));
        }
        if (depth > MAX_RAY_DEPTH) {
            break;
        }
        depth += curPoint.dist;
    }
    return Hit(false, 0.0f, 0, vec3(0.0f, 0.0f, 0.0f));
}

struct Light
{
    vec3 pos;
};

#define LIGHTS_NUM 2
Light lights[LIGHTS_NUM] = Light[LIGHTS_NUM](
  Light(vec3(2.0f, 4.0f, 5.0f)),
  Light(vec3(0.0f, 4.2f, 0.0f))
  );

struct Visible_ret
{
    bool light;
    float soft_shadow;
};

Visible_ret Visible(vec3 hit_point, int light_num)
{
    float res = 1.0;
    vec3 dir = normalize(lights[light_num].pos - hit_point);
    float t = distance(lights[light_num].pos, hit_point);
    float curPoint_dist;
    float ph = 1e20;
    for (float depth = 10 * EPS; depth < t;) {
        curPoint_dist = sceneSDF(hit_point + depth * dir).dist;
        if (curPoint_dist < EPS) {
            return Visible_ret(false, 0.0);
        }
        float y = curPoint_dist * curPoint_dist / (2 * ph);
        float d = sqrt(curPoint_dist * curPoint_dist - y * y);
        res = min(res, K * d / max(0.0, depth - y));
        ph = curPoint_dist;
        depth += curPoint_dist;
    }
    return Visible_ret(true, clamp(res, 0.0, 1.0));
}

vec3 ambient_color = vec3(0.1f, 0.1f, 0.1f);
vec3 specular_color = vec3(1.0f, 1.0f, 1.0f);
float shininess = 10.0f;

vec3 Shade(vec3 hit_point, Hit hit, int light_num, vec3 ray_dir, float soft_coeff)
{
    if (g_SharpSoft == 0) {
        soft_coeff = 1.0;
    }
    vec3 lightIntensity = vec3(0.3f);
    vec3 color = vec3(0.0f);
    vec3 pos_light = normalize(lights[light_num].pos - hit_point);
    vec3 pos_eye = - ray_dir;
    vec3 reflection = normalize(reflect(-pos_light, hit.normal));
    float light_normal_cos = dot(pos_light, hit.normal);
    float revl_eye_cos = dot(reflection, pos_eye);
    if (revl_eye_cos < 0.0f) { return lightIntensity * primitive[hit.prim_num].material.color * light_normal_cos * soft_coeff; }
    return lightIntensity * (primitive[hit.prim_num].material.color * light_normal_cos * soft_coeff + specular_color * pow(revl_eye_cos, shininess));
}

vec3 All_Shades(vec3 hit_point, Hit hit, vec3 ray_dir)
{
    vec3 color;
    vec3 ambientLight = 0.7 * vec3(1.0f);
    color = ambientLight * ambient_color;
    Visible_ret cur_light;
    for (int i = 0; i < LIGHTS_NUM; ++i) {
        cur_light = Visible(hit_point, i);
        if (cur_light.light) {
            color += Shade(hit_point, hit, i, ray_dir, cur_light.soft_shadow);
        }
    }
    return color;
}

vec4 RayTrace(Ray ray)
{
    vec3 color = vec3(0.0f, 0.0f, 0.0f);
    float reflection_coeff = 1.0;
    for (int j = 0; j < MAX_REFLECTION_DEPTH; ++j) {
        Hit hit = RaySceneIntersection(ray);
        if (!hit.intersection) {
            color += reflection_coeff * vec3(texture(skybox, -ray.dir));
            break;
        }
        vec3 hit_point = ray.pos + ray.dir * hit.distance;
        color += reflection_coeff * (1.0 - primitive[hit.prim_num].material.reflection) * All_Shades(hit_point, hit, ray.dir);
        if (primitive[hit.prim_num].material.reflection == 0.0) {
            break;
        }
        reflection_coeff *= primitive[hit.prim_num].material.reflection;
        ray.dir = normalize(reflect(normalize(ray.dir), hit.normal));
        ray.pos = hit_point;
    }
    return vec4(color, 1.0f);
}

float3 EyeRayDir(float x, float y, float width, float height)
{
  float fov = 60.0f;
  float3 ray_dir = float3(x + 0.1f - width / 2.0f, y + 0.1f - height / 2.0f, - width / tan(radians(fov) / 2.0f));
  return normalize(ray_dir);
}

void main(void)
{
    float width = float(g_screenWidth);
    float height = float(g_screenHeight);
    float x = fragmentTexCoord.x * width;
    float y = fragmentTexCoord.y * height;
    Ray ray = Ray(EyeRayDir(x, y, width, height), vec3(0.0f, 0.0f, 0.0f));
    ray.pos = (g_rayMatrix * float4(ray.pos, 1)).xyz;
    ray.dir = float3x3(g_rayMatrix) * ray.dir;
    fragColor = RayTrace(ray);
}
