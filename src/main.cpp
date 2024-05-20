#include "beamlib/blib.h"
#include "glad/gl.h"
#include "imgui.h"
#include "mesh.h"
#include "screen.h"
#include "utils/camera.h"
#include "utils/mouse.h"
#include "utils/shader.h"
#include <map>
#include <set>
#include <Eigen/Dense>

Screen main_screen(1920, 1080);
Screen child_screen(500, 500);

std::set<uint> selected_face_ids;
std::set<uint> selected_vertex_ids;
uint selected_vertex_id;
MyMesh main_mesh;
MyMesh selected_mesh;

void reset() {
    selected_mesh.clean();
    main_mesh.copy_vertices(selected_mesh);
    selected_face_ids.clear();
    selected_vertex_ids.clear();
}

void build(uint face_id) {
    if (selected_face_ids.count(face_id)) {
        return;
    }
    selected_face_ids.insert(face_id);

    std::vector<MyMesh::VertexHandle> vhs;
    auto f = main_mesh.face_handle(face_id);

    auto it = main_mesh.cfv_ccwbegin(f);
    vhs.push_back(selected_mesh.vertex_handle(it->idx()));
    ++it;
    vhs.push_back(selected_mesh.vertex_handle(it->idx()));
    ++it;
    vhs.push_back(selected_mesh.vertex_handle(it->idx()));

    selected_mesh.add_face(vhs);
    selected_mesh.calcWeight();
}

glm::vec2 getUV(float progress) {
    progress *= 4;
    if (progress < 1) return glm::vec2(0, 1) * progress;
    progress -= 1;
    if (progress < 1) return glm::vec2(0, 1) + glm::vec2(1, 0) * progress;
    progress -= 1;
    if (progress < 1) return glm::vec2(1, 1) + glm::vec2(0, -1) * progress;
    progress -= 1;
    if (progress < 1) return glm::vec2(1, 0) + glm::vec2(-1, 0) * progress;
    return glm::vec2(0, 0);
}

void solve() {
    OpenMesh::SmartHalfedgeHandle first_heh;
    for (auto heh : selected_mesh.halfedges()) {
        if (heh.is_boundary()) {
            first_heh = heh;

            if (heh.from().idx() == (int)selected_vertex_id) {
                break;
            }
        }
    }

    float total_length = (selected_mesh.point(first_heh.from()) - selected_mesh.point(first_heh.to())).length();
    for (auto heh = first_heh.next(); heh != first_heh; heh = heh.next()) {
        total_length += (selected_mesh.point(heh.from()) - selected_mesh.point(heh.to())).length();
    }

    float accum = (selected_mesh.point(first_heh.from()) - selected_mesh.point(first_heh.to())).length();
    selected_mesh.property(selected_mesh.UV, first_heh.from()) = glm::vec2(0, 0);
    selected_mesh.property(selected_mesh.UV, first_heh.to()) = getUV(accum / total_length);
    for (auto heh = first_heh.next(); heh != first_heh; heh = heh.next()) {
        float length = (selected_mesh.point(heh.from()) - selected_mesh.point(heh.to())).length();
        accum += length;
        selected_mesh.property(selected_mesh.UV, heh.to()) = getUV(accum / total_length);
    }

    // Solving the shites

    auto interior_points = selected_mesh.getInteriorPoints();
    auto edge_points = selected_mesh.getEdgePoints();

    Eigen::MatrixXf A(interior_points.size(), interior_points.size());
    A.setZero();

    Eigen::VectorXf bx(interior_points.size());
    bx.setZero();
    Eigen::VectorXf by(interior_points.size());
    by.setZero();

    std::map<int, MyMesh::VertexHandle> idx_to_vh;
    for (size_t i = 0; i < interior_points.size(); i++) {
        auto& vh = interior_points[i];
        idx_to_vh[i] = vh;
        selected_mesh.property(selected_mesh.idx, vh) = i;
        A(i, i) = selected_mesh.property(selected_mesh.W, vh);
    }
    for (size_t i = 0; i < edge_points.size(); i++) {
        auto vh = edge_points[i];
        int idx = i + interior_points.size();
        idx_to_vh[idx] = vh;
        selected_mesh.property(selected_mesh.idx, vh) = idx;
    }

    for (int i = 0; i < interior_points.size(); i++) {
        auto vh = interior_points[i];

        for (auto it = selected_mesh.voh_iter(vh); it != selected_mesh.voh_end(vh); ++it) {
            auto to = it->to();
            auto eh = it->edge();
            size_t idx = selected_mesh.property(selected_mesh.idx, to);
            float weight = selected_mesh.property(selected_mesh.weight, eh);
            glm::vec2 uv = selected_mesh.property(selected_mesh.UV, to);
            if (idx < interior_points.size()) {
                A(i, idx) = -weight;
            } else {
                bx[i] += weight * uv.x;
                by[i] += weight * uv.y;
            }
        }
    }

    bx = (A.transpose() * A).ldlt().solve(A.transpose() * bx);
    by = (A.transpose() * A).ldlt().solve(A.transpose() * by);

    for (size_t i = 0; i < interior_points.size(); i++) {
        auto vh = interior_points[i];
        selected_mesh.property(selected_mesh.UV, vh).x = bx[i];
        selected_mesh.property(selected_mesh.UV, vh).y = by[i];
    }
}

void highlight(Shader shader) {
    // std::vector<int> indices;
    // for (auto vh : selected_mesh.getEdgePoints()) {
    //     indices.push_back(vh.idx());
    // }
    //
    // shader.use();
    // shader.setVec3("color", {0.3, 1.0, 0.3});
    // glPointSize(20);
    // main_mesh.highlightPoints(shader, indices);

    // for (auto vh : selected_mesh.getEdgePoints()) {
    //     auto uv = selected_mesh.property(selected_mesh.UV, vh);
    //     shader.use();
    //     glPointSize(20);
    //     shader.setVec3("color", {uv[0], uv[1], uv[2]});
    //     main_mesh.highlightPoints(shader, {vh.idx()});
    // }

    shader.use();
    glPointSize(20);
    main_mesh.highlightPoints(shader, {(int)selected_vertex_id});

#if 0
    std::set<int> edge_points;
    std::set<int> interior_points;

    std::vector<int> indices;
    for (auto e : selected_mesh.edges()) {
        if (e.is_boundary()) {
            indices.push_back(e.v0().idx());
            indices.push_back(e.v1().idx());

            edge_points.insert(e.v0().idx());
            edge_points.insert(e.v1().idx());
        } else {
            interior_points.insert(e.v0().idx());
            interior_points.insert(e.v1().idx());
        }
    }

    glLineWidth(20);
    main_mesh.highlightEdges(shader, indices);

    indices.clear();
    std::set_difference(interior_points.begin(), interior_points.end(), edge_points.begin(), edge_points.end(),std::inserter(indices, indices.end()));
    shader.setVec3("color", {0.3, 1.0, 0.3});
    glPointSize(20);
    main_mesh.highlightPoints(shader, indices);

    for (auto idx : edge_points) {
        auto uv = selected_mesh.property(selected_mesh.uv, selected_mesh.vertex_handle(idx));
        shader.use();
        glPointSize(20);
        shader.setVec3("color", {uv[0], uv[1], uv[2]});
        main_mesh.highlightPoints(shader, {idx});
    }

    shader.setVec3("color", {0.3, 0.3, 1.0});
    glPointSize(20);
    main_mesh.highlightPoints(shader, std::vector<int>(edge_points.begin(), edge_points.end()));

    if (indices.empty()) {
        return;
    }

    std::vector<int> interior_edges_indices;
    for (auto p : indices) {
        auto vh = main_mesh.vertex_handle(p);
        for (auto eh = main_mesh.ve_begin(vh); eh != main_mesh.ve_end(vh); ++eh) {
            interior_edges_indices.push_back(eh->v0().idx());
            interior_edges_indices.push_back(eh->v1().idx());
        }
    }
    shader.setVec3("color", {1.0, 1.0, 0.3});
    main_mesh.highlightEdges(shader, interior_edges_indices);
#endif
}

int main() {
    auto *const window = Blib::CreateWindow("texture editor", 1920, 1080);

    glfwSetWindowSizeCallback(window, [](GLFWwindow *, int width, int height) {
        main_screen.resize(width, height);
        camera.setAspect((float)width / height);
        glViewport(0, 0, width, height < 1 ? 1 : height);
    });

    glfwSetScrollCallback(window, [](GLFWwindow *window, double x, double y) {
        (void)window; (void)x;
        camera.offsetRadius(-y * 0.1);
    });

    struct {
        Shader wireframe = Shader::From("shaders/wireframe.vert.glsl", "shaders/wireframe.frag.glsl");
        Shader highlight = Shader::From("shaders/wireframe.vert.glsl", "shaders/highlight.frag.glsl");
        Shader screen = Shader::From("shaders/screen.vert.glsl", "shaders/screen.frag.glsl");
    } programs;

    if (!main_mesh.loadFromFile("models/dragon.obj")) {
        exit(1);
    }
    main_mesh.setup();
    main_mesh.copy_vertices(selected_mesh);
    selected_mesh.setup();

    main_screen.loadResources();
    child_screen.loadResources();

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        Blib::BeginUI();
        mouse.update(window);
        camera.update(window);

        main_screen.bind();
        glViewport(0, 0, 1920, 1080);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        programs.wireframe.use();
        programs.wireframe.setMat4("model", glm::mat4(1));
        main_mesh.render(programs.wireframe);

        main_screen.updateIds();
        auto pos = ImGui::GetMousePos();
        uint faceID;
        if (pos.x <= 0 || pos.y <= 0) {
            faceID = 0;
        } else {
            faceID = main_screen.getFaceId(pos);
        }
        if (ImGui::IsKeyDown(ImGuiKey_MouseLeft) && faceID != 0) {
            faceID -= 1;
            selected_vertex_id = main_mesh.getClosestPoint(faceID, pos).idx();
            build(faceID);
            solve();
        }
        if (ImGui::IsKeyDown(ImGuiKey_MouseRight)) {
            reset();
        }

        programs.highlight.use();
        programs.highlight.setMat4("model", glm::mat4(1));
        programs.highlight.setVec3("color", {1.0, 0.3, 0.3});
        highlight(programs.highlight);

        selected_mesh.update();
        selected_mesh.renderUV(programs.highlight);

        main_screen.unbind();
        glViewport(0, 0, 1920, 1080);
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        main_screen.render(programs.screen);

        Blib::EndUI();
        glfwSwapBuffers(window);
    }

    Blib::DestroyWindow(window);

    return 0;
}
