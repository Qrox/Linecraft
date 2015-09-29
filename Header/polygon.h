#ifndef __LINECRAFT_POLYGON_H__
#define __LINECRAFT_POLYGON_H__

#include <vector>
#include "types.h"
#include "vec.h"

class polygon2d {
public:
    enum class relation {
        unknown,
        inside,
        outside,
        intersects,
        contains,
    };

    typedef vec<float, 2> vertex;

private:
    vertex * vertices;
    u32 cnt;

    static i32 winding(vertex const & a, vertex const & b, vertex const & v);
    static bool intersects(vertex const & a, vertex const & b, vertex const & c, vertex const & d, float * uo = nullptr, float * vo = nullptr, vertex * vert = nullptr);

public:
    polygon2d(vertex const * vs, u32 cnt);

    bool contains(vertex const & v) const;
    relation relationto(polygon2d const & b) const;
    std::vector<polygon2d> operator +(polygon2d const & b) const;

    u32 vertexcount() const;
    vertex & operator [](u32 ind) const;
};

#endif
