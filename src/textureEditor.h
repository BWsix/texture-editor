#pragma once

#include "mesh.h"
#include "screen.h"
#include <GL/gl.h>
#include <set>
#include <unordered_set>
#include <Eigen/Dense>

using I2F = std::map<std::tuple<int, int, int>, int>;

class TextureEditor {
    std::set<GLuint> added_face_ids;
    std::set<GLuint> selected_face_ids;

    I2F indices_to_face_id;
    std::unordered_multiset<GLuint> covered_face_ids;

    GLuint closest_edge_point_id;

public:
    std::vector<MyMesh> saved_meshes;

    MyMesh main_mesh;
    MyMesh selected_mesh;

    int highlighted_mesh_idx = -1;

    Screen main_screen = Screen(1920, 1080);
    Screen child_screen = Screen(500, 500);

    struct {
        int radius = 1;

        bool edit_mode = true;
        bool highlight_hovered_face = true;

        bool render_live_uv_solver = true;

        bool gold_mode = false;
        bool ugly_mode = false;
    } configs;

    struct {
        Shader wireframe;
        Shader highlight;
        Shader uv;
        Shader screen;
        Shader preview;
        Shader face_selector;
        Shader point;
    } programs;

    // handlers
    void handleFaceSelector();
    void addFace(GLuint face_id);
    void deleteFace(GLuint face_id);
    void acuallyAddingFace();

    // utils
    void updateClosestEdgePointId();
    glm::vec2 getUV(float progress);
    std::set<size_t> getFaceIdWithRadius(GLuint face_id);
    void reset();
    void solveUV();
    void load(std::string path);
    void setup();

    // shites
    void fill();
    void h();

    // saving/loading
    void loadSavedMeshes(json j);
    json serialize();
    void saveSelectedMesh();

    // rending
    void bindMainScreen();
    void unbindMainScreen();
    void highlightHovered();
    void renderBaseModle();
    void renderSavedMeshes(Shader prog);
    void renderFacePicker();
    void renderSelected();
    void highlightCoveredFaces();

    // UI
    void renderLiveUVSolver();
    void renderMenu();
    void renderPopupMenu();
    void renderMeshLayerEditor();
};
