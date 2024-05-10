#include "mesh.h"
#include "OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh"
#include "utils/camera.h"

bool MyMesh::loadFromFile(std::string filename) {
    OpenMesh::IO::Options opt = OpenMesh::IO::Options::VertexNormal;
    bool isRead = OpenMesh::IO::read_mesh(*this, filename, opt);
    if (isRead) {
        // If the file did not provide vertex normals and mesh has vertex normal,
        // then calculate them
        if (!opt.check(OpenMesh::IO::Options::VertexNormal) &&
            this->has_vertex_normals()) {
            this->update_normals();
        }
    }
    return isRead;
}

void MyMesh::setup() {
    std::vector<glm::vec3> positions;
    for (VertexHandle vh : this->vertices()) {
        positions.push_back(d2f(point(vh)));
    }

    glGenVertexArrays(1, &vao_positions);
    glBindVertexArray(vao_positions);

    glGenBuffers(1, &vbo_positions);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_positions);
    glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3), &positions[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

    std::vector<MyVertex> vertices;
    for (FaceHandle f : this->faces()) {
        // this is basically a triangle fan for any face valence
        MyMesh::ConstFaceVertexCCWIter it = this->cfv_ccwbegin(f);
        VertexHandle first = *it;
        ++it;
        int face_triangles = this->valence(f) - 2;
        for (int i = 0; i < face_triangles; ++i) {
            vertices.push_back({(GLuint)f.idx() + 1, d2f(point(first)), d2f(normal(first))});
            vertices.push_back({(GLuint)f.idx() + 1, d2f(point(*it)), d2f(normal(*it))});
            ++it;
            vertices.push_back({(GLuint)f.idx() + 1, d2f(point(*it)), d2f(normal(*it))});
        }
    }

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(MyVertex), &vertices[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(MyVertex), (void *)offsetof(MyVertex, position));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(MyVertex), (void *)offsetof(MyVertex, normal));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(MyVertex), (void *)offsetof(MyVertex, texCoords));
    glEnableVertexAttribArray(3);
    glVertexAttribIPointer(3, 1, GL_UNSIGNED_INT, sizeof(MyVertex), (void *)offsetof(MyVertex, faceID));
    glEnableVertexAttribArray(4);
    glVertexAttribIPointer(4, 1, GL_UNSIGNED_INT, sizeof(MyVertex), (void *)offsetof(MyVertex, texID));

    glBindVertexArray(0);
}

void MyMesh::render(Shader &shader) const {
    shader.use();
    shader.setMat4("view", camera.getViewMatrix());
    shader.setMat4("projection", camera.getProjectionMatrix());

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, this->n_faces() * 3);
    glBindVertexArray(0);
}

void MyMesh::highlightFaces(Shader &shader, const std::set<uint> &face_ids) const {
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

    glBindVertexArray(vao_positions);
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
