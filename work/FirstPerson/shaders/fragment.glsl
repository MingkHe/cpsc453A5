// ==========================================================================
// Vertex program for barebones GLFW boilerplate
//
// Author:  Sonny Chan, University of Calgary
// Date:    December 2015
// ==========================================================================
#version 410

// interpolated colour received from vertex stage
uniform vec3 Colour;

// first output is mapped to the framebuffer's colour index by default
out vec4 FragmentColour;

in vec2 Texcoord;

in float dotNL;

in float dotHNpowP;

flat in int earthNightOrNot;

uniform sampler2D imagetexture;
uniform sampler2D earthNight;

uniform bool shading;

void main(void)
{
    // write colour output without modification
    //FragmentColour = texture(imagetexture,Texcoord);
    //FragmentColour = vec4(Colour, 0);


	vec4 cr = texture(imagetexture,Texcoord);
    vec4 ca = vec4(.5f,.5f,.5f,0);
    vec4 cl = vec4(1,1,1,0);
    vec4 cp = vec4(0.7,0.7,0.7,0);
    
    
    if(shading){
		FragmentColour = cr*(ca+cl*dotNL); //cl*cp*dotHNpowP
	}else{
		FragmentColour = texture(imagetexture,Texcoord);
	}
	
	if(earthNightOrNot==1){
		FragmentColour = texture(earthNight,Texcoord);
	}
}
