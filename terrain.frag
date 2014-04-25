// fragment shader for simple terrain application
#version 400 core

#define M_PI 3.1415926535897932384626433832795

// per-frame data
layout(std140)                  // use standard layout
uniform SceneData {             // uniform struct name
    mat4 viewMatrix, viewInverse;
    mat4 projectionMatrix, projectionInverse;
    vec3 lightpos;
    int fog;
	int time;
};

// shader data
uniform sampler2D colorTexture;
uniform sampler2D normalTexture;
uniform sampler2D glossTexture;

// input from vertex shader
in vec4 position, light, vertpos;
in vec3 tangent, bitangent, normal;
in vec2 texcoord;

// output to frame buffer
out vec4 fragColor;

// hash from 2D coordinate to integer (using two rounds of tiny encryption algorithm with key=0)
int hash(ivec2 v) {
    v.x += (v.y<<4)^(v.y-0x61C88647)^(v.y>>5);
    v.y += (v.x<<4)^(v.x-0x61C88647)^(v.x>>5);
    v.x += (v.y<<4)^(v.y+0x3C6EF372)^(v.y>>5);
    v.y += (v.x<<4)^(v.x+0x3C6EF372)^(v.x>>5);
    return v.y;
}

// "Modified noise" (from Graphics Hardware 2005)
float noise(vec2 p) {
    vec2 i = floor(p);          // integer part of point coordinate
    vec2 f = p - i;             // fractional part of coordinate
    vec2 b = (3.-2.*f)*f*f;     // smooth blend factors

    // hash values at four corners
    ivec4 h = ivec4(hash(ivec2(i.x   , i.y   )),
                    hash(ivec2(i.x   , i.y+1.)),
                    hash(ivec2(i.x+1., i.y   )),
                    hash(ivec2(i.x+1., i.y+1.)));

    // random gradients at four corners determined by bits of hash
    vec4 g = (f.x-vec4(0,0,1,1)) * vec4((h & 1)*2 - 1) +
	          (f.y-vec4(0,1,0,1)) * vec4((h & 2) - 1);

    // blend together contribution from each corner
    return mix(mix(g.x,g.z, b.x), mix(g.y,g.w, b.x), b.y);
}

vec3 sand_color() {
	float sand_texture_noise = noise(vertpos.xy * 10) / 8;
	float sand_color_noise = (noise(vertpos.xy / 40) - 0.25) / 4;
	return vec3(.974, .856, .699) + sand_texture_noise + sand_color_noise;
}

vec3 grass_color() {
	vec3 color = vec3(0, .502, 0);

	float grass_texture_noise = noise(vec2(vertpos.x*12, (vertpos.y)/2)) / 5;
	vec3 grass_texture = vec3(grass_texture_noise);

	//Vary Grass Color
	float grass_shading_noise = noise(vertpos.xy/50);
	if (grass_shading_noise > 0) {
		color = mix(vec3(0, .502, 0), vec3(.5, .5, 0), vec3(grass_shading_noise));
	} else {
		color = mix(vec3(0, .502, 0), vec3(.25, .31, .07), vec3(grass_shading_noise));
	}

	float grass_shadow_noise = noise(vec2(vertpos.x/20, vertpos.y/10)) / 3;
	return color + grass_texture + (grass_shading_noise / 3) + grass_shadow_noise;
}

vec3 snow_color() {
	float snow_color_noise = noise(vertpos.xy) / 15;
	return vec3(1) + snow_color_noise;
}

void main() {
    // convert points from homogeneous form to true 3D
    // last column of view matrix contains terrain origin in view space
    vec3 pos = position.xyz / position.w;
    vec3 lpos = light.xyz / light.w;
    vec3 terrainOrigin = viewMatrix[3].xyz / viewMatrix[3].w;
	vec3 N = normalize(normal);
	vec3 world_normal = normalize(N * mat3(viewMatrix));
	
	float time_cycle = mod(time, 150) / 75;
	float time_variance = (cos(time_cycle * M_PI) + 1) / 2;
	
	//Set Sand Color as Default
	vec3 color = sand_color();
	float gloss = -2;

	//Calculate Snow Elevation
	float world_normal_y = (world_normal.y + 1) * 4;
	float snow_edge_noise = (noise(vertpos.xy * 15) - 0.2) * 5;
	float snow_elevation_variance = cos(world_normal_y * M_PI) * 2;
	float snow_elevation = 55 - (25 * time_variance) + snow_elevation_variance;
	float organic_snow_elevation = snow_elevation + snow_edge_noise;

	//Calculate Grass Elevation
	float grass_edge_noise = noise(vec2(vertpos.x*12, vertpos.y/2)) / 5;
	float grass_elevation_offset = cos(world_normal_y * M_PI) * 3;
	float grass_elevation = -11 + grass_elevation_offset + grass_edge_noise * 10;

	if (vertpos.z > organic_snow_elevation) {
		color = snow_color();
		gloss = 6;
		if (vertpos.z < snow_elevation + 2) {
			float normal_scale = (snow_elevation + 2 - vertpos.z) / 2;
			N = normalize(N + vec3(0, 0, normal_scale / 3));
		}
	} else if (vertpos.z > grass_elevation) {
		color = grass_color();
		gloss = 0;
	} else if (vertpos.z > grass_elevation - 0.3) {
		color = color - 0.3;
	}

    // light vectors and dot products
    // for point light, use normalize(lpos - pos)
    vec3 L = normalize(lpos - terrainOrigin);   // direction to light
    vec3 V = normalize(/*eye at 0,0,0*/ - pos); // direction to view
    vec3 H = normalize(V + L);
    float N_L = max(0., dot(N,L)), N_H = max(0., dot(N,H));
    float V_L = dot(V,L), V_H = dot(V,H);
    
    // specular: normalized Blinn-Phong with Kelemen/Szirmay Kalos shadow/mask
    // Schlick approximation to Fresnel for index of refraction 1.5
    float spec = (gloss+2) * pow(N_H, gloss) / (1 + max(0.,V_L));
    float fresnel = 0.04 + 0.96 * pow(1 - V_H, 5);
    
	color = mix(color, vec3(spec), fresnel) * N_L;

    // fade to white with fog
    if (fog != 0)
        color = mix(vec3(1,1,1), color, exp2(.005 * pos.z));

    // final color
    fragColor = vec4(color, 1);
}
