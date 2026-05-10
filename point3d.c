// point3d.c
/*
I basically defined many operations of a 3D point in space that I will use
for my 3D graphics. It uses math.h and not MATHACL (though that can be changed maybe...)
*/

#include "point3d.h"
#include <math.h>

// creates a Point3D object from coordinates
Point3D makePoint3D(float x, float y, float z) {
    Point3D p = { x, y, z };
    return p;
}



// --------------------------------
// defined operations on points here...
// --------------------------------
Point3D addPoint3D(Point3D a, Point3D b) {
    return makePoint3D(a.x + b.x, a.y + b.y, a.z + b.z);
}

Point3D subtractPoint3D(Point3D a, Point3D b) {
    return makePoint3D(a.x - b.x, a.y - b.y, a.z - b.z);
}

Point3D scalePoint3D(Point3D a, float s) {
    return makePoint3D(a.x * s, a.y * s, a.z * s);
}

float dotProduct3D(Point3D a, Point3D b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

Point3D crossProduct3D(Point3D a, Point3D b) {
    return makePoint3D(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
}

float magnitudePoint3D(Point3D a) {
    return sqrtf(a.x * a.x + a.y * a.y + a.z * a.z);
}



// ------------------------------------------
// these functions are for moving the point in space, they
// will be extremely relevant for graphics3d.c
// ------------------------------------------

void translatePoint3D(Point3D *p, float dx, float dy, float dz) {
    p->x += dx;
    p->y += dy;
    p->z += dz;
}

void rotatePoint3D_XY(Point3D *p, float theta) {
    float c = cosf(theta);
    float s = sinf(theta);
    float ox = p->x;
    float oy = p->y;
    p->x = ox * c - oy * s;
    p->y = ox * s + oy * c;
    // z unchanged
}

void rotatePoint3D_YZ(Point3D *p, float phi) {
    float c = cosf(phi);
    float s = sinf(phi);
    float oy = p->y;
    float oz = p->z;
    p->y = oy * c - oz * s;
    p->z = oy * s + oz * c;
    // x unchanged
}

void rotatePoint3D_XZ(Point3D *p, float rho) {
    float c = cosf(rho);
    float s = sinf(rho);
    float ox = p->x;
    float oz = p->z;
    p->x = ox * c - oz * s;
    p->z = ox * s + oz * c;
    // y unchanged
}

void rotatePoint3D(Point3D *p, float theta, float phi, float rho) {
    rotatePoint3D_XY(p, theta);
    rotatePoint3D_YZ(p, phi);
    rotatePoint3D_XZ(p, rho);
}
