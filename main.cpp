#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include <iostream>
#include <fstream>
#include <vector>
#include <map>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <linux/input.h>

#define STB_IMAGE_IMPLEMENTATION  
#include "include/stb_image.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

const int FOVX = 32;
const int FOVY = 24;
const int FOVE = 8;
const int FOVT = 20;

const int INTERFACE = 9;
const int FILLIN = 10;
const int INVENTORY = 11;

#define ANSI_COLOR_RED     "//R"
#define ANSI_COLOR_GREEN   "//G"
#define ANSI_COLOR_YELLOW  "//Y"
#define ANSI_COLOR_BLUE    "//B"
#define ANSI_COLOR_MAGENTA "//M"
#define ANSI_COLOR_CYAN    "//C"
#define ANSI_COLOR_RESET   "//S"

class Player;
class Map;
class Entities;

const int shift = 3;
int AdvanceX;
int AdvanceY = 25;

int fd;

struct stats{
private:
	static int counter;
public:
	int id;
	char* name;
	char* type;
	int dmg;
	int accuracy;
	int health;
	
	stats(){
		name = NULL;
		type = NULL;
		dmg = 0;
		accuracy = 0;
		health = 0;
		id = counter++;
	}

	stats(const char* n, const char* t, int d, int a){
		name = new char [strlen(n)+1];
		strcpy(name, n);
		type = new char [strlen(t)+1];
		strcpy(type, t);
		dmg = d;
		accuracy = a;
		id = counter++;
	}

	stats(const char*n, const char* t, int h){
		name = new char [strlen(n)+1];
		strcpy(name, n);
		type = new char [strlen(t)+1];
		strcpy(type, t);
		dmg = 0;
		accuracy = 0;
		health = h;
		id = counter++;
	}

	~stats(){
		if(name) delete [] name;
		if(type) delete [] type;
	}

	void showStats(const char mode = 0);

	stats &operator=(const stats &ob){
		if(name) delete [] name;
		if(type) delete [] type;
		name = new char[strlen(ob.name)+1];
		strcpy(name, ob.name);
		type = new char [strlen(ob.type)+1];
		strcpy(type, ob.type);
		dmg = ob.dmg;
		accuracy = ob.accuracy;
		id = ob.id;
		health = ob.health;
	}
};

struct fontChar{
	GLuint TextureID;
	ivec2 Size;
	ivec2 Bearing;
	GLuint Advance;
};

template <class T>
struct node{
	T value;
	node* next;	
};

struct pool{
	char** names;
	int amount;
	pool(){
		amount = 1;
		names = new char*;
		names[0] = new char [2];
		strcpy(names[0], "N");
	}
	~pool(){
		for(int i = 0; i < amount; i++)
			delete [] names[i];
		delete [] names;
	}
	void setPool(const char* fileName){
		FILE* file = fopen(fileName, "r");

		for(int i = 0; i < amount; i++)
			delete [] names[i];
		delete [] names;

		fscanf(file, "%d", &amount);
		names = new char* [amount];
		for(int i = 0; i < amount; i++){
			int j = 0;
			names[i] = new char [100];
			while((names[i][j++] = fgetc(file)) != '\n')
				if(names[i][j-1] == ' ' && names[i][j-2] != ' ')
					names[i][j-1] = '\n';
			names[i][j-1] = '\0';
		}
		fclose(file);
	}
} names;

class DrawImage{
	GLuint vertexbuffer, uvbuffer, FontVertexBuffer, FontUVBuffer;
	GLuint ProgramID, MatrixID, TextureID, VertexArrayID, FontID, TextID, TextColorID;

	GLFWwindow* window;
	std::vector<vec3> vertices;
	std::vector<vec2> UVs;

	vec2 resolution;

	vec3 TextColor;

	std::vector<GLuint> textures;

	int devide;

	std::map<GLchar, fontChar> Characters;
public:
	DrawImage(){

		resolution = vec2(2.0f/(FOVX + shift*2), 2.0f/(FOVY + shift*2));
		TextColor = vec3(1, 1, 1);

		glfwInit();
		glfwWindowHint(GLFW_SAMPLES, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

		window = glfwCreateWindow(1024, 768, "Game", NULL, NULL);
		glfwMakeContextCurrent(window);

		glewExperimental = true;
		glewInit();	

		glClearColor(0.0f, 0.0f, 0.4f, 0.0f);	

		glGenVertexArrays(1, &VertexArrayID);
		glBindVertexArray(VertexArrayID);	

		ProgramID = LoadShaders("shaders/vertex", "shaders/fragment");
		MatrixID = glGetUniformLocation(ProgramID, "MVP");
		TextureID = glGetUniformLocation(ProgramID, "Texture");
		
		FontID = LoadShaders("shaders/Fontvertex", "shaders/Fontfragment");
		TextID = glGetUniformLocation(FontID, "myTextureSampler");
		TextColorID = glGetUniformLocation(FontID, "textColor");

		textures.push_back(loadBMP("textures/floor.bmp"));
		textures.push_back(loadBMP("textures/wall.bmp"));
		textures.push_back(loadBMP("textures/player.bmp"));
		textures.push_back(loadBMP("textures/enemy.bmp"));
		textures.push_back(loadBMP("textures/turret.bmp"));
		textures.push_back(loadBMP("textures/chest.bmp"));
		textures.push_back(loadBMP("textures/projectile.bmp"));
		textures.push_back(loadBMP("textures/sayan.bmp"));
		textures.push_back(loadBMP("textures/explosion.bmp"));
		textures.push_back(loadBMP("textures/interface.bmp"));
		textures.push_back(loadBMP("textures/fillin.bmp"));
		textures.push_back(loadBMP("textures/inventory.bmp"));

		loadOBJ("models/tile.obj", vertices, UVs);
		devide = vertices.size();
		loadOBJ("models/player.obj", vertices, UVs);

		glGenBuffers(1, &FontVertexBuffer);
		glGenBuffers(1, &FontUVBuffer);

		glGenBuffers(1, &vertexbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(vec3), &vertices[0], GL_STATIC_DRAW);	

		glGenBuffers(1, &uvbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glBufferData(GL_ARRAY_BUFFER, UVs.size()*sizeof(vec2), &UVs[0], GL_STATIC_DRAW);  	

		FT_Library ft;
		FT_Init_FreeType(&ft);

		FT_Face face;
		FT_New_Face(ft, "fonts/font.ttf", 0, &face);

		FT_Set_Pixel_Sizes(face, 0, 15);

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		for(GLubyte c = 0; c < 128; c++){

			FT_Load_Char(face, c, FT_LOAD_RENDER);

			GLuint texture;
			glGenTextures(1, &texture);
			glBindTexture(GL_TEXTURE_2D, texture);
			glTexImage2D(
				GL_TEXTURE_2D, 
				0, 
				GL_RED, 
				face->glyph->bitmap.width,
				face->glyph->bitmap.rows,
				0,
				GL_RED,
				GL_UNSIGNED_BYTE,
				face->glyph->bitmap.buffer
			);

 		   	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	   		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			
			fontChar character = {
				texture,
				ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
				ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
				face->glyph->advance.x
			};

			AdvanceX = face->glyph->advance.x;

			Characters.insert(std::pair<GLchar, fontChar>(c, character));
		}

		FT_Done_Face(face);
		FT_Done_FreeType(ft);
	}

	~DrawImage(){

		glDeleteBuffers(1, &vertexbuffer);
    	glDeleteBuffers(1, &uvbuffer);
    	glDeleteBuffers(1, &FontVertexBuffer);
    	glDeleteBuffers(1, &FontUVBuffer);

	    glDeleteProgram(ProgramID);
	    glDeleteProgram(FontID);

	    for(int i = 0; i < textures.size(); i++)
	    	glDeleteTextures(1, &textures[i]);

	    glDeleteVertexArrays(1, &VertexArrayID);	

	    glfwTerminate();
	}

	void cleanup(){
		glClear(GL_COLOR_BUFFER_BIT);
	}

	void refresh(){
		glfwSwapBuffers(window);
	}

	void drawXY(int type, int x, int y){

		if(type == -1) return;	

		glUseProgram(ProgramID);

		glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textures[type]);
       	glUniform1i(TextureID, 0);

        mat4 Model = translate(mat4(1.0f), vec3(resolution*vec2(x - shift, -y + shift), 0))*scale(mat4(1.0f), vec3(resolution, 1));	

        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &Model[0][0]);

		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);	

		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

		if(type != 2)
			glDrawArrays(GL_TRIANGLES, 0, devide);		
		else
			glDrawArrays(GL_TRIANGLES, devide, vertices.size() - devide);

		glDisableVertexAttribArray(0);		
		glDisableVertexAttribArray(1);
	}

	void drawInterface(){

		glUseProgram(ProgramID);

		glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textures[INTERFACE]);
       	glUniform1i(TextureID, 0);	

        mat4 Model = translate(mat4(1.0f), vec3(resolution*vec2(FOVX/2 - shift, -FOVY/2 + shift), 0));
        Model *= scale(mat4(1.0f), vec3(resolution*vec2(2*shift, FOVY), 1));	

        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &Model[0][0]);

		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);	

		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

		glDrawArrays(GL_TRIANGLES, 0, devide);		

		Model = translate(mat4(1.0f), vec3(resolution*vec2(-FOVX/2 - shift, -FOVY/2 - shift), 0));
        Model *= scale(mat4(1.0f), vec3(resolution*vec2(FOVX, 2*shift), 1));	

        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &Model[0][0]);

		glDrawArrays(GL_TRIANGLES, 0, devide);

		Model = translate(mat4(1.0f), vec3(resolution*vec2(FOVX/2 - shift, -FOVY/2 - shift), 0));
        Model *= scale(mat4(1.0f), vec3(resolution*vec2(2*shift, 2*shift), 1));	

        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &Model[0][0]);

		glBindTexture(GL_TEXTURE_2D, textures[FILLIN]);        

		glDrawArrays(GL_TRIANGLES, 0, devide);                

		glDisableVertexAttribArray(0);		
		glDisableVertexAttribArray(1);
	}

	void drawStats(const char mode  = 0){

		glUseProgram(ProgramID);

		glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textures[INVENTORY]);
       	glUniform1i(TextureID, 0);	

        mat4 Model;

        if(mode)
        	if(mode == 'l')
        		Model = translate(mat4(1.0f), vec3(vec2(-1.5f, -0.5f), 0));
        	else
        		Model = translate(mat4(1.0f), vec3(vec2(0.5f, -0.5f), 0));
        else
        	Model = translate(mat4(1.0f), vec3(vec2(-2, -0.5f), 0));

        Model = scale(mat4(1.0f), vec3(0.5f, 1.5f, 1))*Model;	

        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &Model[0][0]);

		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);	

		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

		glDrawArrays(GL_TRIANGLES, 0, devide);		       

		glDisableVertexAttribArray(0);		
		glDisableVertexAttribArray(1);
	}

	void drawInventory(){

		glUseProgram(ProgramID);

		glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textures[INVENTORY]);
       	glUniform1i(TextureID, 0);	

        mat4 Model = translate(mat4(1.0f), vec3(vec2(-0.5f, -0.5f), 0));
        Model = scale(mat4(1.0f), vec3(1, 1, 1))*Model;	

        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &Model[0][0]);

		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);	

		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

		glDrawArrays(GL_TRIANGLES, 0, devide);		       

		glDisableVertexAttribArray(0);		
		glDisableVertexAttribArray(1);
	}

	void drawYesNo(){

		glUseProgram(ProgramID);

		glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textures[INVENTORY]);
       	glUniform1i(TextureID, 0);	

        mat4 Model = translate(mat4(1.0f), vec3(vec2(-0.5f, -0.5f), 0));
        Model = scale(mat4(1.0f), vec3(1.5f, 0.5f, 1))*Model;	

        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &Model[0][0]);

		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);	

		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

		glDrawArrays(GL_TRIANGLES, 0, devide);		       

		glDisableVertexAttribArray(0);		
		glDisableVertexAttribArray(1);
	}

	void drawChest(){

		glUseProgram(ProgramID);

		glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textures[INVENTORY]);
       	glUniform1i(TextureID, 0);	

        mat4 Model = translate(mat4(1.0f), vec3(vec2(-1, -0.5f), 0));
        Model = scale(mat4(1.0f), vec3(1, 1, 1))*Model;	

        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &Model[0][0]);

		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);	

		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

		glDrawArrays(GL_TRIANGLES, 0, devide);

		Model = translate(mat4(1.0f), vec3(vec2(0, -0.5f), 0));
        Model = scale(mat4(1.0f), vec3(1, 1, 1))*Model;	

        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &Model[0][0]);

        glDrawArrays(GL_TRIANGLES, 0, devide);		       

		glDisableVertexAttribArray(0);		
		glDisableVertexAttribArray(1);
	}

	void drawWholeScreen(int n){

		glUseProgram(ProgramID);

		glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textures[n]);
       	glUniform1i(TextureID, 0);	

        mat4 Model = translate(mat4(1.0f), vec3(vec2(-0.5f, -0.5f), 0));
        Model = scale(mat4(1.0f), vec3(2, 2, 1))*Model;	

        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &Model[0][0]);

		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);	

		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

		glDrawArrays(GL_TRIANGLES, 0, devide);		       

		glDisableVertexAttribArray(0);		
		glDisableVertexAttribArray(1);
	}

	int printText(const char* text, int x, int y, int size){

		glUseProgram(FontID);
		glActiveTexture(GL_TEXTURE0);
		glUniform1i(TextID, 0);

		int length = strlen(text);
		int strStart = x;
		int end = 0;

   		for (int i = 0; i < length; i++){

   			glUniform3f(TextColorID, TextColor.x, TextColor.y, TextColor.z);

   			if(text[i] == '/' && text[i+1] == '/'){
   				switch(text[i+2]){
   					case 'S': TextColor = vec3(1, 1, 1); break;
   					case 'R': TextColor = vec3(1, 0, 0); break;
   					case 'G': TextColor = vec3(0, 1, 0); break;
   					case 'Y': TextColor = vec3(1, 1, 0); break;
   					case 'M': TextColor = vec3(1, 0, 1); break;
   					case 'C': TextColor = vec3(0, 1, 1); break;
   					case 'B': TextColor = vec3(0, 0, 1); break;
   				}
   				i += 2;
   				continue;
   			} else if(text[i] == '\n'){
   				x = strStart;
   				y -= AdvanceY*size;
   				end++;
   				continue; 
   			}
	
			fontChar ch = Characters[text[i]];

			glBindTexture(GL_TEXTURE_2D, ch.TextureID);	

	        GLfloat xpos = x + ch.Bearing.x * size;
	        GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * size;		

	        GLfloat w = ch.Size.x * size;
	        GLfloat h = ch.Size.y * size;
		    
	        std::vector<vec2> vertices;
	    	std::vector<vec2> UVs;		

		    UVs.push_back(vec2(1.0f, 0.0f));
		    UVs.push_back(vec2(1.0f, 1.0f));
		    UVs.push_back(vec2(0.0f, 1.0f));		

		    UVs.push_back(vec2(0.0f, 0.0f));
		    UVs.push_back(vec2(1.0f, 0.0f));
		    UVs.push_back(vec2(0.0f, 1.0f));	

		    vertices.push_back(vec2(xpos + w, ypos + h));
		    vertices.push_back(vec2(xpos + w, ypos));
		    vertices.push_back(vec2(xpos, ypos));		

		    vertices.push_back(vec2(xpos, ypos + h));
		    vertices.push_back(vec2(xpos + w, ypos + h));
		    vertices.push_back(vec2(xpos, ypos));

	        glBindBuffer(GL_ARRAY_BUFFER, FontVertexBuffer);
			glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(vec2), &vertices[0], GL_STATIC_DRAW);
			
			glBindBuffer(GL_ARRAY_BUFFER, FontUVBuffer);
			glBufferData(GL_ARRAY_BUFFER, UVs.size()*sizeof(vec2), &UVs[0], GL_STATIC_DRAW);					

			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, FontVertexBuffer);
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);		

			glEnableVertexAttribArray(1);
			glBindBuffer(GL_ARRAY_BUFFER, FontUVBuffer);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);		

			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);		

			glDrawArrays(GL_TRIANGLES, 0, vertices.size());		

			glDisable(GL_BLEND);		

			glDisableVertexAttribArray(0);
			glDisableVertexAttribArray(1);
	        
	        x += (ch.Advance >> 6) * size;
	    }

	    return end;
	}

	GLuint LoadShaders(const char* vertex_file_path, const char* fragment_file_path){

   	 	std::string Line;	

	    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);	

	    //Read vertex code
	    std::string VertexShaderCode;
	    std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	    Line = "";
	    while(getline(VertexShaderStream, Line))
	        VertexShaderCode += "\n" + Line;
	    VertexShaderStream.close();	

	    //Read fragment code
	    std::string FragmentShaderCode;
	    std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	    Line = "";
	    while(getline(FragmentShaderStream, Line))
	        FragmentShaderCode += "\n" + Line;
	    FragmentShaderStream.close();	

	    GLint Result = GL_FALSE;
	    int InfoLogLength;	

	    //Comipile vertex 
	    char const* VertexSourcePointer = VertexShaderCode.c_str();
	    glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
	    glCompileShader(VertexShaderID);	

	    //Check vertex
	    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	    if ( InfoLogLength > 0 ){
	        std::vector<char> VertexShaderErrorMessage(InfoLogLength+1);
	        glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
	        printf("%s\n", &VertexShaderErrorMessage[0]);
	    }	

	    //Compile fragment
	    char const* FragmentSourcePointer = FragmentShaderCode.c_str();
	    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
	    glCompileShader(FragmentShaderID);	

	    //Check fragment
	    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	    if ( InfoLogLength > 0 ){
	        std::vector<char> FragmentShaderErrorMessage(InfoLogLength+1);
	        glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
	        printf("%s\n", &FragmentShaderErrorMessage[0]);
	    }	

	    //Link program
	    GLuint ProgramID = glCreateProgram();
	    glAttachShader(ProgramID, VertexShaderID);
	    glAttachShader(ProgramID, FragmentShaderID);
	    glLinkProgram(ProgramID);	

	    // Check program
	    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	    if ( InfoLogLength > 0 ){
	        std::vector<char> ProgramErrorMessage(InfoLogLength+1);
	        glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
	        printf("%s\n", &ProgramErrorMessage[0]);
	    }	

	    glDetachShader(ProgramID, VertexShaderID);
	    glDetachShader(ProgramID, FragmentShaderID);	

	    glDeleteShader(VertexShaderID);
	    glDeleteShader(FragmentShaderID);	

	    return ProgramID;
	}

	GLuint loadBMP(const char* imagepath){

		int width, height, comp;

		unsigned char* data = stbi_load(imagepath, &width, &height, &comp, STBI_rgb);

		GLuint textureID;
	    glGenTextures(1, &textureID);
	    
	    glBindTexture(GL_TEXTURE_2D, textureID);	

	    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);	

	    delete [] data;	

	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); 
	    glGenerateMipmap(GL_TEXTURE_2D);	

	    return textureID;
	}

	void loadOBJ(
		const char* path,
		std::vector<vec3> &out_vertices,
		std::vector<vec2> &out_uvs)
	{

		std::vector<unsigned int> vertexIndices, uvIndices;
		std::vector<vec3> temp_vertices;
		std::vector<vec2> temp_uvs;

		FILE* file = fopen(path, "r");	

		while(1){
			char lineHeader[128];	

			int res = fscanf(file, "%s", lineHeader);
			if(res == EOF) break;	

			if(!strcmp(lineHeader, "v")){
				vec3 vertex;
				fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
				temp_vertices.push_back(vertex);
			}else if(!strcmp(lineHeader, "vt")){
				vec2 uv;
				fscanf(file, "%f %f\n", &uv.x, &uv.y);
				temp_uvs.push_back(uv);
			}else if(!strcmp(lineHeader, "f")){
				unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
				fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d", 
					&vertexIndex[0], &uvIndex[0], &normalIndex[0], 
					&vertexIndex[1], &uvIndex[1], &normalIndex[1], 
					&vertexIndex[2], &uvIndex[2], &normalIndex[2]);	

				vertexIndices.push_back(vertexIndex[0]);
				vertexIndices.push_back(vertexIndex[1]);
				vertexIndices.push_back(vertexIndex[2]);	

				uvIndices.push_back(uvIndex[0]);
				uvIndices.push_back(uvIndex[1]);
				uvIndices.push_back(uvIndex[2]);	
			}
		}

		for(unsigned int i = 0; i < vertexIndices.size(); i++){
			unsigned int vertexIndex = vertexIndices[i];
			unsigned int uvIndex = uvIndices[i];

			vec3 vertex = temp_vertices[vertexIndex - 1];
			vec2 uv = temp_uvs[uvIndex - 1];

			out_vertices.push_back(vertex);
			out_uvs.push_back(uv);
		}	

		fclose(file);
	}
};

DrawImage drawer;

void quit(){
	input_event ev;
	char* s = new char [200];
	
	drawer.drawYesNo();

	sprintf(s, "Would you like to quit the game? (Y/N)");
	drawer.printText(s, 235, 383, 1);

	drawer.refresh();

	while(1){
		read(fd, &ev, sizeof(ev));
		if(ev.value){
			switch(ev.code){
				case KEY_Y: system("stty echo"); exit(0); return;
				case KEY_N: return;
				default: continue;
			}
		}
	}	

	delete [] s;
}

int sign(int x){
	return (x > 0) - (x < 0);
}

class Inventory{
	node<stats> *list, *last;
	int amount;
public:
	Inventory(){
		list = new node<stats>;
		list->next = NULL;
		last = list;
		amount = 0;
	}

	~Inventory(){
		node<stats>* tmp;
		while(list){
			tmp = list->next;
			delete list;
			list = tmp;
		}
	}

	void addItem(stats &st){
		last->value = st;
		last->next = new node<stats>;
		last = last->next;
		last->next = NULL;
		amount++;
	}

	void rmvItem(int id){
		node<stats>* tmp1 = list, *tmp2;
		if(tmp1->value.id == id){
			tmp1 = tmp1->next;
			delete list;
			list = tmp1;
		}else{
			while(tmp1->value.id != id){
				tmp2 = tmp1;
				tmp1 = tmp1->next;
			}
			tmp2->next = tmp1->next;
			delete tmp1;
		}
		tmp1 = list;
		while(tmp1->next) tmp1 = tmp1->next;
		last = tmp1;
		amount--;
	}

	node<stats> *getList(){
		return list;
	}

	int getAmount(){
		return amount;
	}

	void callInter(Player &player, Entities &ents);

	void setItem(int n, Player &player);

	void delItem(int n, Player &player);
};

class Chest: public Inventory{
	int x, y, id;
	static int counter;
public:
	Chest(int x1 = 0, int y1 = 0){
		x = x1;
		y = y1;
		id = counter++;
	}

	void randChest(int n){
		for(int i = 0; i < n; i++){
			stats st(names.names[rand()%names.amount], "weapon", rand()%31, rand()%101-70);
			addItem(st);

			if(rand()%2){
				stats st("MedKit", "heal", 10);
				addItem(st); 
			}
		}
	}

	int getId(){
		return id;
	}

	int getX(){
		return x;
	}

	int getY(){
		return y;
	}

	void callInter(Player &player, Entities &ents);
};

class ListOfChests{
	node<Chest*> *list, *last;
public:
	ListOfChests(){
		list = new node<Chest*>;
		list->next = NULL;
		last = list;
	}

	~ListOfChests(){
		node<Chest*> *tmp;
		while(list){
			tmp = list->next;
			delete list;
			list = tmp;
		}
	}

	void addChest(Chest* chest){
		last->value = chest;
		last->next = new node<Chest*>;
		last = last->next;
		last->next = NULL;
	}

	Chest* findByXY(int x, int y){
		node<Chest*> *tmp = list;
		while(tmp->next){
			if(tmp->value->getX() == x && tmp->value->getY() == y)
				return tmp->value;
			tmp = tmp->next;
		}
	}

	void formFromFile(const char* fileName){
		FILE* file;
		int x, y, n;

		file = fopen(fileName, "r");
		fscanf(file, "%d", &n);

		for(int i = 0; i < n; i++){
			fscanf(file, "%d%d", &x, &y);
			addChest(new Chest(x, y));
		}

		fclose(file);
	}

	void randList(){
		node<Chest*> *tmp = list;
		while(tmp->next){
			tmp->value->randChest(rand()%5 + 1);
			tmp = tmp->next;
		}
	}

	node<Chest*> *getList(){
		return list;
	}
};

class MessageQ{
	node<char*> *list, *last;
	int n;
public:
	MessageQ(){
		n = 0;
		list = new node<char*>;
		list->next = NULL;
		last = list;
	}

	~MessageQ(){
		node<char*> *tmp = list;
		while(list->next){
			tmp = list;
			list = list->next;
			delete tmp;
		}
	}

	void addMes(const char* mes){
		n++;
		last->value = new char [strlen(mes) + 1];
		strcpy(last->value, mes);
		last->next = new node<char*>;
		last = last->next;
		last->next = NULL;
		if(n > 6){
			node<char*> *tmp;
			tmp = list;
			list = list->next;
			delete tmp;
			n--;
		}
	}

	void writeMes(){
		char* s = new char [300];
		node<char*> *tmp = list;

		int x = 1024.0f/(FOVX + shift);
		int y = (shift + 1.7f)*768.0f/(FOVY + shift);

		if(tmp->next){
			while(tmp->next){
				sprintf(s, "%s\n", tmp->value);
				drawer.printText(s, x, y, 1);
				y -= AdvanceY;
				tmp = tmp->next;
			}
		}

		delete [] s;
	}
};

MessageQ messageQ;

class Creature{
protected:
	static int counter;
	int x, y, id, health, maxHealth;
	int baseDmg, baseAccuracy, type;
	bool deadFlag;
public:

	Creature(int x, int y, int dmg, int accuracy, int hp, int type){
		deadFlag = false;
		baseDmg = dmg;
		baseAccuracy = accuracy;
		maxHealth = health = hp;
		id = counter++;
		this->x = x;
		this->y = y;
		this->type = type;
	}

	friend Creature* operator+(Creature &ob1, Creature &ob2);

	virtual void doTurn(Map &map, Entities &ents, ListOfChests &chests) = 0;

	virtual void writeMSG(int n, int dmg = 0) = 0;
	
	virtual bool death(Entities &ents) = 0;
	
	bool getHurt(int dmg, Entities &ents, Map &map);

	int move(int dx, int dy, Map &map, Entities &ents);

	bool ifDead(){
		return deadFlag;
	}

	void makeDead(){
		deadFlag = true;
	}

	int getBaseDmg(){
		return baseDmg;
	}

	int getBaseAccuracy(){
		return baseAccuracy;
	}

	int getHealth(){
		return health;
	}

	int getMaxHealth(){
		return maxHealth;
	}

	int getX(){
		return x;
	}

	int getY(){
		return y;
	}

	int getType(){
		return type;
	}

	int getId(){
		return id;
	}
};

class Player: public Creature{
	int curLevel, curPoints, maxPoints;
	stats st;
	Creature* enemyAttacked;
public:

	Inventory invertory;

	Player(int x, int y):Creature(x, y, 5, 70, 100, 2), st("Hands", "weapon", 0, 0){
		curLevel = 1;
		curPoints = 0;
		maxPoints = 100;

		enemyAttacked = NULL;
	}

	void doTurn(Map &map, Entities &ents, ListOfChests &chests);

	bool openMode(Map &map, ListOfChests &chests, Entities &ents);

	void open(Map &map, ListOfChests &chests, int x, int y, Entities &ents);

	bool attackMode(Map &map, Entities &ents);

	void attack(Map &map, Entities &ents, int x, int y);

	void writeMSG(int n, int dmg = 0){
		switch(n){
			case 0: messageQ.addMes("You missed"); break;
			case 1: char* s = new char [200]; 
					sprintf(s, "You've "ANSI_COLOR_RED"recieved %d"ANSI_COLOR_RESET" damage", dmg);
					messageQ.addMes(s);
					delete [] s; 
					break;
		}
	}

	bool death(Entities &ents){
		if(deadFlag){
			while(1) quit();
			return true;
		}
		return false;
	}

	stats &getSt(){
		return st;
	}

	void writeStats(){
		drawer.drawInterface();
		int x = (FOVX + 0.2f)*1024.0f/(FOVX + 2*shift);
		int y = (FOVY + shift)*768.0f/(FOVY + 2*shift);
		char* s = new char [300];
		sprintf(s, "Health:\n"      ANSI_COLOR_RED"%d/%d\n\n"ANSI_COLOR_RESET
				"Weapon:\n"        ANSI_COLOR_CYAN"%s\n\n"ANSI_COLOR_RESET
				"DMG:\n"           ANSI_COLOR_CYAN"%d\n\n"ANSI_COLOR_RESET
				"Accuracy:\n"      ANSI_COLOR_CYAN"%d%%\n\n"ANSI_COLOR_RESET
				"Current\nlevel:\n"ANSI_COLOR_CYAN"%d\n\n"ANSI_COLOR_RESET
				"Exp:\n"        ANSI_COLOR_CYAN"%d/%d\n\n"ANSI_COLOR_RESET,
				 health, maxHealth, st.name, baseDmg + st.dmg, baseAccuracy + st.accuracy, curLevel, curPoints, maxPoints);
		drawer.printText(s, x, y, 1);

		if(enemyAttacked){
			sprintf(s, "Enemy:\n"ANSI_COLOR_MAGENTA"%d/%d"ANSI_COLOR_RESET, enemyAttacked->getHealth(), enemyAttacked->getMaxHealth());
			drawer.printText(s, 512, (shift + 1.7f)*768.0f/(FOVY + shift), 1);
		}

		delete [] s;
	}

	void addExp(int exp){
		curPoints += exp;
		if(curPoints >= maxPoints){
			curPoints = 0;
			curLevel++;
			baseDmg += 2;
			baseAccuracy += 1;
			maxPoints += 100;
		}
	}

	friend void Inventory::delItem(int n, Player &player);

	void setItem(stats &st1){
		if(!strcmp(st1.type, "weapon"))
			st = st1;
		else
			health = (health + st1.health > maxHealth? maxHealth: health + st1.health);
	}
};

class Enemy: public Creature{
public:
	Enemy(int x, int y, int d, int a, int h, int t):Creature(x, y, d, a, h, t){}

	virtual void doTurn(Map &map, Entities &ents, ListOfChests &chests) = 0;

	void attack(Map &map, Entities &ents, int x, int y);

	void writeMSG(int n, int dmg = 0){
		switch(n){
			case 0: messageQ.addMes("Enemy missed"); break;
			case 1: char* s = new char [200]; 
					sprintf(s, "enemy has "ANSI_COLOR_GREEN"recieved %d"ANSI_COLOR_RESET" damage", dmg);
					messageQ.addMes(s);
					delete [] s; 
					break;
		}
	}

	bool death(Entities &ents);
};

class MeleeEnemy: public Enemy{
public:

	MeleeEnemy(int x, int y):Enemy(x, y, 5, 70, 30, 3){}

	void doTurn(Map &map, Entities &ents, ListOfChests &chests);
};

class Sayan: public Enemy{
public:

	Sayan(int x, int y):Enemy(x, y, 5, 70, 30, 7){}

	Sayan(int x, int y, int dmg, int health):Enemy(x, y, dmg, 70, health, 7){}

	void doTurn(Map &map, Entities &ents, ListOfChests &chests);
};

class Turret: public Enemy{
	int reload;
public:

	Turret(int x, int y):Enemy(x, y, 10, 100, 30, 4){
		reload = 0;
	}

	void doTurn(Map &map, Entities &ents, ListOfChests &chests);
};

class Projectile: public Enemy{
	int dirX, dirY;
public:

	Projectile(int x, int y, int dX, int dY):Enemy(x, y, 10, 100, 30, 6){
		dirX = dX;
		dirY = dY;
	}

	void doTurn(Map &map, Entities &ents, ListOfChests &chests);
};

class Entities{
	node<Creature*> *list, *last;
public:
	int amount;

	Entities(){
		amount = 0;
		list = new node<Creature*>;
		list->next = NULL;
		last = list;
	}

	~Entities(){
		node<Creature*> *tmp;
		while(list){
			tmp = list->next;
			delete list;
			list = tmp;
		}
	}

	void addEnt(Creature* ent){
		amount++;
		last->value = ent;
		last->next = new node<Creature*>;
		last = last->next;
		last->next = NULL;
	}

	void rmvEnt(int id){
		amount--;
		node<Creature*> *tmp1 = list, *tmp2;
		if(list->value->getId() == id){
			tmp1 = tmp1->next;
			delete list;
			list = tmp1;
		}else{
			while(tmp1->value->getId() != id){
				tmp2 = tmp1;
				tmp1 = tmp1->next;
			}
			tmp2->next = tmp1->next;
			delete tmp1;
		}
		tmp1 = list;
		while(tmp1->next) tmp1 = tmp1->next;
		last = tmp1;
	}

	Player& formFromFile(const char* fileName){
		FILE* file;
		Player *pl;
		Creature* ob;
		int x, y, type, n;

		file = fopen(fileName, "r");
		fscanf(file, "%d", &n);

		for(int i = 0; i < n; i++){
			fscanf(file, "%d%d%d", &x, &y, &type);
			switch(type){
				case 2: pl = new Player(x, y); ob = pl; break;
				case 3: ob = new MeleeEnemy(x, y); break;
				case 4: ob = new Turret(x, y); break;
				case 7: ob = new Sayan(x, y); break;
			}
			addEnt(ob);
		}

		fclose(file);

		return *pl;
	}

	Creature* findByXY(int x, int y){
		node<Creature*> *tmp = list;
		while(tmp->next){
			if(tmp->value->getX() == x && tmp->value->getY() == y)
				return tmp->value;
			tmp = tmp->next;
		}
		return NULL;
	}

	node<Creature*> *getList(){
		return list;
	}
};

class Map{
	int **layer1, n, m;
public:
	Map(const char *fileName){
		FILE *file = fopen(fileName, "r");
		fscanf(file, "%d", &n);
		fscanf(file, "%d", &m);
		layer1 = new int* [n];
		for(int i = 0; i < n; i++){
			layer1[i] = new int [m];
			for(int j = 0; j < m; j++)
				fscanf(file, "%d", &(layer1[i][j]));
		}
		fclose(file);
	}

	~Map(){
		for(int i = 0; i < n; i++)
			delete [] layer1[i];
		delete [] layer1;
	}

	int getL1Pos(int x, int y){
		if(x < 0 || x > m - 1 || y < 0 || y > n - 1)
			return -1;
		else
			return layer1[y][x];
	}

	void drawMap(int x, int y, Entities &ents){
		int k;
		node<Creature*> *tmp = ents.getList();
		
		for(int i = y - FOVY/2 + 1; i <= y + FOVY/2; i++)
			for(int j = x - FOVX/2; j < x + FOVX/2; j++){
				if(i < 0 || i > n - 1 || j < 0 || j > m - 1)
					k = -1;
				else
					k = layer1[i][j];
				drawer.drawXY(k, j - x, i - y);
			}

		while(tmp->next){
			int tmpX = tmp->value->getX();
			int tmpY = tmp->value->getY();
			if(abs(x - tmpX) < FOVX/2 && abs(y - tmpY) < FOVY/2)
				drawer.drawXY(tmp->value->getType(), tmpX - x, tmpY - y);
			tmp = tmp->next;
		}
	}

	void synchChests(ListOfChests &list){
		node<Chest*> *tmp = list.getList();
		while(tmp->next){
			layer1[tmp->value->getY()][tmp->value->getX()] = 5;
			tmp = tmp->next;
		}
	}
};

int Creature::counter = 0;
int stats::counter = 0;
int Chest::counter = 0;

Map map("maps/layer1");

int main(void){

	srand(time(0));

	fd = open("/dev/input/by-path/platform-i8042-serio-0-event-kbd", O_RDONLY);

	node<Creature*> *tmp;
	Entities ents;

	Player &player = ents.formFromFile("maps/layer2");

	names.setPool("rand/randNames");

	ListOfChests chests;
	chests.formFromFile("maps/layer3");
	chests.randList();

	map.synchChests(chests);

	while(1){
		tmp = ents.getList();
		while(tmp->next->next){
			if(!(tmp->next->value->death(ents)))
				tmp = tmp->next;
		}
		tmp = ents.getList();
		if(tmp->value->death(ents))
			tmp = ents.getList();
		
		drawer.cleanup();
		map.drawMap(player.getX(), player.getY(), ents);
		player.writeStats();
		messageQ.writeMes();
		drawer.refresh();
		player.doTurn(map, ents, chests);

		while(tmp->next){
			if(tmp->value->getType() != 2 && !(tmp->value->ifDead()))
				tmp->value->doTurn(map, ents, chests);
			tmp = tmp->next;
		}
	}

	return 0;
}

void Chest::callInter(Player &player, Entities &ents){
	input_event ev;
	node<stats> *tmp, *chosenTmp;
	int n = 0, m = 0, flag = 0;
	int n1 = 0, n2 = 0;
	int amountCh;
	int amountInv;

	bool spaceFlag = false;

	while(1){
		if(!flag){

			char* s = new char [200];
			int end1 = 0, end2 = 0;
			int first, last;

			drawer.cleanup();
			map.drawMap(player.getX(), player.getY(), ents);
			player.writeStats();
			messageQ.writeMes();
			drawer.drawChest();

			drawer.printText("INVENTORY", 1024/4 - (AdvanceX >> 6)*4.5f, 543, 1);
			drawer.printText("    NAME", 0, 493, 1);
			drawer.printText("                       TYPE", 0, 493, 1);

			drawer.printText("CHEST", 3*1024/4 - (AdvanceX >> 6)*2.5f, 543, 1);
			drawer.printText("    NAME", 1024/2, 493, 1);
			drawer.printText("                       TYPE", 1024/2, 493, 1);

			drawer.printText("Base DMG:", 870, 120, 1);
			sprintf(s, ANSI_COLOR_CYAN"%d"ANSI_COLOR_RESET, player.getBaseDmg());
			drawer.printText(s, 870 + (AdvanceX >> 6)*3.5f, 120 - AdvanceY, 1);

			drawer.printText("Base", 870 + (AdvanceX >> 6)*2, 120 - 2*AdvanceY, 1);
			drawer.printText("Accuracy:", 870, 120 - 3*AdvanceY, 1);
			sprintf(s, ANSI_COLOR_CYAN"%d%%"ANSI_COLOR_RESET, player.getBaseAccuracy());
			drawer.printText(s, 870 + (AdvanceX >> 6)*2.5f, 120 - 4*AdvanceY, 1);

			amountCh = getAmount();
			amountInv = player.invertory.getAmount();
			tmp = player.invertory.getList();
			if(!(m?amountCh:amountInv)) m = (m + 1)%2;

			if(n1 < 2){
				first = 0;
				last = 5;
			} else if(n1 > amountInv - 3){
				first = amountInv - 5;
				if(first < 0) first = 0;
				last = amountInv;
			}else{
				first = n1 - 2;
				last = n1 + 3;
			}

			for(int i = 0; i < first; i++)
				tmp = tmp->next;

			for(int i = first; i < last && tmp->next; i++){
				if(n == i & !m){ 
					chosenTmp = tmp;
					drawer.printText(ANSI_COLOR_YELLOW, 0, 0, 0);
				}

				int lineX = end1;	

				sprintf(s, "%d)", i + 1);
				drawer.printText(s, 10, 493 - AdvanceY*(lineX + i + 1 - first), 1);	

				int shift = strlen(s);
				sprintf(s, "%s", tmp->value.name);
				end1 += drawer.printText(s, 10 + (shift + 1)*(AdvanceX >> 6), 493 - AdvanceY*(lineX + i + 1 - first), 1);							
				
				drawer.printText(ANSI_COLOR_CYAN, 0, 0, 0);
				drawer.printText(tmp->value.type, 10 + 23*(AdvanceX >> 6), 493 - AdvanceY*(lineX + i + 1 - first), 1);
				drawer.printText(ANSI_COLOR_RESET, 0, 0, 0);	

				if(n == i && !m) drawer.printText(ANSI_COLOR_RESET, 0, 0, 0);
				tmp = tmp->next;
			}

			tmp = getList();

			if(n2 < 2){
				first = 0;
				last = 5;
			} else if(n2 > amountCh - 3){
				first = amountCh - 5;
				if(first < 0) first = 0;
				last = amountCh;
			}else{
				first = n2 - 2;
				last = n2 + 3;
			}

			for(int i = 0; i < first; i++)
				tmp = tmp->next;

			for(int i = first; i < last && tmp->next; i++){					
				if(n == i & m){
					chosenTmp = tmp;
					drawer.printText(ANSI_COLOR_YELLOW, 0, 0, 0);
				}
				int lineX = end2;	

				sprintf(s, "%d)", i + 1);
				drawer.printText(s, 1024/2 + 10, 493 - AdvanceY*(lineX + i + 1 - first), 1);	

				int shift = strlen(s);
				sprintf(s, "%s", tmp->value.name);
				end2 += drawer.printText(s, 1024/2 + 10 + (shift + 1)*(AdvanceX >> 6), 493 - AdvanceY*(lineX + i + 1 - first), 1);							
				
				drawer.printText(ANSI_COLOR_CYAN, 0, 0, 0);
				drawer.printText(tmp->value.type, 1024/2 + 10 + 23*(AdvanceX >> 6), 493 - AdvanceY*(lineX + i + 1 - first), 1);	
				drawer.printText(ANSI_COLOR_RESET, 0, 0, 0);

				if(n == i && m) drawer.printText(ANSI_COLOR_RESET, 0, 0, 0);
				tmp = tmp->next;
			}

			if(spaceFlag) chosenTmp->value.showStats(m?'l':'r');

			delete [] s;

			drawer.refresh();
		}

		flag = 0;
		read(fd, &ev, sizeof(ev));
		if(ev.value){
			switch(ev.code){
				case KEY_ENTER: if(!m){
									stats ob("Hands", "weapon", 0, 0);
									tmp = player.invertory.getList();
									for(int i = 0; i < n; i++) tmp = tmp->next;
									addItem(tmp->value);
									if(player.getSt().id == tmp->value.id);
										player.setItem(ob);
									player.invertory.rmvItem(tmp->value.id);
								} else{
									tmp = getList();
									for(int i = 0; i < n; i++) tmp = tmp->next;
									player.invertory.addItem(tmp->value);
									rmvItem(tmp->value.id);
								}
								if(n != 0) n--;
								break;
				case KEY_UP: n = (n + (m?amountCh:amountInv) - 1)%(m?amountCh:amountInv); (m?n2 = n:n1 = n); break;
				case KEY_DOWN: n = (n + 1)%(m?amountCh:amountInv); (m?n2 = n:n1 = n); break;
				case KEY_LEFT:
				case KEY_RIGHT: m = (m + 1)%2;
								if(!(m?amountCh:amountInv)) break;
								if(n%(m?amountCh:amountInv) != n)
									n = (m?amountCh:amountInv) - 1;
								else
									n %= (m?amountCh:amountInv);
								(m?n2 = n:n1 = n);
								break;
				case KEY_SPACE: if(m?amountCh:amountInv) spaceFlag = !spaceFlag; break;
				case KEY_S:
				case KEY_Q: return;
				default: flag++;
			}
		} else flag++;
	}
}

void Inventory::setItem(int n, Player &player){
	node<stats> *tmp = list; 
	input_event ev;
	char*s = new char [200];
	bool flag = false;
	
	for(int i = 0; i < n; i++) tmp = tmp->next;

	drawer.drawYesNo();

	if(!strcmp(tmp->value.type, "weapon")){

		sprintf(s, "Would you like to set");
		drawer.printText(s, 355, 418, 1);	

		sprintf(s, "the selected weapon as active? (Y/N)");
		drawer.printText(s, 258, 418 - AdvanceY, 1);
	}else{

		flag = true;

		sprintf(s, "Would you like to use");
		drawer.printText(s, 355, 418, 1);	

		sprintf(s, "the MedKit? (Y/N)");
		drawer.printText(s, 380, 418 - AdvanceY, 1);
	}

	drawer.refresh();

	while(1){
		read(fd, &ev, sizeof(ev));
		if(ev.value){
			switch(ev.code){
				case KEY_Y: player.setItem(tmp->value); if(flag) rmvItem(tmp->value.id); return;
				case KEY_N: return;
				default: continue;
			}
		}
	}

	delete [] s;
}

void Inventory::delItem(int n, Player &player){
	node<stats> *tmp = list; 
	input_event ev;
	char* s= new char [200];
	stats ob("Hands", "weapon", 0 ,0);
	
	for(int i = 0; i < n; i++) tmp = tmp->next;
	
	drawer.drawYesNo();

	sprintf(s, "Would you like to remove");
	drawer.printText(s, 355, 418, 1);

	sprintf(s, "this item from your inventory? (Y/N)");
	drawer.printText(s, 258, 363, 1);

	drawer.refresh();

	while(1){
		read(fd, &ev, sizeof(ev));
		if(ev.value){
			switch(ev.code){
				case KEY_Y: 
					if(tmp->value.id == player.st.id)
						player.setItem(ob);
					rmvItem(tmp->value.id);
					return;
				case KEY_N: return;
				default: continue;
			}
		}
	}	

	delete [] s;
}

void Inventory::callInter(Player &player, Entities &ents){
	input_event ev;
	node<stats>* tmp, *chosenTmp;

	bool spaceFlag = false;
	
	int n = 0, flag = 0;
	
	int first = 0;
	int last = 5;

	while(1){
		if(!flag){
			char* s = new char [300];
			int end = 0;

			if(n < 2){
				first = 0;
				last = 5;
			} else if(n > amount - 3){
				first = amount - 5;
				if(first < 0) first = 0;
				last = amount;
			}else{
				first = n - 2;
				last = n + 3;
			}

			tmp = list;
			for(int i = 0; i < first; i++)
				tmp = tmp->next;
			
			drawer.cleanup();
			map.drawMap(player.getX(), player.getY(), ents);
			player.writeStats();
			messageQ.writeMes();
			drawer.drawInventory();

			drawer.printText("INVENTORY", 440, 543, 1);
			drawer.printText("    NAME", 271, 493, 1);
			drawer.printText("                       TYPE", 271, 493, 1);

			drawer.printText("Base DMG:", 870, 120, 1);
			sprintf(s, ANSI_COLOR_CYAN"%d"ANSI_COLOR_RESET, player.getBaseDmg());
			drawer.printText(s, 870 + (AdvanceX >> 6)*3.5f, 120 - AdvanceY, 1);

			drawer.printText("Base", 870 + (AdvanceX >> 6)*2, 120 - 2*AdvanceY, 1);
			drawer.printText("Accuracy:", 870, 120 - 3*AdvanceY, 1);
			sprintf(s, ANSI_COLOR_CYAN"%d%%"ANSI_COLOR_RESET, player.getBaseAccuracy());
			drawer.printText(s, 870 + (AdvanceX >> 6)*2.5f, 120 - 4*AdvanceY, 1);

			for(int i = first; i < last && tmp->next; i++){
				if(n == i) {
					chosenTmp = tmp;
					drawer.printText(ANSI_COLOR_YELLOW, 0, 0, 0);
				}
				int lineX = end;

				sprintf(s, "%d)", i + 1);
				drawer.printText(s, 271, 493 - AdvanceY*(lineX + i + 1 - first), 1);

				int shift = strlen(s);
				end += drawer.printText(tmp->value.name, 271 + (shift + 1)*(AdvanceX >> 6), 493 - AdvanceY*(lineX + i + 1 - first), 1);							
				
				drawer.printText(ANSI_COLOR_CYAN, 0, 0, 0);

				drawer.printText(tmp->value.type, 271 + 23*(AdvanceX >> 6), 493 - AdvanceY*(lineX + i + 1 - first), 1);

				drawer.printText(ANSI_COLOR_RESET, 0, 0, 0);
				tmp = tmp->next;
			}

			if(spaceFlag && amount) chosenTmp->value.showStats();

			drawer.refresh();

			delete [] s;
		}

		flag = 0;
		read(fd, &ev, sizeof(ev));
		if(ev.value){
			switch(ev.code){
				case KEY_UP: if(amount) n = (n + amount - 1)%amount; break;
				case KEY_DOWN: if(amount) n = (n + 1)%amount; break;
				case KEY_ENTER: if(amount) setItem(n, player); break;
				case KEY_SPACE: if(amount) spaceFlag = !spaceFlag; break;
				case KEY_BACKSPACE: if(amount) delItem(n, player); if(n) n--; break;
				case KEY_I:
				case KEY_Q: return;
				default: flag++;
			}
		} else flag++;
	}
}

int Creature::move(int dx, int dy, Map &map, Entities &ents){
	if(!ents.findByXY(x + dx, y + dy) && !map.getL1Pos(x + dx, y + dy)){
		x += dx;
		y += dy;
		return 0;
	}
	return 1;
}

bool Creature::getHurt(int dmg, Entities &ents, Map &map){
	writeMSG(1, dmg);
	health -= dmg;
	if(health < 1){
		makeDead();
		return true;
	}
	return false;
}

void Player::doTurn(Map &map, Entities &ents, ListOfChests &chests){
	input_event ev;

	while(1){
		read(fd, &ev, sizeof(ev));
		if(ev.value){
			switch(ev.code){
				case KEY_UP: move(0, -1, map, ents); return;
				case KEY_DOWN: move(0, 1, map, ents); return;
				case KEY_LEFT: move(-1, 0, map, ents); return;
				case KEY_RIGHT: move(1, 0, map, ents); return;
				case KEY_A: if(attackMode(map, ents)) return; else continue;
				case KEY_I: invertory.callInter(*this, ents);
							drawer.cleanup(); 
							map.drawMap(getX(), getY(), ents);
							writeStats();
							messageQ.writeMes();
							drawer.refresh();
							continue;
				case KEY_S: if(openMode(map, chests, ents)) return; else continue;
				case KEY_SPACE: return;
				case KEY_Q: quit(); 
							drawer.cleanup();
							map.drawMap(getX(), getY(), ents);
							writeStats();
							messageQ.writeMes();
							drawer.refresh();
							continue;
				default: continue;
			}
		}
	}
}

bool Player::openMode(Map &map, ListOfChests &chests, Entities &ents){
	input_event ev;

	drawer.printText("Open mode", 870, 120, 1);
	drawer.printText("is", 873 + (AdvanceX >> 6)*3.5f, 120 - 2*AdvanceY, 1);
	drawer.printText(ANSI_COLOR_GREEN"on"ANSI_COLOR_RESET, 873 + (AdvanceX >> 6)*3.5f, 120 - 4*AdvanceY, 1);
	drawer.refresh();

	while(1){
		read(fd, &ev, sizeof(ev));
		if(ev.value){
			switch(ev.code){
				case KEY_UP: open(map, chests, x, y - 1, ents); return true;
				case KEY_DOWN: open(map, chests, x, y + 1, ents); return true;
				case KEY_LEFT: open(map, chests, x - 1, y, ents); return true;
				case KEY_RIGHT: open(map, chests, x + 1, y, ents); return true;
				case KEY_S: drawer.cleanup();
							map.drawMap(getX(), getY(), ents);
							writeStats();
							messageQ.writeMes();
							drawer.refresh();
							return false;
				default: continue;
			}
		}
	}	
}

void Player::open(Map &map, ListOfChests &chests, int x, int y, Entities &ents){
	if(map.getL1Pos(x, y) == 5)
		(chests.findByXY(x, y))->callInter(*this, ents);
}

bool Player::attackMode(Map &map, Entities &ents){
	input_event ev;

	drawer.printText("Attack mode", 852, 120, 1);
	drawer.printText("is", 852 + (AdvanceX >> 6)*4.5f, 120 - 2*AdvanceY, 1);
	drawer.printText(ANSI_COLOR_GREEN"on"ANSI_COLOR_RESET, 852 + (AdvanceX >> 6)*4.5f, 120 - 4*AdvanceY, 1);
	drawer.refresh();

	while(1){
		read(fd, &ev, sizeof(ev));
		if(ev.value){
			switch(ev.code){
				case KEY_UP: attack(map, ents, x, y - 1); return true;
				case KEY_DOWN: attack(map, ents, x, y + 1); return true;
				case KEY_LEFT: attack(map, ents, x - 1, y); return true;
				case KEY_RIGHT: attack(map, ents, x + 1, y); return true;
				case KEY_A: drawer.cleanup();
							map.drawMap(getX(), getY(), ents);
							writeStats();
							messageQ.writeMes();
							drawer.refresh();
							return false; 
				default: continue;
			}
		}
	}	
}

void Player::attack(Map &map, Entities &ents, int x, int y){	
	int b = 0;
	Creature* tmp;

	if(rand()%100 < baseAccuracy + st.accuracy) b = 1; 
	
	if((tmp = ents.findByXY(x, y)) && tmp->getType() != 6){
		enemyAttacked = tmp;
		if(!b)
			writeMSG(0);
		else
			if(tmp->getHurt(baseDmg + st.dmg, ents, map)){
				enemyAttacked = NULL;
				addExp(10);
			}
	}

	return;
}

void Enemy::attack(Map &map, Entities &ents, int x, int y){	
	int b = 0;
	Creature* tmp;

	if(rand()%100 < baseAccuracy) b = 1; 
	
	if(tmp = ents.findByXY(x, y))
		if(b)
			tmp->getHurt(baseDmg, ents, map); 
		else
			writeMSG(0);

	return;
}

bool Enemy::death(Entities &ents){
		if(deadFlag){
			messageQ.addMes("Enemy has fallen");
			ents.rmvEnt(id);
			return true;
		}
		return false;
	}

void MeleeEnemy::doTurn(Map &map, Entities &ents, ListOfChests &chests){
	Creature* tmp;
	for(int i = y - FOVE/2; i < y + FOVE/2; i++)
		for(int j = x - FOVE/2; j < x + FOVE/2; j++){
			if((tmp = ents.findByXY(j, i)) && tmp->getType() == 2){
				if(abs(j-x) == 1 && abs(i-y) == 0 || abs(j-x) == 0 && abs(i-y) == 1)
					attack(map, ents, j, i);
				else
					if(j - x && !move(sign(j-x), 0, map, ents));
					else 
						if(i-y) move(0, sign(i-y), map, ents);				
				return;
			}
		}
}

void Sayan::doTurn(Map &map, Entities &ents, ListOfChests &chests){
	Creature* tmp;
	bool findFlag = false;
	int X, Y;
	for(int i = y - FOVE/2; i < y + FOVE/2; i++)
		for(int j = x - FOVE/2; j < x + FOVE/2; j++){
			if(tmp = ents.findByXY(j, i)){
				if(tmp->getType() == 2){
					findFlag = true;
					Y = i;
					X = j;
				}
				if((tmp->getType() == 7) 
				&& (abs(j-x) + abs(i-y) == 1)
				&& !(tmp->ifDead())){
					drawer.cleanup();
					drawer.drawWholeScreen(8);
					drawer.refresh();
					sleep(1);
					ents.addEnt(*this + *tmp);
					makeDead();
					tmp->makeDead();
					return;
				}
			}
		}
	if(!findFlag) return;
	if(abs(X-x) + abs(Y-y) == 1) 
		attack(map, ents, X, Y);
	else if(X - x && !move(sign(X-x), 0, map, ents));
	else if(Y - y && !move(0, sign(Y-y), map, ents));				
}

void Turret::doTurn(Map &map, Entities &ents, ListOfChests &chests){
	Creature *tmp;
	int a = 0, b = 0;

	reload -= (!reload? 0: 1);
	if(reload) return;

	for(int i = -FOVT/2; i < FOVT/2; i++){
		if((tmp = ents.findByXY(x + i, y)) && tmp->getType() == 2) a = 1;
		else if((tmp = ents.findByXY(x, y + i)) && tmp->getType() == 2) b = 1;
		if(a || b){
			ents.addEnt(new Projectile(x, y, sign(i)*a, sign(i)*b));
			reload = 3;
			return;
		}
	}
}

void Projectile::doTurn(Map &map, Entities &ents, ListOfChests &chests){
	Creature* tmp;

	if(!(tmp = ents.findByXY(x + dirX, y + dirY)) && !map.getL1Pos(x + dirX, y + dirY)){
		x += dirX;
		y += dirY;
	} else{ 
		if(tmp)
			tmp->getHurt(baseDmg, ents, map);
		
		makeDead();
	}
}

void stats::showStats(const char mode){
	input_event ev;
	char* s = new char [200];

	drawer.drawStats(mode);

	if(!strcmp(type, "weapon"))
		sprintf(s, "Damage:\n"ANSI_COLOR_CYAN"%d\n\n"ANSI_COLOR_RESET
			   	   "Accuracy\n"ANSI_COLOR_CYAN"%d%%"ANSI_COLOR_RESET, dmg, accuracy);
	else
		sprintf(s, "HP+\n"ANSI_COLOR_RED"%d"ANSI_COLOR_RESET, health);
	
	if(mode)
		if(mode == 'l')
			drawer.printText(s, 1024/8 + 10, 635, 1);
		else
			drawer.printText(s, 5*1024/8 + 10, 635, 1);
	else
		drawer.printText(s, 10, 635, 1);

	delete [] s;
}

Creature* operator+(Creature &ob1, Creature &ob2){
	return new Sayan(ob1.x, ob1.y, ob1.baseDmg + ob2.baseDmg, ob1.health + ob2.health);
}