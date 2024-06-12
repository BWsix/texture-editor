#pragma once

// windows support
#define _USE_MATH_DEFINES
#include <cmath>
#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>

#include "utils/includes.h"
#include "utils/shader.h"
#include <set>

#include "nlohmann/json.hpp"
using json = nlohmann::json;

using I2F = std::map<std::tuple<int, int, int>, int>;

struct MyTraits : OpenMesh::DefaultTraits {
    // Define vertex and normal as double
    using Point = OpenMesh::Vec3d;
    using Normal = OpenMesh::Vec3d;

    // Add normal property to vertices and faces
    VertexAttributes(OpenMesh::Attributes::Normal);
    FaceAttributes(OpenMesh::Attributes::Normal);
};

struct MyVertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords = {0, 0};

    static MyVertex Load(const json& j) {
        return {
            glm::vec3(
                j["p"][0].is_null() ? 0.0 : float(j["p"][0]),
                j["p"][1].is_null() ? 0.0 : float(j["p"][1]),
                j["p"][2].is_null() ? 0.0 : float(j["p"][2])
            ),
            glm::vec3(
                j["n"][0].is_null() ? 0.0 : float(j["n"][0]),
                j["n"][1].is_null() ? 0.0 : float(j["n"][1]),
                j["n"][2].is_null() ? 0.0 : float(j["n"][2])
            ),
            glm::vec2(
                j["t"][0].is_null() ? 0.0 : float(j["t"][0]),
                j["t"][1].is_null() ? 0.0 : float(j["t"][1])
            ),
        };
    }

    json serialize() const {
        json j;
        j["p"] = json::array({position.x, position.y, position.z});
        j["n"] = json::array({normal.x, normal.y, normal.z});
        j["t"] = json::array({texCoords.x, texCoords.y});
        return j;
    }
};

class MyMesh : public OpenMesh::TriMesh_ArrayKernelT<MyTraits> {
    std::vector<MyVertex> my_vertices;
    std::vector<uint> my_indices;
    std::vector<uint> my_indices_original;

    GLuint vao;
    GLuint vbo;
    GLuint ebo;

public:
    std::string name;
    std::string texture_id;

    float scale = 1.0;
    bool flip_horizontally = false;
    bool flip_vertically = false;

    OpenMesh::FPropHandleT<bool> selected;
    OpenMesh::EPropHandleT<float> weight;
    OpenMesh::VPropHandleT<glm::vec2> UV;
    OpenMesh::VPropHandleT<float> W;
    OpenMesh::VPropHandleT<size_t> idx;
    OpenMesh::HPropHandleT<size_t> face_idx;

    MyMesh() {
        add_property(selected);
        add_property(weight);
        add_property(UV);
        add_property(W);
        add_property(idx);
        add_property(face_idx);
    }

    MyMesh(std::string texture_id, std::string name, float scale) : name(name), texture_id(texture_id), scale(scale) {}

    static MyMesh Load(const json& j) {
        MyMesh m(j["texture_id"], j["name"], (j["scale"].is_number() ? float(j["scale"]) : 1.0));
        std::vector<MyVertex> my_vertices;
        for (const auto& v : j["vertices"]) {
            my_vertices.push_back(MyVertex::Load(v));
        }
        m.loadVertices(my_vertices, j["indices"], j["original_indices"]);

        if (j.find("fv") != j.end()) m.flip_vertically = j["fv"];
        if (j.find("fh") != j.end()) m.flip_horizontally = j["fh"];

        return m;
    }

    void select(uint face_id) { property(selected, face_handle(face_id)) = true; }
    void reset() { std::for_each(faces_begin(), faces_end(), [this](auto f){ property(selected, f) = false; }); }

    bool loadFromFile(std::string filename);
    void loadVertices(std::vector<MyVertex> vertices, std::vector<GLuint> indices, std::vector<GLuint> original_indices);
    void setup(I2F& i2f, bool is_main_mesh = false);
    void calcWeight();

    json serialize() const {
        json j;
        j["texture_id"] = texture_id;
        j["name"] = name;
        j["scale"] = scale;

        if (flip_horizontally) j["fh"] = true;
        if (flip_vertically) j["fv"] = true;

        j["vertices"] = json::array();
        for (const auto& v : my_vertices) {
            j["vertices"].push_back(v.serialize());
        }

        j["indices"] = my_indices;
        j["original_indices"] = my_indices_original;
        return j;
    }

    void render(Shader &shader) const;
    void highlightEdges(Shader &shader, const std::vector<int> &vertex_id) const;
    void highlightPoints(Shader &shader, const std::vector<int> &vertex_id, glm::vec3 color) const;
    void highlightFaces(Shader &shader, const std::set<size_t> &face_ids, glm::vec3 color) const;

    std::vector<VertexHandle> getEdgePoints() {
        std::set<VertexHandle> edge_points;
        for (auto& e : edges()) {
            if (e.is_boundary()) {
                edge_points.insert(e.v0());
                edge_points.insert(e.v1());
            }
        }
        return std::vector<VertexHandle>(edge_points.begin(), edge_points.end());
    }

    VertexHandle getClosestEdgePoint(glm::vec3 wp) {
        auto vhs = getEdgePoints();

        std::map<int, float> distance;
        for (const auto& vh : vhs) {
            distance[vh.idx()] = glm::length(wp - d2f(point(vh)));
        }

        std::partial_sort(vhs.begin(), vhs.begin() + 1, vhs.end(), [&distance](VertexHandle& vh1, VertexHandle& vh2){
            return distance[vh1.idx()] < distance[vh2.idx()];
        });

        return vhs[0];
    }

    std::vector<VertexHandle> getInteriorPoints() {
        auto edges = getEdgePoints();
        std::set<VertexHandle> edge_set(edges.begin(), edges.end());
        auto total = getPoints();

        std::vector<VertexHandle> vhs;
        for (auto vh : total) {
            if (edge_set.count(vh) == 0) {
                vhs.push_back(vh);
            }
        }
        return vhs;
    }

    std::vector<GLuint> getFaceIds(I2F& i2f) {
        std::vector<GLuint> face_ids;

        for (size_t i = 0; i < my_indices.size(); i += 3) {
            GLuint a = my_indices_original[i];
            GLuint b = my_indices_original[i + 1];
            GLuint c = my_indices_original[i + 2];
            face_ids.push_back(i2f[{a, b, c}]);
        }

        return face_ids;
    }

    std::vector<FaceHandle> getFaces() {
        std::vector<FaceHandle> fhs;
        for (auto fh : faces()) {
            fhs.push_back(fh);
        }
        return fhs;
    }

    std::vector<VertexHandle> getPoints() {
        std::set<VertexHandle> edge_points;
        for (auto e : edges()) {
            edge_points.insert(e.v0());
            edge_points.insert(e.v1());
        }
        return std::vector<VertexHandle>(edge_points.begin(), edge_points.end());
    }

    void updateUV(const std::vector<VertexHandle>& vhs) {
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        for (const auto& vh : vhs) {
            MyVertex v = {d2f(point(vh)), d2f(normal(vh)), property(UV, vh)};
            glBufferSubData(GL_ARRAY_BUFFER, vh.idx() * sizeof(v), sizeof(v), &v);
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    MyVertex getMyVertex(VertexHandle vh) {
        return {d2f(point(vh)), d2f(normal(vh)), property(UV, vh)};
    }

    void updateUV() {
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        for (const auto& vh : getPoints()) {
            MyVertex v = {d2f(point(vh)), d2f(normal(vh)), property(UV, vh)};
            glBufferSubData(GL_ARRAY_BUFFER, vh.idx() * sizeof(v), sizeof(v), &v);
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    void renderUV(Shader shader);

    std::vector<GLuint> getIndices()  {
        std::vector<GLuint> indices;
        for (const auto& fh : getFaces()) {
            auto it = this->cfv_ccwbegin(fh);
            indices.push_back(it->idx());
            ++it;
            indices.push_back(it->idx());
            ++it;
            indices.push_back(it->idx());
        }
        return indices;
    }

    void copy_vertices(MyMesh& other) {
        for (auto it : this->vertices()) {
            other.add_vertex(point(it));
        }
    }

    glm::vec3 d2f(OpenMesh::Vec3d v) const { return glm::vec3(v[0], v[1], v[2]); }

    // halfedge, face, and vertex normals
    OpenMesh::Vec3d normal(const HalfedgeHandle he) const;
    OpenMesh::Vec3d normal(const EdgeHandle e) const;
    OpenMesh::Vec3d normal(const FaceHandle f) const;
    OpenMesh::Vec3d normal(const VertexHandle v) const;
};
