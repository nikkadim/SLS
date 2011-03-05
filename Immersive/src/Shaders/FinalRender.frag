uniform sampler2D normals;

varying vec3 v;

void main()
{
	vec3 Normal = normalize(gl_NormalMatrix * vec3(texture2D(normals, gl_TexCoord[0].st)));
	vec3 L = normalize(gl_LightSource[0].position.xyz - v);
	vec3 E = normalize(-v);
	vec3 R = normalize(-reflect(L,Normal));
	
	//	Ambient light
	vec4 Iamb = gl_FrontLightProduct[0].ambient;
	
	//	Diffuse light
	vec4 Idiff = gl_FrontLightProduct[0].diffuse * max(dot(Normal,L), 0.0);
	
	//	Specular light
	vec4 Ispec = gl_FrontLightProduct[0].specular * pow(max(dot(R,E),0.0), gl_FrontMaterial.shininess);
	
	//	Total color
	gl_FragColor = gl_FrontLightModelProduct.sceneColor + Iamb + Idiff + Ispec;
	//gl_FragColor = vec4(1.0,0.0,0.0,1.0);
	//gl_FragData[0] = vec4(Normal, 1.0);
	//gl_FragData[0] = vec4(gl_TexCoord[0].s, gl_TexCoord[0].t, 0.0, 0.0);
}