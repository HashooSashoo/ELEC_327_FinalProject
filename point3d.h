#ifndef point3d_include
#define point3d_include

// 3D point / vector. Maps to your Python Point3D class.
// We drop write_char (no chars on a monochrome bitmap LCD) and the various
// __dunder__ overloads (no operator overloading in C); each Python operator
// becomes a named function below.
typedef struct {
    float x;
    float y;
    float z;
} Point3D;

// ---- Construction ----------------------------------------------------------
Point3D makePoint3D(float x, float y, float z);

// ---- Pure operations (return a new value; arguments unchanged) -------------
Point3D addPoint3D(Point3D a, Point3D b); 
Point3D subtractPoint3D(Point3D a, Point3D b);     
Point3D scalePoint3D(Point3D a, float s);       
float   dotProduct3D(Point3D a, Point3D b);       
Point3D crossProduct3D(Point3D a, Point3D b);    
float   magnitudePoint3D(Point3D a);               

// ---- In-place operations ---------------------------------------------------
void translatePoint3D(Point3D *p, float dx, float dy, float dz);

// Rotation about each axis. Angle convention matches your Python code:
//   rotate_xy(theta) -> rotation in the XY plane (i.e. about the Z axis)
//   rotate_yz(phi)   -> rotation in the YZ plane (i.e. about the X axis)
//   rotate_xz(rho)   -> rotation in the XZ plane (i.e. about the Y axis)
void rotatePoint3D_XY(Point3D *p, float theta);
void rotatePoint3D_YZ(Point3D *p, float phi);
void rotatePoint3D_XZ(Point3D *p, float rho);

// Apply theta, phi, rho in sequence. Matches Python's rotateSpecified().
void rotatePoint3D(Point3D *p, float theta, float phi, float rho);

#endif
