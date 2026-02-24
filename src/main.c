#define _GNU_SOURCE
#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "types.h"
#include "config.h"
#include "astro.h"
#include "ui.h"

/* * shaders for day/night transition 
 * uses dot product between surface normal and sun direction
 */
const char* fs3D = 
    "#version 330\n"
    "in vec2 fragTexCoord;\n"
    "in vec4 fragColor;\n"
    "out vec4 finalColor;\n"
    "uniform sampler2D texture0;\n"
    "uniform sampler2D texture1;\n"
    "uniform vec3 sunDir;\n"
    "void main() {\n"
    "    vec4 day = texture(texture0, fragTexCoord);\n"
    "    vec4 night = texture(texture1, fragTexCoord);\n"
    "    float theta = (fragTexCoord.x - 0.5) * 6.28318530718;\n"
    "    float phi = fragTexCoord.y * 3.14159265359;\n"
    "    vec3 normal = vec3(cos(theta)*sin(phi), cos(phi), -sin(theta)*sin(phi));\n"
    "    float intensity = dot(normal, sunDir);\n"
    "    float blend = smoothstep(-0.15, 0.15, intensity);\n"
    "    finalColor = mix(night, day, blend) * fragColor;\n"
    "}\n";

const char* fs2D = 
    "#version 330\n"
    "in vec2 fragTexCoord;\n"
    "in vec4 fragColor;\n"
    "out vec4 finalColor;\n"
    "uniform sampler2D texture0;\n"
    "uniform sampler2D texture1;\n"
    "uniform vec3 sunDir;\n"
    "void main() {\n"
    "    vec4 day = texture(texture0, fragTexCoord);\n"
    "    vec4 night = texture(texture1, fragTexCoord);\n"
    "    float theta = (fragTexCoord.x - 0.5) * 6.28318530718;\n"
    "    float phi = fragTexCoord.y * 3.14159265359;\n"
    "    vec3 normal = vec3(cos(theta)*sin(phi), cos(phi), -sin(theta)*sin(phi));\n"
    "    float intensity = dot(normal, sunDir);\n"
    "    float blend = smoothstep(-0.15, 0.15, intensity);\n"
    "    finalColor = mix(night, day, blend) * fragColor;\n"
    "}\n";

/* cloud shader handles transparency based on sun position */
const char* fsCloud3D = 
    "#version 330\n"
    "in vec2 fragTexCoord;\n"
    "in vec4 fragColor;\n"
    "out vec4 finalColor;\n"
    "uniform sampler2D texture0;\n"
    "uniform vec3 sunDir;\n"
    "void main() {\n"
    "    vec4 texel = texture(texture0, fragTexCoord);\n"
    "    float theta = (fragTexCoord.x - 0.5) * 6.28318530718;\n"
    "    float phi = fragTexCoord.y * 3.14159265359;\n"
    "    vec3 normal = vec3(cos(theta)*sin(phi), cos(phi), -sin(theta)*sin(phi));\n"
    "    float intensity = dot(normal, sunDir);\n"
    "    float alpha = smoothstep(-0.15, 0.05, intensity);\n"
    "    finalColor = vec4(texel.rgb, texel.a * alpha) * fragColor;\n"
    "}\n";

/* application state and resources */
static AppConfig cfg = {
    .window_width = 1280, .window_height = 720, .target_fps = 120, .ui_scale = 1.0f,
    .show_clouds = false, .show_night_lights = true, .show_markers = true,
    .show_statistics = false,
    .bg_color = {0, 0, 0, 255}, .text_main = {255, 255, 255, 255}, .theme = "default",
    .ui_primary = {32, 32, 32, 255}, .ui_secondary = {64, 64, 64, 255}, .ui_accent = {0, 255, 0, 255}
};

static Font customFont;
static Texture2D satIcon, markerIcon, earthTexture, moonTexture, cloudTexture, earthNightTexture;
static Texture2D periMark, apoMark;
static Model earthModel, moonModel, cloudModel;

/* manual mesh generation for the planetary spheres */
static Mesh GenEarthMesh(float radius, int slices, int rings) {
    Mesh mesh = { 0 };
    int vertexCount = (rings + 1) * (slices + 1);
    int triangleCount = rings * slices * 2;

    mesh.vertexCount = vertexCount;
    mesh.triangleCount = triangleCount;
    mesh.vertices = (float *)MemAlloc(vertexCount * 3 * sizeof(float));
    mesh.texcoords = (float *)MemAlloc(vertexCount * 2 * sizeof(float));
    mesh.normals = (float *)MemAlloc(vertexCount * 3 * sizeof(float));
    mesh.indices = (unsigned short *)MemAlloc(triangleCount * 3 * sizeof(unsigned short));

    int vIndex = 0;
    for (int i = 0; i <= rings; i++) {
        float v = (float)i / (float)rings;
        float phi = v * PI; 
        for (int j = 0; j <= slices; j++) {
            float u = (float)j / (float)slices;
            float theta = (u - 0.5f) * 2.0f * PI; 
            float x = cosf(theta) * sinf(phi);
            float y = cosf(phi);
            float z = -sinf(theta) * sinf(phi); 

            mesh.vertices[vIndex * 3 + 0] = x * radius;
            mesh.vertices[vIndex * 3 + 1] = y * radius;
            mesh.vertices[vIndex * 3 + 2] = z * radius;
            mesh.normals[vIndex * 3 + 0] = x;
            mesh.normals[vIndex * 3 + 1] = y;
            mesh.normals[vIndex * 3 + 2] = z;
            mesh.texcoords[vIndex * 2 + 0] = u;
            mesh.texcoords[vIndex * 2 + 1] = v;
            vIndex++;
        }
    }

    int iIndex = 0;
    for (int i = 0; i < rings; i++) {
        for (int j = 0; j < slices; j++) {
            int first = (i * (slices + 1)) + j;
            int second = first + slices + 1;
            mesh.indices[iIndex++] = first;
            mesh.indices[iIndex++] = second;
            mesh.indices[iIndex++] = first + 1;
            mesh.indices[iIndex++] = second;
            mesh.indices[iIndex++] = second + 1;
            mesh.indices[iIndex++] = first + 1;
        }
    }
    UploadMesh(&mesh, false);
    return mesh;
}

/* render orbit lines in 3d space */
static void draw_orbit_3d(Satellite* sat, double current_epoch, bool is_highlighted, float alpha) {
    Color orbitColor = ApplyAlpha(is_highlighted ? cfg.orbit_highlighted : cfg.orbit_normal, alpha);

    if (is_highlighted) {
        Vector3 prev_pos = {0};
        int segments = fmin(4000, fmax(90, (int)(400 * cfg.orbits_to_draw)));
        double orbits_count = (double)cfg.orbits_to_draw;
        
        double period_days = (2.0 * PI / sat->mean_motion) / 86400.0;
        double time_step = (period_days * orbits_count) / segments;

        for (int i = 0; i <= segments; i++) {
            double t = current_epoch + (i * time_step);
            double t_unix = get_unix_from_epoch(t);
            Vector3 pos = Vector3Scale(calculate_position(sat, t_unix), 1.0f / DRAW_SCALE);

            if (i > 0) DrawLine3D(prev_pos, pos, orbitColor);
            prev_pos = pos;
        }
    } else {
        if (!sat->orbit_cached) return;
        Vector3 prev_pos = sat->orbit_cache[0];
        for (int i = 1; i < ORBIT_CACHE_SIZE; i++) {
            Vector3 pos = sat->orbit_cache[i];
            DrawLine3D(prev_pos, pos, orbitColor);
            prev_pos = pos;
        }
    }
}

/* simple progress bar during init */
static void DrawLoadingScreen(float progress, const char* message, Texture2D logoTex) {
    BeginDrawing();
    ClearBackground(cfg.bg_color);
    
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();
    float barW = 400 * cfg.ui_scale;
    float barH = 16 * cfg.ui_scale;
    
    float startY = screenH / 2.0f;
    
    if (logoTex.id != 0) {
        float logoScale = (120.0f * cfg.ui_scale) / logoTex.height; 
        Vector2 logoPos = { (screenW - logoTex.width * logoScale) / 2.0f, startY - (logoTex.height * logoScale) - 40 * cfg.ui_scale };
        DrawTextureEx(logoTex, logoPos, 0.0f, logoScale, WHITE);
    }

    Rectangle barOutline = { (screenW - barW)/2, startY, barW, barH };
    Rectangle barProgress = { barOutline.x + 3, barOutline.y + 3, (barW - 6) * progress, barH - 6 };
    
    #if defined(_WIN32) || defined(_WIN64)
        DrawRectangleRoundedLines(barOutline, 0.5f, 16, 2.0f, cfg.text_main);
    #else
        DrawRectangleRoundedLines(barOutline, 0.5f, 16, cfg.text_main);
    #endif
    if (progress > 0.0f) DrawRectangleRounded(barProgress, 0.5f, 16, cfg.text_secondary);
    
    Vector2 msgSize = MeasureTextEx(customFont, message, 18 * cfg.ui_scale, 1.0f);
    DrawUIText(customFont, message, (screenW - msgSize.x)/2, barOutline.y + barH + 20 * cfg.ui_scale, 18 * cfg.ui_scale, cfg.text_main);
    
    EndDrawing();
}

int main(void) {
    LoadAppConfig("settings.json", &cfg);

    /* window setup and msaa */
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(cfg.window_width, cfg.window_height, "TLEscope");

    int monitor = GetCurrentMonitor();
    int max_w = GetMonitorWidth(monitor);
    int max_h = GetMonitorHeight(monitor);

    if (cfg.window_width >= max_w || cfg.window_height >= max_h) {
        SetWindowState(FLAG_WINDOW_MAXIMIZED);
    } else {
        Vector2 monitorPos = GetMonitorPosition(monitor);
        SetWindowPosition(
            (int)monitorPos.x + (max_w - cfg.window_width) / 2,
            (int)monitorPos.y + (max_h - cfg.window_height) / 2
        );
    }

    SetExitKey(0); 

    /* logo and icon loading */
    Image logoImg = LoadImage("logo.png");
    Texture2D logoTex = {0};
    if (logoImg.data != NULL) {
        SetWindowIcon(logoImg);
        logoTex = LoadTextureFromImage(logoImg);
        UnloadImage(logoImg);
    }
    Image logoLImg = LoadImage("logo.png");
    Texture2D logoLTex = {0};
    if (logoLImg.data != NULL) {
        logoLTex = LoadTextureFromImage(logoLImg);
        UnloadImage(logoLImg);
    }
    
    /* font loading with specific glyph range */
    int glyphsCount = 0;
    int *glyphs = LoadCodepoints(" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~", &glyphsCount);
    customFont = LoadFontEx(TextFormat("themes/%s/font.ttf", cfg.theme), 64, glyphs, glyphsCount);
    GenTextureMipmaps(&customFont.texture);
    SetTextureFilter(customFont.texture, TEXTURE_FILTER_BILINEAR);

    /* resource loading phase */
    DrawLoadingScreen(0.1f, "Fetching TLE Data...", logoLTex);
    load_tle_data("data.tle");

    DrawLoadingScreen(0.25f, "Initializing Textures...", logoTex);
    earthTexture = LoadTexture(TextFormat("themes/%s/earth.png", cfg.theme));
    earthNightTexture = LoadTexture(TextFormat("themes/%s/earth_night.png", cfg.theme));
    
    DrawLoadingScreen(0.4f, "Compiling Shaders...", logoTex);
    Shader shader3D = LoadShaderFromMemory(NULL, fs3D);
    int sunDirLoc3D = GetShaderLocation(shader3D, "sunDir");
    shader3D.locs[SHADER_LOC_MAP_EMISSION] = GetShaderLocation(shader3D, "texture1");

    Shader shader2D = LoadShaderFromMemory(NULL, fs2D);
    int sunDirLoc2D = GetShaderLocation(shader2D, "sunDir");
    int nightTexLoc2D = GetShaderLocation(shader2D, "texture1");

    Shader shaderCloud = LoadShaderFromMemory(NULL, fsCloud3D);
    int sunDirLocCloud = GetShaderLocation(shaderCloud, "sunDir");
    
    DrawLoadingScreen(0.6f, "Generating Meshes...", logoTex);
    float draw_earth_radius = EARTH_RADIUS_KM / DRAW_SCALE;
    Mesh sphereMesh = GenEarthMesh(draw_earth_radius, 64, 64);
    earthModel = LoadModelFromMesh(sphereMesh);
    earthModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = earthTexture;
    earthModel.materials[0].maps[MATERIAL_MAP_EMISSION].texture = earthNightTexture;
    Shader defaultEarthShader = earthModel.materials[0].shader;
    
    DrawLoadingScreen(0.8f, "Loading Celestial Bodies...", logoTex);
    float draw_cloud_radius = (EARTH_RADIUS_KM + 25.0f) / DRAW_SCALE;
    Mesh cloudMesh = GenEarthMesh(draw_cloud_radius, 64, 64);
    cloudModel = LoadModelFromMesh(cloudMesh);
    cloudTexture = LoadTexture(TextFormat("themes/%s/clouds.png", cfg.theme));
    cloudModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = cloudTexture;
    Shader defaultCloudShader = cloudModel.materials[0].shader;
    
    float draw_moon_radius = MOON_RADIUS_KM / DRAW_SCALE;
    Mesh moonMesh = GenEarthMesh(draw_moon_radius, 32, 32); 
    moonModel = LoadModelFromMesh(moonMesh);
    moonTexture = LoadTexture(TextFormat("themes/%s/moon.png", cfg.theme));
    moonModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = moonTexture;

    DrawLoadingScreen(0.95f, "Finalizing UI...", logoTex);
    satIcon = LoadTexture(TextFormat("themes/%s/sat_icon.png", cfg.theme));
    markerIcon = LoadTexture(TextFormat("themes/%s/marker_icon.png", cfg.theme));
    periMark = LoadTexture(TextFormat("themes/%s/smallmark.png", cfg.theme));
    apoMark = LoadTexture(TextFormat("themes/%s/smallmark.png", cfg.theme));
    
    SetTextureFilter(satIcon, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(markerIcon, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(periMark, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(apoMark, TEXTURE_FILTER_BILINEAR);

    DrawLoadingScreen(1.0f, "Ready!", logoTex);

    /* camera defaults */
    Camera Camera3DParams = { 0 };
    Camera3DParams.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    Camera3DParams.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    Camera3DParams.fovy = 45.0f;
    Camera3DParams.projection = CAMERA_PERSPECTIVE;

    Camera2D Camera2DParams = { 0 };
    Camera2DParams.zoom = 1.0f;
    Camera2DParams.offset = (Vector2){ GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f };
    Camera2DParams.target = (Vector2){ 0.0f, 0.0f };

    float target_camera2d_zoom = Camera2DParams.zoom;
    Vector2 target_camera2d_target = Camera2DParams.target;

    float map_w = 2048.0f, map_h = 1024.0f;
    float camDistance = 35.0f, camAngleX = 0.785f, camAngleY = 0.5f;

    float target_camDistance = camDistance;
    float target_camAngleX = camAngleX;
    float target_camAngleY = camAngleY;
    Vector3 target_camera3d_target = Camera3DParams.target;

    double current_epoch = get_current_real_time_epoch();
    double time_multiplier = 1.0; 
    double saved_multiplier = 1.0; 
    bool is_2d_view = false;
    
    bool hide_unselected = false;
    float unselected_fade = 1.0f;

    bool is_auto_warping = false;
    double auto_warp_target = 0.0;
    double auto_warp_initial_diff = 0.0;

    Satellite* hovered_sat = NULL;
    Satellite* selected_sat = NULL;
    TargetLock active_lock = LOCK_EARTH;
    double last_left_click_time = 0.0;

    SetTargetFPS(cfg.target_fps);
    int current_update_idx = 0;

    /* main loop */
    while (!WindowShouldClose()) {
        bool is_typing = IsUITyping();
        bool over_ui = IsMouseOverUI(&cfg);

        if (is_2d_view && !is_typing) {
            Camera2DParams.offset = (Vector2){ GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f };
        }

        /* input handling */
        if (!is_typing) {
            if (IsKeyPressed(KEY_SPACE)) {
                is_auto_warping = false;
                if (time_multiplier != 0.0) {
                    saved_multiplier = time_multiplier;
                    time_multiplier = 0.0;
                } else {
                    time_multiplier = saved_multiplier != 0.0 ? saved_multiplier : 1.0;
                }
            }
            if (IsKeyPressed(KEY_PERIOD)) { is_auto_warping = false; time_multiplier = StepTimeMultiplier(time_multiplier, true); }
            if (IsKeyPressed(KEY_COMMA)) { is_auto_warping = false; time_multiplier = StepTimeMultiplier(time_multiplier, false); }
            if (IsKeyPressed(KEY_M)) is_2d_view = !is_2d_view;
            if (IsKeyPressed(KEY_RIGHT_BRACKET)) ToggleTLEWarning();
            
            if (IsKeyPressed(KEY_SLASH)) {
                is_auto_warping = false;
                if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) current_epoch = get_current_real_time_epoch(); 
                else {
                    time_multiplier = 1.0; 
                    saved_multiplier = 1.0;
                }
            }

            if (IsKeyPressed(KEY_EQUAL) || IsKeyPressed(KEY_KP_ADD)) cfg.ui_scale += 0.1f;
            if (IsKeyPressed(KEY_MINUS) || IsKeyPressed(KEY_KP_SUBTRACT)) cfg.ui_scale -= 0.1f;
        }

        if (cfg.ui_scale < 0.5f) cfg.ui_scale = 0.5f;
        if (cfg.ui_scale > 4.0f) cfg.ui_scale = 4.0f;

        /* time warp logic for jumping to specific dates */
        if (is_auto_warping) {
            double diff_sec = (auto_warp_target - current_epoch) * 86400.0;
            if (diff_sec <= 0.0) {
                current_epoch = auto_warp_target;
                time_multiplier = 1.0;
                saved_multiplier = 1.0;
                is_auto_warping = false;
            } else {
                double base_speed = auto_warp_initial_diff / 2.0;
                double eased_speed = fmin(base_speed, diff_sec * 3.0); 
                if (eased_speed < 1.0) eased_speed = 1.0;
                time_multiplier = eased_speed;
                
                if (diff_sec <= time_multiplier * GetFrameTime()) {
                    current_epoch = auto_warp_target;
                    time_multiplier = 1.0;
                    saved_multiplier = 1.0;
                    is_auto_warping = false;
                }
            }
        }

        current_epoch += (GetFrameTime() * time_multiplier) / 86400.0; 
        current_epoch = normalize_epoch(current_epoch);

        /* asynchronous orbit path updates to keep fps high */
        if (sat_count > 0) {
            int updates_per_frame = 50; 
            for (int i = 0; i < updates_per_frame; i++) {
                if (satellites[current_update_idx].is_active) {
                    update_orbit_cache(&satellites[current_update_idx], current_epoch);
                }
                current_update_idx = (current_update_idx + 1) % sat_count;
            }
        }

        double current_unix = get_unix_from_epoch(current_epoch);

        /* update current positions of all active sats */
        for (int i = 0; i < sat_count; i++) {
            if (!satellites[i].is_active) continue;
            if (hide_unselected && selected_sat != NULL && &satellites[i] != selected_sat) continue;
            satellites[i].current_pos = calculate_position(&satellites[i], current_unix);
        }

        /* fading logic for selection isolation */
        bool should_hide = (hide_unselected && selected_sat != NULL);
        if (should_hide) {
            unselected_fade -= 3.0f * GetFrameTime();
            if (unselected_fade < 0.0f) unselected_fade = 0.0f;
        } else {
            unselected_fade += 3.0f * GetFrameTime();
            if (unselected_fade > 1.0f) unselected_fade = 1.0f;
        }

        char datetime_str[64];
        epoch_to_datetime_str(current_epoch, datetime_str);
        double gmst_deg = epoch_to_gmst(current_epoch);

        /* calculate moon orientation and position */
        Vector3 moon_pos_km = calculate_moon_position(current_epoch);
        Vector3 draw_moon_pos = Vector3Scale(moon_pos_km, 1.0f / DRAW_SCALE);
        float moon_mx, moon_my;
        get_map_coordinates(moon_pos_km, gmst_deg, cfg.earth_rotation_offset, map_w, map_h, &moon_mx, &moon_my);

        Vector3 dirToEarth = Vector3Normalize(Vector3Negate(draw_moon_pos));
        float moon_yaw = atan2f(-dirToEarth.z, dirToEarth.x);
        float moon_pitch = asinf(dirToEarth.y);
        moonModel.transform = MatrixMultiply(MatrixRotateZ(moon_pitch), MatrixRotateY(moon_yaw));

        Vector2 mouseDelta = GetMouseDelta();
        hovered_sat = NULL;

        /* handle picking and camera in 2d mode */
        if (is_2d_view) {
            if (!over_ui) {
                if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT) || (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE) && IsKeyDown(KEY_LEFT_SHIFT))) {
                    target_camera2d_target = Vector2Add(target_camera2d_target, Vector2Scale(mouseDelta, -1.0f / target_camera2d_zoom));
                    active_lock = LOCK_NONE;
                }
                float wheel = GetMouseWheelMove();
                if (wheel != 0 && !is_typing) {
                    target_camera2d_zoom += wheel * 0.1f * target_camera2d_zoom;
                    if (target_camera2d_zoom < 0.1f) target_camera2d_zoom = 0.1f;
                    active_lock = LOCK_NONE;
                }
            }

            if (!over_ui) {
                Vector2 mousePos = GetMousePosition();
                float closest_dist = 9999.0f;
                float hit_radius_pixels = 12.0f * cfg.ui_scale;

                for (int i = 0; i < sat_count; i++) {
                    if (!satellites[i].is_active) continue;
                    if (hide_unselected && selected_sat != NULL && &satellites[i] != selected_sat) continue;
                    
                    float mx, my;
                    get_map_coordinates(satellites[i].current_pos, gmst_deg, cfg.earth_rotation_offset, map_w, map_h, &mx, &my);
                    
                    Vector2 screenPos = GetWorldToScreen2D((Vector2){mx, my}, Camera2DParams);
                    float dist = Vector2Distance(mousePos, screenPos);
                    
                    if (dist < hit_radius_pixels && dist < closest_dist) { closest_dist = dist; hovered_sat = &satellites[i]; }
                }
            }
        } else {
            /* handle picking and camera in 3d mode */
            if (!over_ui) {
                if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
                    if (IsKeyDown(KEY_LEFT_SHIFT)) {
                        Vector3 forward = Vector3Normalize(Vector3Subtract(Camera3DParams.target, Camera3DParams.position));
                        Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, Camera3DParams.up));
                        Vector3 upVector = Vector3Normalize(Vector3CrossProduct(right, forward));
                        float panSpeed = target_camDistance * 0.001f; 
                        target_camera3d_target = Vector3Add(target_camera3d_target, Vector3Scale(right, -mouseDelta.x * panSpeed));
                        target_camera3d_target = Vector3Add(target_camera3d_target, Vector3Scale(upVector, mouseDelta.y * panSpeed));
                        active_lock = LOCK_NONE;
                    } else {
                        target_camAngleX -= mouseDelta.x * 0.005f;
                        target_camAngleY += mouseDelta.y * 0.005f;
                        if (target_camAngleY > 1.5f) target_camAngleY = 1.5f;
                        if (target_camAngleY < -1.5f) target_camAngleY = -1.5f;
                    }
                }
                if (!is_typing) {
                    target_camDistance -= GetMouseWheelMove() * (target_camDistance * 0.1f);
                    if (target_camDistance < draw_earth_radius + 1.0f) target_camDistance = draw_earth_radius + 1.0f;
                }
            }

            if (!over_ui) {
                Ray mouseRay = GetMouseRay(GetMousePosition(), Camera3DParams);
                float closest_dist = 9999.0f;

                for (int i = 0; i < sat_count; i++) {
                    if (!satellites[i].is_active) continue;
                    if (hide_unselected && selected_sat != NULL && &satellites[i] != selected_sat) continue;

                    Vector3 draw_pos = Vector3Scale(satellites[i].current_pos, 1.0f / DRAW_SCALE);
                    if (Vector3DistanceSqr(Camera3DParams.target, draw_pos) > (camDistance * camDistance * 16.0f)) continue; 
                    
                    float distToCam = Vector3Distance(Camera3DParams.position, draw_pos);
                    float hit_radius_3d = 0.015f * distToCam * cfg.ui_scale;

                    RayCollision col = GetRayCollisionSphere(mouseRay, draw_pos, hit_radius_3d); 
                    if (col.hit && col.distance < closest_dist) { closest_dist = col.distance; hovered_sat = &satellites[i]; }
                }
            }
        }

        /* selection and double click for planet locking */
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            if (!over_ui) {
                selected_sat = hovered_sat; 

                double current_time = GetTime();
                if (current_time - last_left_click_time < 0.3) {
                    if (is_2d_view) {
                        Vector2 mouseWorld = GetScreenToWorld2D(GetMousePosition(), Camera2DParams);
                        bool hit_moon = false;
                        for (int offset_i = -1; offset_i <= 1; offset_i++) {
                            float x_off = offset_i * map_w;
                            if (Vector2Distance(mouseWorld, (Vector2){moon_mx + x_off, moon_my}) < (15.0f * cfg.ui_scale / Camera2DParams.zoom)) {
                                hit_moon = true;
                                break;
                            }
                        }
                        active_lock = hit_moon ? LOCK_MOON : LOCK_EARTH;
                    } else {
                        Ray mouseRay = GetMouseRay(GetMousePosition(), Camera3DParams);
                        RayCollision earthCol = GetRayCollisionSphere(mouseRay, Vector3Zero(), draw_earth_radius);
                        RayCollision moonCol = GetRayCollisionSphere(mouseRay, draw_moon_pos, draw_moon_radius);

                        if (moonCol.hit && (!earthCol.hit || moonCol.distance < earthCol.distance)) {
                            active_lock = LOCK_MOON;
                        } else if (earthCol.hit) {
                            active_lock = LOCK_EARTH;
                        }
                    }
                }
                last_left_click_time = current_time;
            }
        }

        /* update camera interpolation targets */
        if (active_lock == LOCK_EARTH) {
            if (is_2d_view) target_camera2d_target = Vector2Zero();
            else target_camera3d_target = Vector3Zero();
        } else if (active_lock == LOCK_MOON) {
            if (is_2d_view) target_camera2d_target = (Vector2){moon_mx, moon_my};
            else target_camera3d_target = draw_moon_pos;
        }

        float smooth_speed = 10.0f * GetFrameTime();
        
        Camera2DParams.zoom = Lerp(Camera2DParams.zoom, target_camera2d_zoom, smooth_speed);
        Camera2DParams.target = Vector2Lerp(Camera2DParams.target, target_camera2d_target, smooth_speed);
        
        camAngleX = Lerp(camAngleX, target_camAngleX, smooth_speed);
        camAngleY = Lerp(camAngleY, target_camAngleY, smooth_speed);
        camDistance = Lerp(camDistance, target_camDistance, smooth_speed);
        Camera3DParams.target = Vector3Lerp(Camera3DParams.target, target_camera3d_target, smooth_speed);

        if (!is_2d_view) {
            Camera3DParams.position.x = Camera3DParams.target.x + camDistance * cosf(camAngleY) * sinf(camAngleX);
            Camera3DParams.position.y = Camera3DParams.target.y + camDistance * sinf(camAngleY);
            Camera3DParams.position.z = Camera3DParams.target.z + camDistance * cosf(camAngleY) * cosf(camAngleX);
        }

        Satellite* active_sat = hovered_sat ? hovered_sat : selected_sat;

        /* calculate radio footprint (visibility cone) */
        #define FP_RINGS 12
        #define FP_PTS 120
        Vector3 fp_grid[FP_RINGS + 1][FP_PTS];
        bool has_footprint = false;
        
        if (active_sat && active_sat->is_active) {
            float r = Vector3Length(active_sat->current_pos);
            if (r > EARTH_RADIUS_KM) {
                has_footprint = true;
                float theta = acosf(EARTH_RADIUS_KM / r);
                Vector3 s_norm = Vector3Normalize(active_sat->current_pos);
                Vector3 up = fabsf(s_norm.y) > 0.99f ? (Vector3){1, 0, 0} : (Vector3){0, 1, 0};
                Vector3 u = Vector3Normalize(Vector3CrossProduct(up, s_norm));
                Vector3 v = Vector3CrossProduct(s_norm, u);
                
                for (int i = 0; i <= FP_RINGS; i++) {
                    float a = theta * ((float)i / FP_RINGS);
                    float d_plane = EARTH_RADIUS_KM * cosf(a), r_circle = EARTH_RADIUS_KM * sinf(a);
                    for (int k = 0; k < FP_PTS; k++) {
                        float alpha = (2.0f * PI * k) / FP_PTS;
                        fp_grid[i][k] = Vector3Add(Vector3Scale(s_norm, d_plane),
                            Vector3Add(Vector3Scale(u, cosf(alpha) * r_circle), Vector3Scale(v, sinf(alpha) * r_circle)));
                    }
                }
            }
        }

        BeginDrawing();
        ClearBackground(cfg.bg_color);

        float m_size_2d = 24.0f * cfg.ui_scale / Camera2DParams.zoom;
        float m_text_2d = 16.0f * cfg.ui_scale / Camera2DParams.zoom;
        float mark_size_2d = 32.0f * cfg.ui_scale / Camera2DParams.zoom;

        /* 2d projection rendering */
        if (is_2d_view) {
            BeginMode2D(Camera2DParams);
                if (cfg.show_night_lights) {
                    BeginShaderMode(shader2D);
                    SetShaderValueTexture(shader2D, nightTexLoc2D, earthNightTexture);
                    
                    Vector3 sunEci = calculate_sun_position(current_epoch);
                    float earth_rot_rad = (gmst_deg + cfg.earth_rotation_offset) * DEG2RAD;
                    Vector3 sunEcef = Vector3Transform(sunEci, MatrixRotateY(-earth_rot_rad));
                    SetShaderValue(shader2D, sunDirLoc2D, &sunEcef, SHADER_UNIFORM_VEC3);
                }

                DrawTexturePro(earthTexture, (Rectangle){0, 0, earthTexture.width, earthTexture.height}, 
                    (Rectangle){-map_w/2, -map_h/2, map_w, map_h}, (Vector2){0,0}, 0.0f, WHITE);

                if (cfg.show_night_lights) EndShaderMode();

                /* scissor mode for map boundaries */
                Vector2 mapMin = GetWorldToScreen2D((Vector2){-map_w/2.0f, -map_h/2.0f}, Camera2DParams);
                Vector2 mapMax = GetWorldToScreen2D((Vector2){map_w/2.0f, map_h/2.0f}, Camera2DParams);
                
                int sc_x = (int)mapMin.x, sc_y = (int)mapMin.y;
                int sc_w = (int)(mapMax.x - mapMin.x), sc_h = (int)(mapMax.y - mapMin.y);
                
                if (sc_x < 0) { sc_w += sc_x; sc_x = 0; }
                if (sc_y < 0) { sc_h += sc_y; sc_y = 0; }
                if (sc_x + sc_w > GetScreenWidth()) sc_w = GetScreenWidth() - sc_x;
                if (sc_y + sc_h > GetScreenHeight()) sc_h = GetScreenHeight() - sc_y;

                if (sc_w > 0 && sc_h > 0) {
                    BeginScissorMode(sc_x, sc_y, sc_w, sc_h);

                    /* draw 2d footprint */
                    if (active_sat && has_footprint && active_sat->is_active) {
                        for (int i = 0; i < FP_RINGS; i++) {
                            for (int k = 0; k < FP_PTS; k++) {
                                int next = (k + 1) % FP_PTS;
                                float x1, y1, x2, y2, x3, y3, x4, y4;
                                get_map_coordinates(fp_grid[i][k], gmst_deg, cfg.earth_rotation_offset, map_w, map_h, &x1, &y1);
                                get_map_coordinates(fp_grid[i][next], gmst_deg, cfg.earth_rotation_offset, map_w, map_h, &x2, &y2);
                                get_map_coordinates(fp_grid[i+1][k], gmst_deg, cfg.earth_rotation_offset, map_w, map_h, &x3, &y3);
                                get_map_coordinates(fp_grid[i+1][next], gmst_deg, cfg.earth_rotation_offset, map_w, map_h, &x4, &y4);
                                
                                if (x2 - x1 > map_w * 0.6f) x2 -= map_w; else if (x2 - x1 < -map_w * 0.6f) x2 += map_w;
                                if (x3 - x1 > map_w * 0.6f) x3 -= map_w; else if (x3 - x1 < -map_w * 0.6f) x3 += map_w;
                                if (x4 - x1 > map_w * 0.6f) x4 -= map_w; else if (x4 - x1 < -map_w * 0.6f) x4 += map_w;
                                
                                for (int offset_i = -1; offset_i <= 1; offset_i++) {
                                    float x_off = offset_i * map_w;
                                    DrawTriangle((Vector2){x1+x_off, y1}, (Vector2){x3+x_off, y3}, (Vector2){x2+x_off, y2}, cfg.footprint_bg);
                                    DrawTriangle((Vector2){x2+x_off, y2}, (Vector2){x3+x_off, y3}, (Vector2){x4+x_off, y4}, cfg.footprint_bg);
                                }
                            }
                        }
                        for (int k = 0; k < FP_PTS; k++) {
                            int next = (k + 1) % FP_PTS;
                            float x1, y1, x2, y2;
                            get_map_coordinates(fp_grid[FP_RINGS][k], gmst_deg, cfg.earth_rotation_offset, map_w, map_h, &x1, &y1);
                            get_map_coordinates(fp_grid[FP_RINGS][next], gmst_deg, cfg.earth_rotation_offset, map_w, map_h, &x2, &y2);
                            if (x2 - x1 > map_w * 0.6f) x2 -= map_w; else if (x2 - x1 < -map_w * 0.6f) x2 += map_w;
                            for (int offset_i = -1; offset_i <= 1; offset_i++) {
                                if (fabs(x2 - x1) < map_w * 0.6f) {
                                    DrawLineEx((Vector2){x1 + offset_i*map_w, y1}, (Vector2){x2 + offset_i*map_w, y2}, 2.0f/Camera2DParams.zoom, cfg.footprint_border);
                                }
                            }
                        }
                    }

                    /* render all sats on 2d map */
                    for (int i = 0; i < sat_count; i++) {
                        if (!satellites[i].is_active) continue;
                        bool is_unselected = (selected_sat != NULL && &satellites[i] != selected_sat);
                        float sat_alpha = is_unselected ? unselected_fade : 1.0f;
                        if (sat_alpha <= 0.0f) continue;

                        bool is_hl = (active_sat == &satellites[i]);
                        Color sCol = (selected_sat == &satellites[i]) ? cfg.sat_selected : (hovered_sat == &satellites[i]) ? cfg.sat_highlighted : cfg.sat_normal;
                        sCol = ApplyAlpha(sCol, sat_alpha);

                        if (is_hl) {
                            int segments = fmin(4000, fmax(50, (int)(400 * cfg.orbits_to_draw))); 
                            Vector2 track_pts[4001]; 

                            double period_days = (2.0 * PI / satellites[i].mean_motion) / 86400.0;
                            double time_step = (period_days * cfg.orbits_to_draw) / segments;

                            for (int j = 0; j <= segments; j++) {
                                double t = current_epoch + (j * time_step);
                                double t_unix = get_unix_from_epoch(t);
                                get_map_coordinates(calculate_position(&satellites[i], t_unix), epoch_to_gmst(t), cfg.earth_rotation_offset, map_w, map_h, &track_pts[j].x, &track_pts[j].y);
                            }

                            for (int offset_i = -1; offset_i <= 1; offset_i++) {
                                float x_off = offset_i * map_w;
                                for (int j = 1; j <= segments; j++) {
                                    if (fabs(track_pts[j].x - track_pts[j-1].x) < map_w * 0.6f) {
                                        DrawLineEx((Vector2){track_pts[j-1].x+x_off, track_pts[j-1].y}, (Vector2){track_pts[j].x+x_off, track_pts[j].y}, 2.0f/Camera2DParams.zoom, ApplyAlpha(cfg.orbit_highlighted, sat_alpha));
                                    }
                                }

                                Vector2 peri2d, apo2d;
                                get_apsis_2d(&satellites[i], current_epoch, false, gmst_deg, cfg.earth_rotation_offset, map_w, map_h, &peri2d);
                                get_apsis_2d(&satellites[i], current_epoch, true, gmst_deg, cfg.earth_rotation_offset, map_w, map_h, &apo2d);
                                
                                DrawTexturePro(periMark, (Rectangle){0,0,periMark.width,periMark.height}, 
                                    (Rectangle){peri2d.x + x_off, peri2d.y, mark_size_2d, mark_size_2d}, (Vector2){mark_size_2d/2.f, mark_size_2d/2.f}, 0.0f, ApplyAlpha(cfg.periapsis, sat_alpha));
                                DrawTexturePro(apoMark, (Rectangle){0,0,apoMark.width,apoMark.height}, 
                                    (Rectangle){apo2d.x + x_off, apo2d.y, mark_size_2d, mark_size_2d}, (Vector2){mark_size_2d/2.f, mark_size_2d/2.f}, 0.0f, ApplyAlpha(cfg.apoapsis, sat_alpha));
                            }
                        }

                        float sat_mx, sat_my;
                        get_map_coordinates(satellites[i].current_pos, gmst_deg, cfg.earth_rotation_offset, map_w, map_h, &sat_mx, &sat_my);
                        for (int offset_i = -1; offset_i <= 1; offset_i++) {
                            DrawTexturePro(satIcon, (Rectangle){0,0,satIcon.width,satIcon.height}, 
                                (Rectangle){sat_mx+(offset_i*map_w), sat_my, m_size_2d, m_size_2d}, (Vector2){m_size_2d/2.f, m_size_2d/2.f}, 0.0f, sCol);
                        }
                    }

                    /* ground station markers */
                    float hx = (home_location.lon / 360.0f) * map_w;
                    float hy = -(home_location.lat / 180.0f) * map_h;
                    for (int offset_i = -1; offset_i <= 1; offset_i++) {
                        float x_off = offset_i * map_w;
                        DrawTexturePro(markerIcon, (Rectangle){0,0,markerIcon.width,markerIcon.height}, 
                            (Rectangle){hx+x_off, hy, m_size_2d, m_size_2d}, (Vector2){m_size_2d/2.f, m_size_2d/2.f}, 0.0f, WHITE);
                        
                        if (Camera2DParams.zoom > 0.1f) {
                            DrawUIText(customFont, home_location.name, hx+x_off+(m_size_2d/2.f)+4.f, hy-(m_size_2d/2.f), m_text_2d, WHITE);
                        }
                    }

                    if (cfg.show_markers) {
                        for (int m = 0; m < marker_count; m++) {
                            float mx = (markers[m].lon / 360.0f) * map_w;
                            float my = -(markers[m].lat / 180.0f) * map_h;
                            for (int offset_i = -1; offset_i <= 1; offset_i++) {
                                float x_off = offset_i * map_w;
                                DrawTexturePro(markerIcon, (Rectangle){0,0,markerIcon.width,markerIcon.height}, 
                                    (Rectangle){mx+x_off, my, m_size_2d, m_size_2d}, (Vector2){m_size_2d/2.f, m_size_2d/2.f}, 0.0f, WHITE);
                                
                                if (Camera2DParams.zoom > 0.1f) {
                                    DrawUIText(customFont, markers[m].name, mx+x_off+(m_size_2d/2.f)+4.f, my-(m_size_2d/2.f), m_text_2d, WHITE);
                                }
                            }
                        }
                    }

                    EndScissorMode();
                }
            EndMode2D();
        } else {
            /* 3d globe rendering */
            BeginMode3D(Camera3DParams);
                earthModel.transform = MatrixRotateY((gmst_deg + cfg.earth_rotation_offset) * DEG2RAD);
                
                if (cfg.show_night_lights) {
                    earthModel.materials[0].shader = shader3D;
                    Vector3 sunEci = calculate_sun_position(current_epoch);
                    float earth_rot_rad = (gmst_deg + cfg.earth_rotation_offset) * DEG2RAD;
                    Vector3 sunEcef = Vector3Transform(sunEci, MatrixRotateY(-earth_rot_rad));
                    SetShaderValue(shader3D, sunDirLoc3D, &sunEcef, SHADER_UNIFORM_VEC3);
                } else {
                    earthModel.materials[0].shader = defaultEarthShader;
                }

                DrawModel(earthModel, Vector3Zero(), 1.0f, WHITE);

                /* atmosphere/cloud layer */
                if (cfg.show_clouds) {
                    double continuous_cloud_angle = fmod(gmst_deg + cfg.earth_rotation_offset + (current_epoch * 360.0 * 0.04), 360.0);
                    float cloud_rot_rad = (float)(continuous_cloud_angle * DEG2RAD);
                    cloudModel.transform = MatrixRotateY(cloud_rot_rad);
                    
                    if (cfg.show_night_lights) {
                        cloudModel.materials[0].shader = shaderCloud;
                        Vector3 sunEci = calculate_sun_position(current_epoch);
                        Vector3 sunCloudSpace = Vector3Transform(sunEci, MatrixRotateY(-cloud_rot_rad));
                        SetShaderValue(shaderCloud, sunDirLocCloud, &sunCloudSpace, SHADER_UNIFORM_VEC3);
                    } else {
                        cloudModel.materials[0].shader = defaultCloudShader;
                    }

                    DrawModel(cloudModel, Vector3Zero(), 1.0f, WHITE);
                }

                DrawModel(moonModel, draw_moon_pos, 1.0f, WHITE);

                /* 3d footprint triangles */
                if (active_sat && has_footprint && active_sat->is_active) {
                    for (int i = 0; i < FP_RINGS; i++) {
                        for (int k = 0; k < FP_PTS; k++) {
                            int next = (k + 1) % FP_PTS;
                            Vector3 p1 = Vector3Scale(fp_grid[i][k], 1.01f/DRAW_SCALE), p2 = Vector3Scale(fp_grid[i][next], 1.01f/DRAW_SCALE);
                            Vector3 p3 = Vector3Scale(fp_grid[i+1][k], 1.01f/DRAW_SCALE), p4 = Vector3Scale(fp_grid[i+1][next], 1.01f/DRAW_SCALE);
                            DrawTriangle3D(p1, p3, p2, cfg.footprint_bg);
                            DrawTriangle3D(p2, p3, p4, cfg.footprint_bg);
                        }
                    }
                    for (int k = 0; k < FP_PTS; k++) {
                        int next = (k + 1) % FP_PTS;
                        DrawLine3D(Vector3Scale(fp_grid[FP_RINGS][k], 1.01f/DRAW_SCALE), Vector3Scale(fp_grid[FP_RINGS][next], 1.01f/DRAW_SCALE), cfg.footprint_border);
                    }
                }

                for (int i = 0; i < sat_count; i++) {
                    if (!satellites[i].is_active) continue;
                    bool is_unselected = (selected_sat != NULL && &satellites[i] != selected_sat);
                    float sat_alpha = is_unselected ? unselected_fade : 1.0f;
                    if (sat_alpha <= 0.0f) continue;

                    bool is_hl = (active_sat == &satellites[i]);
                    draw_orbit_3d(&satellites[i], current_epoch, is_hl, sat_alpha);

                    if (is_hl) {
                        Vector3 draw_pos = Vector3Scale(satellites[i].current_pos, 1.0f / DRAW_SCALE);
                        DrawLine3D(Vector3Zero(), draw_pos, ApplyAlpha(cfg.orbit_highlighted, sat_alpha));
                    }
                }
            EndMode3D();

            /* screen-space icons/text for 3d objects */
            float m_size_3d = 24.0f * cfg.ui_scale;
            float m_text_3d = 16.0f * cfg.ui_scale;
            float mark_size_3d = 32.0f * cfg.ui_scale;

            if (active_sat && active_sat->is_active) {
                bool is_unselected = (selected_sat != NULL && active_sat != selected_sat);
                float sat_alpha = is_unselected ? unselected_fade : 1.0f;
                
                double t_peri_unix, t_apo_unix;
                get_apsis_times(active_sat, current_epoch, &t_peri_unix, &t_apo_unix);

                Vector3 draw_p = Vector3Scale(calculate_position(active_sat, t_peri_unix), 1.0f/DRAW_SCALE);
                Vector3 draw_a = Vector3Scale(calculate_position(active_sat, t_apo_unix), 1.0f/DRAW_SCALE);

                if (!IsOccludedByEarth(Camera3DParams.position, draw_p, draw_earth_radius)) {
                    Vector2 sp = GetWorldToScreen(draw_p, Camera3DParams);
                    DrawTexturePro(periMark, (Rectangle){0,0,periMark.width,periMark.height}, 
                        (Rectangle){sp.x, sp.y, mark_size_3d, mark_size_3d}, (Vector2){mark_size_3d/2.f, mark_size_3d/2.f}, 0.0f, ApplyAlpha(cfg.periapsis, sat_alpha));
                }
                if (!IsOccludedByEarth(Camera3DParams.position, draw_a, draw_earth_radius)) {
                    Vector2 sp = GetWorldToScreen(draw_a, Camera3DParams);
                    DrawTexturePro(apoMark, (Rectangle){0,0,apoMark.width,apoMark.height}, 
                        (Rectangle){sp.x, sp.y, mark_size_3d, mark_size_3d}, (Vector2){mark_size_3d/2.f, mark_size_3d/2.f}, 0.0f, ApplyAlpha(cfg.apoapsis, sat_alpha));
                }
            }

            for (int i = 0; i < sat_count; i++) {
                if (!satellites[i].is_active) continue;
                bool is_unselected = (selected_sat != NULL && &satellites[i] != selected_sat);
                float sat_alpha = is_unselected ? unselected_fade : 1.0f;
                if (sat_alpha <= 0.0f) continue;

                Vector3 draw_pos = Vector3Scale(satellites[i].current_pos, 1.0f / DRAW_SCALE);
                Vector3 toTarget = Vector3Subtract(draw_pos, Camera3DParams.position);
                Vector3 camForward = Vector3Normalize(Vector3Subtract(Camera3DParams.target, Camera3DParams.position));

                if (Vector3DotProduct(toTarget, camForward) > 0.0f && !IsOccludedByEarth(Camera3DParams.position, draw_pos, draw_earth_radius)) {
                    Color sCol = (selected_sat == &satellites[i]) ? cfg.sat_selected : (hovered_sat == &satellites[i]) ? cfg.sat_highlighted : cfg.sat_normal;
                    sCol = ApplyAlpha(sCol, sat_alpha);
                    Vector2 sp = GetWorldToScreen(draw_pos, Camera3DParams);
                    DrawTexturePro(satIcon, (Rectangle){0,0,satIcon.width,satIcon.height}, 
                        (Rectangle){sp.x, sp.y, m_size_3d, m_size_3d}, (Vector2){m_size_3d/2.f, m_size_3d/2.f}, 0.0f, sCol);
                }
            }

            Vector3 camForward = Vector3Normalize(Vector3Subtract(Camera3DParams.target, Camera3DParams.position));

            float h_lat_rad = home_location.lat * DEG2RAD, h_lon_rad = (home_location.lon + gmst_deg + cfg.earth_rotation_offset) * DEG2RAD;
            Vector3 h_pos = { cosf(h_lat_rad)*cosf(h_lon_rad)*draw_earth_radius, sinf(h_lat_rad)*draw_earth_radius, -cosf(h_lat_rad)*sinf(h_lon_rad)*draw_earth_radius };
            Vector3 h_normal = Vector3Normalize(h_pos);
            Vector3 h_viewDir = Vector3Normalize(Vector3Subtract(Camera3DParams.position, h_pos));
            Vector3 h_toTarget = Vector3Subtract(h_pos, Camera3DParams.position);
            
            if (Vector3DotProduct(h_normal, h_viewDir) > 0.0f && Vector3DotProduct(h_toTarget, camForward) > 0.0f) {
                Vector2 sp = GetWorldToScreen(h_pos, Camera3DParams);
                DrawTexturePro(markerIcon, (Rectangle){0,0,markerIcon.width,markerIcon.height}, 
                    (Rectangle){sp.x, sp.y, m_size_3d, m_size_3d}, (Vector2){m_size_3d/2.f, m_size_3d/2.f}, 0.0f, WHITE);
                
                if (camDistance < 50.0f) {
                    DrawUIText(customFont, home_location.name, sp.x+(m_size_3d/2.f)+4.f, sp.y-(m_size_3d/2.f), m_text_3d, WHITE);
                }
            }

            if (cfg.show_markers) {
                for (int m = 0; m < marker_count; m++) {
                    float lat_rad = markers[m].lat * DEG2RAD, lon_rad = (markers[m].lon + gmst_deg + cfg.earth_rotation_offset) * DEG2RAD;
                    Vector3 m_pos = { cosf(lat_rad)*cosf(lon_rad)*draw_earth_radius, sinf(lat_rad)*draw_earth_radius, -cosf(lat_rad)*sinf(lon_rad)*draw_earth_radius };
                    Vector3 normal = Vector3Normalize(m_pos);
                    Vector3 viewDir = Vector3Normalize(Vector3Subtract(Camera3DParams.position, m_pos));
                    Vector3 toTarget = Vector3Subtract(m_pos, Camera3DParams.position);

                    if (Vector3DotProduct(normal, viewDir) > 0.0f && Vector3DotProduct(toTarget, camForward) > 0.0f) {
                        Vector2 sp = GetWorldToScreen(m_pos, Camera3DParams);
                        DrawTexturePro(markerIcon, (Rectangle){0,0,markerIcon.width,markerIcon.height}, 
                            (Rectangle){sp.x, sp.y, m_size_3d, m_size_3d}, (Vector2){m_size_3d/2.f, m_size_3d/2.f}, 0.0f, WHITE);
                        
                        if (camDistance < 50.0f) {
                            DrawUIText(customFont, markers[m].name, sp.x+(m_size_3d/2.f)+4.f, sp.y-(m_size_3d/2.f), m_text_3d, WHITE);
                        }
                    }
                }
            }
        }

        /* ui overlay rendering */
        UIContext uiCtx = {
            .current_epoch = &current_epoch,
            .time_multiplier = &time_multiplier,
            .saved_multiplier = &saved_multiplier,
            .is_auto_warping = &is_auto_warping,
            .auto_warp_target = &auto_warp_target,
            .auto_warp_initial_diff = &auto_warp_initial_diff,
            .is_2d_view = &is_2d_view,
            .hide_unselected = &hide_unselected,
            .selected_sat = &selected_sat,
            .hovered_sat = hovered_sat,
            .active_sat = active_sat,
            .active_lock = &active_lock,
            .datetime_str = datetime_str,
            .gmst_deg = gmst_deg,
            .map_w = map_w,
            .map_h = map_h,
            .camera2d = &Camera2DParams,
            .camera3d = &Camera3DParams
        };
        DrawGUI(&uiCtx, &cfg, customFont);

        EndDrawing();
    }

    /* cleanup */
    UnloadTexture(logoTex);
    UnloadTexture(satIcon);
    UnloadTexture(markerIcon);
    UnloadTexture(periMark);
    UnloadTexture(apoMark);
    UnloadTexture(earthTexture);
    UnloadTexture(earthNightTexture);
    UnloadModel(earthModel);
    UnloadShader(shader3D);
    UnloadShader(shader2D);
    UnloadShader(shaderCloud);
    UnloadTexture(cloudTexture);
    UnloadModel(cloudModel);
    UnloadTexture(moonTexture);
    UnloadModel(moonModel);
    UnloadFont(customFont);
    CloseWindow();
    return 0;
}