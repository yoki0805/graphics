#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gl/glew.h"

static int printOglError(char *file, int line)
{
    //
    // Returns 1 if an OpenGL error occurred, 0 otherwise.
    //
    GLenum glErr;
    int    retCode = 0;

    glErr = glGetError();
    while (glErr != GL_NO_ERROR)
    {
        printf("glError in file %s @ line %d: %s\n", file, line, gluErrorString(glErr));
        retCode = 1;
        glErr = glGetError();
    }
    return retCode;
}

static void printProgramInfoLog(GLuint program)
{
    int infologLen = 0;
    int charsWritten  = 0;
    GLchar *infoLog;

    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infologLen);
  
    if (infologLen > 0)
    { 
        infoLog = (GLchar*) malloc(infologLen);
        if (infoLog == NULL)
        {
            printf("ERROR: Could not allocate InfoLog buffer\n");
            exit(1);
        }
        glGetProgramInfoLog(program, infologLen, &charsWritten, infoLog);
        printf("InfoLog:\n%s\n\n", infoLog);
        free(infoLog);
    }
}

static void printShaderInfoLog(GLuint shader)
{
    int infologLen = 0;
    int charsWritten  = 0;
    GLchar *infoLog;

    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infologLen);
  
    if (infologLen > 0)
    { 
        infoLog = (GLchar*) malloc(infologLen);
        if (infoLog == NULL)
        {
            printf("ERROR: Could not allocate InfoLog buffer\n");
            exit(1);
        }
        glGetShaderInfoLog(shader, infologLen, &charsWritten, infoLog);
        if (charsWritten) printf("InfoLog:\n%s\n\n", infoLog);
        free(infoLog);
    }
}
 


// textfile.cpp
//
// simple reading and writing for text files
//
// www.lighthouse3d.com
//
// You may use these functions freely.
// they are provided as is, and no warranties, either implicit,
// or explicit are given
//////////////////////////////////////////////////////////////////////


//
// convert file into a NULL-terminated string
//

static char *textFileRead(char *fn) {


	FILE *fp;
	char *content = NULL;

	int count=0;

	if (fn != NULL) {
		fp = fopen(fn,"rt");

		if (fp != NULL) {
      
      fseek(fp, 0, SEEK_END); // to end of file
      count = ftell(fp);   // get current position
      rewind(fp);

			if (count > 0) {
				content = (char *)malloc(sizeof(char) * (count+1));
				count = fread(content,sizeof(char),count,fp);
				content[count] = '\0';
			}
			fclose(fp);
		}
	}
	return content;
}

static int textFileWrite(char *fn, char *s) {

	FILE *fp;
	int status = 0;

	if (fn != NULL) {
		fp = fopen(fn,"w");

		if (fp != NULL) {
			
			if (fwrite(s,sizeof(char),strlen(s),fp) == strlen(s))
				status = 1;
			fclose(fp);
		}
	}
	return(status);
}



GLuint setShaders(char* vert_file, char* frag_file) 
{
    GLuint v,f, p;

	char *vs = NULL,*fs = NULL;

	p = glCreateProgram();
	if (vert_file) {
		v = glCreateShader(GL_VERTEX_SHADER);
		vs = textFileRead(vert_file);
		const char * vv = vs;
		glShaderSource(v, 1, &vv,NULL);
		free(vs);
		glCompileShader(v);
		printShaderInfoLog(v);
		glAttachShader(p,v);
	}

	if (frag_file) {
		f = glCreateShader(GL_FRAGMENT_SHADER);
		fs = textFileRead(frag_file);
		const char * ff = fs;
		glShaderSource(f, 1, &ff,NULL);
		free(fs);
		glCompileShader(f);
		printShaderInfoLog(f);
		glAttachShader(p,f);
	}
	
	glLinkProgram(p);
//	printProgramInfoLog(p);

	return p;
}

