//
// I leave out the specular & ambient intentionally
// please add them in.
//
uniform sampler2DShadow shadowMap;
uniform sampler2D sceneMap;
varying vec4 LSCoord;
varying vec4 diffuse;

uniform bool textured_object;
uniform float xPixelOffset;
uniform float yPixelOffset;

uniform bool use_pcf;

float lookup( vec2 offSet)
{
	vec4 ShadowCoord = LSCoord;
	
	// Values are multiplied by ShadowCoord.w because shadow2DProj does a W division for us.
	return shadow2DProj(shadowMap, 
	                    ShadowCoord + vec4(offSet.x * xPixelOffset * ShadowCoord.w, 
										   offSet.y * yPixelOffset * ShadowCoord.w, 0.0, 0.0) ).r;
}

void main ()
{
    vec4 color = vec4( 0.95 ); //diffuse; // only diffuse
    if (textured_object) 
		color = color*texture2D (sceneMap, gl_TexCoord[0].st);
	
	float kTransparency = 0.2;
    float rValue;

	if (use_pcf) {
		float shadow = 0.0;
		if (LSCoord.w > 0.0) {
			float x,y;

#if 0			
			for (y = -1.5 ; y <=1.5 ; y+=1.0) {  // -1.5, -0.5, 0.5, 1.5
				for (x = -1.5 ; x <=1.5 ; x+=1.0) {
					shadow += lookup(vec2(x,y));
				}		
			}
			shadow /= 16.0 ;
#else
			for (y = -3.0 ; y <=3.0 ; y+=1.0) {  // -3, -2,-1,0,1,2,3
				for (x = -3.0 ; x <=3.0 ; x+=1.0) {
					shadow += lookup(vec2(x,y));
				}		
			}
			shadow /= 49.0 ;
#endif		
			rValue = shadow + kTransparency;
		}
	} else 
		rValue = shadow2DProj (shadowMap, LSCoord).r + kTransparency;

    if (LSCoord.w > 0.0)
        gl_FragColor = color*rValue;
    else
        gl_FragColor = color;
}
