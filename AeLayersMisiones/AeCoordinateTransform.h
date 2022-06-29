#ifndef AECOORDINATETRANSFORM_H
#define AECOORDINATETRANSFORM_H

#include <ogr_spatialref.h>

class AeCoordinateTransform {
public:
    static OGRCoordinateTransformation* createGeoTransformation(int epsgSrc, int epsgDst, int &errorCode);
    static bool transformCoordinates(OGRCoordinateTransformation* coordTrans, double x, double y, double &xOut, double &yOut);
};


#endif // AECOORDINATETRANSFORM_H
