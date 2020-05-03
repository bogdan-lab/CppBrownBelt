#include "Common.h"
#include <sstream>
using namespace std;

// Этот файл сдаётся на проверку
// Здесь напишите реализацию необходимых классов-потомков `IShape`


class SimpleShape : public IShape{
public:

    SimpleShape(){
        //texture_ = make_shared<ITexture>();
        size_ = {0, 0};
        pos_ = {0, 0};
    }

    virtual bool CheckForPainting(Point p) const = 0;
    virtual bool CheckPointInShape(Point p) const = 0;

    void SetPosition(Point p) override{
        pos_ = move(p);
    }

    Point GetPosition() const override {
        return pos_;
    }

    void SetSize(Size s) override {
        size_ = move(s);
    }

    Size GetSize() const override {
        return size_;
    }

    void SetTexture(shared_ptr<ITexture> tx) override {
        texture_ = move(tx);
    }

    ITexture* GetTexture() const override{
        return texture_.get();
    }

    Image PaintDefault(const char symbol) const {
        Size shp_s = this->GetSize();
        Image painted_shape(shp_s.height, string(shp_s.width, ' '));
        //Image painted_shape(shp_s.height, string());
        for(int i=0; i<painted_shape.size(); i++){
            for(int j=0; j<painted_shape[0].size(); j++){
                if (CheckPointInShape({j, i})){
                    painted_shape[i][j]= symbol;
                }
            }
        }
        return painted_shape;
    }

    void PaintWithTexture(Image& img) const {
        if(this->GetTexture()){
            const Image& txt_img = this->GetTexture()->GetImage();
            for(int i=0; i<img.size(); i++){
                for(int j=0; j<img[0].size(); j++){
                    if (CheckForPainting({j, i})){
                        img[i][j] = txt_img[i][j];
                    }
                }
            }
        }
    }

    void Draw(Image & img) const override {
        Size shp_s = this->GetSize();
        //Size txt_s = this->GetTexture()->GetSize();
        //Image painted_shape(shp_s.height, string(shp_s.width, '.'));    //all default
        Image painted_shape = PaintDefault('.');
        //Paint image
        PaintWithTexture(painted_shape);
        //Now put painted image in the given image
        Point shp_pos = this->GetPosition();
        for(int i=0; i<img.size(); i++){
            for(int j=0; j<img[0].size(); j++){
                if(CheckPointInShape({(j-shp_pos.x), (i-shp_pos.y)}))
                //if((i>=shp_pos.y && i<(shp_pos.y+shp_s.height)) &&
                //   (j>=shp_pos.x && j<(shp_pos.x+shp_s.width)))
                {
                  img[i][j] = painted_shape[i-shp_pos.y][j-shp_pos.x];
                }
            }
        }
    }

    shared_ptr<ITexture> CopyTexture() const {
        return texture_;
    }

    //std::unique_ptr<IShape> Clone() const = 0;
private:
    shared_ptr<ITexture> texture_;
    Size size_;
    Point pos_;
};


class Rectangle : public SimpleShape {
public:

    Rectangle(): SimpleShape() {}

    unique_ptr<IShape> Clone() const {
        unique_ptr<IShape> clone = make_unique<Rectangle>();
        clone->SetPosition(this->GetPosition());
        clone->SetSize(this->GetSize());
        clone->SetTexture(this->CopyTexture());
        return clone;
    }

    bool CheckPointInShape(Point p) const override {
        Size shp_s = this->GetSize();
        return p.y>=0 && p.y<shp_s.height &&
               p.x>=0 && p.x<shp_s.width;
    }

    virtual bool CheckForPainting(Point p) const override {
        Size shp_s = this->GetSize();
        Size txt_s = this->GetTexture()->GetSize();
        return p.y<min(shp_s.height, txt_s.height) && p.x<min(shp_s.width, txt_s.width);
    }

};


class Ellipse : public SimpleShape{
public:
    Ellipse(): SimpleShape() {}

    unique_ptr<IShape> Clone() const override {
        unique_ptr<IShape> clone = make_unique<Ellipse>();
        clone->SetPosition(this->GetPosition());
        clone->SetSize(this->GetSize());
        clone->SetTexture(this->CopyTexture());
        return clone;
    }

    bool CheckPointInShape(Point p) const override {
        return IsPointInEllipse(p, this->GetSize());
    }

    bool CheckForPainting (Point p) const override {
        bool in_shape = IsPointInEllipse(p, this->GetSize());
        Size txt_s = this->GetTexture()->GetSize();
        bool in_texture = p.y<txt_s.height && p.x<txt_s.width;
        return  in_shape && in_texture;
    }
};




// Напишите реализацию функции
unique_ptr<IShape> MakeShape(ShapeType shape_type) {
    if(shape_type == ShapeType::Rectangle){
        return make_unique<Rectangle>();
    }
    else if (shape_type == ShapeType::Ellipse){
        return make_unique<Ellipse>();
    }
    return {nullptr};
}
