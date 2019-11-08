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
    Wind = glm::vec2(0.5f, 0.5f);
    L_x = 100;
    L_z = 100;

    //Enable certain OpenGL states
    glEnable(GL_DEPTH_TEST); //Enable Z-buffer
    glEnable(GL_MULTISAMPLE); //Draw smoothed polygons

    //Create program
    program = createProgram("./shaders/vshader2.vs", "./shaders/fshader2.fs");

    glUseProgram(program);
  //  ( load_texture( "./Resources/Water.png", &tex ) );
    load_image();

	glEnable( GL_CULL_FACE ); // cull face
	glCullFace( GL_BACK );		// cull back face
	glFrontFace( GL_CCW );		// GL_CCW for counter clock-wise



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

    //printOpenGLError();
    glUseProgram(program);
    displace();
	glBindVertexArray(sphere_VAO);

    glUseProgram(program);
    
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, tex1);

    
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

    time(&timeInSec);

    Mat_Texture(DisplacedLayer(img, double(timeInSec)));
    //Mat_Texture(img);
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

bool OpenGLWidget::Mat_Texture(Mat L)
{
    Mat L_;

    flip(L, L_, 0);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
         glGenTextures(1, &tex1);
         glActiveTexture( GL_TEXTURE0 );
         glBindTexture(GL_TEXTURE_2D, tex1);

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
    if(timeInSec != 0)
    {
        D = glm::vec2(pixel.x, pixel.y - (float)( 2* height_(pixel, t)) - 2);
    }
    else
    {
        D = glm::vec2(pixel.x, pixel.y - (float)( 2* height_(pixel, t)) - 4);
    }
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
    double p = cos(2*time + (x.y));
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
