//tilesheet

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

#include "FlareMap.h"



SDL_Window* displayWindow;

// linear interpolation (curve fitting); value changes smoothly
float lerp(float v0, float v1, float t) {
    return (1.0-t)*v0 + t*v1;
}

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

class SheetSprite {
public:
    SheetSprite();
    SheetSprite(unsigned int ID, float U, float V, float Width, float Height, float Size);
    
    void Draw(ShaderProgram &program);
    
    float size;
    unsigned int textureID;
    float u;
    float v;
    float width;
    float height;
};


SheetSprite::SheetSprite(){
    
}

SheetSprite::SheetSprite(unsigned int ID, float U, float V, float Width, float Height, float Size) {
    
    textureID = ID;
    u = U;
    v = V;
    width = Width;
    height = Height;
    size = Size;
}

void SheetSprite::Draw(ShaderProgram &program) {
    glBindTexture(GL_TEXTURE_2D, textureID);
    GLfloat texCoords[] = {
        u, v+height,
        u+width, v,
        u, v,
        u+width, v,
        u, v+height,
        u+width, v+height
    };
    float aspect = width / height;
    float vertices[] = {
        -0.5f * size * aspect, -0.5f * size,
        0.5f * size * aspect, 0.5f * size,
        -0.5f * size * aspect, 0.5f * size,
        0.5f * size * aspect, 0.5f * size,
        -0.5f * size * aspect, -0.5f * size ,
        0.5f * size * aspect, -0.5f * size};
    
    glUseProgram(program.programID);
    
    glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
    glEnableVertexAttribArray(program.texCoordAttribute);
    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program.positionAttribute);
    glDrawArrays(GL_TRIANGLES, 0, 6);
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


enum EntityType {ENTITY_PLAYER, ENTITY_ENEMY, ENTITY_COIN};

class Entity{
public:
    
    Entity();
    void DrawSprite(ShaderProgram &program, int index, int spriteCountX,int spriteCountY);

    
    float xPos;
    float yPos;
    float Vx;
    float Vy;
    float Ax;
    float Ay;
    
    float width;
    float height;
    
    float health;
    float rotation;
    
    bool collision = false;
    bool isStatic;
    
    EntityType entityType;
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

//gamestates
class MainMenu {
    
    //text
public:
    Entity welcome;
};


void worldToTileCoordinates(float worldX, float worldY, int *gridX, int *gridY, float TILE_SIZE) {
    *gridX = (int)(worldX / TILE_SIZE);
    *gridY = (int)(worldY / -TILE_SIZE);
}


bool checkCollision(float x1, float y1, float w1, float h1, float x2, float y2, float w2, float h2) {
    
    float  distanceX = abs(x1 - x2) - ((w1 + w2)/2);
    float  distanceY = abs(y1 - y2) - ((h1 + h2)/2);
                                       
    if(distanceX < 0 && distanceY < 0) { return true; }
    else{ return false; }
 }

/******************************************************************************************/

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
    
    program.SetViewMatrix(viewMatrix);

    //background color
    glClearColor(0.06f, 0.596f, 0.675, 1.0f);
    
    //time keeping
    float ticks;
    float elapsedTime;
    float lastFrameTicks = 0;
    
    GLuint LoadTexture(const char *filePath);
    // prep screens
    MainMenu mainMenu;
    
    enum GameMode { STATE_MAIN_MENU, STATE_GAME_LEVEL};
    GameMode mode;
    mode = STATE_MAIN_MENU;
    
    int EntitySheetTexture = LoadTexture(RESOURCE_FOLDER"spritesheet.png");
    
    //initialize player at center
    Entity player;
    Entity platform;
    
    
    #define FIXED_TIMESTEP 0.0166666f
    #define MAX_TIMESTEPS 6
    float accumulator = 0.0f;


    float velocityX = 3.0f;
    float velocityY = 0.0f;
    float accelerationX = 2.0f;
    float accelerationY = 2.5f;
    float frictionX = 0.7f;
    float frictionY = 0.7f;
    float gravityY = - 20.0;
    
    
    float playerWidth;
    float playerHeight;
    float platformWidth;
    float platformHeight;
    
    #define LEVEL_HEIGHT 2
    #define LEVEL_WIDTH 22
    #define SPRITE_COUNT_X 16
    #define SPRITE_COUNT_Y 8
    
    playerWidth = 0.1f;
    playerHeight = 0.1f;
    platformWidth = 3.5f;
    platformHeight = 0.15;
    
    player.xPos = 0.5;
    player.yPos = - 1.5;
    
    float TILE_SIZE = 0.1;
    float numberOfBlocks = 0;
    
    float count = 0;
    float tilePenetrationLeft;
    float tilePenetrationRight;
    float tilePenetrationTop;

    bool hasCollidedwithTile = false;
    bool rightCollision = false;
    
    Entity key;
    key.xPos = 8.85f;
    key.yPos = -0.375f;
    float keyWidth = 0.1f;
    float keyHeight = 0.1f;
    
 

    FlareMap map;
    map.Load(RESOURCE_FOLDER"FinalMap.txt");
    
    
    float tilePenetration;
  
  
//Adding entities from tiled map; works but index is way off (?)
/*********************************
    std::vector<Entity> enemies;
    std::vector<Entity> coins;

    int enemyCount = 0;
    int coinCount = 0;
 
    for (int i = 0; i < map.entities.size(); i++){
        if (map.entities[i].type == "enemy"){
            enemyCount++;
            Entity enemy;
            enemy.xPos = map.entities[i].x * TILE_SIZE;
            enemy.yPos = map.entities[i].y * - TILE_SIZE;
            enemies.push_back(enemy);
            std::cout << map.entities[i].x << "\n";
            std::cout << map.entities[i].y << "\n";

        }
        else if(map.entities[i].type == "coin"){
            coinCount++;
            Entity coin;
            coin.xPos = map.entities[i].x * TILE_SIZE;
            coin.yPos = map.entities[i].y * - TILE_SIZE;
            coins.push_back(coin);
        }
    }

    std::cout << "coin count: " << coinCount << "\n";
    std::cout << "enemy count: " << enemyCount << "\n";

    for (int i = 0; i < map.entities.size(); i++){
        count++;
        if (map.entities[i].type == "enemy") {
            Entity enemy;
            enemy.xPos = map.entities[i].x;
            enemy.yPos = map.entities[i].y;


            modelMatrix = glm::mat4(1.0f);
            modelMatrix = glm::translate(modelMatrix, glm::vec3(enemy.xPos, enemy.yPos, 0.0f));
            //modelMatrix = glm::scale(modelMatrix, glm::vec3(TILE_SIZE, TILE_SIZE, 1.0f));
            texturedProgram.SetModelMatrix(modelMatrix);

            enemy.DrawSprite(texturedProgram, 1, 16, 8);
        }
    }
  std::cout << count;


            if (entity.type == "Coin") {
                Entity Coin;
                modelMatrix = glm::mat4(1.0f);
                Coin.DrawSprite(texturedProgram,tileIndex, 16, 6);
                texturedProgram.SetModelMatrix(modelMatrix);
    //        }
            //entity.DrawSprite(texturedProgram, tileIndex, 6, 4);
        }


******************/
 
    
    /************************************/
    SDL_Event event;
    bool done = false;
    while (!done) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                done = true;
            } else if(event.type == SDL_KEYDOWN) {
                if(event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
                    velocityY += 5.0f; //jump
                } }
        }
    
        
        const Uint8 *keys = SDL_GetKeyboardState(NULL);
                glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(program.programID);
        glUseProgram(texturedProgram.programID);
        
        ticks = (float)SDL_GetTicks()/1000.0f;
        elapsedTime = ticks - lastFrameTicks;
        lastFrameTicks = ticks;
        
        elapsedTime += accumulator;
        if(elapsedTime < FIXED_TIMESTEP) {
            accumulator = elapsedTime;
            continue;
        }
        while(elapsedTime >= FIXED_TIMESTEP) {
            //Update(FIXED_TIMESTEP);
            elapsedTime -= FIXED_TIMESTEP;
        }
        accumulator = elapsedTime;
        
        velocityY = lerp(velocityY, 0.0f, elapsedTime * frictionY);
        velocityY += accelerationY * elapsedTime;
        player.yPos += velocityY * elapsedTime;

        velocityY += gravityY * elapsedTime; //apply gravity -- constant acceleration

        /********
        //draw enemies
        for (int i = 0; i < enemies.size(); i++) {
            
            modelMatrix = glm::mat4(1.0f);
            modelMatrix = glm::translate(modelMatrix, glm::vec3(enemies[i].xPos,  enemies[i].yPos, 0.0f));
            modelMatrix = glm::scale(modelMatrix, glm::vec3(TILE_SIZE,TILE_SIZE,1.0f));
            texturedProgram.SetModelMatrix(modelMatrix);
            
            enemies[i].DrawSprite(texturedProgram, 81, 16, 8);
        }
        
        //draw coins
        for (int i = 0; i < coins.size(); i++) {
            
            modelMatrix = glm::mat4(1.0f);
            modelMatrix = glm::translate(modelMatrix, glm::vec3(coins[i].xPos,  coins[i].yPos, 0.0f));
            modelMatrix = glm::scale(modelMatrix, glm::vec3(TILE_SIZE,TILE_SIZE,1.0f));
            texturedProgram.SetModelMatrix(modelMatrix);
            
            coins[i].DrawSprite(texturedProgram, 51, 16, 8);
        }
    *******/
        
        modelMatrix = glm::mat4(1.0f);
       modelMatrix = glm::translate(modelMatrix, glm::vec3( -1.77f, 0.0f, 0.0f));
        texturedProgram.SetModelMatrix(modelMatrix);
        
        numberOfBlocks = 0;
        
        std::vector<float> vertexData;
        std::vector<float> texCoordData;

        for(int y=0; y < map.mapHeight; y++) {
            for(int x=0; x < map.mapWidth; x++) {
                
                if(map.mapData[y][x] != 0) {

                numberOfBlocks++;

                float u = (float)(((int)map.mapData[y][x]) % SPRITE_COUNT_X) / (float) SPRITE_COUNT_X;
                float v = (float)(((int)map.mapData[y][x]) / SPRITE_COUNT_X) / (float) SPRITE_COUNT_Y;
                float spriteWidth = 1.0f/(float)SPRITE_COUNT_X;
                float spriteHeight = 1.0f/(float)SPRITE_COUNT_Y;
    
                    vertexData.insert(vertexData.end(),{
                        TILE_SIZE * x, -TILE_SIZE * y,
                        TILE_SIZE * x, (-TILE_SIZE * y)-TILE_SIZE,
                        (TILE_SIZE * x)+TILE_SIZE, (-TILE_SIZE * y)-TILE_SIZE,
                        TILE_SIZE * x, -TILE_SIZE * y,
                        (TILE_SIZE * x)+TILE_SIZE, (-TILE_SIZE * y)-TILE_SIZE,
                        (TILE_SIZE * x)+TILE_SIZE, -TILE_SIZE * y });
                    
                    texCoordData.insert(texCoordData.end(), {
                        u, v,
                        u, v+(spriteHeight),
                        u+spriteWidth, v+(spriteHeight),
                        u, v,
                        u+spriteWidth, v+(spriteHeight),
                        u+spriteWidth, v});
                    
                    modelMatrix = glm::mat4(1.0f);
                    texturedProgram.SetModelMatrix(modelMatrix);
                    
                    glBindTexture(GL_TEXTURE_2D, EntitySheetTexture);
                    glUseProgram(texturedProgram.programID);
                    glVertexAttribPointer(texturedProgram.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
                    glEnableVertexAttribArray(texturedProgram.texCoordAttribute);
                    glVertexAttribPointer(texturedProgram.positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
                    glEnableVertexAttribArray(texturedProgram.positionAttribute);
                    
                    glDrawArrays(GL_TRIANGLES, 0, numberOfBlocks * 6 );
                }
            }
        }
        
   
        glBindTexture(GL_TEXTURE_2D, EntitySheetTexture);

    
        modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(player.xPos, player.yPos, 0.0f));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(playerWidth, playerHeight, 1.0f));
        texturedProgram.SetModelMatrix(modelMatrix);
        
        
    
        //track  player
        float playerBottomX = player.xPos;
        float playerBottomY = player.yPos - playerHeight/2;
        float playerTopX = player.xPos;
        float playerTopY = player.yPos + playerHeight/2;
        float playerLeftX = player.xPos - playerWidth/2;
        float playerLeftY = player.yPos ;
        float playerRightX = player.xPos + playerWidth/2;
        float playerRightY = player.yPos ;
        
        //convert player bottom coordinates to tiled coordinates
        float playerGridX = (int)(playerBottomX / TILE_SIZE);
        float playerGridY = (int)(playerBottomY/ - TILE_SIZE);
        
        float playerGridTopX = (int)(playerTopX / TILE_SIZE);
        float playerGridTopY = (int)(playerTopY/ - TILE_SIZE);
        
        float playerGridRightX = (int)(playerRightX / TILE_SIZE);
        float playerGridRightY = (int)(playerRightY/ - TILE_SIZE);
        
        float playerGridLeftX = (int)(playerLeftX / TILE_SIZE);
        float playerGridLeftY = (int)(playerLeftY/ - TILE_SIZE);
        
        //move player
        if(keys[SDL_SCANCODE_LEFT]) {
            if(player.xPos - playerWidth/2 > -1.777f + 1.777f/2 + 1.35f) {
                velocityX = lerp(velocityX, 0.0f, elapsedTime * frictionX);
                velocityX += accelerationX * elapsedTime;
                player.xPos -= velocityX * elapsedTime * 3.0;
                
            }
            
            
            
//            for(int y=0; y < map.mapHeight; y++) {
//                for(int x=0; x < map.mapWidth; x++) {
//
//                    if (map.mapData[y][x] != 0)
//                        if ( y == playerGridLeftY && x == playerGridLeftX) {
//                            rightCollision = true;
//                            tilePenetrationLeft = playerLefttX - ((TILE_SIZE * x) + TILE_SIZE);
//                            player.xPos -= tilePenetrationLeft + 0.005;
//                            velocityY = 0;
//                        }
//                }
//            }
        
            
        }
        else if(keys[SDL_SCANCODE_RIGHT]) {
            velocityX = lerp(velocityX, 0.0f, elapsedTime * frictionX);
            velocityX += accelerationX * elapsedTime;
            player.xPos += velocityX * elapsedTime * 3.0;
            
    

//            for(int y=0; y < map.mapHeight; y++) {
//                for(int x=0; x < map.mapWidth; x++) {
//
//                    if (map.mapData[y][x] != 0)
//                        if ( y == playerGridRightY && x == playerGridRightX) {
//                            rightCollision = true;
//                            tilePenetrationRight = playerRightX - (TILE_SIZE * x);
//                            player.xPos -= tilePenetrationRight + 0.005;
//                            velocityY = 0;
//                        }
//                }
//            }

            
            
        }
        if(keys[SDL_SCANCODE_SPACE]) {
            
            player.yPos += elapsedTime * 1.5;
        
        }
        

        
        //keep player on platform
        for(int y = 0; y < map.mapHeight; y++) {
            for(int x = 0; x < map.mapWidth; x++) {
                
            /* PLEASE READ

            The sprite indexes are completely off (not just by 1 cause of the differences in the start index for our function and tiles -- way off) 
            and I'm not sure why it's doing that but becasue I can't accuractly indicate which indices to look at for the collisions, they collision codes are
            interfering with the bottom collision and the player gets sent upwards and diagnonal with a left or right collsion -- the code works but the mechanism
            is off cause of the indices so I commented it out 

            Also please note that I managed to get entities off of tile map, see the commented code above but because the indices are off, it's way
            too many entities that are drawn with the wrong sprite so I opted to just manually placing one dynamic entity. It's called key cause I wanted it to be 
            the key sprite but genuinly couldn't get the index that points to it.
            */ 
                
                //                 if ( map.mapData[y][x] == 1  || map.mapData[y][x] == 2 || map.mapData[y][x] == 3 || map.mapData[y][x] == 16 || map.mapData[y][x] == 17 || map.mapData[y][x] == 18  ||   map.mapData[y][x] == 19 || map.mapData[y][x] == 36){
                
                if (map.mapData[y][x] != 0){
                    if ( y == playerGridY && x == playerGridX) {
                        // std::cout << "collision!\n";
                        
                        hasCollidedwithTile = true;
                        
                        tilePenetration = (-TILE_SIZE * y) - playerBottomY;
                        player.yPos += tilePenetration + 0.005 ;
                        velocityY = 0;
                    }
                }
            }
        }

        
        
        player.DrawSprite(texturedProgram, 115, 16, 8);
    
        
        modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(key.xPos, key.yPos, 0.0f));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(keyWidth, keyHeight, 1.0f));
        texturedProgram.SetModelMatrix(modelMatrix);
        key.DrawSprite(texturedProgram, 87, 16, 8);
        

         if(checkCollision(player.xPos, player.yPos, playerWidth, playerHeight, key.xPos, key.yPos, keyWidth, keyHeight)) {
             std::cout << "collision";
                key.xPos = -100.0f;
                }
        
        
        
        //scroll view w/ player
        viewMatrix = glm::mat4(1.0f);
    
        viewMatrix = glm::translate(viewMatrix, glm::vec3(-1 * (player.xPos + 1.777/2 + 0.65), -1 * (player.yPos + 0.65), 0.0f));
        program.SetViewMatrix(viewMatrix);
        
        
        /*******************************/
        texturedProgram.SetViewMatrix(viewMatrix);
        texturedProgram.SetProjectionMatrix(projectionMatrix);
        program.SetViewMatrix(viewMatrix);
        program.SetProjectionMatrix(projectionMatrix);
        
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(texturedProgram.positionAttribute);
        glDisableVertexAttribArray(texturedProgram.texCoordAttribute);
        SDL_GL_SwapWindow(displayWindow);
    }
    
    SDL_Quit();
    return 0;
}



