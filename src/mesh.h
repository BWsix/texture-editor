#include "utils/includes.h"
#include "utils/shader.h"
#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>

struct MyTraits : OpenMesh::DefaultTraits {
    // Define vertex and normal as double
    using Point = OpenMesh::Vec3d;
    using Normal = OpenMesh::Vec3d;

    // Add normal property to vertices and faces
    VertexAttributes(OpenMesh::Attributes::Normal);
    FaceAttributes(OpenMesh::Attributes::Normal);
};

struct MyVertex {
    uint faceID;
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords = {0, 0};
    uint texID = 0;
};

class MyMesh : public OpenMesh::TriMesh_ArrayKernelT<MyTraits> {
    GLuint VAO;
    GLuint VAO_POSITIONS;
    GLuint VBO;
    GLuint VBO_POSITIONS;

public:
    bool loadFromFile(std::string filename);
    void setup();

    void render(Shader &shader) const;
    void highlight(Shader &shader, const std::vector<uint> &indicies) const;

    glm::vec3 d2f(OpenMesh::Vec3d v) { return glm::vec3(v[0], v[1], v[2]); }

private:
    // halfedge, face, and vertex normals
    OpenMesh::Vec3d normal(const HalfedgeHandle he) const;
    OpenMesh::Vec3d normal(const EdgeHandle e) const;
    OpenMesh::Vec3d normal(const FaceHandle f) const;
    OpenMesh::Vec3d normal(const VertexHandle v) const;
};