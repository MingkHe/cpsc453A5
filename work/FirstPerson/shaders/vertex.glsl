// ==========================================================================
// Vertex program for barebones GLFW boilerplate
//
// Author:  Sonny Chan, University of Calgary
// Date:    December 2015
// ==========================================================================
#version 410

// location indices for these attributes correspond to those specified in the
// InitializeGeometry() function of the main program
layout(location = 0) in vec3 VertexPosition;
layout(location = 1) in vec2 texcoord;

out vec2 Texcoord;

out float dotNL;

out float dotHNpowP;

flat out int earthNightOrNot;

uniform mat4 modelViewProjection;

uniform float time;

uniform vec3 cameraPos;

uniform bool earth;



vec2 circleOffset(float t){
	return vec2(cos(t), sin(t));
}

uniform mat4 transistion;

void main()
{
    // assign vertex position without modification
    gl_Position = modelViewProjection*transistion*vec4(VertexPosition, 1.0);
    
    vec3 origin =  (transistion*vec4(0,0,0,1)).xyz;
    vec3 currentPosition = (transistion*vec4(VertexPosition, 1.0)).xyz;
    
    vec3 n = normalize(currentPosition-origin);
    vec3 l = normalize(vec3(0,0,0)-currentPosition);
    vec3 v = normalize(cameraPos-currentPosition);
    vec3 h = normalize(v+l);
    
    dotNL = max(0,dot(n,l));
    dotHNpowP = pow(max(0,dot(h,n)),4);
    
    Texcoord = texcoord;
    
    if(earth){
		if(dot(n,l)<=0){
			earthNightOrNot=1;
		}else{
		earthNightOrNot=0;
		}
	}else{
		earthNightOrNot=0;
	}
}
