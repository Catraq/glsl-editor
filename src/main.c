#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <assert.h>

#include <unistd.h>
#include <time.h>
#include <error.h>

#include <GL/glew.h>
#include <GL/glu.h>
#include <GLFW/glfw3.h>


typedef uint32_t u32 ;
typedef uint8_t u8;
typedef int8_t s8;


const u8 vertex_shader_source[] = 
{
	"#version 330 core				\n"
	"layout(location = 0) in vec2 in_position; 	\n"
	"out vec2 texcoord;				\n"
	"void main(){					\n"
	"	gl_Position.xy = in_position;		\n"
	"	gl_Position.w = 1.0; 			\n"
	"	texcoord = (in_position+1.0)/2.0;	\n"
	"}						\n"
};

char *load_file(const char *filename, u32 *size)
{
	assert(filename && size);

	u32 s;
	FILE *fp = fopen(filename, "rb");
	if(!fp)
	{
		perror("Could not open file");
		goto error;
	}

	
	fseek(fp, 0, SEEK_END);
	s= ftell(fp);
	fseek(fp, 0, SEEK_SET);

	char *buffer = malloc(s);
	if(!buffer)
	{
		perror("could not allocate memory");
		fclose(fp);
		goto error;
	}

	u32 read = fread(buffer, 1, s, fp);
	if(read != s)
	{
		perror("could not read file");
		fclose(fp);
		goto error;
	}
	
	fclose(fp);

	*size = s;
	return(buffer);

error:
	free(buffer);

	buffer = NULL;
	*size = 0;

	return buffer;
}

int compile_shader(GLuint shader, const u8 *source, u32 size, int verbose)
{
	int result = 0;

	glShaderSource(shader, 1, (const GLchar**) &source, (const GLint*)&size);
       	glCompileShader(shader);
	
	GLint shader_status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &shader_status);
	if(shader_status == GL_FALSE)
	{
		GLsizei log_length = 0;
		GLchar error_buffer[1024];
		glGetShaderInfoLog(shader, 1024, &log_length, error_buffer);

		if(verbose)
 	       		fprintf(stderr, "%s \n", error_buffer);

		result = -1;
	}


	return(result);
}



int load_fragment_file_to_program(const char *filename, GLuint program, GLuint shader, int verbose)
{	
	int result = 1;
	
	u32 size = 0;
	char *buffer = load_file(filename, &size);
	if(buffer)
	{
		int compiled = compile_shader(shader, buffer, size, verbose);
		if(compiled < 0)
		{
			if(verbose)
				fprintf(stderr, "could not compile shader file.\n");

			result = -1;	
		}
		else
		{
			glAttachShader(program, shader);
			glLinkProgram(program);
			
			GLint state;
			glGetProgramiv(program, GL_LINK_STATUS, &state);
			if(state == GL_FALSE)
			{
				if(verbose)
					fprintf(stderr, "could not link shader program.\n");

				GLint length;
				glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
				
				GLsizei real_length = 0;
				GLchar buffer[4092];
				glGetProgramInfoLog(program, 4092, &real_length, buffer);
				if(verbose)
					fprintf(stderr, "%s \n", buffer);	
				
				result = -1;

			}
			glDetachShader(program, shader);
		}
		free(buffer);
	}else{
		fprintf(stderr, "could not load shader file.\n");
	}
	
	return(result);
}





GLint shader_uniform_location(GLint program, const char *name, int verbose)
{
	GLint location = glGetUniformLocation(program, name);
	if(location < 0 && verbose){
		fprintf(stdout, "Could not get \"%s\" uniform location.\n", name);
	}

	return(location);
}

struct shader_uniforms{
	GLint resolution;
	GLint mouse;
	GLint scroll;
	GLint time;
};


int shader_uniforms_locate(GLint program, struct shader_uniforms *uniforms, int verbose)
{
	int result = 0;
	
	uniforms->resolution 	= shader_uniform_location(program, "resolution", verbose);
	uniforms->mouse 	= shader_uniform_location(program, "mouse", verbose);
	uniforms->time		= shader_uniform_location(program, "time", verbose);

	return(result);
}

const char usage_str[] = {
	"Usage: [OPTIONS] -i file		\n"
	" 				\n"
	"Commands:			\n"
	" -f[ullscreen]			\n"
	" -w[idth]	Default 640.	\n"
	" -h[eight]	Default 480.	\n"
	" -v[erbose]	Show output of shader compilation when auto reloading.	\n"
	" -t[imer]	Time between shader file reloads in ms. Default 1000. 	\n"
	"		Value 0 disables auto reload. 				\n"
	" -i[input]	Fragment shader file. Required.				\n"
	"									\n"
	" To manually reload the shader, press R. Allways verbose.		\n"
	"									\n"
};


int main(int argc, char *argv[])
{
	int result = 0;
	
	int 	fullscreen 		= 0;
	int 	verbose 		= 0;
	int	width 			= 640,
		height 			= 480;
	int 	reload_interval_ms 	= 1000;
	char 	*filename 		= NULL;	
	
	if(argc < 2){
		fprintf(stderr, usage_str);
		exit(EXIT_FAILURE);
	}

	int c = 0;
	int arg_error = 0;	
	while((c = getopt(argc, argv, "fvt:w:h:i:")) != -1){
	
		switch(c)
		{
		
		case 'f':
			fullscreen = 1;		
		break;
	
		break;

		case  't':
			if(optarg){
				reload_interval_ms = atoi(optarg);
				if(reload_interval_ms < 0)
					reload_interval_ms = 0;
			}
		break;

		case  'w':
			if(optarg){
				width = atoi(optarg);
				if(width <= 0)
					width = 1;
			}
		break;


		case 'h':
			if(optarg){
				height = atoi(optarg);
				if(height <= 0)
					height = 1;
			}
		break;

		case 'i':
			if(optarg)
				filename = optarg;	
			
		break;
		
		case '?':
			arg_error = 1;
		break;

		default:
			arg_error = 1;
		break;

		}
	}

	if(arg_error){
		fprintf(stderr, usage_str);
		exit(EXIT_FAILURE);
	}
	
	if(!filename){
		fprintf(stderr, "error: -i argument required.\n");
		exit(EXIT_FAILURE);
	}
	
	
	if(!glfwInit()){
		fprintf(stderr, "glfwInit(): failed \n");
 		goto exit;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);

	GLFWwindow *window = glfwCreateWindow(width, height, "shader viewer.",	fullscreen ? glfwGetPrimaryMonitor() : NULL , NULL);
	if(!window){
		fprintf(stderr, "glfwCreateWindow(): failed \n");
 		goto exit;
	}

	glfwMakeContextCurrent(window);

	glewExperimental = GL_TRUE;
	GLenum glew_init_res;
	if((glew_init_res == glewInit())){
		fprintf(stderr, "glewInit(): failed, %s \n", glewGetErrorString(glew_init_res));
		goto exit;
	}

	const GLfloat quad_vertices[] = 
	{	
		-1.0f, 1.0f,
		-1.0f,-1.0f,
		1.0f, -1.0f,

		1.0f, -1.0f,
		1.0f, 1.0f,
		-1.0f,1.0f	
	};


	GLuint vertex_array;
	glGenVertexArrays(1, &vertex_array);
	glBindVertexArray(vertex_array);	

	GLuint quad_buffer;
	glGenBuffers(1, &quad_buffer);	
	glBindBuffer(GL_ARRAY_BUFFER, quad_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);


	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)0);
	


	GLuint shader_program = glCreateProgram();	

	//Vertex shader for rendering quad. 
	GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);	
	int compiled = compile_shader(vertex_shader, vertex_shader_source, sizeof(vertex_shader_source), 1);	
	if(compiled < 0){
		fprintf(stderr, "Could not compile build in shader. \n");
		goto exit;
	}
	glAttachShader(shader_program, vertex_shader);


	
	//Load fragmentshader from file 
	const int frag_shader_init_verbose = 1;
	GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	result = load_fragment_file_to_program(filename, shader_program, fragment_shader, frag_shader_init_verbose);
	if(result < 0){
		fprintf(stdout, "Failed to load fragment shader.\n");
		//TODO: add fallback shader.
	}
	glUseProgram(shader_program);
	

	struct shader_uniforms uniforms;
	shader_uniforms_locate(shader_program, &uniforms, frag_shader_init_verbose);


	glDisable(GL_DEPTH_TEST);
	
	int recently_reloaded = 0;
      	double mouse_x, mouse_y;	
	clock_t reload_timer = clock();
	while(!glfwWindowShouldClose(window))
	{
		//Screensize
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
	

		//Mouse position in screen space. 
		glfwGetCursorPos(window, &mouse_x, &mouse_y);
		float x, y;
		x = (mouse_x-(width/2.0f))/width;
		y = (mouse_y-(height/2.0f))/height;

		//CPU time 
		float time = (float)clock()/CLOCKS_PER_SEC;


		
		//Update shader unfiforms 
		glUniform2f(uniforms.resolution, (float)width, (float)height);	
		glUniform2f(uniforms.mouse, x, y);	
		glUniform1f(uniforms.time, time);
	
		//Render clear. 	
		glViewport(0,0, width, height);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		
		//Render quad. 
		glBindVertexArray(vertex_array);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		

			
		int reload = 0;
		int reload_verbose = 0;
		int last_reload_ms = (float)((clock() - reload_timer)/(float)(CLOCKS_PER_SEC/100000));

		if(reload_interval_ms){
			if(reload_interval_ms < last_reload_ms){
				reload = 1;
				reload_verbose = verbose;
			}
		}

	
		int reload_key = glfwGetKey(window, GLFW_KEY_R);
		if(reload_key == GLFW_PRESS){
			//Avoid reloading while key is pressed. 	
			if(!recently_reloaded){
				reload = 1;
				reload_verbose = 1;
				recently_reloaded = 1;
				fprintf(stdout, "Reloading shader.\n");
			}
		}else{
			recently_reloaded = 0;
		}

		
		if(reload){
			
			result = load_fragment_file_to_program(filename, shader_program, fragment_shader, reload_verbose);
			if(result < 0){}
			else{
				shader_uniforms_locate(shader_program, &uniforms, reload_verbose);
			}

			reload_timer = clock();
		}

		glfwPollEvents();
		glfwSwapBuffers(window);
	}
	
	glDeleteShader(fragment_shader);
	glDeleteShader(vertex_shader);
	glDeleteProgram(shader_program);
	glDeleteBuffers(1, &quad_buffer);

exit:	
	glfwDestroyWindow(window);	
	glfwTerminate();

	return(result);	
}
