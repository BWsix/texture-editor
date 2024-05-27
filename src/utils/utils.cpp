#include "utils.h"
#include "imgui.h"
#include <cstdio>
#include <cstdlib>
#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

double getTimeElapsed() { return glfwGetTime(); }
float getFrameRate() { return ImGui::GetIO().Framerate; }
float getDeltaTime() { return ImGui::GetIO().DeltaTime; }

const char *slurpFile(const char *path) {
    FILE *fp = std::fopen(path, "rb");
    if (!fp) {
        std::fprintf(stderr, "Failed to open %s\n", path);
        exit(1);
    }

    fseek(fp, 0, SEEK_END);
    size_t sz = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char *src = new char[sz + 1];
    src[sz] = '\0';
    size_t bytes_read = fread(src, sizeof(char), sz, fp);
    if (bytes_read != sz) {
        std::fprintf(stderr, "Failed to fully read content of %s\n", path);
        exit(1);
    }
    fclose(fp);
    return src;
}
