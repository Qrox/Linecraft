#include "gl/glew.h"
#include "ui.h"
#include "graphics.h"
#include "polygon.h"
#include <vector>
#include <iostream>

using namespace std;
using namespace ui;

polygon2d::vertex pvs1[] = {
    polygon2d::vertex(37, 97),
    polygon2d::vertex(158, 189),
    polygon2d::vertex(118, 35),
    polygon2d::vertex(128, 146),
};

polygon2d::vertex pvs2[] = {
    polygon2d::vertex(149, 63),
    polygon2d::vertex(43, 189),
    polygon2d::vertex(152, 98),
    polygon2d::vertex(263, 186),
};

class testpanel : public component {
private:
    typedef polygon2d::vertex vertex;

    static float constexpr ps = 5, sqrps = ps * ps;

    polygon2d poly1, poly2;
    vertex cursor;

public:
    testpanel() : poly1(pvs1, 4), poly2(pvs2, 4), cursor(0, 0) {
    }

    void paint(opengl & gl) {
        component::paint(gl);
        gl.mode_2d();
        glColor3f(1, 0, 0);
        glBegin(GL_LINE_LOOP);
        for (u32 i = 0; i < poly1.vertexcount(); ++i) {
            glVertex2fv(poly1[i]);
        }
        glEnd();
        glColor3f(0, 0, 1);
        glBegin(GL_LINE_LOOP);
        for (u32 i = 0; i < poly2.vertexcount(); ++i) {
            glVertex2fv(poly2[i]);
        }
        glEnd();
        auto polys = poly1 + poly2;
        for (auto i = polys.begin(); i != polys.end(); ++i) {
            glBegin(GL_LINE_LOOP);
            for (u32 j = 0; j < i->vertexcount(); ++j) {
                glColor3f(1, 0, 1);
                glVertex2fv((*i)[j]);
            }
            glEnd();
        }

        glPointSize(ps);
        glBegin(GL_POINTS);
        glColor3f(1, 1, 0);
        bool get = false;
        for (u32 i = 0; i < poly1.vertexcount(); ++i) {
            if ((poly1[i] - cursor).sqr() < sqrps) {
                glVertex2fv(poly1[i]);
                get = true;
                break;
            }
        }
        if (!get) for (u32 i = 0; i < poly2.vertexcount(); ++i) {
            if ((poly2[i] - cursor).sqr() < sqrps) {
                glVertex2fv(poly2[i]);
                break;
            }
        }
        glEnd();

        glColor3f(0, 1, 1);
        switch (poly1.relationto(poly2)) {
        case polygon2d::relation::unknown:
            gl.drawText("unknown", 20, false, h_align::left, v_align::top);
            break;
        case polygon2d::relation::inside:
            gl.drawText("inside", 20, false, h_align::left, v_align::top);
            break;
        case polygon2d::relation::outside:
            gl.drawText("outside", 20, false, h_align::left, v_align::top);
            break;
        case polygon2d::relation::intersects:
            gl.drawText("intersects", 20, false, h_align::left, v_align::top);
            break;
        case polygon2d::relation::contains:
            gl.drawText("contains", 20, false, h_align::left, v_align::top);
            break;
        }
    }

    void onKeyDown(int x, int y, vector<vk> const & keystroke) {
        if (keystroke.size() == 2) {
            vk k1 = keystroke[0], k2 = keystroke[1];
            if ((k1 == vk::ctrl && k2 == 'S') ||
                (k2 == vk::ctrl && k1 == 'S')) {
                cout << "polygon2d::vertex pvs1[] = {" << endl;
                for (u32 i = 0; i < poly1.vertexcount(); ++i) {
                    cout << "    polygon2d::vertex(" << poly1[i].x() << ", " << poly1[i].y() << ")," << endl;
                }
                cout << "};" << endl << endl;

                cout << "polygon2d::vertex pvs2[] = {" << endl;
                for (u32 i = 0; i < poly2.vertexcount(); ++i) {
                    cout << "    polygon2d::vertex(" << poly2[i].x() << ", " << poly2[i].y() << ")," << endl;
                }
                cout << "};" << endl << endl;
            }
        }
    }

    void onMouseMove(int oldx, int oldy, int newx, int newy, std::vector<vk> const & keystate) {
        if (keystate.size() == 1) {
            u32 key = keystate[0];
            switch (key) {
            case vk::mouseleft:
                vertex old(oldx, oldy);
                bool get = false;
                for (u32 i = 0; i < poly1.vertexcount(); ++i) {
                    if ((poly1[i] - old).sqr() < sqrps) {
                        get = true;
                        poly1[i] = vertex(newx, newy);
                        break;
                    }
                }
                if (!get) for (u32 i = 0; i < poly2.vertexcount(); ++i) {
                    if ((poly2[i] - old).sqr() < sqrps) {
                        poly2[i] = vertex(newx, newy);
                        break;
                    }
                }
                break;
            }
        }
        cursor = vertex(newx, newy);
        frame::repaint();
    }
};

int main() {
    frame frm("Linecraft");
    testpanel pnl;
    frm.setContent(&pnl);
    frm.setVisible(true);
    frame::startMessageLoop();
    return 0;
}
