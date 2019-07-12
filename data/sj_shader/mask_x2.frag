#version 120
#extension GL_ARB_texture_rectangle : enable
#extension GL_EXT_gpu_shader4 : enable

uniform sampler2DRect Front;
uniform sampler2DRect Back;
uniform sampler2DRect mask_0;
uniform sampler2DRect mask_1;

void main(){
	/********************
	********************/
	vec2 tex_pos = gl_TexCoord[0].xy;
	
	vec4 col_mask_0	= texture2DRect(mask_0, tex_pos);
	vec4 col_mask_1	= texture2DRect(mask_1, tex_pos);
	vec4 col_Front	= texture2DRect(Front, tex_pos);
	vec4 col_Back	= texture2DRect(Back, tex_pos);
	
	col_mask_1.rgb = vec3(1.0, 1.0, 1.0) - col_mask_1.rgb;
	vec4 col_mask = min(col_mask_0, col_mask_1);
	
	/********************
	********************/
	float alpha = col_mask.r;
	vec4 color = col_Front * alpha + col_Back * (1.0 - alpha);
	color.a = 1.0;
	
	/********************
	********************/
	gl_FragColor = color;
}
