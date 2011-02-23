#version 330

uniform sampler2D phaseA;
uniform float width;
uniform float height;

in vec2 fragTexCoord;
out vec4 normalMap;

float step_w = 1.0/width;
float step_h = 1.0/height;

void main(void)
{
  vec2 offset[9];
  offset[3] = vec2(-step_w, step_h); 	offset[2] = vec2(0.0, step_h); 	offset[1] = vec2(step_w, step_h);
  offset[4] = vec2(-step_w, 0.0);   					offset[0] = vec2(step_w, 0.0);		offset[8] = vec2(step_w, 0.0);
  offset[5] = vec2(-step_w, -step_h);  	offset[6] = vec2(0.0, -step_h);	offset[7] = vec2(step_w, -step_h); 

  float pi = 3.14159265; 		// Mmmmmm PI
  float theta = pi / 6.0;		// Angle between camera and projector. 30 degrees
  float W = width;			// Width of the image
  float fringeFrequency = 4.0;
  float P = W / (2.0 * fringeFrequency);			// Pixels per period of the projector
  
  vec3 newVertex = vec3(fragTexCoord.s-.5, fragTexCoord.t-.5, 0.0);

  float phaseReference = (newVertex.x * W) * ((2.0*pi*cos(theta))/P);
  float phaseActual = texture2D(phaseA, fragTexCoord).x;

  newVertex.z = (phaseActual - phaseReference) * P / (W * sin(theta)*2.0*pi);

  vec3 normal = vec3(0.0);


  for(int i=0; i<8; ++i)
  {
    vec3 currentNeighbor = newVertex;
    currentNeighbor.xy = currentNeighbor.xy + offset[i];
    float currentNeighborPhase = texture2D(phaseA, fragTexCoord + offset[i]).x;
    float neighborPhaseReference = ((newVertex.x + offset[i].x) * W) * ((2.0*pi*cos(theta))/P);
    currentNeighbor.z = (currentNeighborPhase - neighborPhaseReference) * P / (W * sin(theta)*2.0*pi);

    vec3 nextNeighbor = newVertex;
    nextNeighbor.xy = nextNeighbor.xy + offset[i+1];
    float nextNeighborPhase = texture2D(phaseA, fragTexCoord + offset[i+1]).x;
    float nextNeighborPhaseReference = ((newVertex.x + offset[i+1].x) * W) * ((2.0*pi*cos(theta))/P);
    nextNeighbor.z = (nextNeighborPhase - nextNeighborPhaseReference) * P / (W * sin(theta)*2.0*pi);

    vec3 v1 = normalize(currentNeighbor - newVertex);
    vec3 v2 = normalize(nextNeighbor - newVertex);
    normal += cross(v1, v2);
  }

  normal /= 8.0;
  normal = normalize(normal);
  
  normalMap = vec4(normal,0.0);
}