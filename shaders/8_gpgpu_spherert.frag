#version 410
#define M_PI 3.14159265358979323846
#define EPSILLON 0.0001

uniform mat4 mat_inverse;
uniform mat4 persp_inverse;
uniform sampler2D envMap;
uniform vec3 center;
uniform float radius;

uniform bool transparent;
uniform float shininess;
uniform float eta;

in vec4 position;

out vec4 fragColor;

vec4 getColorFromEnvironment(in vec3 direction)
{
	float r = sqrt(dot(direction,direction)); //length
	float theta = atan(direction.y,direction.x);
	float phi = acos(direction.z/r);
	vec2 pos = vec2(theta/(2*M_PI) + 0.5, phi/M_PI);
	return texture(envMap,pos);
}

bool raySphereIntersect(in vec3 start, in vec3 direction, out vec3 newPoint) {
    vec3 cp = start - center;
    float prod_cp_u = dot(cp, direction);
    float delta = prod_cp_u * prod_cp_u + radius*radius - length(cp)*length(cp);
    if (delta > 0) {
        float dist;
        float a = 1.0; // = dot(direction, direction);
        float b = 2.0 * dot(direction, cp);
        dist = (-b - sqrt(delta)) / (2.0*a);
        if (dist < 0) {
            dist = (-b + sqrt(delta)) / (2.0*a);
        }
        newPoint = start + dist * direction;
    }
    return delta > 0; 
    /*
    vec3 cp = start-center;
    float b = dot(direction,cp);
    float c = dot(cp,cp);
    float delta = b*b -c + radius*radius;
    if (delta > 0) {
        float a = dot(direction,direction);
        b *= 2;
        delta = b*b-4*a*c;
        float t;
        if(delta < EPSILLON && delta > -EPSILLON) {
            t = -b/(2*a);
        } else {
            t = (-sqrt(delta)-b)/(2*a);
        }
        newPoint = direction*t+start;
    }
    return (delta > EPSILLON);
    */
}

float computeFresnelCoef(vec3 eyeVect3, vec3 lightVect3, vec3 halfVector3) 
{
    vec4 eyeVector = vec4(eyeVect3, 0);
    vec4 lightVector = vec4(lightVect3, 0);
    vec4 halfVector = vec4(halfVector3, 0);

    halfVector = vec4(1,1,1,1);	//H
    halfVector = eyeVector + lightVector;
    halfVector = normalize(halfVector);

    float cosTheta = dot(halfVector,lightVector); 
    float sinTheta2 = 1-cosTheta*cosTheta ;

    float ci = eta*eta-sinTheta2;
    if(ci < 0)
    {	
        ci = 0;
    } else {
        ci = sqrt(ci);
    }

    float F_s = (cosTheta-ci)/(cosTheta+ci);
    float F_p = (eta*cosTheta-ci)/(eta*cosTheta+ci);
    F_s *= F_s; 
    F_p *= F_p; 

    return (F_s + F_p) / 2;
}

vec4 computeRefractionRayColor(vec3 eye, vec3 u, int nb) 
{
    vec4 C = vec4(0);
    float reflF = 1;
    for (int i = 0; i < nb; i++) {
	    vec3 point = vec3(0);
	    raySphereIntersect(eye,u,point);
        vec3 normalVector = normalize(center - point);
        // Le rayon réfléchi reste dans la sphère
        vec3 refl = normalize(reflect(u, normalVector));
        // Le rayon réfracté part vers l'environnement
        // On vient de l'intérieur de la sphère donc on utilise eta
        vec3 refr = normalize(refract(u, normalVector, eta));

        if (isnan(refr[0])) {
            return vec4(0);
        }

        float F = computeFresnelCoef(-u, refl, normalVector);

        C += reflF * (1 - F) * getColorFromEnvironment(refr);
        reflF *= F;

        eye = point;
        u = refl;
    }
    return C;
}

// RECURSIVE IDEA
//vec4 computeRayColor(vec3 eye, vec3 u, int nb) 
//{
//    if (nb == 0) 
//    {
//        return vec4(0);
//    }
//	vec3 point = vec3(0);
//	if (raySphereIntersect(eye,u,point))
//    {
//        vec3 normalVector = normalize(point - center);
//        vec3 refl = reflect(u, normalVector);
//        vec3 refr = refract(u, normalVector, eta);
//        vec4 C1 = computeRayColor(point, refl, nb--);
//        vec4 C2 = computeRayColor(point, refr, nb--);
//		return C1 + C2;
//	} else 
//    {
//        return getColorFromEnvironment(u);
//    }
//}
//

vec4 computeRayColor(vec3 eye, vec3 u, int nb) 
{
    vec3 point = vec3(0);
    if (raySphereIntersect(eye,u,point))
    {
        vec3 normalVector = normalize(point - center);
        //DEBUG : on veut obtenir une répartition uniforme des couleurs
        //return vec4(normalVector, 1);

        vec3 refl = normalize(reflect(u, normalVector));
        // On vient de l'extérieur, donc on utilise 1/eta dans refract
        vec3 refr = normalize(refract(u, normalVector, 1/eta));

        float F = computeFresnelCoef(-u, refl, normalVector);

        // Rayon réfléchi part vers l'environnement
        vec4 C1 = getColorFromEnvironment(refl);
        // Rayon réfracté part dans la sphère
        vec4 C2 = computeRefractionRayColor(point, refr, nb);

        //DEBUG rayon réfracté
        return F * C1 + (1 - F) * C2;
        return C2;
    } else 
    {
        return getColorFromEnvironment(u);
    }
}

void main(void)
{
    // Step 1: I need pixel coordinates. Division by w?
    vec4 worldPos = position;
    worldPos.z = 1; // near clipping plane
    worldPos = persp_inverse * worldPos;
    worldPos /= worldPos.w;
    worldPos.w = 0;
    worldPos = normalize(worldPos);

    // Step 2: ray direction:
    // u = Vecteur incident
    vec3 u = normalize((mat_inverse * worldPos).xyz);
    // Position de la caméra
    vec3 eye = (mat_inverse * vec4(0, 0, 0, 1)).xyz;

    vec4 resultColor = computeRayColor(eye, u, 1);
    //DEBUG
    //vec4 resultColor = getColorFromEnvironment(u);
    fragColor = resultColor;
}
