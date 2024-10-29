#include "glad/gl.h"
#include "glfw/include/GLFW/glfw3.h"
#include "linmath.h"
#include <stdlib.h>
#include <stdio.h>

using namespace std;

// Class for each rectangle object
class Rect
{
public:
    float x, y, vx, vy, width, height;
    int id;
    bool collided;
    Rect(int id, float x, float y, float width, float height, float velocityX, float velocityY)
    {
        this->x = x;
        this->y = y;
        this->id = id;
        this->vx = velocityX;
        this->vy = velocityY;
        this->width = width;
        this->height = height;
    }
};

// Main QuadTree class
class QuadTree
{
public:
    QuadTree(int level, Rect *bounds)
    {
        //+++ Initialize the QuadTree here
        this->level = level;
        this->bounds = bounds;
        nodes.reserve(4);
        objects.reserve(1024);
    }

    ~QuadTree()
    {
        Clear();
        delete bounds;
    }
    
    void Clear()
    {
        //+++ Clear the objects and nodes
        this->objects.clear();
        for (QuadTree* node : nodes) {
            if (node)
            {
                node->Clear();
            }
        }

    }
    
    void Insert(Rect *rect)
    {
        //+++ This code has to be written to insert a new Rect object into the tree
        if (!nodes.empty())
        {
            int index = GetIndex(rect);
            if (index != -1)
            {
                nodes[index]->Insert(rect);
                return;
            }
        }

        this->objects.emplace_back(rect);

        if (objects.size() > MAX_OBJECTS && level < MAX_LEVELS) {
            if (nodes.empty())
            {
                Split();
            }

            auto it = objects.begin();
            while (it != objects.end())
            {
                int index = GetIndex(*it);
                if (index != -1)
                {
                    nodes[index]->Insert(*it);
                    it = objects.erase(it);
                }
                else {
                    ++it;
                }
            }
        }
    }
    
    std::vector<int> *Retrieve(std::vector<int> *result, Rect *rect)
    {
        int index = GetIndex(rect);

        if (!nodes.empty() && index != -1)
        {
            nodes[index]->Retrieve(result, rect);
        }

        //+++ This code has to be written to retrieve all the rectangles
        //+++ that are in the same node in the quadtree as rect
        for (const Rect* obj : objects)
        {
            result->emplace_back(obj->id);
        }

        return result;
    }
    
private:
    static const int MAX_OBJECTS = 1;
    static const int MAX_LEVELS = 8;
    
    int level;
    Rect *bounds;
    std::vector<Rect*> objects;
    std::vector<QuadTree*> nodes;
    
    void Split()
    {
        //+++ This code has to be written to split a node
        QuadTree* topLeft, *topRight, *bottomLeft, *bottomRight;
        Rect* topLeftRect, *topRightRect, *bottomLeftRect, *bottomRightRect;

        
        topLeftRect = new Rect(bounds->id + 1, bounds->x, bounds->y, bounds->width / 2, bounds->height / 2, 0.0f, 0.0f);
        topLeft = new QuadTree(level + 1, topLeftRect);

        topRightRect = new Rect(bounds->id + 2, (bounds->x + bounds->width / 2), bounds->y, bounds->width / 2, bounds->height / 2, 0.0f, 0.0f);
        topRight = new QuadTree(level + 1, topRightRect);

        bottomLeftRect = new Rect(bounds->id + 3, bounds->x, (bounds->y + bounds->height / 2), bounds->width / 2, bounds->height / 2, 0.0f, 0.0f);
        bottomLeft = new QuadTree(level + 1, bottomLeftRect);

        bottomRightRect = new Rect(bounds->id + 4, (bounds->x + bounds->width / 2), (bounds->y + bounds->height / 2), bounds->width / 2, bounds->height / 2, 0.0f, 0.0f);
        bottomRight = new QuadTree(level + 1, bottomRightRect);


        nodes.emplace_back(topLeft);
        nodes.emplace_back(topRight);
        nodes.emplace_back(bottomLeft);
        nodes.emplace_back(bottomRight);
    }
    
    int GetIndex(Rect *rect)
    {
        // Calculate midpoints of both quadtree boundary and rectangle.
        double horizontalMidpoint = bounds->x + (bounds->width * 0.5);
        double verticalMidpoint = bounds->y + (bounds->height * 0.5);

        // Verify rect location in quadrant.
        // Returns:
        // - 0 for northwest
        // - 1 for northeast
        // - 2 for southwest
        // - 3 for southeast
        bool inLeft = (rect->x + rect->width < horizontalMidpoint);
        bool inRight = (rect->x + rect->width >= horizontalMidpoint);
        bool inTop = (rect->y + rect->height >= verticalMidpoint);
        bool inBottom = (rect->y < verticalMidpoint);

        if (inLeft == inRight || inTop == inBottom)
        {
            return -1;
        }

        return inRight + 2 * inBottom;

    }
};








///////////////////////////////////////////////////////
// Code to use QuadTree to display squares
///////////////////////////////////////////////////////

// OpenGL shader code
static const char* vertexShaderText =
"#version 110\n"
"uniform mat4 MVP;\n"
"uniform float vCol;\n"
"attribute vec2 vPos;\n"
"varying vec4 color;\n"
"void main()\n"
"{\n"
"    gl_Position = MVP * vec4(vPos, 0.0, 1.0);\n"
"   if (vCol > 0.0)\n"
"       color = vec4(1.0, 0.0, 0.0, 1.0);\n"
"   else\n"
"       color = vec4(1.0, 1.0, 1.0, 1.0);\n"
"}\n";

static const char* fragmentShaderText =
"#version 110\n"
"varying vec4 color;\n"
"void main()\n"
"{\n"
"    gl_FragColor = color;\n"
"}\n";


// Helper class
class QuadtreeTest
{
public:
    bool useQuadTree = true;      // Whether to use quadtree or brute force
    QuadtreeTest(int squareSize = 5.0f, int numSquares = 5000, int randSize = 10000,
                 int scrWidth = 1280, int scrHeight = 720, float speed = 1.0f);

    bool Initialize()
    {
        quad->Clear();
        for (int i = 0; i < numSquares; ++i)
        {
            float x = (rand() % randSIZE) / (float)randSIZE * (float)scrWIDTH;
            float y = (rand() % randSIZE) / (float)randSIZE * (float)scrHEIGHT;
            float vx = (rand() % randSIZE) / (float)randSIZE * 2*speed - speed;
            float vy = (rand() % randSIZE) / (float)randSIZE * 2*speed - speed;
            Rect* rect = new Rect(i, x, y, squareSIZE, squareSIZE, vx, vy);
            rects.emplace_back(rect);
            quad->Insert(rect);
        }

        glfwSetErrorCallback(ErrorCallback);

        if (!glfwInit())
            return false;

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
        theWindow = glfwCreateWindow(scrWIDTH, scrHEIGHT, "QuadTree demo", NULL, NULL);
        if (!theWindow)
        {
            glfwTerminate();
            return false;
        }

        glfwSetKeyCallback(theWindow, KeyCallback);
        glfwMakeContextCurrent(theWindow);
        gladLoadGL(glfwGetProcAddress);
        glfwSwapInterval(1);

        return InitializeGL();

        return true;
    }
    
    bool InitializeGL()
    {
        static const struct
        {
            float x, y;
        } vertices[] =
        {
            { -squareSIZE, -squareSIZE },
            { squareSIZE, -squareSIZE },
            { squareSIZE, squareSIZE },
            { -squareSIZE, -squareSIZE },
            { squareSIZE, squareSIZE },
            { -squareSIZE, squareSIZE },
        };

        vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderText, NULL);
        glCompileShader(vertexShader);
        GLint shaderCompiled = GL_FALSE;
        glGetShaderiv( vertexShader, GL_COMPILE_STATUS, &shaderCompiled );
        if( shaderCompiled != GL_TRUE )
        {
            printf( "Unable to compile vertex shader %d!\n", vertexShader );
            PrintShaderLog( vertexShader );
            return false;
        }

        fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderText, NULL);
        glCompileShader(fragmentShader);
        shaderCompiled = GL_FALSE;
        glGetShaderiv( fragmentShader, GL_COMPILE_STATUS, &shaderCompiled );
        if( shaderCompiled != GL_TRUE )
        {
            printf( "Unable to compile fragment shader %d!\n", fragmentShader );
            PrintShaderLog( fragmentShader );
            return false;
        }

        shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);
        GLint programSuccess = GL_TRUE;
        glGetProgramiv( shaderProgram, GL_LINK_STATUS, &programSuccess );
        if( programSuccess != GL_TRUE )
        {
            printf("Error linking program %d!\n", shaderProgram);
            PrintProgramLog(shaderProgram);
            return false;
        }

        mvpLocation = glGetUniformLocation(shaderProgram, "MVP");
        vposLocation = glGetAttribLocation(shaderProgram, "vPos");
        vcolLocation = glGetUniformLocation(shaderProgram, "vCol");

        glGenBuffers(1, &vertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices),
                     vertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(vposLocation);
        glVertexAttribPointer(vposLocation, 2, GL_FLOAT, GL_FALSE,
                              sizeof(vertices[0]), (void *)0);

        return true;
    }
    
    void Update(double elapsedTime)
    {
        quad->Clear();
        for (vector<Rect*>::iterator it = rects.begin(); it != rects.end(); ++it)
        {
            auto rectVal = *it;

            float x = rectVal->x;
            float y = rectVal->y;
            x += rectVal->vx;
            y += rectVal->vy;
            rectVal->x = x;
            rectVal->y = y;
            if (x < 0 || x > scrWIDTH)
                rectVal->vx *= -1;
            if (y < 0 || y > scrHEIGHT)
                rectVal->vy *= -1;
            rectVal->collided = false;
            quad->Insert(rectVal);
        }
        
        std::vector<int> closeBy;
        for (vector<Rect*>::iterator it = rects.begin(); it != rects.end(); ++it)
        {
            if (useQuadTree)
            {                
                //+++ Use the Retrieve() method of the quadtree
                //+++ and the IsCollided() method to detect if a
                //+++ collision has happened and set the collided
                //+++ flag of the rectangles that have collided
                //+++ as needed.
                closeBy.clear();
                auto rectVal = *it;
                quad->Retrieve(&closeBy, rectVal);

                for (int id : closeBy)
                {
                    if (rectVal->id == id) {
                        continue;
                    }

                    Rect* other = rects[id];
                    if (IsCollided(rectVal, other))
                    {
                        rectVal->collided = true;
                        other->collided = true;
                        break;
                    }
                }
            } else {
                for (vector<Rect*>::iterator it2 = rects.begin(); it2 != rects.end(); ++it2)
                {
                    if ((*it)->id == (*it2)->id)
                        continue;
                    if (IsCollided((*it), (*it2)))
                    {
                        (*it)->collided = true;
                        (*it2)->collided = true;
                        break;
                    }
                }
            }
        }
        cout << "FPS = " << 1.0f / elapsedTime << "         \r" << std::flush;
    }
    
    void Render()
    {
        static bool isFirst = true;
        int width, height;
        glfwGetFramebufferSize(theWindow, &width, &height);
        glViewport(0, 0, width, height);

        glUseProgram(shaderProgram);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        mat4x4 m, p, mvp;
        for (vector<Rect*>::iterator it = rects.begin(); it != rects.end(); ++it)
        {
            mat4x4_identity(m);
            mat4x4_translate_in_place(m, (*it)->x, (*it)->y, 0);
            mat4x4_ortho(p, 0.0, (float)scrWIDTH, 0.0, (float)scrHEIGHT, -1.0, 1.0);
            mat4x4_mul(mvp, p, m);
            glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, (const GLfloat *)mvp);
            glUniform1f(vcolLocation, (*it)->collided ? 1 : 0);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
        isFirst = false;
    }
    
    void Close()
    {
        glfwDestroyWindow(theWindow);
        glfwTerminate();
    }
    
    void Run()
    {
        if (!Initialize())
            exit(EXIT_FAILURE);

        double lastTime = glfwGetTime();
        double elapsedTime;
        while (!glfwWindowShouldClose(theWindow))
        {
            elapsedTime = glfwGetTime() - lastTime;
            lastTime = glfwGetTime();
            Update(elapsedTime);
            Render();
            glfwSwapBuffers(theWindow);
            glfwPollEvents();
        }

        Close();
    }

private:
    QuadTree *quad;
    vector<Rect *> rects;
    GLFWwindow *theWindow;
    GLuint vertexBuffer, vertexShader, fragmentShader, shaderProgram;
    GLint mvpLocation, vposLocation, vcolLocation;
    float squareSIZE = 5.0f;        // Size of squares
    int numSquares = 5000;        // Number of squares
    int randSIZE = 10000;         // Parameter for randomization of velocities of squares
    int scrWIDTH = 1280;          // Screen width
    int scrHEIGHT = 720;          // Screen height
    float speed = 1.0f;           // Speed multiplier
    

    static void ErrorCallback(int error, const char *description)
    {
        cerr << "Error: " << description << endl;
    }

    static void KeyCallback(GLFWwindow* theWindow, int key,
                            int scancode, int action, int mods)
    {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            glfwSetWindowShouldClose(theWindow, GLFW_TRUE);
    }
    
    
    void PrintProgramLog( GLuint program )
    {
        //Make sure name is shader
        if( glIsProgram( program ) )
        {
            //Program log length
            int infoLogLength = 0;
            int maxLength = infoLogLength;
            
            //Get info string length
            glGetProgramiv( program, GL_INFO_LOG_LENGTH, &maxLength );
            
            //Allocate string
            char* infoLog = new char[ maxLength ];
            
            //Get info log
            glGetProgramInfoLog( program, maxLength, &infoLogLength, infoLog );
            if( infoLogLength > 0 )
            {
                //Print Log
                printf( "%s\n", infoLog );
            }
            
            //Deallocate string
            delete[] infoLog;
        }
        else
        {
            printf( "Name %d is not a program\n", program );
        }
    }
    
    void PrintShaderLog( GLuint shader )
    {
        //Make sure name is shader
        if( glIsShader( shader ) )
        {
            //Shader log length
            int infoLogLength = 0;
            int maxLength = infoLogLength;
            
            //Get info string length
            glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &maxLength );
            
            //Allocate string
            char* infoLog = new char[ maxLength ];
            
            //Get info log
            glGetShaderInfoLog( shader, maxLength, &infoLogLength, infoLog );
            if( infoLogLength > 0 )
            {
                //Print Log
                printf( "%s\n", infoLog );
            }
            
            //Deallocate string
            delete[] infoLog;
        }
        else
        {
            printf( "Name %d is not a shader\n", shader );
        }
    }

    bool IsCollided(Rect* r1, Rect* r2)
    {
        ////+++  Implement this function to test if rectangles r1 and r2 have collided
        //float d1x = (r2->x) - (r1->x + r1->width);
        //float d1y = (r2->y) - (r1->y + r1->height);
        //float d2x = (r1->x) - (r2->x + r2->width);
        //float d2y = (r1->y) - (r2->y + r2->height);

        //if (d1x > 0.0f || d1y > 0.0f) {
        //    return false;
        //}
        //if (d2x > 0.0f || d2y > 0.0f) {
        //    return false;
        //}

        //return true;
        
        // Optimization
        float r1x = r1->x;
        float r1y = r1->y;
        float r2x = r2->x;
        float r2y = r2->y;

		return !(r2x >= r1x + r1->width ||
		 r2y >= r1y + r1->height ||
		 r1x >= r2x + r2->width ||
		 r1y >= r2y + r2->height);
    }

};

QuadtreeTest::QuadtreeTest(int squareSize, int numSquares, int randSize,
                           int scrWidth, int scrHeight, float speed)
{
    squareSIZE = squareSize;
    this->numSquares = numSquares;
    randSIZE = randSize;
    scrWIDTH = scrWidth;
    scrHEIGHT = scrHeight;
    this->speed = speed;
    quad = new QuadTree(0, new Rect(-1, 0.0f, 0.0f, (float)scrWIDTH, (float)scrHEIGHT, 0.0f, 0.0f));
}
