#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>

#include "ShaderProgram.h"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"


#define STB_IMAGE_IMPLEMENTATION //always include this before including stb image library
#include "stb_image.h"


#ifdef _WINDOWS
#define RESOURCE_FOLDER
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

SDL_Window* displayWindow;

GLuint LoadTexture(const char *filePath);

int main(int argc, char *argv[])
{
    
    
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360
                                     , SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
    
#ifdef _WINDOWS
    glewInit();
#endif

//SETUP

    glViewport(0, 0, 640, 360);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    ShaderProgram program;
    ShaderProgram untexturedProgram;
    untexturedProgram.Load(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");
    program.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
    
    GLuint catTexture = LoadTexture(RESOURCE_FOLDER"nyancat.png");
    GLuint spaceTexture = LoadTexture(RESOURCE_FOLDER"space.png");
    GLuint blackCatTexture = LoadTexture(RESOURCE_FOLDER"blacknyancat.png");
    
    glm::mat4 projectionMatrix = glm::mat4(1.0f);
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    glm::mat4 viewMatrix = glm::mat4(1.0f);
    
    projectionMatrix = glm::ortho(-1.777f, 1.777f, -1.0f, 1.0f, -1.0f, 1.0f);
    

    //background color
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    //program.SetColor(0.2f, 0.8f, 0.4f, 1.0f);
    
    float lastFrameTicks = 0.0f;
    float posX= 0.0f;
    float posX2 = 0.0f;

    
//GAME LOOP
    SDL_Event event;
    bool done = false;
    while (!done) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                done = true;
            }
        }
        
        //KEEP TIME -- ANIMATE
        float ticks = (float)SDL_GetTicks()/1000.0f;
        float elapsed = (ticks - lastFrameTicks) * 1.5;
        lastFrameTicks = ticks;
        
    
        glClear(GL_COLOR_BUFFER_BIT);
        
     
    
//SPACE BACKGROUND

        glUseProgram(program.programID);
        
        modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::scale(modelMatrix, glm::vec3(5.0f, 5.0f, 1.0f));

        program.SetModelMatrix(modelMatrix);
        program.SetProjectionMatrix(projectionMatrix);
        program.SetViewMatrix(viewMatrix);
        
        
        glBindTexture(GL_TEXTURE_2D, spaceTexture);

        
        float vertices[] = {-0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5};
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(program.positionAttribute);


        float texCoords[] = {0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0};
        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
        glEnableVertexAttribArray(program.texCoordAttribute);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
        

//TRIANGLE OVERLAY
        
        
        glUseProgram(untexturedProgram.programID);
        
        modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::scale(modelMatrix, glm::vec3(3.0f, 2.5f, 1.0f));
        untexturedProgram.SetColor(255.0f,255.0f,255.0f,0.1f);
        
        untexturedProgram.SetModelMatrix(modelMatrix);
        untexturedProgram.SetProjectionMatrix(projectionMatrix);
        untexturedProgram.SetViewMatrix(viewMatrix);

        
        //float vsertices3[] = {-0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5};
        float vertices3[] = {0.5f, -0.5f, 0.0f, 0.5f, -0.5f, -0.5f};
        glVertexAttribPointer(untexturedProgram.positionAttribute, 2, GL_FLOAT, false, 0, vertices3);
        glEnableVertexAttribArray(untexturedProgram.positionAttribute);

        glDrawArrays(GL_TRIANGLES, 0, 3);
        

//NYAN CAT
        
        glUseProgram(program.programID);
        
        modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::scale(modelMatrix, glm::vec3(2.0f, 1.0f, 0.0f));
        modelMatrix = glm::translate(modelMatrix, glm::vec3(-3, -0.5f, 0.0f));
        modelMatrix = glm::translate(modelMatrix, glm::vec3(posX, 0.0f, 0.0f));
        program.SetModelMatrix(modelMatrix);
        
        posX += elapsed - 0.0005;
        if (posX > 4.44) { posX = 0;}
        
        glBindTexture(GL_TEXTURE_2D, catTexture);
        
        float vertices1[] = {-0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5};
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices1);
        glEnableVertexAttribArray(program.positionAttribute);
        
        float texCoords1[] = {0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0};
        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords1);
        glEnableVertexAttribArray(program.texCoordAttribute);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
//BLACK NYAN CAT
        
        modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::scale(modelMatrix, glm::vec3(1.7f, 1.5f, 0.0f));
        modelMatrix = glm::translate(modelMatrix, glm::vec3(3, 0.3f, 0.0f));
        modelMatrix = glm::translate(modelMatrix, glm::vec3(-posX, 0.0f, 0.0f));
        program.SetModelMatrix(modelMatrix);
        
        posX2 += elapsed - 0.0005;
        if (posX2 > 4.44) { posX2 = 0;}
        
        glBindTexture(GL_TEXTURE_2D, blackCatTexture);
        
        float vertices2[] = {-0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5};
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices2);
        glEnableVertexAttribArray(program.positionAttribute);
        
        float texCoords2[] = {0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0};
        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords2);
        glEnableVertexAttribArray(program.texCoordAttribute);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        
     //CLEANUP
        glDisableVertexAttribArray(untexturedProgram.positionAttribute);
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);

        SDL_GL_SwapWindow(displayWindow);
    }
    
    SDL_Quit();
    return 0;
}


// texture function, returns textureID
GLuint LoadTexture(const char *filePath) {
    
    int w,h,comp;
    
    unsigned char* image = stbi_load(filePath, &w, &h, &comp, STBI_rgb_alpha);
    
    if(image == NULL) { //check if loaded
        std::cout << "Unable to load image. Make sure the path is correct\n";
        assert(false); // triggers an exception; no point in continuing program
    }
    
    GLuint retTexture;
    glGenTextures(1, &retTexture);
    glBindTexture(GL_TEXTURE_2D, retTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    stbi_image_free(image);
    return retTexture;
}
