#include "AeCoordinateTransform.h"

OGRCoordinateTransformation *AeCoordinateTransform::createGeoTransformation(int epsgSrc, int epsgDst, int &errorCode)
{
    //----------------------------------------------------------------------------------
    // Configure Spatial Reference Reprojection
    //----------------------------------------------------------------------------------
    OGRSpatialReference srcSpatialReference;
    OGRErr error = srcSpatialReference.importFromEPSG(epsgSrc);

    OGRSpatialReference dstSpatialReference;
    error = error | dstSpatialReference.importFromEPSG(epsgDst);

    if (error != 0){
        errorCode = error;
        return nullptr;
    }

    OGRCoordinateTransformation* coordTrans = OGRCreateCoordinateTransformation(&srcSpatialReference, &dstSpatialReference);
    return coordTrans;
}

bool AeCoordinateTransform::transformCoordinates(OGRCoordinateTransformation *coordTrans, double x, double y, double &xOut, double &yOut)
{
    xOut = x;
    yOut = y;

    coordTrans->Transform(1, &xOut, &yOut);
    return true;
}
