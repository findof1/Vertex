#version 330 core
layout(location = 0) in vec3 aPos; // grid positions (x,z) in local space, y = 0
layout(location = 1) in vec2 aTexPos;

out vec3 vWorldPos;
out vec3 vNormal;
out vec4 vClipPos;
out vec2 vTexPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float time;
uniform int numWaves;

struct Wave {
  vec2 dir;
  float wavelength;
  float amplitude;
  float speed;
  float steepness;
};

uniform Wave waves[32]; // set numWaves <= 32

// Clipping plane, required in all vertex shaders for modules
uniform vec4 clipPlane;
uniform bool enableClip;

// gerstner function for a single wave
vec3 gerstnerWave(vec3 pos, Wave w, float t) {
  float k = 2.0 * 3.14159265 / w.wavelength;
  float dotp = dot(w.dir, pos.xz);
  float phase = k * dotp - w.speed * k * t;
  float cosP = cos(phase);
  float sinP = sin(phase);

  float Q = w.steepness / (k * w.amplitude + 1e-6); // controls choppiness

  vec3 disp = vec3(0.0);
  disp.x += Q * w.amplitude * w.dir.x * cosP;
  disp.y += w.amplitude * sinP;
  disp.z += Q * w.amplitude * w.dir.y * cosP;
  return disp;
}

void main() {
  vec3 displaced = aPos;
    // sum waves
  for(int i = 0; i < numWaves; ++i) {
    displaced += gerstnerWave(aPos, waves[i], time);
  }

  float eps = 0.1;
  vec3 posX = aPos + vec3(eps, 0.0, 0.0);
  vec3 posZ = aPos + vec3(0.0, 0.0, eps);

  vec3 dX = posX;
  vec3 dZ = posZ;
  for(int i = 0; i < numWaves; ++i) {
    dX += gerstnerWave(posX, waves[i], time);
    dZ += gerstnerWave(posZ, waves[i], time);
  }
  vec3 n = normalize(cross(dZ - displaced, dX - displaced));

  vec4 worldPos = model * vec4(displaced, 1.0); //model * vec4(aPos, 1.0); 
  vWorldPos = worldPos.xyz;
  vNormal = mat3(transpose(inverse(model))) * n;
  vClipPos = projection * view * worldPos;
  vTexPos = aTexPos;
  gl_Position = vClipPos;

  if(enableClip)
    gl_ClipDistance[0] = dot(worldPos, clipPlane);
  else
    gl_ClipDistance[0] = 1.0;
}