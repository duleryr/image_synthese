#version 410

uniform float lightIntensity;
uniform sampler2D colorTexture;
uniform bool blinnPhong;
uniform float shininess;
uniform float eta;
uniform float k_a;		//ambient reflection coefficient 
uniform float k_d = 0.7;		//diffuse reflection coefficient 
uniform sampler2D shadowMap;

in vec4 eyeVector;
in vec4 lightVector;
in vec4 vertColor;
in vec4 vertNormal;
in vec2 textCoords;
in vec4 lightSpace;

out vec4 fragColor;

#define EPS 0.0001

void main( void )
{
    //fragColor = vertColor;
	vec4 localColor = texture2D(colorTexture,textCoords);

    if(!blinnPhong) {
        fragColor = lightIntensity*(3*localColor+0.1*vertNormal);
    } else {
        vec4 C_a = localColor*lightIntensity*k_a;
        fragColor = C_a;

        vec4 light = (lightSpace / lightSpace.w)*0.5 + 0.5;
        vec4 shadowMapPos = texture(shadowMap, light.xy);

        if (shadowMapPos.x > (1-EPS) * light.z) {
            vec4 eyeV = normalize(eyeVector);
            vec4 lightV = normalize(lightVector); 
            vec4 vertN = normalize(vertNormal);

            float k_F = 1.0;
            vec4 halfV = vec4(1,1,1,1);	//H

            halfV = eyeV + lightV;
            halfV = normalize(halfV);

            float cosTheta = dot(halfV,lightV); 
            float sinTheta2 = 1-cosTheta*cosTheta ;

            float ci = eta*eta-sinTheta2;
            if (ci < 0)
            {	
                ci = 0;
            } else {
                ci = sqrt(ci);
            }

            float F_s = (cosTheta-ci)/(cosTheta+ci);
            float F_p = (eta*cosTheta-ci)/(eta*cosTheta+ci);
            F_s *= F_s; 
            F_p *= F_p; 

            k_F = (F_s + F_p) / 2;

            float nDotL = dot(vertN,lightV);
            float nDotH = dot(vertN,halfV);

            if(nDotL < 0){nDotL = 0;}
            if(nDotH < 0){nDotH = 0;}

            vec4 C_d = localColor*lightIntensity*k_d*nDotL;
            vec4 C_s = localColor*lightIntensity*k_F*pow(nDotH,shininess);

            fragColor += C_d + C_s;


		}
	}
}
