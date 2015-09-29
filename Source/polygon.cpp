#include "polygon.h"
#include <set>
#include <map>

using namespace std;

polygon2d::polygon2d(vertex const * vs, u32 cnt) : vertices(new vertex[cnt]), cnt(cnt) {
    std::copy(vs, vs + cnt, vertices);
}

i32 polygon2d::winding(vertex const & a, vertex const & b, vertex const & v) {
    if ((b.y() >= v.y()) ^ (a.y() >= v.y())) {
        float diffy = b.y() - a.y();
        float crs = (a - v).cross(b - v);
        if (crs >= 0) {
            if (diffy > 0) return 1;
        } else {
            if (diffy < 0) return -1;
        }
    }
    return 0;
}

bool polygon2d::intersects(vertex const & a, vertex const & b, vertex const & c, vertex const & d, float * uo, float * vo, vertex * vert) {
    vertex ab = b - a, cd = d - c, ac = c - a;
    float denom = ab.cross(cd);
    float u = ac.cross(cd) / denom,
          v = ac.cross(ab) / denom;
    if (uo) *uo = u;
    if (vo) *vo = v;
    if (vert) *vert = a * (1 - u) + b * u;
    return 0 <= u && u <= 1 && 0 <= v && v <= 1;
}

bool polygon2d::contains(vertex const & v) const {
    i32 wndng = 0;
    for (u32 i = 0; i < cnt - 1; ++i) {
        wndng += winding(vertices[i], vertices[i + 1], v);
    }
    wndng += winding(vertices[cnt - 1], vertices[0], v);
    return wndng != 0;
}

polygon2d::relation polygon2d::relationto(polygon2d const & p2) const {
    bool hasin = false, hasout = false;
    for (u32 i = 0; i < cnt; ++i) {
        vertex & a = vertices[i];
        if (p2.contains(a)) {
            hasin = true;
            if (hasout) break;
        } else {
            hasout = true;
            if (hasin) break;
        }
        bool end = false;
        vertex & b = vertices[i == cnt - 1 ? 0 : i + 1];
        for (u32 j = 0; j < p2.cnt; ++j) {
            vertex & c = p2.vertices[j],
                    d = p2.vertices[j == p2.cnt - 1 ? 0 : j + 1];
            if (intersects(a, b, c, d)) {
                hasin = hasout = true;
                end = true;
                break;
            }
        }
        if (end) break;
    }
    if (hasin) {
        if (hasout) return relation::intersects;
        else return relation::inside;
    } else {
        if (hasout) {
            if (p2.cnt && contains(p2.vertices[0])) return relation::contains;
            else return relation::outside;
        } else return relation::unknown;
    }
}

vector<polygon2d> polygon2d::operator +(polygon2d const & n) const {
    vector<polygon2d> res;
    switch (relationto(n)) {
    case relation::unknown:
        res.push_back(*this);
        res.push_back(n);
        break;
    case relation::inside:
        res.push_back(n);
        break;
    case relation::outside:
        res.push_back(*this);
        res.push_back(n);
        break;
    case relation::intersects: {
        polygon2d const & m = *this;

        polygon2d const * polygon[2] = {&m, &n};
        u32 vertexcount[2] = {m.vertexcount(), n.vertexcount()};

        set<u32> outside[2];    // [0]: n's points outside m; [1]: m's points outside n
        for (u32 i = 0; i < vertexcount[1]; ++i)
            if (!m.contains(n[i])) outside[0].insert(i);
        for (u32 i = 0; i < vertexcount[0]; ++i)
            if (!n.contains(m[i])) outside[1].insert(i);

        struct intersection_info {
            float u;
            u32 ind;

            bool operator <(intersection_info const & b) const {
                return u < b.u;
            }
        };
        vector<intersection_info> * intersection[2];    // [0]: m's each side's intersection with n;
        intersection[0] = new vector<intersection_info>[vertexcount[0]];
        intersection[1] = new vector<intersection_info>[vertexcount[1]];
        for (u32 i = 0; i < vertexcount[0]; ++i) {
            u32 j = i + 1 == vertexcount[0] ? 0 : i + 1;
            for (u32 k = 0; k < vertexcount[1]; ++k) {
                u32 l = k + 1 == vertexcount[1] ? 0 : k + 1;
                float u, v;
                vertex inter;
                if (intersects(m[i], m[j], n[k], n[l], &u, &v, nullptr) && 0 < u && u < 1 && 0 < v && v < 1) {
                    intersection[0][i].push_back({u, k});
                    intersection[1][k].push_back({v, i});
                }
            }
        }
        for (u32 i = 0; i < vertexcount[0]; ++i) {
            std::sort(intersection[0][i].begin(), intersection[0][i].end());
        }
        for (u32 i = 0; i < vertexcount[1]; ++i) {
            std::sort(intersection[1][i].begin(), intersection[1][i].end());
        }

        map<u32, u32> * intersection_index[2];
        intersection_index[0] = new map<u32, u32>[vertexcount[0]];
        intersection_index[1] = new map<u32, u32>[vertexcount[1]];
        for (u32 i = 0; i < vertexcount[0]; ++i) {
            vector<intersection_info> & vec = intersection[0][i];
            u32 cnt = vec.size();
            for (u32 j = 0; j < cnt; ++j) {
                intersection_index[0][i].insert(std::pair<u32, u32>(vec[j].ind, j));
            }
        }
        for (u32 i = 0; i < vertexcount[1]; ++i) {
            vector<intersection_info> & vec = intersection[1][i];
            u32 cnt = vec.size();
            for (u32 j = 0; j < cnt; ++j) {
                intersection_index[1][i].insert(std::pair<u32, u32>(vec[j].ind, j));
            }
        }

        struct vertex_info {
            u32 mn; // m: 0, n: 1
            u32 ind;
        };
        auto next = [vertexcount] (vertex_info const & a) -> vertex_info {
            return {a.mn, a.ind + 1 == vertexcount[a.mn] ? 0 : a.ind + 1};
        };
        auto prev = [vertexcount] (vertex_info const & a) -> vertex_info {
            return {a.mn, a.ind == 0 ? vertexcount[a.mn] - 1 : a.ind - 1};
        };
        set<u32> remaining_outside[2] = outside;

        while (!remaining_outside[0].empty() || !remaining_outside[1].empty()) {
            vertex_info side_begin;
            if (!remaining_outside[0].empty()) side_begin = {1, *remaining_outside[0].begin()};
            else side_begin = {0, *remaining_outside[1].begin()};

            vertex_info curr_side = side_begin;  // current side's smaller id'ed vertex
            i32 curr_dir = +1, curr_int = -1; // current direction of the side; previous intersection that makes the current start point
            vector<vertex> poly;

            do {
                if (curr_int == -1) {
                    if (curr_dir == +1) {
                        if (!remaining_outside[!curr_side.mn].erase(curr_side.ind)) {
                            res.push_back(polygon2d(poly.data(), poly.size()));
                            return res;
                        }
                    } else {
                        vertex_info next_side = next(curr_side);
                        if (!remaining_outside[!next_side.mn].erase(next_side.ind)) {
                            res.push_back(polygon2d(poly.data(), poly.size()));
                            return res;
                        }
                    }
                }
                if (curr_int == -1) {
                    if (curr_dir == +1) {
                        poly.push_back((*polygon[curr_side.mn])[curr_side.ind]);
                    } else {
                        vertex_info next_side = next(curr_side);
                        poly.push_back((*polygon[next_side.mn])[next_side.ind]);
                    }
                } else {
                    vertex_info next_side = next(curr_side);
                    float u = intersection[curr_side.mn][curr_side.ind][intersection_index[curr_side.mn][curr_side.ind][curr_int]].u;
                    poly.push_back((*polygon[curr_side.mn])[curr_side.ind] * (1 - u) + (*polygon[next_side.mn])[next_side.ind] * u);
                }

                vector<intersection_info> & inters = intersection[curr_side.mn][curr_side.ind];
                map<u32, u32> & intinf = intersection_index[curr_side.mn][curr_side.ind];
                vertex_info next_side;
                i32 next_dir, next_int;
                if (curr_int == -1) {
                    if (curr_dir == +1) {
                        if (inters.empty()) {
                            next_side = next(curr_side);
                            next_dir = +1;
                            next_int = -1;
                        } else {
                            next_side = {!curr_side.mn, inters.front().ind};
                            if (polygon[curr_side.mn]->contains((*polygon[next_side.mn])[next_side.ind]) ^
                                    (intersection_index[next_side.mn][next_side.ind][curr_side.ind] & 1)) {
                                next_dir = +1;
                            } else next_dir = -1;
                            next_int = curr_side.ind;
                        }
                    } else {
                        if (inters.empty()) {
                            next_side = prev(curr_side);
                            next_dir = -1;
                            next_int = -1;
                        } else {
                            next_side = {!curr_side.mn, inters.back().ind};
                            if (polygon[curr_side.mn]->contains((*polygon[next_side.mn])[next_side.ind]) ^
                                    (intersection_index[next_side.mn][next_side.ind][curr_side.ind] & 1)) {
                                next_dir = +1;
                            } else next_dir = -1;
                            next_int = curr_side.ind;
                        }
                    }
                } else {
                    if (curr_dir == +1) {
                        if (intinf[curr_int] + 1 == inters.size()) {
                            next_side = next(curr_side);
                            next_dir = +1;
                            next_int = -1;
                        } else {
                            next_side = {!curr_side.mn, inters[intinf[curr_int] + 1].ind};
                            if (polygon[curr_side.mn]->contains((*polygon[next_side.mn])[next_side.ind]) ^
                                    (intersection_index[next_side.mn][next_side.ind][curr_side.ind] & 1)) {
                                next_dir = +1;
                            } else next_dir = -1;
                            next_int = curr_side.ind;
                        }
                    } else {
                        if (intinf[curr_int] == 0) {
                            next_side = prev(curr_side);
                            next_dir = -1;
                            next_int = -1;
                        } else {
                            next_side = {!curr_side.mn, inters[intinf[curr_int] - 1].ind};
                            if (polygon[curr_side.mn]->contains((*polygon[next_side.mn])[next_side.ind]) ^
                                    (intersection_index[next_side.mn][next_side.ind][curr_side.ind] & 1)) {
                                next_dir = +1;
                            } else next_dir = -1;
                            next_int = curr_side.ind;
                        }
                    }
                }
                curr_side = next_side;
                curr_dir = next_dir;
                curr_int = next_int;
            } while (curr_int != -1 || curr_dir != +1 || curr_side.mn != side_begin.mn || curr_side.ind != side_begin.ind);
            res.push_back(polygon2d(poly.data(), poly.size()));
        }
        delete [] intersection[0];
        delete [] intersection[1];
        delete [] intersection_index[0];
        delete [] intersection_index[1];
    }   break;
    case relation::contains:
        res.push_back(*this);
        break;
    }
    return res;
}

u32 polygon2d::vertexcount() const {
    return cnt;
}

polygon2d::vertex & polygon2d::operator[] (u32 ind) const {
    return vertices[ind];
}
