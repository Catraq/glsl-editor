#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>

#include <unistd.h>
#include <time.h>
#include <error.h>

#include <GL/glew.h>
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

int compile_shader(GLuint shader, const u8 *source, u32 size)
{
	int result = 0;

	glShaderSource(shader, 1, (const GLchar**) &source, &size);
       	glCompileShader(shader);
	
	GLint shader_status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &shader_status);
	if(shader_status == GL_FALSE)
	{
		GLsizei log_length = 0;
		GLchar error_buffer[1024];
		glGetShaderInfoLog(shader, 1024, &log_length, error_buffer);
 	       	fprintf(stderr, "%s \n", error_buffer);

		result = -1;
	}


	return(result);
}

struct file_desc
{
	u32 	 size;
	u8 	*buffer;
};

struct file_desc load_file(const char *filename)
{
	struct file_desc desc = {};	

	FILE *fp = fopen(filename, "rb");
	if(!fp)
	{
		perror("Could not open file");
		goto error;
	}

	
	fseek(fp, 0, SEEK_END);
	desc.size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	desc.buffer = malloc(desc.size);
	if(!desc.buffer)
	{
		perror("could not allocate memory");
		fclose(fp);
		goto error;
	}

	u32 read = fread(desc.buffer, 1, desc.size, fp);
	if(read != desc.size)
	{
		perror("could not read file");
		fclose(fp);
		goto error;
	}
	
	fclose(fp);
	return(desc);

error:
	free(desc.buffer);

	desc.buffer = NULL;
	desc.size = 0;

	return(desc);
}


int load_fragment_file_to_program(const char *filename, GLuint program, GLuint shader)
{	
	int result = 0;

	struct file_desc desc = load_file(filename);
	if(desc.buffer)
	{
		int compiled = compile_shader(shader, desc.buffer, desc.size);
		if(compiled < 0)
		{
			fprintf(stderr, "could not compile shader file.\n");
			result = compiled;
		}else{
			glAttachShader(program, shader);
			glLinkProgram(program);
			
			GLint state;
			glGetProgramiv(program, GL_LINK_STATUS, &state);
			if(state == GL_FALSE)
			{
				fprintf(stderr, "could not link shader program.\n");

				GLint length;
				glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
				
				GLsizei real_length = 0;
				GLchar buffer[4092];
				glGetProgramInfoLog(program, 4092, &real_length, buffer);
				fprintf(stderr, "%s \n", buffer);	
				
				result = -1;

			}
			glDetachShader(program, shader);
		}
		free(desc.buffer);
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
	uniforms->scroll 	= shader_uniform_location(program, "scroll", verbose);
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
	
	int 	fullscreen 	= 0;
	int 	verbose 	= 0;
	int	width 		= 640,
		height 		= 480;
	int 	reload_interval = 1000;
	char 	*filename 	= NULL;	
	
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
	
		case 'v':
			verbose = 1;
		break;

		case  't':
			if(optarg){
				reload_interval = atoi(optarg);
				if(reload_interval < 0)
					reload_interval = 0;
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
	if(glewInit() != GLEW_OK){
		fprintf(stderr, "glewInit(): failed \n");
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
	GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);	

	int compiled = compile_shader(vertex_shader, vertex_shader_source, sizeof(vertex_shader_source));	
	if(compiled < 0){
		fprintf(stderr, "Could not compile build in shader. \n");
		goto exit;
	}
	glAttachShader(shader_program, vertex_shader);



	GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	int loaded = load_fragment_file_to_program(filename, shader_program, fragment_shader);
	if(loaded < 0){
		fprintf(stdout, "Failed to load fragment shader.\n");
		//TODO: add fallback shader.
	
	}
	glUseProgram(shader_program);
	

	struct shader_uniforms uniforms;
	shader_uniforms_locate(shader_program, &uniforms, 1);


	glDisable(GL_DEPTH_TEST);
	
	int recently_reloaded = 0;
	
	float scroll;
      	double mouse_x, mouse_y;	
	clock_t reload_timer = clock();

	while(!glfwWindowShouldClose(window))
	{
		//Get screen size and update uniform 
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		glUniform2f(uniforms.resolution, (float)width, (float)height);	
		
		//Get cursor pos, scale, map  and set uniform	
		glfwGetCursorPos(window, &mouse_x, &mouse_y);
		float x, y;
		x = (mouse_x-(width/2.0f))/width;
		y = (mouse_y-(height/2.0f))/height;
		glUniform2f(uniforms.mouse, x, y);	

		//Get and set time 
		float time = (float)clock()/CLOCKS_PER_SEC;
		glUniform1f(uniforms.time, time);
		
		//Render
		glViewport(0,0, width, height);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		
		glBindVertexArray(vertex_array);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		


			
		int reload = 0;
		int reload_verbose = 1;
		int last_reload_ms = (float)((clock() - reload_timer)/(float)(CLOCKS_PER_SEC/100000));

		if(reload_interval){
			printf("%i %i \n", last_reload_ms, reload_interval);
			if(reload_interval < last_reload_ms){
				reload = 1;
				reload_verbose = verbose;
				reload_timer = clock();
			}
		}

	
		int reload_key = glfwGetKey(window, GLFW_KEY_R);
		if(reload_key == GLFW_PRESS){
		
			//Used to make sure 1 click eq 1 reload.
			if(!recently_reloaded){
				reload = 1;
				recently_reloaded = 1;
				fprintf(stdout, "Reloading shader.\n");
			}
		}else{
			recently_reloaded = 0;
		}

		
		if(reload){
			
			loaded = load_fragment_file_to_program(filename, shader_program, fragment_shader);
			if(loaded < 0){
			}else{
				shader_uniforms_locate(shader_program, &uniforms, verbose);
			}
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
