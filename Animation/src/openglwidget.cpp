#include <stdio.h>
#include <stdlib.h>

#include "openglwidget.h"

#define GLM_FORCE_RADIANS
#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <QDebug>
#include <iostream>
#include <vector>
#include <QGLContext>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define NUM_POINTS 100

#define PI 3.14159265359f
#define DIVISIONS 24

#define DELTA_ANGLE 3
#define VERTEX_OFFSET_Z 3
using namespace std;
using namespace cv;

GLfloat points[] = { -10.5f, -10.5f, 0.0f, 10.5f,  -10.5f, 0.0f, 10.5f,  10.5f,   0.0f,
                                             10.5f,  10.5f,   0.0f, -10.5f, 10.5f,  0.0f, -10.5f, -10.5f, 0.0f };

glm::vec3 top_left(-10.5f, -10.5f, 0.0f);
glm::vec3 top_right(10.5f, -10.5f, 0.0f);

GLfloat texcoords[] = { 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
                                                    1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f };


OpenGLWidget::OpenGLWidget(QWidget *parent) : QOpenGLWidget(parent){


    nTheta = 180/DELTA_ANGLE + 1;
    nPhi = 360/DELTA_ANGLE + 1;
}

OpenGLWidget::~OpenGLWidget()
{
    glDeleteProgram(program);
}

void OpenGLWidget::initializeGL()
{
    initializeOpenGLFunctions();
    printContextInformation();
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    timer->start();

    //Initialize Distribution
    std::normal_distribution<double> p(0.0, 1.0);
    distribution = p;
    G_constant = 9.81;
    N = 16;
    M = 16;
    A = 1;
    Wind = glm::vec2(-1.0f, 0.5f);
    L_x = 100;
    L_z = 100;
    length = 500;

    //Enable certain OpenGL states
    glEnable(GL_DEPTH_TEST); //Enable Z-buffer
    glEnable(GL_MULTISAMPLE); //Draw smoothed polygons

    //Create program
    program = createProgram("./shaders/vshader2.vs", "./shaders/fshader2.fs");
    program_clouds = createProgram("./shaders/vshaderclouds.vs", "./shaders/fshaderclouds.fs");
    program_waves = createProgram("./shaders/vshaderwave.vs", "./shaders/fshaderwave.fs");
    program_boat = createProgram("./shaders/vshaderboat.vs", "./shaders/fshaderboat.fs");
    program_trees = createProgram("./shaders/vshadertrees.vs", "./shaders/fshadertrees.fs");

    glUseProgram(program);
    load_texture("./BG.png", &tex_background);
    glUseProgram(program_clouds);
    load_texture("./clouds3.png", &tex_clouds);
    glUseProgram(program_waves);
    load_texture("./Water.png", &tex_waves);
    glUseProgram(program_boat);
    load_texture("./boat.png", &tex_boat);
    glUseProgram(program_trees);
    load_texture("./trees.png", &tex_trees);


    glEnable( GL_CULL_FACE ); // cull face
    glCullFace( GL_BACK );      // cull back face
    glFrontFace( GL_CCW );      // GL_CCor counter clock-wise



    //Setup Transformations
    setupModelTransformation();
    setupViewTransformation();
    setupProjectionTransformation();
    createSphereObject();
}

void OpenGLWidget::resizeGL(int w, int h)
{
    screen_width = w;
    screen_height = h;
    glViewport(0, 0, screen_width, screen_height);
    setupProjectionTransformation();// Redo projection matrix
}

void OpenGLWidget::paintGL()
{
    /* Clear the background as white */
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    glUseProgram(program);
    glBindVertexArray(sphere_VAO);



    displace();

    //Boat
    glUseProgram(program_boat);
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, tex_boat);
    glDrawArrays(GL_TRIANGLES, 0, 6);


    //Trees
    glUseProgram(program_trees);
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, tex_trees);
    glDrawArrays(GL_TRIANGLES, 0, 6);


    //Clouds
    glUseProgram(program_clouds);
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, tex_clouds);
     glDrawArrays(GL_TRIANGLES, 0, 6);



    //Wave
    glUseProgram(program_waves);
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, tex_waves);
    glDrawArrays(GL_TRIANGLES, 0, 6);


    //Background
    glUseProgram(program);
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, tex_background);
    glDrawArrays(GL_TRIANGLES, 0, 6);






    
    
}


void OpenGLWidget::setupModelTransformation()
{
    //Modelling transformations (Model -> World coordinates)
    model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0, 0.0, 0.0));//Model coordinates are the world coordinates

    //Pass on the modelling matrix to the vertex shader
    glUseProgram(program);
    vModel_uniform = glGetUniformLocation(program, "vModel");
    if(vModel_uniform == -1){
        fprintf(stderr, "Could not bind location: vModel\n");
        exit(0);
    }
    glUniformMatrix4fv(vModel_uniform, 1, GL_FALSE, glm::value_ptr(model));

    glUseProgram(program_clouds);
    vModel_uniform = glGetUniformLocation(program_clouds, "vModel");
    if(vModel_uniform == -1){
        fprintf(stderr, "Could not bind location: vModel\n");
        exit(0);
    }
    glUniformMatrix4fv(vModel_uniform, 1, GL_FALSE, glm::value_ptr(model));

    glUseProgram(program_waves);
    vModel_uniform = glGetUniformLocation(program_waves, "vModel");
    if(vModel_uniform == -1){
        fprintf(stderr, "Could not bind location: vModel\n");
        exit(0);
    }
    glUniformMatrix4fv(vModel_uniform, 1, GL_FALSE, glm::value_ptr(model));

    glUseProgram(program_boat);
    vModel_uniform = glGetUniformLocation(program_boat, "vModel");
    if(vModel_uniform == -1){
        fprintf(stderr, "Could not bind location: vModel\n");
        exit(0);
    }
    glUniformMatrix4fv(vModel_uniform, 1, GL_FALSE, glm::value_ptr(model));


    glUseProgram(program_trees);
    vModel_uniform = glGetUniformLocation(program_trees, "vModel");
    if(vModel_uniform == -1){
        fprintf(stderr, "Could not bind location: vModel\n");
        exit(0);
    }
    glUniformMatrix4fv(vModel_uniform, 1, GL_FALSE, glm::value_ptr(model));


}

void OpenGLWidget::setupViewTransformation()
{
    //Viewing transformations (World -> Camera coordinates
    view = glm::lookAt(glm::vec3(0.0, 0.0, 20.0), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));

    //Pass on the viewing matrix to the vertex shader
    glUseProgram(program);
    vView_uniform = glGetUniformLocation(program, "vView");
    if(vView_uniform == -1){
        fprintf(stderr, "Could not bind location: vView\n");
        exit(0);
    }
    glUniformMatrix4fv(vView_uniform, 1, GL_FALSE, glm::value_ptr(view));

    glUseProgram(program_clouds);
    vView_uniform = glGetUniformLocation(program_clouds, "vView");
    if(vView_uniform == -1){
        fprintf(stderr, "Could not bind location: vView\n");
        exit(0);
    }
    glUniformMatrix4fv(vView_uniform, 1, GL_FALSE, glm::value_ptr(view));
    glUseProgram(program_waves);
    vView_uniform = glGetUniformLocation(program_waves, "vView");
    if(vView_uniform == -1){
        fprintf(stderr, "Could not bind location: vView\n");
        exit(0);
    }
    glUniformMatrix4fv(vView_uniform, 1, GL_FALSE, glm::value_ptr(view));

    glUseProgram(program_boat);
    vView_uniform = glGetUniformLocation(program_boat, "vView");
    if(vView_uniform == -1){
        fprintf(stderr, "Could not bind location: vView\n");
        exit(0);
    }
    glUniformMatrix4fv(vView_uniform, 1, GL_FALSE, glm::value_ptr(view));

    glUseProgram(program_trees);
    vView_uniform = glGetUniformLocation(program_trees, "vView");
    if(vView_uniform == -1){
        fprintf(stderr, "Could not bind location: vView\n");
        exit(0);
    }
    glUniformMatrix4fv(vView_uniform, 1, GL_FALSE, glm::value_ptr(view));

}

void OpenGLWidget::setupProjectionTransformation()
{
    //Projection transformation (Perspective projection)
    float aspect = (float)screen_width/(float)screen_height;

    glm::mat4 projection = glm::perspective(45.0f, aspect, 0.1f, 1000.0f);
    //Pass on the projection matrix to the vertex shader
    glUseProgram(program);
    vProjection_uniform = glGetUniformLocation(program, "vProjection");
    if(vProjection_uniform == -1){
        fprintf(stderr, "Could not bind location: vProjection\n");
        exit(0);
    }
    glUniformMatrix4fv(vProjection_uniform, 1, GL_FALSE, glm::value_ptr(projection));



    //Pass on the projection matrix to the vertex shader
    glUseProgram(program_clouds);
    vProjection_uniform = glGetUniformLocation(program_clouds, "vProjection");
    if(vProjection_uniform == -1){
        fprintf(stderr, "Could not bind location: vProjection\n");
        exit(0);
    }
    glUniformMatrix4fv(vProjection_uniform, 1, GL_FALSE, glm::value_ptr(projection));

    glUseProgram(program_waves);
    vProjection_uniform = glGetUniformLocation(program_waves, "vProjection");
    if(vProjection_uniform == -1){
        fprintf(stderr, "Could not bind location: vProjection\n");
        exit(0);
    }
    glUniformMatrix4fv(vProjection_uniform, 1, GL_FALSE, glm::value_ptr(projection));

    glUseProgram(program_boat);
    vProjection_uniform = glGetUniformLocation(program_boat, "vProjection");
    if(vProjection_uniform == -1){
        fprintf(stderr, "Could not bind location: vProjection\n");
        exit(0);
    }
    glUniformMatrix4fv(vProjection_uniform, 1, GL_FALSE, glm::value_ptr(projection));

    glUseProgram(program_trees);
    vProjection_uniform = glGetUniformLocation(program_trees, "vProjection");
    if(vProjection_uniform == -1){
        fprintf(stderr, "Could not bind location: vProjection\n");
        exit(0);
    }
    glUniformMatrix4fv(vProjection_uniform, 1, GL_FALSE, glm::value_ptr(projection));

}

long OpenGLWidget::get_time_milli()
{
    gettimeofday(&tp, NULL);
    long int ms = tp.tv_sec * 1000 + tp.tv_usec / 1000;
    return ms;
}

long OpenGLWidget::get_time_sec()
{
    gettimeofday(&tp, NULL);
    long int ms = tp.tv_sec;
    return ms;
}

bool OpenGLWidget:: load_texture( const char *file_name, GLuint *tex ) {
       int x, y, n;
    int force_channels = 4;
    unsigned char *image_data = stbi_load( file_name, &x, &y, &n, force_channels );
    if ( !image_data ) {
        fprintf( stderr, "ERROR: could not load %s\n", file_name );
        return false;
    }
    // NPOT check
    if ( ( x & ( x - 1 ) ) != 0 || ( y & ( y - 1 ) ) != 0 ) {
        fprintf( stderr, "WARNING: texture %s is not power-of-2 dimensions\n",
                         file_name );
    }
    int width_in_bytes = x * 4;
    unsigned char *top = NULL;
    unsigned char *bottom = NULL;
    unsigned char temp = 0;
    int half_height = y / 2;

    for ( int row = 0; row < half_height; row++ ) {
        top = image_data + row * width_in_bytes;
        bottom = image_data + ( y - row - 1 ) * width_in_bytes;
        for ( int col = 0; col < width_in_bytes; col++ ) {
            temp = *top;
            *top = *bottom;
            *bottom = temp;
            top++;
            bottom++;
        }
    }
    glGenTextures( 1, tex );
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, *tex );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                                image_data );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    return true;

}

void OpenGLWidget::WindSpeed(int value)
{
    Wind = glm::vec2(float (value - 1) , 0.5f);
}


void OpenGLWidget::createSphereObject(){
    glUseProgram(program);


    vVertex_attrib = glGetAttribLocation(program, "vVertex");
    if(vVertex_attrib == -1) {
        fprintf(stderr, "Could not bind location: vVertex\n");
        exit(0);
    }

    GLuint tex_attrib = glGetUniformLocation(program, "basic_texture");
    if(tex_attrib == -1) {
        fprintf(stderr, "Could not bind location: basic_texture\n");
        exit(0);
    }
    // glUseProgram(program);
    glUniform1i(tex_attrib, 0);

   

    //Generate vertex buffer and index buffer

    glGenVertexArrays(1, &sphere_VAO);
    glBindVertexArray(sphere_VAO);

    GLuint vertex_VBO, normal_VBO;
    glGenBuffers(1, &vertex_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_VBO);
    glBufferData(GL_ARRAY_BUFFER, 18*sizeof(GLfloat), points, GL_STATIC_DRAW);
    glEnableVertexAttribArray(vVertex_attrib);
    glVertexAttribPointer(vVertex_attrib, 3, GL_FLOAT, GL_FALSE, 0, 0);


    GLuint tex_VBO;
    glGenBuffers(1, &tex_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, tex_VBO);
    glBufferData(GL_ARRAY_BUFFER, 12*sizeof(GLfloat), texcoords, GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);


    glBindBuffer(GL_ARRAY_BUFFER, 0);


}


void OpenGLWidget::load_image()

{
    img = cv::imread("1_5.png", CV_LOAD_IMAGE_COLOR);
}
void OpenGLWidget::displace()
{

   glm::vec2 wind = glm::vec2(0.0f, 1.0f);
    long int millisec = get_time_milli();
    glUseProgram(program_waves);
    uniform_time = glGetUniformLocation(program_waves, "Time");
    if(vProjection_uniform == -1){
        fprintf(stderr, "Could not bind location: Time\n");
        exit(0);
    }
    glUniform1f(uniform_time, millisec);
    displace_cloud(Wind);
    displace_boat(Wind);
    displace_trees(Wind);
    //Mat_Texture(DisplacedLayer(img, double(timeInSec)), tex1);
    //Mat_Texture(img);
}

void OpenGLWidget::displace_cloud(glm::vec2 wind)
{
    long int millisec = get_time_milli();

    glm::mat4 displaced = translate_cloud(millisec, wind);
    glUseProgram(program_clouds);
    vCloud_uniform = glGetUniformLocation(program_clouds, "vCloud");
    if(vCloud_uniform == -1){
        fprintf(stderr, "Could not bind location: vCloud\n");
        exit(0);
    }
    glUniformMatrix4fv(vCloud_uniform, 1, GL_FALSE, glm::value_ptr(displaced));


}

void OpenGLWidget::displace_boat(glm::vec2 wind)
{
    long int millisec = get_time_milli();
    glm::mat4 displaced = translate_boat(millisec, wind);
    glUseProgram(program_boat);
    tboat_uniform = glGetUniformLocation(program_boat, "tboat");
    if(tboat_uniform == -1){
        fprintf(stderr, "Could not bind location: tboat\n");
        exit(0);
    }
    glUniformMatrix4fv(tboat_uniform, 1, GL_FALSE, glm::value_ptr(displaced));

    glm::mat4 rotated = rotate_boat(millisec, wind);
    glUseProgram(program_boat);
    rboat_uniform = glGetUniformLocation(program_boat, "rboat");
    if(rboat_uniform == -1){
        fprintf(stderr, "Could not bind location: rboat\n");
        exit(0);
    }
    glUniformMatrix4fv(rboat_uniform, 1, GL_FALSE, glm::value_ptr(rotated));

}

glm::mat4 OpenGLWidget::translate_cloud(long int time, glm::vec2 wind)
{
    //cloud displacement
    int wind_x = wind.x + 1;
    float displacement = (((int)(time * wind_x/180) % 50)+10);
    qDebug()<<time<<" "<<displacement<<endl;
    glm::mat4 l_position = glm::translate(glm::mat4(1.0f), glm::vec3(displacement, 0.0f, 0.0f));
    return l_position;
}

glm::mat4 OpenGLWidget::translate_boat(long int time, glm::vec2 wind)
{
    int wind_x = wind.x + 1;
    float displacement = 0.02*wind_x*cos(((int)(time *wind_x /200) % 50)+20);
    qDebug()<<time<<" "<<displacement<<endl;
    glm::mat4 l_position = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f,displacement, 0.0f));
    return l_position;
}

glm::mat4 OpenGLWidget::rotate_boat(long int time, glm::vec2 wind)
{
    int wind_x = wind.x + 1;
    float displacement = 0.2*sin(((int)(time*wind_x/200) % 50));
    qDebug()<<time<<" "<<displacement<<endl;
    glm::mat4 l_position = glm::rotate(glm::mat4(1.0f), glm::radians(displacement), glm::vec3(0,0,1));
    return l_position;
}

void OpenGLWidget::displace_trees(glm::vec2 wind)
{
    long int millisec = get_time_milli();
    glm::mat4 displaced = rotate_trees(millisec, wind);
    glUseProgram(program_trees);
    trees_uniform = glGetUniformLocation(program_trees, "trees");
    if(tboat_uniform == -1){
        fprintf(stderr, "Could not bind location: trees\n");
        exit(0);
    }
    glUniformMatrix4fv(trees_uniform, 1, GL_FALSE, glm::value_ptr(displaced));
}

glm::mat4 OpenGLWidget::rotate_trees(long time, glm::vec2 wind)
{
    float wind_x = wind.x+1;
    float displacement = 0.1*wind_x*sin(((int)(time/500) % 100));
    qDebug()<<time<<" "<<displacement<<endl;
    glm::mat4 l_position = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f,-2.0f, 0.0f));
    l_position = glm::rotate(l_position, glm::radians(displacement), glm::vec3(0,0,1));
    l_position = glm::translate(l_position, glm::vec3(0.0f,2.0f, 0.0f));

    return l_position;
}

Mat OpenGLWidget::DisplacedLayer(Mat L, double time)
{

    Mat L_(L);

    for(int i = 0; i < L.rows; i++)
        for(int j = 0; j < L.cols; j++)
        {
            Vec3b color = L.at<Vec3b>(Point(i,j));
            //get displacement vector
            glm::vec2 pix(i, j);
            glm::vec2 dis_pix = D_map(pix, time);
            int i_ = (int)dis_pix.x;
            int j_ = (int)dis_pix.y;

                L_.at<Vec3b>(Point(i_, j_)) = color;

        }
    return L_;
}

bool OpenGLWidget::Mat_Texture(Mat L, GLuint tex)
{
    Mat L_;

    flip(L, L_, 0);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
         glGenTextures(1, &tex);
         glActiveTexture( GL_TEXTURE0 );
         glBindTexture(GL_TEXTURE_2D, tex);

         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

         cv::cvtColor(L_, L_, CV_RGB2BGR);

         glTexImage2D(GL_TEXTURE_2D,
                           0,                   // Pyramid level (for mip-mapping) - 0 is the top level
               GL_RGB,              // Internal colour format to convert to
                           L_.cols,          // Image width  i.e. 640 for Kinect in standard mode
                           L_.rows,          // Image height i.e. 480 for Kinect in standard mode
                           0,                   // Border width in pixels (can either be 1 or 0)
               GL_BGR,              // Input image format (i.e. GL_RGB, GL_RGBA, GL_BGR etc.)
               GL_UNSIGNED_BYTE,    // Image data type
               L_.ptr());



return true;
}



glm::vec2 OpenGLWidget::genWaves()
{
       time(&timeInSec);
       GenHeightMap(double(timeInSec));
       glUseProgram(program);
       tim = glGetUniformLocation(program, "time");
       glUniform1f(tim, (float)timeInSec);

       vMap_attrib = glGetAttribLocation(program, "Map");
       GLuint Waves_VBO;
       glGenBuffers(1, &Waves_VBO);
       glBindBuffer(GL_ARRAY_BUFFER,  Waves_VBO);
       glBufferData(GL_ARRAY_BUFFER, M*N*sizeof(GLfloat),map, GL_STATIC_DRAW);
       glEnableVertexAttribArray(vMap_attrib);
       glVertexAttribPointer(vMap_attrib, 1, GL_FLOAT, GL_FALSE, 0, 0);

}

glm::vec2 OpenGLWidget::D_map(glm::vec2 pixel, double t)
{

    glm::vec2 D;
   // build M 3x3s
    /*
    glm::mat3 M;
    glm::vec3 V_x(top_left);
    glm::vec3 V_y(top_right);
    glm::vec3 V_z = glm::cross(V_x, V_y);
    M = glm::transpose(glm::mat3(V_x, V_y, V_z));
    glm::mat3 M_inverse = glm::inverse(M);
    glm::vec3 Pixel_3 = glm::vec3(pixel, 1);
    glm::vec3 Q = M * Pixel_3;
    glm::vec3 Q_add = glm::vec3(0, height_(Q, t), 0);
    glm::vec3 Q_ = Q + Q_add;
    glm::vec3 P_ = M_inverse * Q;
    if(P_.z == 0)
        P_.z = 1;

    P_ = P_ / P_.z;
    Pixel_3 = P_ - Pixel_3;
    D = glm::vec2(Pixel_3.x, Pixel_3.y);
    */
    time(&timeInSec);
     D = glm::vec2(pixel.x, pixel.y - (float)( 2* height_(pixel, t)));



    //qDebug()<<"O: "<<pixel.x<<" "<<pixel.y<<endl;

    //qDebug()<<"Shift: "<<D.x<<" "<<D.y<<endl;

   return D;
}

void OpenGLWidget::GenHeightMap(double t)
{
    int n, m;
    float k_x, k_z;
    map = new double*[M];
    for(int i = 0; i < M; i++)
            map[i] = new double[N];

    for(int m_ = 0; m_ < M; m_++)
    {
        for(int n_ = 0; n_ < N; n_++)
        {
            int index = (M * m_) + n_;
            n = n_ - N/2;
            m = m_ - M/2;
            k_x = (2 * M_PI * n)/L_x;
            k_z = (2 * M_PI * m)/L_z;
            glm::vec2 x = glm::vec2(k_x, k_z);
            map[m_][n_] = height(x, t).real();
        }
    }
}

double OpenGLWidget::height_(glm::vec2 x, double time)
{
    double p = 0; //cos(2*time + (x.y));
    //qDebug()<<p;
    return p;
}

std::complex<double> OpenGLWidget::height(glm::vec2 x, double time)
{
    int n, m;
    float k_x, k_z;
    std::complex<double> Height_x_t(0.0, 0.0);
    /*
    for(int m_ = 0; m_ < M; m_++)
    {
        for(int n_ = 0; n_ < N; n_++)
        {
            n = n_ - N/2;
            m = m_ - M/2;
            k_x = (2 * M_PI * n)/L_x;
            k_z = (2 * M_PI * m)/L_z;
            glm::vec2 k = glm::vec2(k_x, k_z);
            double kdx = glm::dot(k, x);
            //qDebug()<<"kdx"<<kdx;
            std::complex<double> exp_(cosf(kdx), sinf(kdx));
            //qDebug()<<exp_.real()<<" "<<exp_.imag()<<endl;
            std::complex<double> h = H(k, time);
            std::complex<double> ex = h*exp_;
            qDebug()<<ex.real()<<" "<<ex.imag()<<endl;
            Height_x_t = Height_x_t + ex;
        }
    }*/

    //qDebug()<<Height_x_t.real();
    return Height_x_t;
}

double OpenGLWidget::gaussdraw()
{
    double number = distribution(generator);
    //qDebug() << number<<endl;
    return number;
}

double OpenGLWidget::dispersion(glm::vec2 S)
{
        double value =  G_constant * glm::length(S);
        //qDebug()<<S.x<<"  "<<S.y<<endl;
        return value;

}

double OpenGLWidget::Phillips(glm::vec2 S)
{
    glm::vec2 Wind_m = glm::normalize(Wind);
    double L = glm::dot(Wind, Wind) / G_constant;
    glm::vec2 S_m = glm::normalize(S);
    double P_s = (A * exp(-1/pow(glm::length(S) * L, 2)) * pow(glm::dot(S_m, Wind_m),2))  / pow(glm::length(S), 4);
    //qDebug()<<P_s;
    return P_s;
}
std::complex<double> OpenGLWidget::H_O(glm::vec2 S)
{
    double rand1 = gaussdraw();
    double rand2 = gaussdraw();
    double P_s_root = pow(Phillips(S), 0.5);
    double coef = double((1 / pow(2, 0.5))) * P_s_root;
    std::complex<double> my(rand1, rand2);
    std::complex<double> h_o = coef * my;
    //qDebug()<<h_o.real()<<" "<<h_o.imag();
    return h_o;
}

std::complex<double> OpenGLWidget::H_O_conjugate(glm::vec2 S)
{
    double rand1 = gaussdraw();
    double rand2 = gaussdraw();
    double P_s_root = pow(Phillips(S), 0.5);
    double coef = double((1 / pow(2, 0.5))) * P_s_root;
    std::complex<double> my(rand1, -rand2);
    std::complex<double> h_o_ = coef * my;
    //qDebug()<<h_o_.real()<<" "<<h_o_.imag();
    return h_o_;
}

std::complex<double> OpenGLWidget::H(glm::vec2 S, double t)
{
    std::complex<double> exp1(cosf(dispersion(S) * t), sinf(dispersion(S)* t));
    std::complex<double> exp2(cosf(dispersion(S) * t), -sinf(dispersion(S)* t));
    std::complex<double> H = H_O(S) * exp1 + H_O_conjugate(-S) * exp2;
    //qDebug()<<H.real()<<" "<<H.imag();
    return H;
}

glm::vec3 OpenGLWidget::getTrackBallVector(int x, int y)
{
    glm::vec3 p = glm::vec3(2.0*x/screen_width - 1.0, 2.0*y/screen_height - 1.0, 0.0); //Normalize to [-1, +1]
    p.y = -p.y; //Invert Y since screen coordinate and OpenGL coordinates have different Y directions.

    float mag2 = p.x*p.x + p.y*p.y;
    if(mag2 <= 1.0)
        p.z = sqrt(1.0 - mag2);
    else
        p = glm::normalize(p); //Nearest point, close to the sides of the trackball
    return p;
}

void OpenGLWidget::printContextInformation()
{
    QString glType;
    QString glVersion;
    QString glProfile;

    //Get GL version info
    glType = (context()->isOpenGLES())?"OpenGL ES" : "OpenGL";
    glVersion = reinterpret_cast<const char*>(glGetString(GL_VERSION));

    //Get profile information

#define CASE(c) case QSurfaceFormat::c: glProfile = #c; break
    switch(format().profile())
    {
        CASE(NoProfile);
        CASE(CoreProfile);
        CASE(CompatibilityProfile);
    }
#undef CASE

    qDebug() <<qPrintable(glType)<<qPrintable(glVersion)<<"("<<qPrintable(glProfile) << ")";

}

GLuint OpenGLWidget::createProgram(const char *vshader_filename, const char* fshader_filename)
{
    //Create shader objects
    GLuint vs, fs;
    if ((vs = createShader(vshader_filename, GL_VERTEX_SHADER))   == 0) return 0;
    if ((fs = createShader(fshader_filename, GL_FRAGMENT_SHADER)) == 0) return 0;

    //Creare program object and link shader objects
    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    GLint link_ok;
    glGetProgramiv(program, GL_LINK_STATUS, &link_ok);
    if (!link_ok) {
        fprintf(stderr, "glLinkProgram error:");
        printLog(program);
        glDeleteShader(vs);
        glDeleteShader(fs);
        glDeleteProgram(program);
        return 0;
    }

    return program;
}

//Read shader source as a string
char* OpenGLWidget::getShaderCode(const char* filename)
{
    FILE* input = fopen(filename, "rb");
    if(input == NULL) return NULL;

    if(fseek(input, 0, SEEK_END) == -1) return NULL;
    long size = ftell(input);
    if(size == -1) return NULL;
    if(fseek(input, 0, SEEK_SET) == -1) return NULL;

    /*if using c-compiler: dont cast malloc's return value*/
    char *content = (char*) malloc( (size_t) size +1  );
    if(content == NULL) return NULL;

    fread(content, 1, (size_t)size, input);
    if(ferror(input)) {
        free(content);
        return NULL;
    }

    fclose(input);
    content[size] = '\0';
    return content;
}

//Print error log
void OpenGLWidget::printLog(GLuint object)
{
    GLint log_length = 0;
    if (glIsShader(object))
        glGetShaderiv(object, GL_INFO_LOG_LENGTH, &log_length);
    else if (glIsProgram(object))
        glGetProgramiv(object, GL_INFO_LOG_LENGTH, &log_length);
    else {
        fprintf(stderr, "printlog: Not a shader or a program\n");
        return;
    }

    char* log = (char*)malloc(log_length);

    if (glIsShader(object))
        glGetShaderInfoLog(object, log_length, NULL, log);
    else if (glIsProgram(object))
        glGetProgramInfoLog(object, log_length, NULL, log);

    fprintf(stderr, "%s", log);
    free(log);
}

//Create shader object
GLuint OpenGLWidget::createShader(const char* filename, GLenum type)
{
    const GLchar* source = getShaderCode(filename);
    if (source == NULL) {
        fprintf(stderr, "Error opening %s: ", filename); perror("");
        return 0;
    }
    GLuint res = glCreateShader(type);
    glShaderSource(res, 1, &source, NULL);
    free((void*)source);

    glCompileShader(res);
    GLint compile_ok = GL_FALSE;
    glGetShaderiv(res, GL_COMPILE_STATUS, &compile_ok);
    if (compile_ok == GL_FALSE) {
        fprintf(stderr, "%s:", filename);
        printLog(res);
        glDeleteShader(res);
        return 0;
    }

    return res;
}
