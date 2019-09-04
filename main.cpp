#define SFML_STATIC
#include "geo.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <Windows.h>
#include "img.h"

using namespace std;

enum State {
    NoneSelected,
    ShapeSelected,
    OnePointSelected,
    TwoPointSelected
} state;

const int kScreenWidth = 1024,
          kScreenHeight = 700;
const sf::Color kNotSelected(0,0,0),
                kToBeSelected(0,255,0), //(144, 238, 144),
                kSelected(255, 0, 0);

//Playground Viewport
const int groundx = 0, groundX = 1024,
          groundy = 0, groundY = 600;

const int kNearRadius = 10;

vector<Shape> shape;
int selectingShape, toBeSelectedShape;

//Two selected points
int xs1, ys1, xs2, ys2;
//To be selected point
int xs, ys;

bool mouseLeftHolding;
sf::Vector2i mouseOldPosition;

sf::RenderWindow window;
sf::Font font;

//State_HandleEvent should set the according global variables, even if invalid.

void ChangeState (State newState);

void NoneSelected_HandleEvent (sf::Event ev);
void NoneSelected_DrawMenu ();

void ShapeSelected_HandleEvent (sf::Event ev);
void ShapeSelected_DrawMenu ();

void OnePointSelected_HandleEvent (sf::Event ev);
void OnePointSelected_DrawMenu ();

void TwoPointSelected_HandleEvent (sf::Event ev);
void TwoPointSelected_DrawMenu ();

//
void SaveToFile ();
void LoadFromFile ();
void NewWithRect();
void NewWithCircle();

void ShapeToFile (const Shape &sh);
void ShapeToFile (int i);

//Fill with white
void DrawShape (int i, sf::Color border);
void DrawPoint (int x, int y, sf::Color col); //OK
void MoveShape (int i, int deltaX, int deltaY);

sf::IntRect BoundaryRect (int i);
//Nearly coincide
bool coincide (int x1, int y1, int x2, int y2) {
    int dsq = dsq_(x1,y1,x2,y2);
    return dsq <= kNearRadius*kNearRadius; //5 pixel radius
}
//Angle of line segment between two point and the (xbase, ybase)x axis.
//A.k.a: goc De Nhat Quoc Su Bong Ky.
double TigerAngle (double xbase, double ybase, double xp, double yp) {
    double xAC=xp-xbase, yAC=yp-ybase, xAB=1, yAB=0;
    double cos_theta = dot(xAB, yAB, xAC, yAC) / veclen(xAC, yAC);
    double theta = acos(cos_theta);
    if (yp < ybase) { //below
        //double pi = acos(-1.0);
        theta = -theta;
    }
    return theta;
}
//LOOSE INSIDE CHECK.
bool inside (int x, int y, const Shape& sh) {
    const float degreePerPoint = 12.0;
    int ccwline = -2;
    int n = (int)sh.e.size();
    for (int i=0; i<n; ++i)
        if (sh.e[i].isArc) {
            int rsq = dsq_(sh.e[i].xc, sh.e[i].yc, sh.x[i], sh.y[i]);
            int disq = dsq_(sh.e[i].xc, sh.e[i].yc, x, y);
            if (disq > rsq) return false;

            //APPROXIMATE STRICT CHECK goes here.
            sf::Vector2f v1(sh.x[i] - sh.e[i].xc, sh.y[i] - sh.e[i].yc),
                v2(sh.x[(i + 1) % n] - sh.e[i].xc, sh.y[(i + 1) % n] - sh.e[i].yc);
            //cerr<<"v1: "<<v1.x<<" "<<v1.y<<" - v2: "<<v2.x<<" "<<v2.y<<endl;
            float theta = angle(v1, v2);
            int numMidPoint = (int)(abs(theta) / degreePerPoint); //0..numMidPoint-1

            //cerr<<"#Mid points: "<<numMidPoint<<endl;

            float delta_theta = theta / numMidPoint;

            float t = 0.0;
            sf::Vector2f lastPoint ((float)sh.x[i], (float)sh.y[i]);
            for (int k = 0; k < numMidPoint-1; ++k)
            { //actually, it's k to numMidPoint-1, but it's ok.
                sf::Vector2f vec = rotate(v1, t);
                sf::Vector2f pnt = sf::Vector2f((float)sh.e[i].xc, (float)sh.e[i].yc) + vec;
                int c = ccw(lastPoint.x, lastPoint.y, pnt.x, pnt.y, 1.f*x, 1.f*y);
                if (c!=0) {
                    if (ccwline!=-2 && ccwline!=c) return false;
                    ccwline = c;
                }
                t = t + delta_theta;
                lastPoint = pnt;
            }
        }
        else { //All line ccws (except 0) yield same values
            int c = ccw(sh.x[i],sh.y[i],sh.x[(i+1)%n],sh.y[(i+1)%n],x,y);
            if (c==0) {
                //If inside the line segment, good, else, doomed.
                if (min(sh.x[i], sh.x[(i+1)%n])<=x && x<=max(sh.x[i], sh.x[(i+1)%n])
                    && min(sh.y[i], sh.y[(i+1)%n])<=y && y<=max(sh.y[i], sh.y[(i+1)%n])) {

                    }
                else return false;
            }
            else {
                if (ccwline!=-2 && ccwline!=c) return false;
                ccwline = c;
            }
        }
    return true;
}

sf::Vector2i ToBeSelectedPoint (int mousex, int mousey);

//#define TESTING

int main() {
    //INIT CODE GOES HERE
    window.create(sf::VideoMode(kScreenWidth,kScreenHeight),
                    L"Rê và cắt giấy ~",
                    sf::Style::Titlebar+sf::Style::Close);
    window.setFramerateLimit(60);
    window.setPosition({0,0});
    if (!font.loadFromFile("UVNDoiMoi.ttf")) {
        cerr<<"Font loading failed!";
        return 0;
    }
    sf::Image img;
    sf::Texture bgi_texture;
    if (!bgi_texture.loadFromFile("bg.png")) {
        cerr << "Background image not found, using a black background."<<endl;
        img.create(kScreenWidth, kScreenHeight);
        bgi_texture.loadFromImage(img);
    }
    sf::Sprite bgi(bgi_texture);
    selectingShape = -1; toBeSelectedShape=-1;
    shape.clear();
    xs1=ys1=xs2=ys2=xs=ys=-1;
    state = NoneSelected;


#ifdef TESTING
    /* //DrawPoint Test
    DrawPoint(100, 100, kSelected);
    DrawPoint(100,200,kNotSelected);
    DrawPoint(100, 300, kToBeSelected);
    window.display();
    sf::sleep(sf::seconds(5.0)); */

    //inside test
    Shape s;
    s.x = {50, 100, 100, 50};
    s.y = {50,50,100,100};
    
    s.e.resize(4);
    s.e[0].isArc = s.e[1].isArc = s.e[2].isArc = s.e[3].isArc = 0;
    ////cerr<<inside(75,75,s)<<" "<<inside(100,100,s)<<" "<<inside(101,100,s)<<endl; //T T F
    shape.push_back(s);
    DrawShape(0, kSelected);
    //window.display();
    //sf::sleep(sf::seconds(3.0));

    s.x = {200, 200};
    s.y = {100, 200};
    s.e[0].isArc = 0;
    s.e[1].isArc = 1;
    s.e[1].xc = 200; s.e[1].yc=150;
    shape.push_back(s);
    DrawShape(1, kToBeSelected);
    window.display();
    sf::sleep(sf::seconds(3.0));

#else
    while (window.isOpen()) {
        sf::Event ev;
        while (window.pollEvent(ev)) {
            if (ev.type == sf::Event::Closed)
                window.close();
            switch (state) {
                case NoneSelected: {
                    NoneSelected_HandleEvent (ev);
                    break;
                };
                case ShapeSelected: {
                    ShapeSelected_HandleEvent(ev);
                    break;
                };
                case OnePointSelected: {
                    OnePointSelected_HandleEvent(ev);
                    break;
                };
                case TwoPointSelected: {
                    TwoPointSelected_HandleEvent(ev);
                    break;
                };
            }
        }

        ////cerr<<"Event handled. Drawing... "<<endl;

        //DRAW CODE GOES HERE
        /*
            1. Background.
            2. Shapes, from nShape-1 downto 0 (Shape 0 in front), excpt selecting.
            3. Selecting shape, if differ from -1, with special Red border.
            4. Menus, depends on the state.
        */
        window.draw(bgi);
        ////cerr<<"BGI Drawn."<<endl;

        for (int i=(int)shape.size()-1; i>=0; --i)
            if (selectingShape!=i && toBeSelectedShape!=i)
                DrawShape(i, kNotSelected);
        ////cerr<<"Shapes drawn."<<endl;

        if (state==NoneSelected && toBeSelectedShape!=-1)
            DrawShape(toBeSelectedShape, kToBeSelected);
        ////cerr<<"TBS Shape drawn."<<endl;
        
        if (state!=NoneSelected) {
            DrawShape(selectingShape, kSelected);
            if (xs1!=-1)
                DrawPoint(xs1, ys1, kSelected);
            if (xs2!=-1)
                DrawPoint(xs2, ys2, kSelected);
            if (state!=TwoPointSelected && xs!=-1 && !coincide(xs1,ys1,xs,ys) && !coincide(xs2,ys2,xs,ys)) {
                DrawPoint(xs, ys, kToBeSelected);
            }
        }
        ////cerr<<"Points drawn."<<endl;

        switch (state) {
            case NoneSelected: NoneSelected_DrawMenu(); break;
            case ShapeSelected: ShapeSelected_DrawMenu(); break;
            case OnePointSelected: OnePointSelected_DrawMenu(); break;
            case TwoPointSelected: TwoPointSelected_DrawMenu(); break;
        }
        ////cerr<<"Menu drawn."<<endl;

        window.display();
        ////cerr<<"Window display call."<<endl;
    }
    //FREE CODE GOES HERE

#endif

    return 0;
}

void TextOut (sf::String str, int x, int y, int pxSize, sf::Color col = sf::Color::White) {
    sf::Text txt(str,font,pxSize);
    txt.setPosition(1.f*x,1.f*y);
    txt.setFillColor(col);
    window.draw(txt);
}

/*
    SHOULD CHANGE THESE:
    vector<Shape> shape;
    int selectingShape, toBeSelectedShape;

    //Two selected points
    int xs1, ys1, xs2, ys2;
    //To be selected point
    int xs, ys;

    bool mouseLeftHolding;
    sf::Vector2i mouseOldPosition;

    state
*/

void ChangeState (State newState) {
    state = newState;
}

void NoneSelected_HandleEvent (sf::Event ev) {
    switch (ev.type)
    {
    case (sf::Event::KeyReleased): {
        if (ev.key.code==sf::Keyboard::F4 && ev.key.alt 
        && !ev.key.control && !ev.key.shift && !ev.key.system) { //alt-f4
            window.close();
        }
        switch (ev.key.code) {
        case (sf::Keyboard::S): {
            SaveToFile();
            break;
        }
        case (sf::Keyboard::L): {
            LoadFromFile();
            break;
        }
        case (sf::Keyboard::N): {
            NewWithRect();
            break;
        }
        case (sf::Keyboard::M): {
            NewWithCircle();
            break;
        }
        }
    }
    case (sf::Event::MouseMoved): {
        //Determine ToBeSelectedShape
        toBeSelectedShape = -1;
        for (int i=(int)shape.size()-1; i>=0; --i) {
            if (inside(ev.mouseMove.x, ev.mouseMove.y, shape[i])) {
                toBeSelectedShape = i;
                break;
            }
        }
        break;
    }
    case (sf::Event::MouseButtonPressed): {
        if (ev.mouseButton.button == sf::Mouse::Left) {
            mouseLeftHolding = true;
            mouseOldPosition = {ev.mouseButton.x, ev.mouseButton.y};

            if (toBeSelectedShape != -1) {
                selectingShape = toBeSelectedShape;
                toBeSelectedShape = -1;
                ChangeState(State::ShapeSelected);
            }
        }
        break;
    }
    case (sf::Event::MouseButtonReleased): {
        if (ev.mouseButton.button == sf::Mouse::Left) {
            mouseLeftHolding = false;
        }
        break;
    }
    }
}
void NoneSelected_DrawMenu () {
    TextOut(L"(S) Mọi thứ->File", 50, 665, 26);
    TextOut(L"(L) File->Mọi thứ", 350, 665, 26);
    TextOut(L"(N) Dẹp hết, còn HCN", 650, 665, 26);
    TextOut(L"(M) Dẹp hết, còn Tròn", 650, 615, 26);
}

void ShapeSelected_HandleEvent (sf::Event ev) {
    switch (ev.type) {
    case (sf::Event::KeyReleased): {
        if (ev.key.code==sf::Keyboard::F4 && ev.key.alt 
        && !ev.key.control && !ev.key.shift && !ev.key.system) { //alt-f4
            window.close();
        }

        switch (ev.key.code) {
        case (sf::Keyboard::Tab): {
            if (selectingShape == (int)shape.size()-1)
                selectingShape = 0;
            else selectingShape += 1;
            break;
        }
        case (sf::Keyboard::S): {
            ShapeToFile(selectingShape);
            break;
        }
        case (sf::Keyboard::Delete): {
            shape.erase(shape.begin() + selectingShape);
            selectingShape = -1;
            ChangeState(State::NoneSelected);
            break;
        }
        case (sf::Keyboard::Escape): {
            selectingShape = -1;
            ChangeState(State::NoneSelected);
            break;
        }

        }
        break;
    }
    case (sf::Event::MouseButtonPressed): {
        if (ev.mouseButton.button==sf::Mouse::Left) { //Move
            if (inside(ev.mouseButton.x,ev.mouseButton.y,shape[selectingShape])) {
                mouseLeftHolding = true;
                mouseOldPosition = {ev.mouseButton.x, ev.mouseButton.y};
            }
            else if (xs!=-1 && ys!=-1) { //Select a point
                xs1 = xs; ys1 = ys; xs=ys=-1;
                ChangeState(State::OnePointSelected);
            }
        }
        if (ev.mouseButton.button==sf::Mouse::Right) { //Unselect
            selectingShape=-1;
            ChangeState(State::NoneSelected);
        }
        break;
    }
    case (sf::Event::MouseButtonReleased): {
        if (ev.mouseButton.button == sf::Mouse::Left) {
            mouseLeftHolding = false;
        }
        break;
    }
    case (sf::Event::MouseMoved): {
        if (mouseLeftHolding) {
            //cerr<<"OldPos: "<<mouseOldPosition.x<<" "<<mouseOldPosition.y<<endl;
            sf::Vector2i newPos = {ev.mouseMove.x, ev.mouseMove.y};
            //cerr<<"NewPos: "<<newPos.x<<" "<<newPos.y<<endl;
            sf::Vector2i delta = newPos - mouseOldPosition;
            //cerr<<"Delta b4: "<<delta.x<<" "<<delta.y<<endl;

            //Don't allow shape to move out of Playground Viewport.
            sf::IntRect bound = BoundaryRect(selectingShape);
            //cerr<<"Boundrect: "<<bound.left<<" "<<bound.top<<" "<<bound.left+bound.width-1<<" "<<bound.top+bound.height-1<<endl;
            delta.x = max(delta.x, groundx-bound.left);
            delta.x = min(delta.x, groundX - (bound.left+bound.width-1));
            delta.y = max(delta.y, groundy-bound.top);
            delta.y = min(delta.y, groundY - (bound.top+bound.height-1));

            //cerr<<"Delta: "<<delta.x<<" "<<delta.y<<endl<<endl;

            MoveShape (selectingShape, delta.x, delta.y);

            mouseOldPosition = newPos;
        }
        else { //toBeSelectedPoint calculate
            xs = ys = -1;
            sf::Vector2i tbsp = ToBeSelectedPoint(ev.mouseMove.x, ev.mouseMove.y);

            //cerr<<"Mouse: "<<ev.mouseMove.x<<" "<<ev.mouseMove.y<<endl;
            //cerr<<"TBSP: "<<tbsp.x<<" "<<tbsp.y<<endl<<endl;

            if (coincide(tbsp.x, tbsp.y, ev.mouseMove.x, ev.mouseMove.y)) {
                xs = tbsp.x; ys = tbsp.y;
            }
        }
        break;
    }
    }
}
void ShapeSelected_DrawMenu () {
    TextOut(L"(S) Hình này->File", 50, 665, 26);
    TextOut(L"(Del) Xóa hình này", 350, 665, 26);
    TextOut(L"(ESC) Bỏ chọn hình", 650, 665, 26);
    TextOut(L"(Tab) Chọn hình kế", 650, 615, 26);
}

void OnePointSelected_HandleEvent (sf::Event ev) {

}
void OnePointSelected_DrawMenu () {

}

void TwoPointSelected_HandleEvent (sf::Event ev) {
    
}
void TwoPointSelected_DrawMenu () {

}
void SaveToFile () {
    wchar_t filename[512]; fill(filename, filename+512, L'\0');
    OPENFILENAMEW ofn;
    //memset((void*)ofn, 0, sizeof(OPENFILENAMEW));
    ofn.hwndOwner = window.getSystemHandle();
    ofn.lpstrFilter = L"File nào cũng được\0*.*\0\0";
    ofn.lpstrCustomFilter = NULL;
    ofn.nFilterIndex = 0;
    ofn.lpstrFile = filename;
    ofn.nMaxFile = 512;
    ofn.lpstrFileTitle = NULL;
    ofn.lpstrInitialDir = NULL;
    ofn.lpstrTitle = L"Lưu ra đâu đây?";
    ofn.Flags = OFN_DONTADDTORECENT + OFN_OVERWRITEPROMPT + OFN_PATHMUSTEXIST;
    ofn.lpstrDefExt = nullptr;
    ofn.lStructSize = sizeof(OPENFILENAMEW);
    if (GetSaveFileNameW(&ofn)) {
        wstring filenamewstr (ofn.lpstrFile);
        HANDLE handle = CreateFileW(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        int nshape = (int)shape.size();
        DWORD numByteWritten;
        WriteFile(handle, (char*)&nshape, sizeof nshape, &numByteWritten, NULL);
        for (int i=0; i<nshape; ++i) {
            int n=shape[i].e.size();
            WriteFile(handle, (char*)&n, sizeof n, &numByteWritten, NULL);
            for (int j=0; j<n; ++j) {
                WriteFile(handle, (char*)&shape[i].e[j], sizeof shape[i].e[j], &numByteWritten, NULL);
                WriteFile(handle, (char*)&shape[i].x[j], sizeof(int), &numByteWritten, NULL);
                WriteFile(handle, (char*)&shape[i].y[j], sizeof(int), &numByteWritten, NULL);
            }
        }
        CloseHandle(handle);
    }
}
void LoadFromFile () {
    wchar_t filename[512]; fill(filename, filename+512, L'\0');
    OPENFILENAMEW ofn;
    //memset((void*)ofn, 0, sizeof(OPENFILENAMEW));
    ofn.hwndOwner = window.getSystemHandle();
    ofn.lpstrFilter = L"File nào cũng được\0*.*\0\0";
    ofn.lpstrCustomFilter = NULL;
    ofn.nFilterIndex = 0;
    ofn.lpstrFile = filename;
    ofn.nMaxFile = 512;
    ofn.lpstrFileTitle = NULL;
    ofn.lpstrInitialDir = NULL;
    ofn.lpstrTitle = L"Nạp từ đâu đây?";
    ofn.Flags = OFN_DONTADDTORECENT + OFN_OVERWRITEPROMPT + OFN_PATHMUSTEXIST + OFN_FILEMUSTEXIST;
    ofn.lpstrDefExt = nullptr;
    ofn.lStructSize = sizeof(OPENFILENAMEW);
    if (GetOpenFileNameW(&ofn)) {
        shape.clear();
        HANDLE handle = CreateFileW(filename, GENERIC_READ, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        int nshape; DWORD numBytesRead;
        ReadFile(handle, (char*)&nshape, sizeof(int), &numBytesRead, NULL);
        Shape sh;
        for (int i=0; i<nshape; ++i) {
            int n; sh.e.clear(); sh.x.clear(); sh.y.clear();
            Edge e; int x, y;
            ReadFile(handle, (char*)&n, sizeof(int), &numBytesRead, NULL);
            for (int j=0; j<n; ++j) {
                ReadFile(handle, (char*)&e, sizeof e, &numBytesRead, NULL);
                ReadFile(handle, (char*)&x, sizeof(int), &numBytesRead, NULL);
                ReadFile(handle, (char*)&y, sizeof(int), &numBytesRead, NULL);
                sh.e.push_back(e); sh.x.push_back(x); sh.y.push_back(y);
            }
            shape.push_back(sh);
        }
        CloseHandle(handle);
    }
}
void NewWithRect() {
    shape.clear();

    //700 x 500 rectangle topleft at (50,50)
    const int width = 700, height = 500;
    const int x = 50, y = 50;

    Shape sh;
    Edge e; e.isArc = e.xc = e.yc = 0;
    sh.e = {e, e, e, e};
    sh.x = {x, x, x+width-1, x+width-1};
    sh.y = {y, y+height-1, y+height-1, y};
    shape.push_back(sh);
}
void NewWithCircle() {
    shape.clear();

    //Radius 200 centered at (300, 300)
    const int radius = 200;
    const int xc=300, yc=300;

    Shape sh;
    sh.e = {{1,xc,yc}, {1,xc,yc}};
    sh.x = {xc, xc};
    sh.y = {yc-radius+1, yc+radius-1};
    shape.push_back(sh);
}
sf::IntRect BoundaryRect (const Shape &sh) {
    //FOR ARCS, 12 DEGREE A POINT
    float degreePerPoint = 12.0;

    int n = (int)sh.x.size();
    int minx=99999999, maxx=-1, miny=99999999, maxy=-1;
    sf::ConvexShape s;
    int j=0;
    for (int i=0; i<n; ++i) {
        if (sh.e[i].isArc) {
            sf::Vector2f v1 (sh.x[i] - sh.e[i].xc, sh.y[i] - sh.e[i].yc),
                         v2 (sh.x[(i+1)%n] - sh.e[i].xc,sh.y[(i+1)%n] - sh.e[i].yc);
            float theta = angle(v1, v2);
            int numMidPoint = (int)(abs(theta)/degreePerPoint); //0..numMidPoint-1

            float delta_theta = theta / numMidPoint;

            float t = 0.0;
            for (int k=0; k<numMidPoint; ++k) { //actually, it's k to numMidPoint-1, but it's ok.
                sf::Vector2f pnt = rotate(v1, t);
                pnt = {sh.e[i].xc+pnt.x, sh.e[i].yc+pnt.y};
                minx = min(minx, (int)floor(pnt.x)); maxx = max(maxx, (int)ceil(pnt.x));
                miny = min(miny, (int)floor(pnt.y)); maxy = max(maxy, (int)ceil(pnt.y));
                t = t + delta_theta;
            }
        }
        else {
            maxx=max(maxx, sh.x[i]); minx=min(minx, sh.x[i]);
            maxy=max(maxy, sh.y[i]); miny=min(miny, sh.y[i]);
        }
    }
    sf::IntRect rect;
    rect.left = minx; rect.width = maxx-minx+1;
    rect.top = miny; rect.height = maxy-miny+1;
    return rect;
}
sf::IntRect BoundaryRect (int i) {
    return BoundaryRect(shape[i]);
}

//Draw shape to @param rtxt instead of screen.
//No border, please, translate upleft by boundRect.x and boundRect.y
void DrawShape (const Shape &sh, sf::IntRect boundRect, sf::RenderTexture &rtxt) {
    //FOR ARCS, 1 DEGREE A POINT
    float degreePerPoint = 1.0;

    int n = (int)sh.x.size();
    sf::ConvexShape s;
    int ns = 0;
    //Calculate number of points
    for (int i=0; i<n; ++i)
        if (sh.e[i].isArc) {
            sf::Vector2f v1 (sh.x[i] - sh.e[i].xc, sh.y[i] - sh.e[i].yc),
                         v2 (sh.x[(i+1)%n] - sh.e[i].xc,sh.y[(i+1)%n] - sh.e[i].yc);
            float theta = angle(v1, v2);
            ns += (int)(abs(theta)/degreePerPoint);
        }
        else {
            ns ++;
        }
    //cerr<<ns<<" points set."<<endl;

    //Now add point to s
    s.setPointCount(ns);
    s.setFillColor(sf::Color::White);
    s.setOutlineThickness(0);
    int j=0;
    for (int i=0; i<n; ++i) {
        if (sh.e[i].isArc) {
            sf::Vector2f v1 (sh.x[i] - sh.e[i].xc, sh.y[i] - sh.e[i].yc),
                         v2 (sh.x[(i+1)%n] - sh.e[i].xc,sh.y[(i+1)%n] - sh.e[i].yc);
            //cerr<<"v1: "<<v1.x<<" "<<v1.y<<" - v2: "<<v2.x<<" "<<v2.y<<endl;
            float theta = angle(v1, v2);
            int numMidPoint = (int)(abs(theta)/degreePerPoint); //0..numMidPoint-1

            //cerr<<"#Mid points: "<<numMidPoint<<endl;

            float delta_theta = theta / numMidPoint;

            float t = 0.0;
            for (int k=0; k<numMidPoint; ++k) { //actually, it's k to numMidPoint-1, but it's ok.
                sf::Vector2f pnt = rotate(v1, t);
                //cerr<<"Vector #"<<j<<": "<<pnt.x<<" "<<pnt.y<<endl;
                //cerr<<"Point "<<j<<": "<<sh.e[i].xc+pnt.x<<" "<<sh.e[i].yc+pnt.y<<endl;
                s.setPoint(j++, {sh.e[i].xc+pnt.x-boundRect.left, sh.e[i].yc+pnt.y-boundRect.top});
                t = t + delta_theta;
                //cerr<<"t: "<<t<<endl;
            }
        }
        else {
            s.setPoint(j++, {1.0f*sh.x[i]-boundRect.left,1.0f*sh.y[i]-boundRect.top});
            //cerr<<"Point "<<j<<": "<<sh.x[i]<<" "<<sh.y[i]<<endl;
        }
    }
    //Finalize
    rtxt.draw(s);
}

void ShapeToFile (const Shape &sh) {
    //jpg, png, bmp
    //Draw sh to sf::Image
    sf::IntRect boundRect = BoundaryRect(sh);
    sf::RenderTexture rtxt;
    rtxt.setSmooth(true); //Anti-aliasing.
    rtxt.create(boundRect.width, boundRect.height);
    //Fill with black
    sf::RectangleShape blackRect (sf::Vector2f(boundRect.width, boundRect.height));
    blackRect.setFillColor(sf::Color::Black);
    rtxt.draw(blackRect);
    DrawShape(sh, boundRect, rtxt);
    sf::Texture txt = rtxt.getTexture();
    sf::Image img = txt.copyToImage();

    wchar_t filename[512]; fill(filename, filename+512, L'\0');
    OPENFILENAMEW ofn;
    //memset((void*)ofn, 0, sizeof(OPENFILENAMEW));
    ofn.hwndOwner = window.getSystemHandle();
    ofn.lpstrFilter = L"Bitmap BMP\0*.BMP\0\0";
    ofn.lpstrCustomFilter = NULL;
    ofn.nFilterIndex = 0;
    ofn.lpstrFile = filename;
    ofn.nMaxFile = 512;
    ofn.lpstrFileTitle = NULL;
    ofn.lpstrInitialDir = NULL;
    ofn.lpstrTitle = L"Ảnh lưu đâu đây?";
    ofn.Flags = OFN_DONTADDTORECENT + OFN_OVERWRITEPROMPT + OFN_PATHMUSTEXIST;
    ofn.lpstrDefExt = L"BMP\0";
    ofn.lStructSize = sizeof(OPENFILENAMEW);

    //Auto append.


    if (GetSaveFileNameW(&ofn)) {
        //ImageToBMP()
        ImageToBMP(img, filename);
    }
}
void ShapeToFile (int i) {
    ShapeToFile(shape[i]);
}

//Fill with white, util function
void DrawShape (const Shape &sh, sf::Color border) {
    //FOR ARCS, 12 DEGREE A POINT
    float degreePerPoint = 12.0;

    int n = (int)sh.x.size();
    sf::ConvexShape s;
    int ns = 0;
    //Calculate number of points
    for (int i=0; i<n; ++i)
        if (sh.e[i].isArc) {
            sf::Vector2f v1 (sh.x[i] - sh.e[i].xc, sh.y[i] - sh.e[i].yc),
                         v2 (sh.x[(i+1)%n] - sh.e[i].xc,sh.y[(i+1)%n] - sh.e[i].yc);
            float theta = angle(v1, v2);
            ns += (int)(abs(theta)/degreePerPoint);
        }
        else {
            ns ++;
        }
    //cerr<<ns<<" points set."<<endl;

    //Now add point to s
    s.setPointCount(ns);
    s.setFillColor(sf::Color::White);
    s.setOutlineColor(border);
    s.setOutlineThickness(2);
    int j=0;
    for (int i=0; i<n; ++i) {
        if (sh.e[i].isArc) {
            sf::Vector2f v1 (sh.x[i] - sh.e[i].xc, sh.y[i] - sh.e[i].yc),
                         v2 (sh.x[(i+1)%n] - sh.e[i].xc,sh.y[(i+1)%n] - sh.e[i].yc);
            //cerr<<"v1: "<<v1.x<<" "<<v1.y<<" - v2: "<<v2.x<<" "<<v2.y<<endl;
            float theta = angle(v1, v2);
            int numMidPoint = (int)(abs(theta)/degreePerPoint); //0..numMidPoint-1

            //cerr<<"#Mid points: "<<numMidPoint<<endl;

            float delta_theta = theta / numMidPoint;

            float t = 0.0;
            for (int k=0; k<numMidPoint; ++k) { //actually, it's k to numMidPoint-1, but it's ok.
                sf::Vector2f pnt = rotate(v1, t);
                //cerr<<"Vector #"<<j<<": "<<pnt.x<<" "<<pnt.y<<endl;
                //cerr<<"Point "<<j<<": "<<sh.e[i].xc+pnt.x<<" "<<sh.e[i].yc+pnt.y<<endl;
                s.setPoint(j++, {sh.e[i].xc+pnt.x, sh.e[i].yc+pnt.y});
                t = t + delta_theta;
                //cerr<<"t: "<<t<<endl;
            }
        }
        else {
            s.setPoint(j++, {1.0f*sh.x[i],1.0f*sh.y[i]});
            //cerr<<"Point "<<j<<": "<<sh.x[i]<<" "<<sh.y[i]<<endl;
        }
    }
    //Finalize
    window.draw(s);
}
//Fill with white
void DrawShape (int i, sf::Color border) {
    if (i<0 || i>=(int)shape.size()) return;
    DrawShape(shape[i], border);
}
void DrawPoint (int x, int y, sf::Color col) {
    //Radius of the point: equal to kNearRadius
    /* O is (x,y)
         X
        XXX
       XXOXX
        XXX
         X
    Example above: near radius = 2*/
    sf::CircleShape point (1.0*4);
    point.setFillColor(col);
    point.setPosition(1.0*x, 1.0*y);
    point.setOrigin(point.getRadius(), point.getRadius());
    window.draw(point);
}
void MoveShape (Shape &sh, int deltaX, int deltaY) {
    int n = (int)sh.e.size();
    for (int i=0; i<n; ++i) {
        if (sh.e[i].isArc) {
            sh.e[i].xc += deltaX;
            sh.e[i].yc += deltaY;
        }
        sh.x[i] += deltaX;
        sh.y[i] += deltaY;
    }
}
void MoveShape (int i, int deltaX, int deltaY) {
    MoveShape(shape[i], deltaX, deltaY);
}
sf::Vector2i ToBeSelectedPoint (int mousex, int mousey) {
    int mindsq = numeric_limits<int>::max();
    int v = -1;
    int n = (int)shape[selectingShape].e.size();
    const Shape &sh = shape[selectingShape];
    //Ưu tiên đỉnh: chọn đỉnh gần nhất coincide
    for (int i=0; i<n; ++i) 
        if (coincide(mousex, mousey, sh.x[i], sh.y[i])) {
            if (dsq_(mousex, mousey, sh.x[i], sh.y[i]) < mindsq) {
                mindsq = dsq_(mousex, mousey, sh.x[i], sh.y[i]);
                v = i;
            }
        }
    if (v == -1) {//Nếu không có đỉnh nào
        sf::Vector2f pntf = nearest_point(sh, 1.f*mousex, 1.f*mousey);
        sf::Vector2i pnt = {(int)round(pntf.x), (int)round(pntf.y)};

        //cerr<<"Nearest: "<<pnt.x<<" "<<pnt.y<<endl;

        sf::Vector2i ans = {-1,-1}; mindsq = numeric_limits<int>::max();
        if (coincide(mousex, mousey, pnt.x, pnt.y)) {
            if (dsq_(mousex, mousey, pnt.x, pnt.y) < mindsq) {
                mindsq = dsq_(mousex, mousey, pnt.x, pnt.y);
                ans.x = pnt.x; ans.y = pnt.y;
            }
        }
        return ans;
    }
    else {
        return {sh.x[v], sh.y[v]};
    }
}