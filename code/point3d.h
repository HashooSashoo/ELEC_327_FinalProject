#ifndef point3d_include
#define point3d_include

// 3D point / vector, maps to my Python Point3D class
typedef struct {
    float x;
    float y;
    float z;
} Point3D;

// making the point
Point3D makePoint3D(float x, float y, float z);

// operations (return a new value with arguments unchanged)
Point3D addPoint3D(Point3D a, Point3D b); 
Point3D subtractPoint3D(Point3D a, Point3D b);     
Point3D scalePoint3D(Point3D a, float s);       
float dotProduct3D(Point3D a, Point3D b);       
Point3D crossProduct3D(Point3D a, Point3D b);    
float magnitudePoint3D(Point3D a);               

// in-place operations
void translatePoint3D(Point3D *p, float dx, float dy, float dz);

// rotation about each axis
void rotatePoint3D_XY(Point3D *p, float theta); // around z axis
void rotatePoint3D_YZ(Point3D *p, float phi);   // around x axis
void rotatePoint3D_XZ(Point3D *p, float rho);   // around y axis

// apply theta, phi, rho in sequence. Matches my rotateSpecified() in my Python library!
void rotatePoint3D(Point3D *p, float theta, float phi, float rho);

#endif
