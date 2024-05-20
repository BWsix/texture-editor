// windows support
#define _USE_MATH_DEFINES
#include <cmath>
#include "OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh"

#include "mesh.h"
#include "utils/camera.h"

bool MyMesh::loadFromFile(std::string filename) {
    OpenMesh::IO::Options opt = OpenMesh::IO::Options::VertexNormal;
    bool isRead = OpenMesh::IO::read_mesh(*this, filename, opt);
    if (isRead) {
        // If the file did not provide vertex normals and mesh has vertex normal,
        // then calculate them
        if (!opt.check(OpenMesh::IO::Options::VertexNormal) && this->has_vertex_normals()) {
            this->update_normals();
        }
    }
    return isRead;
}

void MyMesh::setup() {
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    for (VertexHandle vh : this->vertices()) {
        my_vertices.push_back({d2f(point(vh)), d2f(normal(vh))});
    }

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, my_vertices.size() * sizeof(my_vertices[0]), my_vertices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(MyVertex), (void *)offsetof(MyVertex, position));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(MyVertex), (void *)offsetof(MyVertex, normal));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(MyVertex), (void *)offsetof(MyVertex, texCoords));
    glEnableVertexAttribArray(3);
    glVertexAttribIPointer(3, 1, GL_UNSIGNED_INT, sizeof(MyVertex), (void *)offsetof(MyVertex, texID));

    for (FaceHandle f : this->faces()) {
        auto it = this->cfv_ccwbegin(f);
        my_indices.push_back(it->idx());
        ++it;
        my_indices.push_back(it->idx());
        ++it;
        my_indices.push_back(it->idx());
    }

    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, my_indices.size() * sizeof(my_indices[0]), my_indices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glBindVertexArray(0);
}

void MyMesh::updateVertexData(GLuint vertexID) {
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    //TODO
}

void MyMesh::render(Shader &shader) const {
    shader.use();
    shader.setMat4("view", camera.getViewMatrix());
    shader.setMat4("projection", camera.getProjectionMatrix());

    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glDrawElements(GL_TRIANGLES, my_indices.size(), GL_UNSIGNED_INT, NULL);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void MyMesh::highlightEdges(Shader &shader, const std::vector<int> &vertex_id) const {
    shader.use();
    shader.setMat4("view", camera.getViewMatrix());
    shader.setMat4("projection", camera.getProjectionMatrix());

    glBindVertexArray(vao);
    glDrawElements(GL_LINES, vertex_id.size(), GL_UNSIGNED_INT, vertex_id.data());
    glBindVertexArray(0);
}

void MyMesh::highlightPoints(Shader &shader, const std::vector<int> &vertex_id) const {
    shader.use();
    shader.setMat4("view", camera.getViewMatrix());
    shader.setMat4("projection", camera.getProjectionMatrix());

    glBindVertexArray(vao);
    glDrawElements(GL_POINTS, vertex_id.size(), GL_UNSIGNED_INT, vertex_id.data());
    glBindVertexArray(0);
}

void MyMesh::highlightFaces(Shader &shader, const std::set<int> &face_ids) const {
    std::vector<uint> indices;
    for (uint id : face_ids) {
        auto f = this->face_handle(id);
        auto it = this->cfv_ccwbegin(f);
        VertexHandle first = *it;
        ++it;
        int face_triangles = this->valence(f) - 2;
        for (int i = 0; i < face_triangles; ++i) {
            indices.push_back(first.idx());
            indices.push_back(it->idx());
            ++it;
            indices.push_back(it->idx());
        }
    }

    shader.use();
    shader.setMat4("view", camera.getViewMatrix());
    shader.setMat4("projection", camera.getProjectionMatrix());

    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, indices.data());
    glBindVertexArray(0);
}

OpenMesh::Vec3d MyMesh::normal(const HalfedgeHandle he) const {
    const FaceHandle f = face_handle(he);
    if (f.is_valid()) {
        return normal(f);
    } else {
        return OpenMesh::Vec3d(0, 0, 0);
    }
}

OpenMesh::Vec3d MyMesh::normal(const EdgeHandle e) const {
    const HalfedgeHandle he0 = halfedge_handle(e, 0);
    const HalfedgeHandle he1 = halfedge_handle(e, 1);
    if (!is_boundary(he0) || !is_boundary(he1)) {
        // free edge, bad
        exit(1);
    }
    if (is_boundary(he0)) {
        return normal(face_handle(he1));
    } else if (is_boundary(he1)) {
        return normal(face_handle(he0));
    } else {
        return (normal(face_handle(he0)) + normal(face_handle(he1))).normalized();
    }
}

OpenMesh::Vec3d MyMesh::normal(const FaceHandle f) const {
    return OpenMesh::TriMesh_ArrayKernelT<MyTraits>::normal(f);
}

OpenMesh::Vec3d MyMesh::normal(const VertexHandle v) const {
    return OpenMesh::TriMesh_ArrayKernelT<MyTraits>::normal(v);
}

void MyMesh::calcWeight() {
    for (auto eh : edges()) {
        double w = 0.0;
        for (int i = 0; i < 2; i++) {
            auto heh0 = eh.halfedge(i).next();
            auto heh1 = heh0.next();
            auto v0 = point(heh0.from()) - point(heh0.to());
            auto v1 = point(heh1.to()) - point(heh1.from());
            double angle = acos(v0.normalized().dot(v1.normalized()));
            w += 1.0 / tan(angle);
        }
        property(weight, eh) = w;
    }

    for (auto vh : this->vertices()) {
        float w = 0;
        for (auto eh = ve_begin(vh); eh != ve_end(vh); ++eh) {
            w += property(weight, eh);
        }
        property(W, vh) = w;
    }
}

void MyMesh::renderUV(Shader shader) {
    std::vector<uint> indices;
    for (auto fh : getFaces()) {
        auto it = this->cfv_ccwbegin(fh);
        indices.push_back(it->idx());
        ++it;
        indices.push_back(it->idx());
        ++it;
        indices.push_back(it->idx());
    }

    shader.use();
    shader.setMat4("view", camera.getViewMatrix());
    shader.setMat4("projection", camera.getProjectionMatrix());
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, indices.data());
    glBindVertexArray(0);
}

MyMesh::VertexHandle MyMesh::getClosestPoint(uint faceID, ImVec2 pos) {
    GLint vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);
    glm::vec4 viewport(vp[0],vp[1],vp[2],vp[3]);

    int windowX = pos.x;
    int windowY = 1080 - pos.y;

    float d;
    glReadPixels(windowX, windowY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &d);

    glm::vec3 win(windowX, windowY, d);
    glm::vec3 wp = glm::unProject(win, camera.getViewMatrix(), camera.getProjectionMatrix(), viewport);

    std::vector<VertexHandle> vhs;
    auto it = this->cfv_ccwbegin(face_handle(faceID));
    vhs.push_back(*it);
    ++it;
    vhs.push_back(*it);
    ++it;
    vhs.push_back(*it);
    
    std::sort(vhs.begin(), vhs.end(), [this, wp](VertexHandle& vh1, VertexHandle& vh2){
        auto p1 = d2f(point(vh1));
        auto p2 = d2f(point(vh2));
        return glm::length(wp - p1) < glm::length(wp - p2);
    });

    return vhs[0];
}
