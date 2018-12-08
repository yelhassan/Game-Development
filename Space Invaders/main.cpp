#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>

#include "ShaderProgram.h"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"

#define STB_IMAGE_IMPLEMENTATION //required for stb image library
#include "stb_image.h"

#ifdef _WINDOWS
#define RESOURCE_FOLDER
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

#include <vector>
#include <unistd.h>
#include <cstdlib>
#include <ctime>


SDL_Window* displayWindow;

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

void DrawText(ShaderProgram &program, int fontTexture, std::string text, float size, float spacing) {
    float character_size = 1.0/16.0f;
    std::vector<float> vertexData;
    std::vector<float> texCoordData;
    for(int i=0; i < text.size(); i++) {
        int spriteIndex = (int)text[i];
        float texture_x = (float)(spriteIndex % 16) / 16.0f;
        float texture_y = (float)(spriteIndex / 16) / 16.0f;
        vertexData.insert(vertexData.end(), {
            ((size+spacing) * i) + (-0.5f * size), 0.5f * size,
            ((size+spacing) * i) + (-0.5f * size), -0.5f * size,
            ((size+spacing) * i) + (0.5f * size), 0.5f * size,
            ((size+spacing) * i) + (0.5f * size), -0.5f * size,
            ((size+spacing) * i) + (0.5f * size), 0.5f * size,
            ((size+spacing) * i) + (-0.5f * size), -0.5f * size,
        });
        texCoordData.insert(texCoordData.end(), {
            texture_x, texture_y,
            texture_x, texture_y + character_size,
            texture_x + character_size, texture_y, //
            texture_x + character_size, texture_y + character_size,
            texture_x + character_size, texture_y,
            texture_x, texture_y + character_size,
        }); }
    glBindTexture(GL_TEXTURE_2D, fontTexture);
    
    glUseProgram(program.programID);
    
    glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
    glEnableVertexAttribArray(program.texCoordAttribute);
    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
    glEnableVertexAttribArray(program.positionAttribute);
    glDrawArrays(GL_TRIANGLES, 0, (int) text.size()*6);
}

class Entity {
public:
    
    Entity();
    void DrawSprite(ShaderProgram &program, int index, int spriteCountX,int spriteCountY);
    
    
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 size;
    
    int xPos;
    int yPos;
    
    float health;
    float rotation;
    
    bool collision = false;
};


Entity::Entity () {
    
}


void Entity::DrawSprite(ShaderProgram &program, int index, int spriteCountX,int spriteCountY) {
    
    float u = (float)(((int)index) % spriteCountX) / (float) spriteCountX;
    float v = (float)(((int)index) / spriteCountY) / (float) spriteCountY;
    float spriteWidth = 1.0/(float)spriteCountX;
    float spriteHeight = 1.0/(float)spriteCountY;
    
    float texCoords[] = {
        u, v+spriteHeight,
        u+spriteWidth, v,
        u, v,
        u+spriteWidth, v,
        u, v+spriteHeight,
        u+spriteWidth, v+spriteHeight};
    
    float vertices[] = {-0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, 0.5f,  -0.5f,-0.5f, 0.5f, -0.5f};
    
    glUseProgram(program.programID);
    
    glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
    glEnableVertexAttribArray(program.texCoordAttribute);
    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program.positionAttribute);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
}

//class GameState {
//public:
//    Entity player;
//    Entity enemies[11];
//    Entity bullets[30];
//    int score;
//};


class MainMenu {
    
    //text
public:
    Entity welcome;
};

/////////////////////////////////////////

int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
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
    ShaderProgram texturedProgram;
    program.Load(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");
    texturedProgram.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
    
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    glm::mat4 viewMatrix = glm::mat4(1.0f);
    glm::mat4 projectionMatrix = glm::mat4(1.0f);
    projectionMatrix = glm::ortho(-1.777f, 1.777f, -1.0f, 1.0f, -1.0f, 1.0f);
    
    //background color
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    
    //time keeping
    float ticks;
    float elapsedTime;
    float lastFrameTicks = 0;
    
    GLuint LoadTexture(const char *filePath);
    int textTexture = LoadTexture(RESOURCE_FOLDER"textsheet.png");
    int trumpTexture = LoadTexture(RESOURCE_FOLDER"trump2.png");
    int twitterTexture = LoadTexture(RESOURCE_FOLDER"twitterlogo.png");
    // prep screens
    MainMenu mainMenu;
    
    enum GameMode { STATE_MAIN_MENU, STATE_GAME_LEVEL};
    GameMode mode;
    mode = STATE_MAIN_MENU;
    
    //initialize player at center
    Entity player;
    player.xPos = 0.0f;
    player.yPos = -4.0f;
    
    //create an array of enemies
    std::vector<Entity> enemies;
    for(int i = 0; i < 10; i++){
        Entity enemy;
        enemies.push_back(enemy);
    }
    
    //prep bullets/flowers
#define MAX_BULLETS 10
    int bulletIndex = 0;
    Entity bullets[MAX_BULLETS];
    for(int i=0; i < MAX_BULLETS; i++) {
//        Entity bullet;
//        bullets[i] = bullet;
        
        bullets[i].xPos = player.xPos;
    }
    
//    Entity enemyShots[10];
//    for(int i= 0; i < 10; i++){
//        Entity enemyAttack;
//        enemyShots[i] = enemyAttack
//    }
//
    bool triggerPulled = false;
    float enemyShotX = 0;
    float enemyShotY = 0;

    
    
    //////////////
    SDL_Event event;
    bool done = false;
    while (!done) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                done = true;
            } else if(event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
               // if(event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
                    // DO AN ACTION WHEN SPACE IS PRESSED!
                    triggerPulled = true;
                //}
            }
        }
        
        const Uint8 *keys = SDL_GetKeyboardState(NULL);
       
        ticks = (float)SDL_GetTicks()/1000.0f;
        elapsedTime = ticks - lastFrameTicks;
        lastFrameTicks = ticks;
        
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(texturedProgram.programID);
        
        texturedProgram.SetProjectionMatrix(projectionMatrix);
        texturedProgram.SetViewMatrix(viewMatrix);
        
        if( mode == STATE_MAIN_MENU) {
            
            modelMatrix = glm::mat4(1.0f);
            modelMatrix = glm::translate(modelMatrix, glm::vec3(-1.5f,0.0f,0.0f));
          
            texturedProgram.SetModelMatrix(modelMatrix);
            texturedProgram.SetProjectionMatrix(projectionMatrix);
            texturedProgram.SetViewMatrix(viewMatrix);
            
            DrawText(texturedProgram, textTexture, "Trump (the) Invader", 0.15, 0);
            
            modelMatrix = glm::mat4(1.0f);
            modelMatrix = glm::translate(modelMatrix, glm::vec3(-1.5f,-0.2f,0.0f));
            texturedProgram.SetModelMatrix(modelMatrix);

            DrawText(texturedProgram, textTexture, "Press enter to start", 0.1, 0);
            
            if(keys[SDL_SCANCODE_RETURN])
            
            {
                mode = STATE_GAME_LEVEL;
            }
        }
        
        if (mode == STATE_GAME_LEVEL) {

        glBindTexture(GL_TEXTURE_2D, trumpTexture);

        //draw grid of enemies
            // really this should be a 2D array
        for(int i = 0; i < enemies.size(); i++){
            if ( i > 4) { enemies[i].yPos = 1.5; }
            if ( i > 0 ) { enemies[i].xPos = enemies[i - 1].xPos + 2.0; }
            if ( i == 5) { enemies[i].xPos = 0;}
            
            modelMatrix = glm::mat4(1.0f);
            modelMatrix = glm::scale(modelMatrix, glm::vec3(0.2f, 0.2f, 1.0f));
            modelMatrix = glm::translate(modelMatrix, glm::vec3(-4.25 + enemies[i].xPos, 1.5 + enemies[i].yPos, 0.0f));
            texturedProgram.SetModelMatrix(modelMatrix);
            
            if (enemies[i].collision == false) {
                enemies[i].DrawSprite(texturedProgram, 1, 6, 4);
            }
        }
        
            
        srand((unsigned)time(0));
        int randomNumber =  (rand()%10)+1;
            std::cout << randomNumber << "\n";
        for(int i = 0; i < enemies.size(); i++){
            if ( i == randomNumber && enemies[i].collision == false) {
                program.SetColor( 1.0f, 1.0f, 1.0f, 1.0f);
                
                enemyShotX = enemies[i].xPos;
                enemyShotY -= 0.55f;

                glBindTexture(GL_TEXTURE_2D, twitterTexture);
                
                modelMatrix = glm::mat4(1.0f);
                modelMatrix = glm::scale(modelMatrix, glm::vec3(0.04f, 0.04f, 1.0f));
                modelMatrix = glm::translate(modelMatrix, glm::vec3(enemyShotX,enemyShotY, 0.0f));
                
                texturedProgram.SetModelMatrix(modelMatrix);

                
                float vertices1[] = {-0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5};
                glVertexAttribPointer(texturedProgram.positionAttribute, 2, GL_FLOAT, false, 0, vertices1);
                glEnableVertexAttribArray(texturedProgram.positionAttribute);
                
                float texCoords1[] = {0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0};
                glVertexAttribPointer(texturedProgram.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords1);
                glEnableVertexAttribArray(texturedProgram.texCoordAttribute);
                
                glDrawArrays(GL_TRIANGLES, 0, 6);
            }
        
        }

        glBindTexture(GL_TEXTURE_2D, trumpTexture);

        //move player
        if(keys[SDL_SCANCODE_LEFT]) {
            if(player.xPos > -7.7f) {
                    player.xPos -= 1.0f;
               // player.xPos -= elapsedTime * 5;

                }
            //player.xPos -= elapsedTime * 1.5;
        }
        else if(keys[SDL_SCANCODE_RIGHT]) {
            if(player.xPos < 7.7f) {
            player.xPos += 1.0f;
                //player.xPos += elapsedTime * 1.5;
            }
        }
            
        modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.2f, 0.2f, 1.0f));
        modelMatrix = glm::translate(modelMatrix, glm::vec3(player.xPos, player.yPos, 0.0f));

        texturedProgram.SetModelMatrix(modelMatrix);
        texturedProgram.SetViewMatrix(viewMatrix);
        texturedProgram.SetProjectionMatrix(projectionMatrix);
            
        //modelMatrix = glm::translate(modelMatrix, glm::vec3(player.xPos, player.yPos, 0.0f));
        player.DrawSprite(texturedProgram, 4, 6, 4);
        
    
       // shoot bullets
        if(triggerPulled == true) {

            bullets[bulletIndex].xPos = player.xPos;
            bullets[bulletIndex].yPos = - 23.0f;
            bulletIndex++;
            if(bulletIndex > MAX_BULLETS-1) {
                bulletIndex = 0;
            }
        }
            
        program.SetColor( 1.0f, 0.0f, 0.0f, 1.0f);

        for(int i=0; i < MAX_BULLETS; i++) {
           
            bullets[i].xPos = player.xPos;
            bullets[i].yPos += 1.0f;

            modelMatrix = glm::mat4(1.0f);
            modelMatrix = glm::scale(modelMatrix, glm::vec3(0.03f, 0.03, 1.0f));
            modelMatrix = glm::translate(modelMatrix, glm::vec3(bullets[i].xPos, bullets[i].yPos, 1.0f));
        
            program.SetModelMatrix(modelMatrix);
            program.SetProjectionMatrix(projectionMatrix);
            program.SetViewMatrix(viewMatrix);

            bullets[i].DrawSprite(program, 0, 6, 4);

            
            for (int i = 0; i < enemies.size(); i++){
                
                float  distanceX = abs(enemies[i].xPos - bullets[i].xPos) - ((0.05 + 0.2)/4);
                float  distanceY = abs(enemies[i].yPos - bullets[i].yPos) - ((0.05 + 0.2)/4);

                if(distanceX < 0 && distanceY < 0){
                    enemies[i].collision = true;
                }
            }
            }
            
        }
            triggerPulled = false;
  
        ///////
        glDisableVertexAttribArray(texturedProgram.positionAttribute);
        glDisableVertexAttribArray(texturedProgram.texCoordAttribute);
        glDisableVertexAttribArray(program.positionAttribute);
        
        
        SDL_GL_SwapWindow(displayWindow);
    }
    
    SDL_Quit();
    return 0;
}


