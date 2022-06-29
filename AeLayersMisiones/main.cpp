#include <iostream>
#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QCoreApplication>
#include "ogrsf_frmts.h"
#include "AeCoordinateTransform.h"
#include "AeCompositeLayers.cpp"
#include <QDir>
using namespace std;


typedef struct MyPoint3D
{
double dX;
double dY;
double dZ;
}MyPoint3D;

vector<MyPoint3D>PointLayer;


typedef struct MultipointFeature
{
vector<MyPoint3D>PointsOfFeature;
}MultipointFeature;


vector<MultipointFeature> MultipointLayer;

typedef struct MyLine3D
{
 vector<MyPoint3D> LineString;
}MyLine3D;

typedef struct LineFeature
{
vector<MyLine3D>LinesOfFeature;
}LineFeature;

vector<LineFeature> LineLayer;

//data structure for rings
typedef struct MyRing3D
{
vector<MyPoint3D> RingString;
bool IsClockwised;
}MyRing3D;

//data structure for polygons
typedef struct MyPolygon3D
{
vector<MyRing3D>Polygon;
}MyPolygon3D;

//data structure for a polygon feature
typedef struct PolygonFeature
{
vector<MyPolygon3D>PolygonsOfFeature;
}PolygonFeature;

//Holds Coordinates of Polygon Shapefile
vector<PolygonFeature> PolygonLayerVector;

void generateLayersShapefile(QString path, QString filename, QString basename, Layer *layer);
bool checkIfChangeFile(string filename, string basename, string &basename_aux);
void setPointsLayer(QString basename, OGRLayer &polayer, Layer *layer);
void setLinestringLayer(QString basename, OGRLayer &polayer, Layer *layer);
void setPolygonLayer(QString basename, OGRLayer &polayer, Layer *layer);


int main(int argc, char *argv[])
{

    //QCoreApplication a(argc, argv);

    QString path = "C:/Users/mochoa/DatosMapa2D/BCN";
    QDir dir(path);

    if(!dir.exists()){
        cout << "Directory not exist" << endl;
        exit(1);
    }

    string basename_aux = " ";
    QJsonObject scene;
    scene["id"] = "SCENE";
    scene["type"] = "Composite";
    scene["metada"] = "None";
    QJsonArray scene_children;

    QJsonObject map;
    map["id"] = "SCENE_MAP";
    map["type"] = "Map";
    map["metada"] = "No metadata";

    QJsonArray wtm_template_layers;
    QJsonObject template_layer;
    QJsonArray array_sector_3857 = {-20037508.342789244,-20037508.342789244, 20037508.342789244,20037508.342789244};
    template_layer["url_template"] = "https://server.arcgisonline.com/ArcGIS/rest/services/World_Imagery/MapServer/tile/{z}/{y}/{x}";
    template_layer["sector_epsg_3857"] = array_sector_3857;
    template_layer["min_level"] = 0;
    template_layer["max_level"] = 18;
    wtm_template_layers.append(template_layer);

    map["wtm_template_layers"] = wtm_template_layers;

    scene_children.append(map);

    QStringList path_list = path.split('/');
    Layer* layer = new Layer(path_list.last().toStdString(), "No metadata");
    foreach(QFileInfo item, dir.entryInfoList()){
        if(item.isFile()){
            if(item.fileName().endsWith(".shp",Qt::CaseSensitive)){
                generateLayersShapefile(item.absoluteFilePath(),
                                       item.fileName(),
                                       item.baseName(),
                                       layer);
            }
        }
    }

    scene_children.append(layer->getDataObject());
    scene["children"] = scene_children;


    QFile file("C:/Users/mochoa/Documents/layers_shapefiles.json");
    if(file.open(QIODevice::WriteOnly)){
        file.write(QJsonDocument(scene).toJson());
        file.flush();
        file.close();
    }else{
        qDebug() << "failed to open json";
    }

    return 0;
}


bool checkIfChangeFile(string filename, string basename, string &basename_aux){
    if (basename.find(basename_aux) != std::string::npos){
        return true;
    }else{
        basename_aux = basename;
        cout << "Basename_aux = " << basename_aux << endl;
        return false;
    }
    return true;
}


void generateLayersShapefile(QString path, QString filename, QString basename, Layer *layer){
    //Modularizar más adelante
    cout << endl << "Generating layers..." << endl;
    GDALAllRegister();
    GDALDataset *poDS;
    poDS = (GDALDataset*) GDALOpenEx(path.toStdString().c_str(), GDAL_OF_VECTOR, NULL, NULL, NULL);

    if(poDS == NULL){
        printf( "Open failed.\n");
        exit(1);
    }

    QJsonObject points_layer, linestring_layer, json_complete;
    for(auto&& poLayer: poDS->GetLayers()){
        OGRwkbGeometryType LayerGeometryType = poLayer ->GetGeomType();
        if(wkbFlatten (LayerGeometryType) == wkbPoint){
            cout << "Points layer\n";
            setPointsLayer(basename, *poLayer, layer);
        }else if(wkbFlatten (LayerGeometryType) == wkbLineString ){
           cout << "Linestring layer\n";
           setLinestringLayer(basename, *poLayer, layer);
        }else if(wkbFlatten (LayerGeometryType) == wkbPolygon){
           cout << "Polygon layer\n";
           setPolygonLayer(basename, *poLayer, layer);
        }
    }
}

void setPointsLayer(QString basename, OGRLayer &polayer, Layer *layer){
    QJsonObject qdata_points;
    QJsonArray array_points;
    QJsonArray points;

    for(auto&& poFeature: polayer){
        const OGRGeometry *poGeometry = poFeature->GetGeometryRef();
        if (poGeometry != nullptr && poGeometry->getGeometryType() == wkbPoint){
            int tempValue = 0;

            OGRCoordinateTransformation* transform;
            transform = AeCoordinateTransform::createGeoTransformation(32631, 3857, tempValue);
            if(transform == nullptr){
                qDebug() << "Transform NULL";
            }
            OGRPoint *poPoint = (OGRPoint *)poGeometry;
            MyPoint3D pt;
            double x;
            double y;
            //Convertimos cada uno de los puntos de 32631 a 3857
            AeCoordinateTransform::transformCoordinates(transform, poPoint ->getX(), poPoint ->getY(), x, y);
            pt.dX = x;
            pt.dY = y;
            pt.dZ = poPoint->getZ();
            points = {x,y,poPoint->getZ()};
            PointLayer.push_back(pt);
            QString qstr_name;
            std::string str_name;

            QString qstr_lineid;
            double coord_z;
            for(auto&& oField: *poFeature){
                if(std::strcmp(oField.GetName(), "Name") == 0){
                    str_name = oField.GetString();
                    qstr_name = QString::fromStdString(oField.GetString());
                    qdata_points["name"] = qstr_name;

                }else if(std::strcmp(oField.GetName(), "LineId") == 0 || std::strcmp(oField.GetName(), "lineID") == 0){
                    qstr_lineid = QString::number(oField.GetAsInteger64());
                    qdata_points["LineId"] = qstr_lineid;
                }else if(std::strcmp(oField.GetName(), "Z") == 0){
                    coord_z = oField.GetAsDouble();
                    points = {x, y, coord_z};
                }
            }

            qdata_points["coords"] = points;
            array_points.append(qdata_points);

        }else{
            printf("no point geometry\n");
        }
    }

    PointsLayer* pylon_layer = new PointsLayer(basename.toStdString(),"No Metadata", array_points);
    layer->add(pylon_layer);
    PointLayer.clear();
}

void setLinestringLayer(QString basename, OGRLayer &polayer, Layer *layer){
    QJsonObject qdata;
    LineStringLayers* layer_circuitline = new LineStringLayers(basename.toStdString(), "No Metadata");
    for(auto&& poFeature: polayer){
            const OGRGeometry *poGeometry = poFeature->GetGeometryRef();
            if( poGeometry != nullptr && poGeometry->getGeometryType() == wkbLineString ){
                int tempValue = 0;
                OGRCoordinateTransformation* transform;
                transform = AeCoordinateTransform::createGeoTransformation(32631, 3857, tempValue);
                if(transform == nullptr){
                    qDebug() << "Transform is NULL";
                }
                LineFeature Polyline;
                OGRPoint ptTemp;
                const OGRLineString *poLine = poGeometry->toLineString();

                //First method with Vector
                Polyline.LinesOfFeature.resize(1);
                int NumberOfVertices = poLine ->getNumPoints();
                Polyline.LinesOfFeature.at(0).LineString.resize(NumberOfVertices);
                for(int k = 0; k < NumberOfVertices; k++){
                    poLine -> getPoint(k,&ptTemp);
                    MyPoint3D pt;
                    pt.dX = ptTemp.getX();
                    pt.dY = ptTemp.getY();
                    pt.dZ = ptTemp.getZ();
                    Polyline.LinesOfFeature.at(0).LineString.at(k) = pt;
                }
                LineLayer.push_back(Polyline);

                //Second method with QJsonArray
                QString qstr_id, qstr_nemo;
                std::string str_id, str_nemo;
                for(auto&& oField: *poFeature){
                    if(std::strcmp(oField.GetName(), "Nombre") == 0 || std::strcmp(oField.GetName(), "ID") == 0){
                        str_id = oField.GetString();
                        qstr_id = QString::fromStdString(oField.GetString());
                    }else if(std::strcmp(oField.GetName(), "Mnemónico") == 0){
                        str_nemo = oField.GetString();
                        qstr_nemo = QString::fromStdString(oField.GetString());
                    }
                }
                QJsonArray tempArray;
                double x;
                double y;
                for(int i = 0; i < poLine->getNumPoints(); i++){
                    OGRPoint *tempPoint = new OGRPoint();
                    poLine->getPoint(i, tempPoint);
                    AeCoordinateTransform::transformCoordinates(transform, tempPoint->getX(), tempPoint->getY(), x, y);
                    tempArray.append(x);
                    tempArray.append(y);
                    tempArray.append(tempPoint->getZ());
                }
                Lines* line = new Lines(str_id, "No metadata", tempArray);
                layer_circuitline->add(line);
            }
        }
    layer->add(layer_circuitline);
    PointLayer.clear();
    LineLayer.clear();
}

void setPolygonLayer(QString basename, OGRLayer &polayer, Layer *layer){
    int tempValue = 0;
    OGRCoordinateTransformation* transform;
    transform = AeCoordinateTransform::createGeoTransformation(32631, 3857, tempValue);
    if(transform == nullptr){
        qDebug() << "Transform NULL";
    }

    QJsonObject properties;
    QJsonArray array_properties;
    for(auto&& poFeature: polayer){
        const OGRGeometry *poGeometry = poFeature->GetGeometryRef();
        if (poGeometry != nullptr && poGeometry->getGeometryType() == wkbPolygon){
            PolygonFeature Polygon;
            OGRPoint ptTemp;

            OGRPolygon *poPolygon = ( OGRPolygon * )poGeometry;
            Polygon.PolygonsOfFeature.resize(1);
            int NumberOfInnerRings = poPolygon ->getNumInteriorRings();

            OGRLinearRing *poExteriorRing = poPolygon ->getExteriorRing();
            Polygon.PolygonsOfFeature.at(0).Polygon.resize(NumberOfInnerRings+1);
            Polygon.PolygonsOfFeature.at(0).Polygon.at(0).IsClockwised = poExteriorRing ->isClockwise();
            int NumberOfExteriorRingVertices = poExteriorRing ->getNumPoints();
            Polygon.PolygonsOfFeature.at(0).Polygon.at(0).RingString.resize(NumberOfExteriorRingVertices);
            for(int k = 0; k < NumberOfExteriorRingVertices; k++){
               poExteriorRing ->getPoint(k,&ptTemp);
               MyPoint3D pt;
               pt.dX = ptTemp.getX();
               pt.dY = ptTemp.getY();
               pt.dZ = ptTemp.getZ();
               Polygon.PolygonsOfFeature.at(0).Polygon.at(0).RingString.at(k) = pt;
           }
            for(int h = 1; h <= NumberOfInnerRings; h++){
               OGRLinearRing *poInteriorRing = poPolygon ->getInteriorRing(h-1);
               Polygon.PolygonsOfFeature.at(0).Polygon.at(h).IsClockwised = poInteriorRing ->isClockwise();
               int NumberOfInteriorRingVertices = poInteriorRing ->getNumPoints();
               Polygon.PolygonsOfFeature.at(0).Polygon.at(h).RingString.resize(NumberOfInteriorRingVertices);

               for (int k = 0; k < NumberOfInteriorRingVertices; k++){
                   poInteriorRing ->getPoint(k,&ptTemp);
                   MyPoint3D pt;
                   pt.dX = ptTemp.getX();
                   pt.dY = ptTemp.getY();
                   pt.dZ = ptTemp.getZ();
                   Polygon.PolygonsOfFeature.at(0).Polygon.at(h).RingString.at(k) = pt;
               }
           }
            PolygonLayerVector.push_back(Polygon);

            //Polygon properties
            for(auto&& oField: *poFeature){
                switch( oField.GetType() ){
                    case OFTInteger:
                        printf( "%d,", oField.GetInteger() );
                        properties[oField.GetName()] = oField.GetInteger();
                        break;
                    case OFTInteger64:
                        properties[oField.GetName()] = oField.GetInteger64();
                        break;
                    case OFTReal:
                        properties[oField.GetName()] = oField.GetDouble();
                        break;
                    case OFTString:
                        properties[oField.GetName()] = oField.GetString();
                        break;
                    default:
                        properties[oField.GetName()] = oField.GetAsString();
                        break;
                }
            }
            array_properties.append(properties);

        }
    }

    //Extract points from Polygon
    QJsonArray array_points;
    double x;
    double y;
    for(auto&& polygonlayer: PolygonLayerVector){
        for(auto&& polygonfeature : polygonlayer.PolygonsOfFeature){
            for(auto&& polygon: polygonfeature.Polygon){
                for(auto&& polyring: polygon.RingString){
                    cout << "eje x: " << polyring.dX << " , eje y: " << polyring.dY << " , eje z: " << polyring.dZ << endl;
                    AeCoordinateTransform::transformCoordinates(transform, polyring.dX, polyring.dY, x, y);
                    array_points.append(x);
                    array_points.append(y);
                    array_points.append(polyring.dZ);
                }
            }
        }
    }

    PolygonLayer* polygon_layer = new PolygonLayer(basename.toStdString(),"No Metadata", array_points, array_properties);
    layer->add(polygon_layer);
    PolygonLayerVector.clear();

}
