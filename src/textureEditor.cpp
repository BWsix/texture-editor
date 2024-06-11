#include "textureEditor.h"
#include "textures.h"
#include "utils/camera.h"
#include <fstream>
#include <iostream>
#include <map>
#include "nlohmann/json.hpp"
#include "utils/utils.h"
using json = nlohmann::json;

void TextureEditor::load(std::string path) {
    if (!main_mesh.loadFromFile(path)) {
        std::cerr << "TextureEditor fails to load " << path << "\n";
        exit(1);
    }
    main_mesh.setup();
    main_mesh.copy_vertices(selected_mesh);
    selected_mesh.setup();
}

void TextureEditor::setup() {
    main_screen.loadResources();
    child_screen.loadResources();

    programs.wireframe = Shader::From("shaders/wireframe.vert.glsl", "shaders/wireframe.frag.glsl");
    programs.highlight = Shader::From("shaders/wireframe.vert.glsl", "shaders/highlight.frag.glsl");
    programs.uv = Shader::From("shaders/wireframe.vert.glsl", "shaders/uv.frag.glsl");
    programs.screen = Shader::From("shaders/screen.vert.glsl", "shaders/screen.frag.glsl");
    programs.preview = Shader::From("shaders/preview.vert.glsl", "shaders/preview.frag.glsl");
    programs.face_selector = Shader::From("shaders/faceSelector.vert.glsl", "shaders/faceSelector.frag.glsl");
    programs.point = Shader::From("shaders/point.vert.glsl", "shaders/point.frag.glsl");
}

void TextureEditor::handleFaceSelector() {
    if (ImGui::GetIO().WantCaptureMouse) {
        return;
    }

    if (ImGui::IsKeyDown(ImGuiKey_MouseLeft)) {
        auto pos = ImGui::GetMousePos();
        size_t faceID = main_screen.getFaceId(pos);
        if (faceID == 0) {
            return;
        }
        faceID -= 1;

        if (ImGui::IsKeyDown(ImGuiKey_LeftShift)) {
            deleteFace(faceID);
        } else {
            addFace(faceID);
        }
        updateClosestEdgePointId();
        solveUV();
    }

    if (ImGui::IsKeyDown(ImGuiKey_F)) {
        updateClosestEdgePointId();
        solveUV();
    }
}

void TextureEditor::deleteFace(GLuint face_id) {
    if (selected_face_ids.count(face_id)) {
        selected_face_ids.erase(face_id);
    }
    added_face_ids.clear();
    selected_mesh.clear();
    main_mesh.copy_vertices(selected_mesh);
    acuallyAddingFace();
}

json TextureEditor::serialize() {
    json j;
    j["meshes"] = json::array();
    for (const auto& m : saved_meshes) {
        j["meshes"].push_back(m.serialize());
    }
    return j;
}

void TextureEditor::updateClosestEdgePointId() {
    if (added_face_ids.empty()) {
        return;
    }

    GLint vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);
    glm::vec4 viewport(vp[0],vp[1],vp[2],vp[3]);

    auto pos = ImGui::GetMousePos();
    int windowX = pos.x;
    int windowY = main_screen.height - pos.y;
    float d;
    glReadPixels(windowX, windowY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &d);

    glm::vec3 win(windowX, windowY, d);
    glm::vec3 wp = glm::unProject(win, camera.getViewMatrix(), camera.getProjectionMatrix(), viewport);
    closest_edge_point_id = selected_mesh.getClosestEdgePoint(wp).idx();
}

void TextureEditor::fill() {
    std::set<int> visited_boundaries;
    std::vector<std::pair<int, OpenMesh::SmartHalfedgeHandle>> boundary_length;

    for (const auto& _heh : selected_mesh.halfedges()) {
        if (!_heh.is_boundary() || visited_boundaries.count(_heh.idx())) {
            continue;
        }

        OpenMesh::SmartHalfedgeHandle first_heh = _heh;
        int length = 1;
        // std::cout << "new edge!\n  " << _heh.idx() << "\n";
        visited_boundaries.insert(_heh.idx());
        for (auto heh = first_heh.next(); heh != first_heh; heh = heh.next()) {
            length += 1;
            // std::cout << "  " << heh.idx() << "\n";
            visited_boundaries.insert(heh.idx());
        }

        boundary_length.push_back({length, first_heh});
    }

    std::sort(boundary_length.begin(), boundary_length.end());
    boundary_length.pop_back();

    for (auto [_, first_heh] : boundary_length) {
        addFace(selected_mesh.property(selected_mesh.face_idx, first_heh));
    }

    if (!boundary_length.empty()) {
        return fill();
    } else {
        return solveUV();
    }
}

void TextureEditor::reset() {
    selected_mesh.clear();
    main_mesh.copy_vertices(selected_mesh);
    selected_face_ids.clear();
    added_face_ids.clear();
}

std::set<size_t> TextureEditor::getFaceIdWithRadius(GLuint face_id) {
    std::vector<MyMesh::FaceHandle> selected;
    std::vector<MyMesh::FaceHandle> todo;
    selected.push_back(main_mesh.face_handle(face_id));

    std::set<size_t> selected_indices;
    for (int i = 0; i < configs.radius; i++) {
        for (size_t i = 0; i < selected.size(); i++) {
            auto fh = selected[i];
            selected_indices.insert(fh.idx());
            for (auto it = main_mesh.ff_begin(fh); it != main_mesh.ff_end(fh); ++it) {
                if (selected_indices.count(it->idx())) {
                    continue;
                }
                todo.push_back(*it);
            }
        }

        selected = todo;
        todo.clear();
    }

    return selected_indices;
}

void TextureEditor::addFace(GLuint face_id) {
    for (auto idx : getFaceIdWithRadius(face_id)) {
        selected_face_ids.insert(idx);
    }

    acuallyAddingFace();
}

void TextureEditor::acuallyAddingFace() {
    for (auto face_id : selected_face_ids) {
        if (added_face_ids.count(face_id)) {
            continue;
        }
        added_face_ids.insert(face_id);

        std::vector<MyMesh::VertexHandle> vhs;
        auto main_fh = main_mesh.face_handle(face_id);

        for (auto it = main_mesh.fv_ccwbegin(main_fh); it != main_mesh.fv_ccwend(main_fh); ++it) {
            vhs.push_back(selected_mesh.vertex_handle(it->idx()));
        }

        auto selected_fh = selected_mesh.add_face(vhs);
        auto selected_heh = *selected_mesh.fh_begin(selected_fh);
        auto main_heh = *main_mesh.fh_begin(main_fh);

        for (int i = 0; i < 3; i++) {
            selected_mesh.property(selected_mesh.face_idx, selected_heh.opp()) = main_heh.opp().face().idx();

            main_heh = main_heh.next();
            selected_heh = selected_heh.next();
        }
    }
    selected_mesh.calcWeight();
}

glm::vec2 TextureEditor::getUV(float progress) {
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

void TextureEditor::solveUV() {
    if (added_face_ids.empty()) {
        return;
    }

    OpenMesh::SmartHalfedgeHandle first_heh;
    for (const auto& heh : selected_mesh.halfedges()) {
        if (!heh.is_boundary()) {
            continue;
        }
        first_heh = heh;
        if (heh.from().idx() == (int)closest_edge_point_id) {
            break;
        }
    }

    // setup edge
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

    // solving the linear equations
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

void TextureEditor::saveSelectedMesh() {
    std::vector<MyVertex> vertices;

    std::unordered_map<size_t, size_t> vertex_idx_mapper;
    int i = 0;
    for (const auto& vh : selected_mesh.getPoints()) {
        vertex_idx_mapper[vh.idx()] = i++;
        vertices.push_back(selected_mesh.getMyVertex(vh));
    }

    std::vector<GLuint> indices = selected_mesh.getIndices();
    for (GLuint& i : indices) {
        i = vertex_idx_mapper[i];
    }

    auto new_mesh = MyMesh(textures.selected_texture, "New Mesh", selected_mesh.scale);
    new_mesh.loadVertices(vertices, indices);
    new_mesh.name = saved_meshes.size() ? saved_meshes.back().name : "New Mesh";
    saved_meshes.push_back(new_mesh);
}

void TextureEditor::loadSavedMeshes(json j) {
    saved_meshes.clear();
    int i = 0;
    for (const auto& m : j["meshes"]) {
        std::cout << i++ << "\n";
        saved_meshes.push_back(MyMesh::Load(m));
    }
}

void TextureEditor::bindMainScreen() {
    main_screen.bind();

    glViewport(0, 0, main_screen.width, main_screen.height);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void TextureEditor::unbindMainScreen() {
    main_screen.unbind();
}

void TextureEditor::renderSavedMeshes(Shader prog) {
    programs.uv.use();
    programs.uv.setMat4("model", glm::mat4(1));

    int idx = 0;
    for (const auto& mesh : saved_meshes) {
        textures.bind(mesh.texture_id, prog);
        mesh.render(prog);

        if (idx++ == highlighted_mesh_idx) {
            programs.highlight.use();
            programs.highlight.setMat4("model", glm::mat4(1));
            programs.highlight.setVec3("color", glm::vec3(0, 1, 1));
            mesh.render(programs.highlight);
        }
    }
}

void TextureEditor::renderBaseModle() {
    programs.face_selector.use();
    programs.face_selector.setMat4("model", glm::mat4(1));
    main_mesh.render(programs.face_selector);
    main_screen.updateIds();
}

void TextureEditor::renderFacePicker() {
    // render face selector to the default frame buffer
    glViewport(0, 0, main_screen.width, main_screen.height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    main_screen.render(programs.screen);
}

void TextureEditor::renderLiveUVSolver() {
    child_screen.bind();
    {
        glViewport(0, 0, child_screen.width, child_screen.height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glLineWidth(3);
        selected_mesh.renderUV(programs.preview);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    child_screen.unbind();

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_AlwaysAutoResize;
    ImGui::Begin("Live UV solver", NULL, flags);
    ImGui::Image((ImTextureID)child_screen.texture, {500, 500});
    ImGui::SliderFloat("Scale", &selected_mesh.scale, 0.1, 2.0);
    ImGui::End();
}

void TextureEditor::renderPopupMenu() {
    if (ImGui::IsKeyDown(ImGuiKey_MouseRight)) {
        ImGui::OpenPopup("my_popup");
    }
    if (ImGui::BeginPopup("my_popup")) {
        ImGui::SliderInt("Radius", &configs.radius, 1, 20);

        ImGui::Separator();

        if (ImGui::MenuItem("Fill")) {
            fill();
        }
        if (ImGui::MenuItem("Save & Clear Selection")) {
            saveSelectedMesh();
            reset();
        }
        if (ImGui::MenuItem("Clear Selection")) {
            reset();
        }

        if (ImGui::BeginMenu("Change Texture")) {
            textures.renderPicker(3);
            ImGui::EndMenu();
        }

        ImGui::Separator();

        ImGui::MenuItem("Toggle Edit Mode", NULL, &configs.edit_mode);
        ImGui::MenuItem("Highlight Hovered Face", NULL, &configs.highlight_hovered_face);
        ImGui::MenuItem("Toggle Live UV Solver", NULL, &configs.render_live_uv_solver);

        ImGui::Separator();

        ImGui::EndPopup();
    }
}

void TextureEditor::h() {
    auto shader = programs.highlight;

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

    // shader.use();
    // glPointSize(20);
    // main_mesh.highlightPoints(shader, {(int)selected_vertex_id});

    std::set<int> edge_points;
    std::set<int> interior_points;

    std::vector<int> indices;
    for (auto e : selected_mesh.edges()) {
        if (e.is_boundary()) {
            indices.push_back(e.v0().idx());
            indices.push_back(e.v1().idx());
        }
    }

    glLineWidth(5);
    main_mesh.highlightEdges(shader, indices);
    return;

    indices.clear();
    std::set_difference(interior_points.begin(), interior_points.end(), edge_points.begin(), edge_points.end(),std::inserter(indices, indices.end()));
    glPointSize(20);
    main_mesh.highlightPoints(shader, indices, {0.3, 1.0, 0.3});

    for (auto idx : edge_points) {
        auto uv = selected_mesh.property(selected_mesh.UV, selected_mesh.vertex_handle(idx));
        shader.use();
        glPointSize(20);
        main_mesh.highlightPoints(shader, {idx}, {uv[0], uv[1], uv[2]});
    }

    shader.setVec3("color", {0.3, 0.3, 1.0});

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
}

void TextureEditor::highlightHovered(){
    if (ImGui::GetIO().WantCaptureMouse) {
        return;
    }

    auto pos = ImGui::GetMousePos();
    size_t faceID = main_screen.getFaceId(pos);
    if (faceID == 0) {
        return;
    }
    faceID -= 1;

    glm::vec3 color(0, 1, 0);
    if (ImGui::IsKeyDown(ImGuiKey_LeftShift)) {
        color = glm::vec3(1, 0, 0);
    }
    main_mesh.highlightFaces(programs.highlight, getFaceIdWithRadius(faceID), color);

    if (ImGui::IsKeyDown(ImGuiKey_Space)) {
        glm::vec3 color(0, 1, 1);
        glPointSize(20);
        main_mesh.highlightPoints(programs.highlight, {(int)closest_edge_point_id}, color);
    }
}

void TextureEditor::renderSelected() {
    programs.uv.use();
    programs.uv.setMat4("model", glm::mat4(1));
    selected_mesh.updateUV();

    if (ImGui::IsKeyDown(ImGuiKey_Q)) {
        textures.bind("_grid.jpg", programs.uv);
    } else {
        textures.bindSelected(programs.uv);
    }

    selected_mesh.renderUV(programs.uv);
}

void TextureEditor::renderMeshLayerEditor() {
    ImGui::Begin("Mesh", NULL);

    static std::string filename = "dragon.json";
    filename.reserve(512);
    ImGui::InputText("##Filename", &filename[0], filename.capacity());
    ImGui::SameLine();
    if (ImGui::Button("Load")) {
        json j = json::parse(slurpFile(filename.c_str()));
        loadSavedMeshes(j);
    }
    ImGui::SameLine();
    if (ImGui::Button("Save")) {
        std::ofstream file(filename);
        file << serialize() << std::endl;
        file.close();
    }

    ImGui::Separator();

    bool hovered = false;
    for (int n = saved_meshes.size() - 1; n >= 0; n--) {
        ImGui::PushID(n);

        auto item = std::to_string(n);

        ImGui::InputText("Name", &saved_meshes[n].name[0], saved_meshes[n].name.capacity());
        if (ImGui::IsItemHovered()) {
            hovered = true;
            highlighted_mesh_idx = n;
        }

        ImGui::SliderFloat("Scale", &saved_meshes[n].scale, 0.1, 2.0);

        if (ImGui::Button("Delete")) {
            saved_meshes.erase(saved_meshes.begin() + n);
        }

        ImGui::SameLine();
        if (ImGui::Button("Move ^")) {
            std::swap(saved_meshes[n], saved_meshes[n - 1]);
        }

        ImGui::SameLine();
        if (ImGui::Button("Move v")) {
            std::swap(saved_meshes[n], saved_meshes[n + 1]);
        }

        ImGui::SameLine();
        if (ImGui::TreeNode("Change Texture")) {
            if (textures.renderPicker(3)) {
                saved_meshes[n].texture_id = textures.selected_texture;
            }
            ImGui::TreePop();
        }

        ImGui::PopID();
    }

    if (!hovered) {
        highlighted_mesh_idx = -1;
    }

    ImGui::End();
}
