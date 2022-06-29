#include <iostream>
#include <vector>
#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
using namespace std;

class ComponentLayer
{
  public:
    ComponentLayer(std::string id, std::string metadata):mId(id), mMetadata(metadata){}
    virtual QJsonObject getDataObject() = 0;

    std::string            mId;
    std::string            mMetadata;
};

//Hoja
class LineStringLayers : public ComponentLayer
{
  private:
     QJsonArray coords;
     vector<ComponentLayer*> lineChildren;
  public:
//    LineStringLayers(std::string id , std::string metadata, QJsonArray coords):ComponentLayer(id, metadata){
//        this->coords = coords;
//    }
     LineStringLayers(std::string id , std::string metadata):ComponentLayer(id, metadata){
     }

    QJsonObject getDataObject(){
        QJsonObject circuitLine;
        circuitLine["id"] = QString::fromStdString(mId);
        circuitLine["type"] = "LineString";
        circuitLine["metadata"] = "No Metadata";
        QJsonArray children;
        if(!lineChildren.empty()){
            for (int i = 0; i < lineChildren.size(); ++i){
                children.append(lineChildren[i]->getDataObject());
            }
        }
        circuitLine["lines"] = children;
        return circuitLine;
    }

    void add(ComponentLayer *cmp){
        lineChildren.push_back(cmp);
    }
};


class Lines : public ComponentLayer
{
  private:
     QJsonArray coords;
     vector<ComponentLayer*> lineChildren;
  public:
    Lines(std::string id , std::string metadata, QJsonArray coords):ComponentLayer(id, metadata){
        this->coords = coords;
    }

    QJsonObject getDataObject(){
        QJsonObject lines;
        lines["id"] = QString::fromStdString(mId);
        lines["coords"] = this->coords;
        return lines;
    }

    void add(ComponentLayer *cmp){
        lineChildren.push_back(cmp);
    }
};

class PointsLayer : public ComponentLayer
{
  private:
    QJsonArray coords;
  public:
    PointsLayer(std::string id , std::string metadata, QJsonArray coords):ComponentLayer(id, metadata){
        this->coords = coords;
    }

    QJsonObject getDataObject(){
        QJsonObject points;
        points["id"] = QString::fromStdString(mId);
        points["type"] = "Points";
        points["coords"] = this->coords;
        return points;
    }
};

class PolygonLayer : public ComponentLayer
{
  private:
    QJsonArray coords;
    QJsonArray properties;
  public:
    PolygonLayer(std::string id , std::string metadata, QJsonArray coords, QJsonArray properties):ComponentLayer(id, metadata){
        this->coords = coords;
        this->properties = properties;
    }

    QJsonObject getDataObject(){
        QJsonObject polygon;
        polygon["id"] = QString::fromStdString(mId);
        polygon["type"] = "Polygon";
        polygon["coords"] = this->coords;
        polygon["properties"] = this->properties;
        return polygon;
    }
};

//Composite
class Layer: public ComponentLayer
{
  public:
    Layer();
    Layer(std::string id, std::string metadata):ComponentLayer(id, metadata){
//        this->zone = {};
    }
    ~Layer(){
        cout << "Destructor executed" << endl;
    }

//    void setZone(QJsonArray zone){
//        this->zone = zone;
//    }

    void add(ComponentLayer *cmp){
        mChildren.push_back(cmp);
    }

    int size_children(){
        return mChildren.size();
    }


    QJsonObject getDataObject(){
        //para quitar el objeto BCN solo hay que quitar estos datos.
        QJsonObject layer;
        layer["id"] = QString::fromStdString(mId);
        layer["type"] = "Composite";
        layer["metada"] = QString::fromStdString(mMetadata);
//        layer["area"] = this->zone;
        QJsonArray children;
        if(!mChildren.empty()){
            for (int i = 0; i < mChildren.size(); ++i){
                children.append(mChildren[i]->getDataObject());
            }
        }
        layer["children"] = children;
        return layer;
    }

   private:
//    QJsonArray zone;
    vector<ComponentLayer*> mChildren;
};
