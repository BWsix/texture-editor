#pragma once

#include "imgui.h"
#include "utils/includes.h"
#include "utils/shader.h"
#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
#include <set>

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
    uint texID = 0;
};

class MyMesh : public OpenMesh::TriMesh_ArrayKernelT<MyTraits> {
    std::vector<MyVertex> my_vertices;
    std::vector<uint> my_indices;

    GLuint vao;
    GLuint vbo;
    GLuint ebo;

public:
    OpenMesh::FPropHandleT<bool> selected;
    OpenMesh::EPropHandleT<float> weight;

    OpenMesh::VPropHandleT<glm::vec2> UV;
    OpenMesh::VPropHandleT<float> W;
    OpenMesh::VPropHandleT<size_t> idx;

    MyMesh() {
        add_property(selected);
        add_property(weight);
        add_property(UV);
        add_property(W);
        add_property(idx);
    }

    void select(uint face_id) { property(selected, face_handle(face_id)) = true; }
    void reset() { std::for_each(faces_begin(), faces_end(), [this](auto f){ property(selected, f) = false; }); }

    bool loadFromFile(std::string filename);
    void setup();
    void calcWeight();
    void updateVertexData(uint vertexID);

    void render(Shader &shader) const;
    void highlightEdges(Shader &shader, const std::vector<int> &vertex_id) const;
    void highlightPoints(Shader &shader, const std::vector<int> &vertex_id) const;
    void highlightFaces(Shader &shader, const std::set<int> &face_ids) const;

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

    VertexHandle getClosestPoint(uint faceID, ImVec2 pos);

    void update() {
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        for (auto vh : getPoints()) {
            MyVertex v = {d2f(point(vh)), d2f(normal(vh)), property(UV, vh)};
            glBufferSubData(GL_ARRAY_BUFFER, vh.idx() * sizeof(v), sizeof(v), &v);
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    void renderUV(Shader shader);

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

