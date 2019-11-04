#ifndef OPENGLWIDGET_H
#define OPENGLWIDGET_H

#include <QMouseEvent>
#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>
#include <QTimer>
#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <random>
#include <cmath>
#include <complex>

class OpenGLWidget : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core
{
    Q_OBJECT
public:
    OpenGLWidget(QWidget *parent);
    ~OpenGLWidget();

protected:
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

    void setupModelTransformation();
    void setupViewTransformation();
    void setupProjectionTransformation();
    void printContextInformation();
    GLuint createProgram(const char *vshader_filename, const char* fshader_filename);
    char* getShaderCode(const char* filename);
    void printLog(GLuint object);
    GLuint createShader(const char* filename, GLenum type);
    glm::vec3 getTrackBallVector(int x, int y);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

    //Wave Generation
    void createWave(int N, int M, double L_x, double L_y);
    std::complex<double> height(glm::vec2 x, double time);
    double gaussdraw();
    double Phillips(glm::vec2 S);
    double dispersion(glm::vec2 S);
    std::complex<double> H_O(glm::vec2 S);
    std::complex<double> H_O_conjugate(glm::vec2 S);
    std::complex<double> H(glm::vec2 S, double t);
    glm::vec2 D_map(glm::vec2 pixel, double t);


    int currentX, currentY;
    int oldX, oldY;
    int size;
    double G_constant;
    glm::vec2 Wind;
    int N, M;
    double L_x, L_z;
    double A;
    std::default_random_engine generator;
    std::normal_distribution<double> distribution;
	void createSphereObject();
    void createSphereObject2();
    size_t nTriangles;
private:
    GLuint program;
	GLint vVertex_attrib, vNormal_attrib, vColor_attrib;
    GLint vModel_uniform, vView_uniform, vProjection_uniform;
    int screen_width, screen_height;


    glm::mat4 model;
    glm::mat4 view;

	GLuint cube_VAO, sphere_VAO, indices_IBO; //Vertex array object for cube
    



    int tri_points;
    int nTheta, nPhi;
    std::vector<float> x,y,z, arr;
    std::vector<float>  tri_vertices;
    std::vector<short> tri;
    bool load_texture( const char *file_name, GLuint *tex );
    std::vector<float> u_c, v_c;
    void generateSphereCoords();
    GLuint texcoords_vbo;
    GLuint tex;

};

#endif // OPENGLWIDGET_H