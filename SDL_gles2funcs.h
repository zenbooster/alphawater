#include "../src/render/opengles2/SDL_gles2funcs.h"
SDL_PROC(void, glUniform1f, (GLint, GLfloat))
SDL_PROC(void, glBindVertexArray, (GLuint))
SDL_PROC(void, glUniform2fv, (GLint, GLsizei, const GLfloat *))
SDL_PROC(void, glGenVertexArrays, (GLsizei, GLuint *))
SDL_PROC(void, glDeleteVertexArrays, (GLsizei, const GLuint *))
SDL_PROC(void, glDrawElements, (GLenum,	GLsizei, GLenum, const GLvoid *))
SDL_PROC(void, glGetShaderSource, (GLuint, GLsizei,	GLsizei *, GLchar *))
SDL_PROC(void, glDetachShader, (GLuint, GLuint))
SDL_PROC(void, glVertexAttrib1f, (GLuint, GLfloat))
SDL_PROC(void, glVertexAttrib2f, (GLuint, GLfloat, GLfloat))
SDL_PROC(void, glVertexAttrib3f, (GLuint, GLfloat, GLfloat, GLfloat))
SDL_PROC(void, glVertexAttrib4f, (GLuint, GLfloat, GLfloat, GLfloat, GLfloat))
SDL_PROC(void, glUniform2f, (GLint, GLfloat, GLfloat))
SDL_PROC(void, glUniform3f, (GLint, GLfloat, GLfloat, GLfloat))
SDL_PROC(void, glUniform1iv, (GLint, GLsizei, const GLint *))
SDL_PROC(void, glUniform1fv, (GLint, GLsizei, const GLfloat *))
SDL_PROC(void, glUniform3fv, (GLint, GLsizei, const GLfloat *))
SDL_PROC(void, glUniform4fv, (GLint, GLsizei, const GLfloat *))
SDL_PROC(void, glDeleteRenderbuffers, (GLsizei,	GLuint *))
SDL_PROC(void, glGenRenderbuffers, (GLsizei, GLuint *))
SDL_PROC(void, glBindRenderbuffer, (GLenum,	GLuint))
SDL_PROC(void, glRenderbufferStorage, (GLenum, GLenum, GLsizei,	GLsizei))
SDL_PROC(void, glFramebufferRenderbuffer, (GLenum, GLenum, GLenum, GLuint))
SDL_PROC(void, glCopyTexSubImage2D, (GLenum, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei))
SDL_PROC(void, glReadBuffer, (GLenum mode))
SDL_PROC(void, glTexStorage2D, (GLenum,	GLsizei, GLenum, GLsizei, GLsizei))
SDL_PROC(void, glTexImage3D, (GLenum, GLint, GLint, GLsizei, GLsizei, GLsizei, GLint, GLenum, GLenum, const GLvoid *))
SDL_PROC(void, glGenerateMipmap, (GLenum))
SDL_PROC(void, glTexEnvf, (GLenum, GLenum, GLfloat))
SDL_PROC(void, glPushMatrix, (void))
SDL_PROC(void, glBegin, (GLenum))
SDL_PROC(void, glEnd, (void))
SDL_PROC(void, glTranslatef, (GLfloat, GLfloat, GLfloat))
SDL_PROC(void, glRotatef, (GLfloat, GLfloat, GLfloat, GLfloat))
SDL_PROC(void, glTexCoord2f, (GLfloat, GLfloat))
SDL_PROC(void, glPopMatrix, (void))
SDL_PROC(void, glVertex3f, (GLfloat, GLfloat, GLfloat))