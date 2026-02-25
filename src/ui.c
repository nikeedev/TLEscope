/* headers and defines */
#define _GNU_SOURCE
#include "ui.h"
#include "astro.h"
#include <raymath.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <ctype.h>

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

typedef struct {
    const char* id;
    const char* name;
    const char* url;
} TLESource_t;

static TLESource_t SOURCES[] = {
    {"1", "Last 30 Days' Launches", "https://celestrak.org/NORAD/elements/gp.php?GROUP=last-30-days&FORMAT=tle"},
    {"2", "Space Stations", "https://celestrak.org/NORAD/elements/gp.php?GROUP=stations&FORMAT=tle"},
    {"3", "100 Brightest", "https://celestrak.org/NORAD/elements/gp.php?GROUP=visual&FORMAT=tle"},
    {"4", "Active Satellites", "https://celestrak.org/NORAD/elements/gp.php?GROUP=active&FORMAT=tle"},
    {"5", "Analyst Satellites", "https://celestrak.org/NORAD/elements/gp.php?GROUP=analyst&FORMAT=tle"},
    {"6", "Russian ASAT (COSMOS 1408)", "https://celestrak.org/NORAD/elements/gp.php?GROUP=cosmos-1408-debris&FORMAT=tle"},
    {"7", "Chinese ASAT (FENGYUN 1C)", "https://celestrak.org/NORAD/elements/gp.php?GROUP=fengyun-1c-debris&FORMAT=tle"},
    {"8", "IRIDIUM 33 Debris", "https://celestrak.org/NORAD/elements/gp.php?GROUP=iridium-33-debris&FORMAT=tle"},
    {"9", "COSMOS 2251 Debris", "https://celestrak.org/NORAD/elements/gp.php?GROUP=cosmos-2251-debris&FORMAT=tle"},
    {"10", "Weather", "https://celestrak.org/NORAD/elements/gp.php?GROUP=weather&FORMAT=tle"},
    {"11", "NOAA", "https://celestrak.org/NORAD/elements/gp.php?GROUP=noaa&FORMAT=tle"},
    {"12", "GOES", "https://celestrak.org/NORAD/elements/gp.php?GROUP=goes&FORMAT=tle"},
    {"13", "Earth Resources", "https://celestrak.org/NORAD/elements/gp.php?GROUP=resource&FORMAT=tle"},
    {"14", "SARSAT", "https://celestrak.org/NORAD/elements/gp.php?GROUP=sarsat&FORMAT=tle"},
    {"15", "Disaster Monitoring", "https://celestrak.org/NORAD/elements/gp.php?GROUP=dmc&FORMAT=tle"},
    {"16", "TDRSS", "https://celestrak.org/NORAD/elements/gp.php?GROUP=tdrss&FORMAT=tle"},
    {"17", "ARGOS", "https://celestrak.org/NORAD/elements/gp.php?GROUP=argos&FORMAT=tle"},
    {"18", "Planet", "https://celestrak.org/NORAD/elements/gp.php?GROUP=planet&FORMAT=tle"},
    {"19", "Spire", "https://celestrak.org/NORAD/elements/gp.php?GROUP=spire&FORMAT=tle"},
    {"20", "Starlink", "https://celestrak.org/NORAD/elements/gp.php?GROUP=starlink&FORMAT=tle"},
    {"21", "OneWeb", "https://celestrak.org/NORAD/elements/gp.php?GROUP=oneweb&FORMAT=tle"},
    {"22", "GPS Operational", "https://celestrak.org/NORAD/elements/gp.php?GROUP=gps-ops&FORMAT=tle"},
    {"23", "Galileo", "https://celestrak.org/NORAD/elements/gp.php?GROUP=galileo&FORMAT=tle"},
    {"24", "Amateur Radio", "https://celestrak.org/NORAD/elements/gp.php?GROUP=amateur&FORMAT=tle"},
    {"25", "CubeSats", "https://celestrak.org/NORAD/elements/gp.php?GROUP=cubesat&FORMAT=tle"}
};

static TLESource_t RETLECTOR_SOURCES[] = {
    {"1", "All active", "https://retlector.eu/tle/active"},
    {"2", "100 Brightest", "https://retlector.eu/tle/visual"},
    {"3", "Analyst satellites", "https://retlector.eu/tle/analyst"},
    {"4", "Cosmos 1408 ASAT test debris", "https://retlector.eu/tle/cosmos-1408"},
    {"5", "Fengyun-1C ASAT test debris", "https://retlector.eu/tle/fengyun-1c-debris"},
    {"6", "Iridium 33 collision debris", "https://retlector.eu/tle/iridium-33-debris"},
    {"7", "Cosmos 2251 collision debris", "https://retlector.eu/tle/cosmos-2251-debris"},
    {"8", "Weather satellites", "https://retlector.eu/tle/weather"},
    {"9", "NOAA satellites", "https://retlector.eu/tle/noaa"},
    {"10", "GOES satellites", "https://retlector.eu/tle/goes"},
    {"11", "Earth resources satellites", "https://retlector.eu/tle/resources"},
    {"12", "SARSAT payload satellites", "https://retlector.eu/tle/sarsat"},
    {"13", "Disaster monitoring satellites", "https://retlector.eu/tle/dmc"},
    {"14", "TDRSS system", "https://retlector.eu/tle/tdrss"},
    {"15", "Argos satellites", "https://retlector.eu/tle/argos"},
    {"16", "Space stations", "https://retlector.eu/tle/station"},
    {"17", "Geostationary satellites", "https://retlector.eu/tle/geo"},
    {"18", "GNSS satellites", "https://retlector.eu/tle/gnss"},
    {"19", "GPS operational satellites", "https://retlector.eu/tle/gps-ops"},
    {"20", "GLONASS operational satellites", "https://retlector.eu/tle/glo-ops"},
    {"21", "Galileo satellites", "https://retlector.eu/tle/galileo"},
    {"22", "BeiDou satellites", "https://retlector.eu/tle/beidou"},
    {"23", "SBAS satellites", "https://retlector.eu/tle/sbas"},
    {"24", "Education satellites", "https://retlector.eu/tle/education"}
};
#define NUM_RETLECTOR_SOURCES 24

/* ui state variables */
static bool show_help = false;
static bool show_settings = false;
static bool show_passes_dialog = false;
static bool show_polar_dialog = false;
static bool show_doppler_dialog = false;
static bool show_tle_warning = false;
static bool show_exit_dialog = false;

static bool opened_once_settings = false;
static bool opened_once_help = false;
static bool opened_once_sat_mgr = false;
static bool opened_once_tle_mgr = false;
static bool opened_once_time = false;
static bool opened_once_passes = false;
static bool opened_once_polar = false;
static bool opened_once_doppler = false;

static bool show_sat_mgr_dialog = false;
static bool drag_sat_mgr = false;
static Vector2 drag_sat_mgr_off = {0};
static float sm_x = 200.0f, sm_y = 150.0f;
static Vector2 sat_mgr_scroll = {0};
static char sat_search_text[64] = "";
static bool edit_sat_search = false;

static bool show_tle_mgr_dialog = false;
static bool drag_tle_mgr = false;
static Vector2 drag_tle_mgr_off = {0};
static float tm_x = 250.0f, tm_y = 150.0f;
static Vector2 tle_mgr_scroll = {0};
static bool celestrak_expanded = false;
static bool retlector_expanded = false;
static bool retlector_selected[NUM_RETLECTOR_SOURCES] = {false};
static bool other_expanded = false;
static bool celestrak_selected[25] = {false};
static long data_tle_epoch = -1;

static int selected_pass_idx = -1;
static bool multi_pass_mode = true;
static Satellite* locked_pass_sat = NULL;
static double locked_pass_aos = 0.0;
static double locked_pass_los = 0.0;
static char text_min_el[8] = "0";
static bool edit_min_el = false;

static float hw_x = 100.0f, hw_y = 250.0f;
static float sw_x = 100.0f, sw_y = 250.0f;
static float pl_x = 550.0f, pl_y = 150.0f;
static float pd_x = 0.0f, pd_y = 0.0f;

static bool drag_help = false, drag_settings = false, drag_passes = false, drag_polar = false;
static Vector2 drag_help_off = {0}, drag_settings_off = {0}, drag_passes_off = {0}, drag_polar_off = {0};

static bool show_time_dialog = false;
static bool drag_time_dialog = false;
static Vector2 drag_time_off = {0};
static float td_x = 300.0f, td_y = 100.0f;

static Vector2 passes_scroll = {0};

static char text_doppler_freq[32] = "137625000"; 
static char text_doppler_res[32] = "1"; 
static char text_doppler_file[128] = "doppler_export.csv";
static bool edit_doppler_freq = false;
static bool edit_doppler_res = false;
static bool edit_doppler_file = false;
static bool drag_doppler = false;
static Vector2 drag_doppler_off = {0};
static float dop_x = 200.0f, dop_y = 150.0f;

static char text_year[8] = "2026", text_month[4] = "1", text_day[4] = "1";
static char text_hour[4] = "12", text_min[4] = "0", text_sec[4] = "0";
static char text_unix[64] = "0";
static bool edit_year = false, edit_month = false, edit_day = false;
static bool edit_hour = false, edit_min = false, edit_sec = false;
static bool edit_unix = false;

static char text_hl_name[64] = "";
static char text_hl_lat[32] = "";
static char text_hl_lon[32] = "";
static char text_hl_alt[32] = "";
static bool edit_hl_name = false, edit_hl_lat = false, edit_hl_lon = false, edit_hl_alt = false;
static float last_hl_lat = -999, last_hl_lon = -999, last_hl_alt = -999;

static float tt_hover[13] = {0};
static bool ui_initialized = false;

/* shared helper functions */
bool IsOccludedByEarth(Vector3 camPos, Vector3 targetPos, float earthRadius) {
    Vector3 v = Vector3Subtract(targetPos, camPos);
    float L = Vector3Length(v);
    Vector3 d = Vector3Scale(v, 1.0f / L);
    float t = -Vector3DotProduct(camPos, d);
    if (t > 0.0f && t < L) {
        Vector3 closest = Vector3Add(camPos, Vector3Scale(d, t));
        if (Vector3Length(closest) < earthRadius * 0.99f) return true; 
    }
    return false;
}

/* apply alpha transparency to color */
Color ApplyAlpha(Color c, float alpha) {
    if (alpha < 0.0f) alpha = 0.0f;
    if (alpha > 1.0f) alpha = 1.0f;
    c.a = (unsigned char)(c.a * alpha);
    return c;
}

/* render text using custom font */
void DrawUIText(Font font, const char* text, float x, float y, float size, Color color) {
    DrawTextEx(font, text, (Vector2){x, y}, size, 1.0f, color);
}

/* handle time warping increments */
double StepTimeMultiplier(double current, bool increase) {
    if (increase) {
        if (current == 0.0) return 0.25;
        if (current >= 0.25) return current * 2.0;
        if (current <= -0.25) {
            if (current == -0.25) return 0.0;
            return current / 2.0;
        }
    } else {
        if (current == 0.0) return -0.25;
        if (current <= -0.25) return current * 2.0;
        if (current >= 0.25) {
            if (current == 0.25) return 0.0;
            return current / 2.0;
        }
    }
    return current;
}

/* convert unix timestamp to astronomical epoch */
double unix_to_epoch(double target_unix) {
    double e0 = get_current_real_time_epoch();
    double u0 = get_unix_from_epoch(e0);
    return e0 + (target_unix - u0) / 86400.0;
}

/* case-insensitive string search */
static bool string_contains_ignore_case(const char* haystack, const char* needle) {
    if (!needle || !*needle) return true;
    int h_len = strlen(haystack);
    int n_len = strlen(needle);
    for (int i = 0; i <= h_len - n_len; i++) {
        int j;
        for (j = 0; j < n_len; j++) {
            if (tolower((unsigned char)haystack[i+j]) != tolower((unsigned char)needle[j])) break;
        }
        if (j == n_len) return true;
    }
    return false;
}

/* snap window to screen edges */
static void SnapWindow(float *x, float *y, float w, float h, AppConfig* cfg) {
    float margin = 10.0f * cfg->ui_scale;
    float threshold = 20.0f * cfg->ui_scale;
    int sw = GetScreenWidth();
    int sh = GetScreenHeight();
    
    if (fabsf(*x - margin) < threshold) *x = margin;
    if (fabsf((*x + w) - (sw - margin)) < threshold) *x = sw - margin - w;
    if (fabsf(*y - margin) < threshold) *y = margin;
    if (fabsf((*y + h) - (sh - margin)) < threshold) *y = sh - margin - h;
}

/* find empty screen coordinates for newly spawned windows */
static void FindSmartWindowPosition(float w, float h, AppConfig* cfg, float* out_x, float* out_y) {
    float margin = 10.0f * cfg->ui_scale;
    int sw = GetScreenWidth();
    int sh = GetScreenHeight();

    Rectangle active[10];
    int count = 0;
    if (show_help) active[count++] = (Rectangle){hw_x, hw_y, 900*cfg->ui_scale, 140*cfg->ui_scale};
    if (show_settings) active[count++] = (Rectangle){sw_x, sw_y, 250*cfg->ui_scale, 440*cfg->ui_scale};
    if (show_time_dialog) active[count++] = (Rectangle){td_x, td_y, 252*cfg->ui_scale, 320*cfg->ui_scale};
    if (show_passes_dialog) active[count++] = (Rectangle){pd_x, pd_y, 357*cfg->ui_scale, 380*cfg->ui_scale};
    if (show_polar_dialog) active[count++] = (Rectangle){pl_x, pl_y, 300*cfg->ui_scale, 430*cfg->ui_scale};
    if (show_doppler_dialog) active[count++] = (Rectangle){dop_x, dop_y, 320*cfg->ui_scale, 480*cfg->ui_scale};
    if (show_sat_mgr_dialog) active[count++] = (Rectangle){sm_x, sm_y, 400*cfg->ui_scale, 500*cfg->ui_scale};
    if (show_tle_mgr_dialog) active[count++] = (Rectangle){tm_x, tm_y, 400*cfg->ui_scale, 500*cfg->ui_scale};

    float candidates_x[] = { margin, sw - w - margin };
    float step_y = 20.0f * cfg->ui_scale;

    for (int i = 0; i < 2; i++) {
        float test_x = candidates_x[i];
        for (float test_y = margin; test_y <= sh - h - margin; test_y += step_y) {
            Rectangle test_rect = { test_x - margin/2, test_y - margin/2, w + margin, h + margin };
            bool collision = false;
            for (int j = 0; j < count; j++) {
                if (CheckCollisionRecs(test_rect, active[j])) {
                    collision = true;
                    break;
                }
            }
            if (!collision) {
                *out_x = test_x;
                *out_y = test_y;
                return;
            }
        }
    }
    
    // fallback - cascade from top left if screen too crowded :ooo1
    static float cascade = 0;
    *out_x = 50 * cfg->ui_scale + cascade;
    *out_y = 50 * cfg->ui_scale + cascade;
    cascade += 20 * cfg->ui_scale;
    if (cascade > 200 * cfg->ui_scale) cascade = 0;
}

/* ui system state checks */
bool IsUITyping(void) {
    return edit_year || edit_month || edit_day || 
           edit_hour || edit_min || edit_sec || edit_unix ||
           edit_doppler_freq || edit_doppler_res || edit_doppler_file ||
           edit_sat_search || edit_min_el ||
           edit_hl_name || edit_hl_lat || edit_hl_lon || edit_hl_alt;
}

/* toggle out-of-date tle notification */
void ToggleTLEWarning(void) {
    show_tle_warning = !show_tle_warning;
}

/* detect if mouse is hovering over ui elements */
bool IsMouseOverUI(AppConfig* cfg) {
    if (show_exit_dialog) return true;
    if (!ui_initialized) {
        pd_x = GetScreenWidth() - 400.0f;
        pd_y = GetScreenHeight() - 400.0f;
        ui_initialized = true;
    }

    bool over_ui = false;
    bool over_window = false;

    float pass_w = 357 * cfg->ui_scale;
    float pass_h = 380 * cfg->ui_scale;

    Rectangle helpWindow = { hw_x, hw_y, 900 * cfg->ui_scale, 140 * cfg->ui_scale };
    Rectangle settingsWindow = { sw_x, sw_y, 250 * cfg->ui_scale, 440 * cfg->ui_scale };
    Rectangle timeWindow = { td_x, td_y, 252 * cfg->ui_scale, 320 * cfg->ui_scale };
    Rectangle tleWindow = { (GetScreenWidth() - 300*cfg->ui_scale)/2.0f, (GetScreenHeight() - 130*cfg->ui_scale)/2.0f, 300*cfg->ui_scale, 130*cfg->ui_scale };
    Rectangle passesWindow = { pd_x, pd_y, pass_w, pass_h };
    Rectangle polarWindow = { pl_x, pl_y, 300 * cfg->ui_scale, 430 * cfg->ui_scale };
    Rectangle dopplerWindow = { dop_x, dop_y, 320 * cfg->ui_scale, 480 * cfg->ui_scale };
    Rectangle smWindow = { sm_x, sm_y, 400 * cfg->ui_scale, 500 * cfg->ui_scale };
    Rectangle tmMgrWindow = { tm_x, tm_y, 400 * cfg->ui_scale, 500 * cfg->ui_scale };

    if (show_help && CheckCollisionPointRec(GetMousePosition(), helpWindow)) over_window = true;
    if (show_settings && CheckCollisionPointRec(GetMousePosition(), settingsWindow)) over_window = true;
    if (show_time_dialog && CheckCollisionPointRec(GetMousePosition(), timeWindow)) over_window = true;
    if (show_passes_dialog && CheckCollisionPointRec(GetMousePosition(), passesWindow)) over_window = true;
    if (show_polar_dialog && CheckCollisionPointRec(GetMousePosition(), polarWindow)) over_window = true;
    if (show_doppler_dialog && CheckCollisionPointRec(GetMousePosition(), dopplerWindow)) over_window = true;
    if (show_sat_mgr_dialog && CheckCollisionPointRec(GetMousePosition(), smWindow)) over_window = true;
    if (show_tle_mgr_dialog && CheckCollisionPointRec(GetMousePosition(), tmMgrWindow)) over_window = true;
    if (show_tle_warning && CheckCollisionPointRec(GetMousePosition(), tleWindow)) over_window = true;

    if (over_window) over_ui = true;

    float buttons_w = (5 * 35 - 5) * cfg->ui_scale;
    float center_x_bottom = (GetScreenWidth() - buttons_w) / 2.0f;
    float btn_start_x = center_x_bottom;

    Rectangle btnRewind = { btn_start_x, GetScreenHeight() - 40 * cfg->ui_scale, 30 * cfg->ui_scale, 30 * cfg->ui_scale };
    Rectangle btnPlayPause = { btn_start_x + 35 * cfg->ui_scale, GetScreenHeight() - 40 * cfg->ui_scale, 30 * cfg->ui_scale, 30 * cfg->ui_scale };
    Rectangle btnFastForward = { btn_start_x + 70 * cfg->ui_scale, GetScreenHeight() - 40 * cfg->ui_scale, 30 * cfg->ui_scale, 30 * cfg->ui_scale };
    Rectangle btnNow = { btn_start_x + 105 * cfg->ui_scale, GetScreenHeight() - 40 * cfg->ui_scale, 30 * cfg->ui_scale, 30 * cfg->ui_scale };
    Rectangle btnClock = { btn_start_x + 140 * cfg->ui_scale, GetScreenHeight() - 40 * cfg->ui_scale, 30 * cfg->ui_scale, 30 * cfg->ui_scale };

    float center_x_top = (GetScreenWidth() - (8 * 35 - 5) * cfg->ui_scale) / 2.0f;
    Rectangle btnSet  = { center_x_top, 10 * cfg->ui_scale, 30 * cfg->ui_scale, 30 * cfg->ui_scale };
    Rectangle btnHelp = { center_x_top + 35 * cfg->ui_scale, 10 * cfg->ui_scale, 30 * cfg->ui_scale, 30 * cfg->ui_scale };
    Rectangle btn2D3D = { center_x_top + 70 * cfg->ui_scale, 10 * cfg->ui_scale, 30 * cfg->ui_scale, 30 * cfg->ui_scale };
    Rectangle btnSatMgr = { center_x_top + 105 * cfg->ui_scale, 10 * cfg->ui_scale, 30 * cfg->ui_scale, 30 * cfg->ui_scale };
    Rectangle btnHideUnselected = { center_x_top + 140 * cfg->ui_scale, 10 * cfg->ui_scale, 30 * cfg->ui_scale, 30 * cfg->ui_scale };
    Rectangle btnPasses = { center_x_top + 175 * cfg->ui_scale, 10 * cfg->ui_scale, 30 * cfg->ui_scale, 30 * cfg->ui_scale };
    Rectangle btnTLEMgr = { center_x_top + 210 * cfg->ui_scale, 10 * cfg->ui_scale, 30 * cfg->ui_scale, 30 * cfg->ui_scale };
    Rectangle btnSunlit = { center_x_top + 245 * cfg->ui_scale, 10 * cfg->ui_scale, 30 * cfg->ui_scale, 30 * cfg->ui_scale };

    if (CheckCollisionPointRec(GetMousePosition(), btnHelp)) over_ui = true;
    if (CheckCollisionPointRec(GetMousePosition(), btnSet)) over_ui = true;
    if (CheckCollisionPointRec(GetMousePosition(), btnRewind)) over_ui = true;
    if (CheckCollisionPointRec(GetMousePosition(), btnPlayPause)) over_ui = true;
    if (CheckCollisionPointRec(GetMousePosition(), btnFastForward)) over_ui = true;
    if (CheckCollisionPointRec(GetMousePosition(), btnNow)) over_ui = true;
    if (CheckCollisionPointRec(GetMousePosition(), btnClock)) over_ui = true;
    if (CheckCollisionPointRec(GetMousePosition(), btnPasses)) over_ui = true;
    if (CheckCollisionPointRec(GetMousePosition(), btn2D3D)) over_ui = true;
    if (CheckCollisionPointRec(GetMousePosition(), btnSatMgr)) over_ui = true;
    if (CheckCollisionPointRec(GetMousePosition(), btnHideUnselected)) over_ui = true;
    if (CheckCollisionPointRec(GetMousePosition(), btnTLEMgr)) over_ui = true;
    if (CheckCollisionPointRec(GetMousePosition(), btnSunlit)) over_ui = true;

    return over_ui;
}

void SaveSatSelection(void) {
    FILE* f = fopen("persistence.bin", "wb");
    if (!f) return;
    int count = 0;
    for (int i = 0; i < sat_count; i++) if (satellites[i].is_active) count++;
    fwrite(&count, sizeof(int), 1, f);
    for (int i = 0; i < sat_count; i++) {
        if (satellites[i].is_active) {
            unsigned char len = (unsigned char)strlen(satellites[i].name);
            fwrite(&len, 1, 1, f);
            fwrite(satellites[i].name, 1, len, f);
        }
    }
    fclose(f);
}

void LoadSatSelection(void) {
    FILE* f = fopen("persistence.bin", "rb");
    if (!f) return;
    int count;
    if (fread(&count, sizeof(int), 1, f) != 1) { fclose(f); return; }
    
    /* disable all satellites first so only saved ones remain active */
    for (int i = 0; i < sat_count; i++) {
        satellites[i].is_active = false;
    }

    for (int i = 0; i < count; i++) {
        unsigned char len;
        char name[64] = {0};
        if (fread(&len, 1, 1, f) != 1) break;
        fread(name, 1, len, f);
        for (int j = 0; j < sat_count; j++) {
            if (strcmp(satellites[j].name, name) == 0) {
                satellites[j].is_active = true;
                break;
            }
        }
    }
    fclose(f);
}

/* main ui rendering loop */
void DrawGUI(UIContext* ctx, AppConfig* cfg, Font customFont) {
    if (IsKeyPressed(KEY_ESCAPE)) {
        if (IsUITyping()) {
            // Drop textbox focus if currently typing
            edit_year = edit_month = edit_day = edit_hour = edit_min = edit_sec = edit_unix = false;
            edit_doppler_freq = edit_doppler_res = edit_doppler_file = false;
            edit_sat_search = edit_min_el = false;
            edit_hl_name = edit_hl_lat = edit_hl_lon = edit_hl_alt = false;
        } else {
            show_exit_dialog = !show_exit_dialog;
        }
    }

    /* Centralized Drag Management prioritizing top-most visually layered windows */
    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
        drag_help = drag_settings = drag_time_dialog = drag_passes = drag_polar = drag_doppler = drag_sat_mgr = drag_tle_mgr = false;
    }

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 m = GetMousePosition();
        /* evaluate in exact reverse draw-order. Deduct 30 pixels from drag width to preserve close button usability. */
        if (show_doppler_dialog && CheckCollisionPointRec(m, (Rectangle){dop_x, dop_y, 320*cfg->ui_scale - 30*cfg->ui_scale, 24*cfg->ui_scale})) {
            drag_doppler = true; drag_doppler_off = Vector2Subtract(m, (Vector2){dop_x, dop_y});
        }
        else if (show_polar_dialog && CheckCollisionPointRec(m, (Rectangle){pl_x, pl_y, 300*cfg->ui_scale - 30*cfg->ui_scale, 24*cfg->ui_scale})) {
            drag_polar = true; drag_polar_off = Vector2Subtract(m, (Vector2){pl_x, pl_y});
        }
        else if (show_passes_dialog && CheckCollisionPointRec(m, (Rectangle){pd_x, pd_y, 357*cfg->ui_scale - 30*cfg->ui_scale, 30*cfg->ui_scale})) {
            drag_passes = true; drag_passes_off = Vector2Subtract(m, (Vector2){pd_x, pd_y});
        }
        else if (show_time_dialog && CheckCollisionPointRec(m, (Rectangle){td_x, td_y, 252*cfg->ui_scale - 30*cfg->ui_scale, 24*cfg->ui_scale})) {
            drag_time_dialog = true; drag_time_off = Vector2Subtract(m, (Vector2){td_x, td_y});
        }
        else if (show_settings && CheckCollisionPointRec(m, (Rectangle){sw_x, sw_y, 250*cfg->ui_scale - 30*cfg->ui_scale, 24*cfg->ui_scale})) {
            drag_settings = true; drag_settings_off = Vector2Subtract(m, (Vector2){sw_x, sw_y});
        }
        else if (show_help && CheckCollisionPointRec(m, (Rectangle){hw_x, hw_y, 900*cfg->ui_scale - 30*cfg->ui_scale, 24*cfg->ui_scale})) {
            drag_help = true; drag_help_off = Vector2Subtract(m, (Vector2){hw_x, hw_y});
        }
        else if (show_sat_mgr_dialog && CheckCollisionPointRec(m, (Rectangle){sm_x, sm_y, 400*cfg->ui_scale - 30*cfg->ui_scale, 24*cfg->ui_scale})) {
            drag_sat_mgr = true; drag_sat_mgr_off = Vector2Subtract(m, (Vector2){sm_x, sm_y});
        }
        else if (show_tle_mgr_dialog && CheckCollisionPointRec(m, (Rectangle){tm_x, tm_y, 400*cfg->ui_scale - 30*cfg->ui_scale, 24*cfg->ui_scale})) {
            drag_tle_mgr = true; drag_tle_mgr_off = Vector2Subtract(m, (Vector2){tm_x, tm_y});
        }
    }

    if (show_passes_dialog) {
        if (multi_pass_mode) {
            if (last_pass_calc_sat != NULL || (num_passes > 0 && *ctx->current_epoch > passes[0].los_epoch + 1.0/1440.0)) {
                CalculatePasses(NULL, *ctx->current_epoch);
            }
        } else {
            if (*ctx->selected_sat == NULL) {
                num_passes = 0;
                last_pass_calc_sat = NULL; 
            } else if (last_pass_calc_sat != *ctx->selected_sat || (num_passes > 0 && *ctx->current_epoch > passes[0].los_epoch + 1.0/1440.0)) {
                CalculatePasses(*ctx->selected_sat, *ctx->current_epoch);
            }
        }
    }

    if (show_polar_dialog && locked_pass_sat != NULL) {
        int found_idx = -1;
        for (int i = 0; i < num_passes; i++) {
            if (passes[i].sat == locked_pass_sat && fabs(passes[i].aos_epoch - locked_pass_aos) < (1.0/86400.0)) {
                found_idx = i;
                break;
            }
        }
        if (found_idx == -1 && *ctx->current_epoch > locked_pass_los) {
            for (int i = 0; i < num_passes; i++) {
                if (passes[i].sat == locked_pass_sat && passes[i].los_epoch > *ctx->current_epoch) {
                    found_idx = i;
                    locked_pass_aos = passes[i].aos_epoch;
                    locked_pass_los = passes[i].los_epoch;
                    break;
                }
            }
        }
        selected_pass_idx = found_idx;
    }

    if (!edit_hl_lat && !edit_hl_lon && !edit_hl_alt && !edit_hl_name) {
        if (home_location.lat != last_hl_lat || home_location.lon != last_hl_lon || home_location.alt != last_hl_alt) {
            sprintf(text_hl_lat, "%.4f", home_location.lat);
            sprintf(text_hl_lon, "%.4f", home_location.lon);
            sprintf(text_hl_alt, "%.4f", home_location.alt);
            strncpy(text_hl_name, home_location.name, 63);
            last_hl_lat = home_location.lat; last_hl_lon = home_location.lon; last_hl_alt = home_location.alt;
        }
    }

     /* configure raygui global styles */
    GuiSetFont(customFont);
    GuiSetStyle(DEFAULT, TEXT_SIZE, 16 * cfg->ui_scale);
    GuiSetStyle(DEFAULT, BACKGROUND_COLOR, ColorToInt(cfg->ui_primary));
    GuiSetStyle(DEFAULT, LINE_COLOR, ColorToInt(cfg->ui_secondary)); 
    GuiSetStyle(DEFAULT, BASE_COLOR_NORMAL, ColorToInt(cfg->ui_primary));
    GuiSetStyle(DEFAULT, BASE_COLOR_FOCUSED, ColorToInt(cfg->ui_secondary));
    GuiSetStyle(DEFAULT, BASE_COLOR_PRESSED, ColorToInt(cfg->ui_accent));
    GuiSetStyle(DEFAULT, BORDER_COLOR_NORMAL, ColorToInt(cfg->ui_secondary));
    GuiSetStyle(DEFAULT, BORDER_COLOR_FOCUSED, ColorToInt(cfg->ui_accent));
    GuiSetStyle(DEFAULT, BORDER_COLOR_PRESSED, ColorToInt(cfg->ui_accent));
    GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, ColorToInt(cfg->text_main));
    GuiSetStyle(DEFAULT, TEXT_COLOR_FOCUSED, ColorToInt(cfg->text_main));
    GuiSetStyle(DEFAULT, TEXT_COLOR_PRESSED, ColorToInt(cfg->text_main)); 
    GuiSetStyle(CHECKBOX, TEXT_PADDING, 8 * cfg->ui_scale);
    GuiSetStyle(TEXTBOX, BORDER_COLOR_FOCUSED, ColorToInt(cfg->ui_secondary));
    GuiSetStyle(TEXTBOX, BORDER_COLOR_PRESSED, ColorToInt(cfg->ui_secondary));
    GuiSetStyle(TEXTBOX, TEXT_COLOR_FOCUSED, ColorToInt(cfg->text_main));
    GuiSetStyle(TEXTBOX, TEXT_COLOR_PRESSED, ColorToInt(cfg->text_main));
    GuiSetStyle(TEXTBOX, BASE_COLOR_PRESSED, ColorToInt(cfg->ui_primary));

    /* layout references */
    float pass_w = 357 * cfg->ui_scale;
    float pass_h = 380 * cfg->ui_scale;
    Rectangle helpWindow = { hw_x, hw_y, 900 * cfg->ui_scale, 140 * cfg->ui_scale };
    Rectangle settingsWindow = { sw_x, sw_y, 250 * cfg->ui_scale, 440 * cfg->ui_scale };
    Rectangle timeWindow = { td_x, td_y, 252 * cfg->ui_scale, 320 * cfg->ui_scale };
    Rectangle tleWindow = { (GetScreenWidth() - 300*cfg->ui_scale)/2.0f, (GetScreenHeight() - 130*cfg->ui_scale)/2.0f, 300*cfg->ui_scale, 130*cfg->ui_scale };
    Rectangle passesWindow = { pd_x, pd_y, pass_w, pass_h };
    Rectangle polarWindow = { pl_x, pl_y, 300 * cfg->ui_scale, 430 * cfg->ui_scale };
    Rectangle dopplerWindow = { dop_x, dop_y, 320 * cfg->ui_scale, 480 * cfg->ui_scale };
    Rectangle smWindow = { sm_x, sm_y, 400 * cfg->ui_scale, 500 * cfg->ui_scale };
    Rectangle tmMgrWindow = { tm_x, tm_y, 400 * cfg->ui_scale, 500 * cfg->ui_scale };

    float buttons_w = (5 * 35 - 5) * cfg->ui_scale;
    float center_x_bottom = (GetScreenWidth() - buttons_w) / 2.0f;
    float btn_start_x = center_x_bottom;

    Rectangle btnRewind = { btn_start_x, GetScreenHeight() - 40 * cfg->ui_scale, 30 * cfg->ui_scale, 30 * cfg->ui_scale };
    Rectangle btnPlayPause = { btn_start_x + 35 * cfg->ui_scale, GetScreenHeight() - 40 * cfg->ui_scale, 30 * cfg->ui_scale, 30 * cfg->ui_scale };
    Rectangle btnFastForward = { btn_start_x + 70 * cfg->ui_scale, GetScreenHeight() - 40 * cfg->ui_scale, 30 * cfg->ui_scale, 30 * cfg->ui_scale };
    Rectangle btnNow = { btn_start_x + 105 * cfg->ui_scale, GetScreenHeight() - 40 * cfg->ui_scale, 30 * cfg->ui_scale, 30 * cfg->ui_scale };
    Rectangle btnClock = { btn_start_x + 140 * cfg->ui_scale, GetScreenHeight() - 40 * cfg->ui_scale, 30 * cfg->ui_scale, 30 * cfg->ui_scale };

    float center_x_top = (GetScreenWidth() - (8 * 35 - 5) * cfg->ui_scale) / 2.0f;
    Rectangle btnSet  = { center_x_top, 10 * cfg->ui_scale, 30 * cfg->ui_scale, 30 * cfg->ui_scale };
    Rectangle btnHelp = { center_x_top + 35 * cfg->ui_scale, 10 * cfg->ui_scale, 30 * cfg->ui_scale, 30 * cfg->ui_scale };
    Rectangle btn2D3D = { center_x_top + 70 * cfg->ui_scale, 10 * cfg->ui_scale, 30 * cfg->ui_scale, 30 * cfg->ui_scale };
    Rectangle btnSatMgr = { center_x_top + 105 * cfg->ui_scale, 10 * cfg->ui_scale, 30 * cfg->ui_scale, 30 * cfg->ui_scale };
    Rectangle btnHideUnselected = { center_x_top + 140 * cfg->ui_scale, 10 * cfg->ui_scale, 30 * cfg->ui_scale, 30 * cfg->ui_scale };
    Rectangle btnPasses = { center_x_top + 175 * cfg->ui_scale, 10 * cfg->ui_scale, 30 * cfg->ui_scale, 30 * cfg->ui_scale };
    Rectangle btnTLEMgr = { center_x_top + 210 * cfg->ui_scale, 10 * cfg->ui_scale, 30 * cfg->ui_scale, 30 * cfg->ui_scale };
    Rectangle btnSunlit = { center_x_top + 245 * cfg->ui_scale, 10 * cfg->ui_scale, 30 * cfg->ui_scale, 30 * cfg->ui_scale };

    /* render context-sensitive satellite information box */
    if (ctx->active_sat && ctx->active_sat->is_active) {
        Vector2 screenPos;
        if (*ctx->is_2d_view) {
            float sat_mx, sat_my;
            get_map_coordinates(ctx->active_sat->current_pos, ctx->gmst_deg, cfg->earth_rotation_offset, ctx->map_w, ctx->map_h, &sat_mx, &sat_my);
            float cam_x = ctx->camera2d->target.x;
            
            while (sat_mx - cam_x > ctx->map_w/2.0f) sat_mx -= ctx->map_w;
            while (sat_mx - cam_x < -ctx->map_w/2.0f) sat_mx += ctx->map_w;
            
            screenPos = GetWorldToScreen2D((Vector2){sat_mx, sat_my}, *ctx->camera2d);
        } else {
            screenPos = GetWorldToScreen(Vector3Scale(ctx->active_sat->current_pos, 1.0f/DRAW_SCALE), *ctx->camera3d);
        }
        
        double r_km = Vector3Length(ctx->active_sat->current_pos);
        double v_kms = sqrt(MU * (2.0/r_km - 1.0/ctx->active_sat->semi_major_axis));
        float lat_deg = asinf(ctx->active_sat->current_pos.y / r_km) * RAD2DEG;
        float lon_deg = (atan2f(-ctx->active_sat->current_pos.z, ctx->active_sat->current_pos.x) - ((ctx->gmst_deg + cfg->earth_rotation_offset)*DEG2RAD)) * RAD2DEG;
        
        while (lon_deg > 180.0f) lon_deg -= 360.0f;
        while (lon_deg < -180.0f) lon_deg += 360.0f;

        Vector3 sun_pos = calculate_sun_position(*ctx->current_epoch);
        Vector3 sun_dir = Vector3Normalize(sun_pos);
        bool eclipsed = is_sat_eclipsed(ctx->active_sat->current_pos, sun_dir);

        char info[512];
        sprintf(info, "Inc: %.2f deg\nRAAN: %.2f deg\nEcc: %.4f\nAlt: %.1f km\nSpeed: %.3f km/s\nLat: %.2f\nLon: %.2f\nEclipsed: %s",
                ctx->active_sat->inclination * RAD2DEG,
                ctx->active_sat->raan * RAD2DEG,
                ctx->active_sat->eccentricity,
                r_km - EARTH_RADIUS_KM,
                v_kms,
                lat_deg, lon_deg,
                eclipsed ? "Yes" : "No");

        float titleFontSize = 18 * cfg->ui_scale;
        float infoFontSize = 14 * cfg->ui_scale;

        Vector2 titleSize = MeasureTextEx(customFont, ctx->active_sat->name, titleFontSize, 1.0f);
        Vector2 infoSize = MeasureTextEx(customFont, info, infoFontSize, 1.0f);

        float padX = 10 * cfg->ui_scale;
        float padY = 10 * cfg->ui_scale;
        float gapY = 6 * cfg->ui_scale;

        float contentW = fmaxf(titleSize.x, infoSize.x);
        float boxW = fmaxf(180 * cfg->ui_scale, contentW + (padX * 2));
        float boxH = padY + titleSize.y + gapY + infoSize.y + padY;

        float boxX = screenPos.x + (15 * cfg->ui_scale);
        float boxY = screenPos.y + (15 * cfg->ui_scale);
        
        if (boxX + boxW > GetScreenWidth()) boxX = screenPos.x - boxW - (15 * cfg->ui_scale);
        if (boxY + boxH > GetScreenHeight()) boxY = screenPos.y - boxH - (15 * cfg->ui_scale);

        Rectangle bgRec = {boxX, boxY, boxW, boxH};
        DrawRectangleRec(bgRec, cfg->ui_bg);
        DrawRectangleLinesEx(bgRec, 1.0f, cfg->ui_secondary);
        
        Color titleColor = (ctx->active_sat == ctx->hovered_sat) ? cfg->sat_highlighted : cfg->sat_selected;
        float currentY = boxY + padY;

        DrawUIText(customFont, ctx->active_sat->name, boxX + padX, currentY, titleFontSize, titleColor);
        currentY += titleSize.y + gapY;
        DrawUIText(customFont, info, boxX + padX, currentY, infoFontSize, cfg->text_main);
        
        Vector2 periScreen, apoScreen;
        bool show_peri = true;
        bool show_apo = true;

        double t_peri_unix, t_apo_unix;
        get_apsis_times(ctx->active_sat, *ctx->current_epoch, &t_peri_unix, &t_apo_unix);

        double real_rp = Vector3Length(calculate_position(ctx->active_sat, t_peri_unix));
        double real_ra = Vector3Length(calculate_position(ctx->active_sat, t_apo_unix));
        
        if (*ctx->is_2d_view) {
            Vector2 p2, a2;
            get_apsis_2d(ctx->active_sat, *ctx->current_epoch, false, ctx->gmst_deg, cfg->earth_rotation_offset, ctx->map_w, ctx->map_h, &p2);
            get_apsis_2d(ctx->active_sat, *ctx->current_epoch, true, ctx->gmst_deg, cfg->earth_rotation_offset, ctx->map_w, ctx->map_h, &a2);
            float cam_x = ctx->camera2d->target.x;
            
            while (p2.x - cam_x > ctx->map_w/2.0f) p2.x -= ctx->map_w; 
            while (p2.x - cam_x < -ctx->map_w/2.0f) p2.x += ctx->map_w;
            while (a2.x - cam_x > ctx->map_w/2.0f) a2.x -= ctx->map_w; 
            while (a2.x - cam_x < -ctx->map_w/2.0f) a2.x += ctx->map_w;
            
            periScreen = GetWorldToScreen2D(p2, *ctx->camera2d);
            apoScreen = GetWorldToScreen2D(a2, *ctx->camera2d);
        } else {
            Vector3 draw_p = Vector3Scale(calculate_position(ctx->active_sat, t_peri_unix), 1.0f/DRAW_SCALE);
            Vector3 draw_a = Vector3Scale(calculate_position(ctx->active_sat, t_apo_unix), 1.0f/DRAW_SCALE);
            
            if (IsOccludedByEarth(ctx->camera3d->position, draw_p, EARTH_RADIUS_KM / DRAW_SCALE)) show_peri = false;
            if (IsOccludedByEarth(ctx->camera3d->position, draw_a, EARTH_RADIUS_KM / DRAW_SCALE)) show_apo = false;
            
            periScreen = GetWorldToScreen(draw_p, *ctx->camera3d);
            apoScreen = GetWorldToScreen(draw_a, *ctx->camera3d);
        }
        
        float text_size = 14.0f * cfg->ui_scale;
        float x_offset = 20.0f * cfg->ui_scale;
        float y_offset = text_size / 2.2f;

        if (show_peri) DrawUIText(customFont, TextFormat("Peri: %.0f km", real_rp-EARTH_RADIUS_KM), periScreen.x + x_offset, periScreen.y - y_offset, text_size, cfg->periapsis);
        if (show_apo) DrawUIText(customFont, TextFormat("Apo: %.0f km", real_ra-EARTH_RADIUS_KM), apoScreen.x + x_offset, apoScreen.y - y_offset, text_size, cfg->apoapsis);
    }

    int normal_border = ColorToInt(cfg->ui_secondary);
    int accent_border = ColorToInt(cfg->ui_accent);


    /* render main control toolbar */
    if (show_settings) GuiSetStyle(DEFAULT, BORDER_COLOR_NORMAL, accent_border);
    if (GuiButton(btnSet, "#142#")) {
        if (!show_settings && !opened_once_settings) {
            FindSmartWindowPosition(250*cfg->ui_scale, 440*cfg->ui_scale, cfg, &sw_x, &sw_y);
            opened_once_settings = true;
        }
        show_settings = !show_settings;
    }
    GuiSetStyle(DEFAULT, BORDER_COLOR_NORMAL, normal_border);

    if (show_help) GuiSetStyle(DEFAULT, BORDER_COLOR_NORMAL, accent_border);
    if (GuiButton(btnHelp, "#193#")) {
        if (!show_help && !opened_once_help) {
            FindSmartWindowPosition(900*cfg->ui_scale, 140*cfg->ui_scale, cfg, &hw_x, &hw_y);
            opened_once_help = true;
        }
        show_help = !show_help;
    }
    GuiSetStyle(DEFAULT, BORDER_COLOR_NORMAL, normal_border);

    if (*ctx->is_2d_view) GuiSetStyle(DEFAULT, BORDER_COLOR_NORMAL, accent_border);
    if (GuiButton(btn2D3D, *ctx->is_2d_view ? "#161#" : "#160#")) *ctx->is_2d_view = !*ctx->is_2d_view;
    GuiSetStyle(DEFAULT, BORDER_COLOR_NORMAL, normal_border);

    if (show_sat_mgr_dialog) GuiSetStyle(DEFAULT, BORDER_COLOR_NORMAL, accent_border);
    if (GuiButton(btnSatMgr, "#43#")) {
        if (!show_sat_mgr_dialog && !opened_once_sat_mgr) {
            FindSmartWindowPosition(400*cfg->ui_scale, 500*cfg->ui_scale, cfg, &sm_x, &sm_y);
            opened_once_sat_mgr = true;
        }
        show_sat_mgr_dialog = !show_sat_mgr_dialog;
    }
    GuiSetStyle(DEFAULT, BORDER_COLOR_NORMAL, normal_border);

    if (*ctx->hide_unselected) GuiSetStyle(DEFAULT, BORDER_COLOR_NORMAL, accent_border);
    if (GuiButton(btnHideUnselected, *ctx->hide_unselected ? "#44#" : "#45#")) *ctx->hide_unselected = !*ctx->hide_unselected;
    GuiSetStyle(DEFAULT, BORDER_COLOR_NORMAL, normal_border);

    if (show_tle_mgr_dialog) GuiSetStyle(DEFAULT, BORDER_COLOR_NORMAL, accent_border);
    if (GuiButton(btnTLEMgr, "#1#")) {
        if (!show_tle_mgr_dialog && !opened_once_tle_mgr) {
            FindSmartWindowPosition(400*cfg->ui_scale, 500*cfg->ui_scale, cfg, &tm_x, &tm_y);
            opened_once_tle_mgr = true;
        }
        show_tle_mgr_dialog = !show_tle_mgr_dialog;
    }
    GuiSetStyle(DEFAULT, BORDER_COLOR_NORMAL, normal_border);

    if (cfg->highlight_sunlit) GuiSetStyle(DEFAULT, BORDER_COLOR_NORMAL, accent_border);
    if (GuiButton(btnSunlit, "#147#")) cfg->highlight_sunlit = !cfg->highlight_sunlit;
    GuiSetStyle(DEFAULT, BORDER_COLOR_NORMAL, normal_border);

    if (GuiButton(btnRewind, "#118#")) { *ctx->is_auto_warping = false; *ctx->time_multiplier = StepTimeMultiplier(*ctx->time_multiplier, false); }
    
    if (*ctx->time_multiplier == 0.0) GuiSetStyle(DEFAULT, BORDER_COLOR_NORMAL, accent_border);
    if (GuiButton(btnPlayPause, (*ctx->time_multiplier == 0.0) ? "#131#" : "#132#")) {
        *ctx->is_auto_warping = false;
        if (*ctx->time_multiplier != 0.0) {
            *ctx->saved_multiplier = *ctx->time_multiplier;
            *ctx->time_multiplier = 0.0;
        } else {
            *ctx->time_multiplier = *ctx->saved_multiplier != 0.0 ? *ctx->saved_multiplier : 1.0;
        }
    }
    GuiSetStyle(DEFAULT, BORDER_COLOR_NORMAL, normal_border);

    if (GuiButton(btnFastForward, "#119#")) { *ctx->is_auto_warping = false; *ctx->time_multiplier = StepTimeMultiplier(*ctx->time_multiplier, true); }
    
    if (GuiButton(btnNow, "#211#")) { 
        *ctx->is_auto_warping = false;
        *ctx->current_epoch = get_current_real_time_epoch();
        *ctx->time_multiplier = 1.0;
        *ctx->saved_multiplier = 1.0;
    }
    
    if (show_time_dialog) GuiSetStyle(DEFAULT, BORDER_COLOR_NORMAL, accent_border);
    if (GuiButton(btnClock, "#139#")) { 
        if (!show_time_dialog) {
            if (!opened_once_time) {
                FindSmartWindowPosition(252*cfg->ui_scale, 320*cfg->ui_scale, cfg, &td_x, &td_y);
                opened_once_time = true;
            }
            time_t t_unix = (time_t)get_unix_from_epoch(*ctx->current_epoch);
            struct tm *tm_info = gmtime(&t_unix); 
            if (tm_info) {
                sprintf(text_year, "%d", tm_info->tm_year + 1900);
                sprintf(text_month, "%d", tm_info->tm_mon + 1);
                sprintf(text_day, "%d", tm_info->tm_mday);
                sprintf(text_hour, "%d", tm_info->tm_hour);
                sprintf(text_min, "%d", tm_info->tm_min);
                sprintf(text_sec, "%d", tm_info->tm_sec);
            }
            sprintf(text_unix, "%ld", (long)t_unix);
        }
        show_time_dialog = !show_time_dialog;
    }
    GuiSetStyle(DEFAULT, BORDER_COLOR_NORMAL, normal_border);

    if (show_passes_dialog) GuiSetStyle(DEFAULT, BORDER_COLOR_NORMAL, accent_border);
    if (GuiButton(btnPasses, "#208#")) {
        if (!show_passes_dialog) {
            if (!opened_once_passes) {
                FindSmartWindowPosition(357*cfg->ui_scale, 380*cfg->ui_scale, cfg, &pd_x, &pd_y);
                opened_once_passes = true;
            }
            if (multi_pass_mode) CalculatePasses(NULL, *ctx->current_epoch);
            else if (*ctx->selected_sat) CalculatePasses(*ctx->selected_sat, *ctx->current_epoch);
            else { num_passes = 0; last_pass_calc_sat = NULL; }
        }
        show_passes_dialog = !show_passes_dialog;
    }
    GuiSetStyle(DEFAULT, BORDER_COLOR_NORMAL, normal_border);

    /* render TLE manager dialog */
    if (show_tle_mgr_dialog) {
        if (data_tle_epoch == -1) {
            FILE* f = fopen("data.tle", "r");
            if (f) {
                char line[256];
                if (fgets(line, sizeof(line), f)) {
                    if (strncmp(line, "# EPOCH:", 8) == 0) {
                        unsigned int mask = 0;
                        unsigned int cust_mask = 0;
                        unsigned int ret_mask = 0;
                        
                        // parse with fallback
                        if (strstr(line, "RET_MASK:")) {
                            sscanf(line, "# EPOCH:%ld MASK:%u CUST_MASK:%u RET_MASK:%u", &data_tle_epoch, &mask, &cust_mask, &ret_mask);
                        } else if (strstr(line, "CUST_MASK:")) {
                            sscanf(line, "# EPOCH:%ld MASK:%u CUST_MASK:%u", &data_tle_epoch, &mask, &cust_mask);
                        } else {
                            int cust_count = 0;
                            sscanf(line, "# EPOCH:%ld MASK:%u CUST:%d", &data_tle_epoch, &mask, &cust_count);
                        }
                        
                        for (int i = 0; i < 25; i++) {
                            celestrak_selected[i] = (mask & (1 << i)) != 0;
                        }
                        for (int i = 0; i < NUM_RETLECTOR_SOURCES; i++) {
                            retlector_selected[i] = (ret_mask & (1 << i)) != 0;
                        }
                        for (int i = 0; i < cfg->custom_tle_source_count; i++) {
                            cfg->custom_tle_sources[i].selected = (cust_mask & (1 << i)) != 0;
                        }
                    }
                }
                fclose(f);
            }
            if (data_tle_epoch == -1) data_tle_epoch = 0;
        }

        if (drag_tle_mgr) {
            tm_x = GetMousePosition().x - drag_tle_mgr_off.x;
            tm_y = GetMousePosition().y - drag_tle_mgr_off.y;
            SnapWindow(&tm_x, &tm_y, tmMgrWindow.width, tmMgrWindow.height, cfg);
        }

        if (GuiWindowBox(tmMgrWindow, "TLE Manager")) show_tle_mgr_dialog = false;

        char age_str[64] = "TLE Age: Unknown";
        if (data_tle_epoch > 0) {
            long diff = time(NULL) - data_tle_epoch;
            if (diff < 60) sprintf(age_str, "TLE Age: %ld mins", diff / 60);
            else if (diff < 3600) sprintf(age_str, "TLE Age: %ld mins", diff / 60);
            else if (diff < 86400) sprintf(age_str, "TLE Age: %ld hours, %ld mins", diff / 3600, (diff % 3600) / 60);
            else sprintf(age_str, "TLE Age: %ld days, %ld hours", diff / 86400, (diff % 86400) / 3600);
        }
        DrawUIText(customFont, age_str, tm_x + 10*cfg->ui_scale, tm_y + 35*cfg->ui_scale, 16*cfg->ui_scale, cfg->text_main);

        if (GuiButton((Rectangle){tm_x + tmMgrWindow.width - 110*cfg->ui_scale, tm_y + 30*cfg->ui_scale, 100*cfg->ui_scale, 26*cfg->ui_scale}, "Pull Data")) {
            FILE* out = fopen("data.tle", "w");
            if (out) {
                unsigned int mask = 0;
                for(int i=0; i<25; i++) if(celestrak_selected[i]) mask |= (1 << i);
                
                unsigned int ret_mask = 0;
                for(int i=0; i<NUM_RETLECTOR_SOURCES; i++) if(retlector_selected[i]) ret_mask |= (1 << i);
                
                unsigned int cust_mask = 0;
                for(int i=0; i<cfg->custom_tle_source_count; i++) if(cfg->custom_tle_sources[i].selected) cust_mask |= (1 << i);

                fprintf(out, "# EPOCH:%ld MASK:%u CUST_MASK:%u RET_MASK:%u\n", (long)time(NULL), mask, cust_mask, ret_mask);
                fclose(out);
                
                for (int i = 0; i < NUM_RETLECTOR_SOURCES; i++) {
                    if (retlector_selected[i]) {
                        char cmd[512];
                        snprintf(cmd, sizeof(cmd), "curl -sL \"%s\" >> data.tle", RETLECTOR_SOURCES[i].url);
                        system(cmd);
                    }
                }
                for (int i = 0; i < 25; i++) {
                    if (celestrak_selected[i]) {
                        char cmd[512];
                        snprintf(cmd, sizeof(cmd), "curl -sL \"%s\" >> data.tle", SOURCES[i].url);
                        system(cmd);
                    }
                }
                for (int i = 0; i < cfg->custom_tle_source_count; i++) {
                    if (cfg->custom_tle_sources[i].selected) {
                        char cmd[512];
                        snprintf(cmd, sizeof(cmd), "curl -sL \"%s\" >> data.tle", cfg->custom_tle_sources[i].url);
                        system(cmd);
                    }
                }

                *ctx->selected_sat = NULL;
                ctx->hovered_sat = NULL;
                ctx->active_sat = NULL;
                *ctx->active_lock = LOCK_EARTH;
                locked_pass_sat = NULL;
                num_passes = 0;
                last_pass_calc_sat = NULL;
                sat_count = 0;
                load_tle_data("data.tle");
                LoadSatSelection();
                data_tle_epoch = -1; 
            }
        }

        float total_height = 0;
        total_height += 28 * cfg->ui_scale;
        if (retlector_expanded) total_height += NUM_RETLECTOR_SOURCES * 25 * cfg->ui_scale;
        total_height += 28 * cfg->ui_scale;
        if (celestrak_expanded) total_height += 25 * 25 * cfg->ui_scale;
        total_height += 28 * cfg->ui_scale;
        if (other_expanded) total_height += cfg->custom_tle_source_count * 25 * cfg->ui_scale;

        Rectangle contentRec = { 0, 0, tmMgrWindow.width - 20*cfg->ui_scale, total_height };
        Rectangle viewRec = {0};

        int oldFocusD = GuiGetStyle(DEFAULT, BORDER_COLOR_FOCUSED);
        int oldPressD = GuiGetStyle(DEFAULT, BORDER_COLOR_PRESSED);
        int oldFocusL = GuiGetStyle(LISTVIEW, BORDER_COLOR_FOCUSED);
        int oldPressL = GuiGetStyle(LISTVIEW, BORDER_COLOR_PRESSED);
        GuiSetStyle(DEFAULT, BORDER_COLOR_FOCUSED, ColorToInt(cfg->ui_secondary));
        GuiSetStyle(DEFAULT, BORDER_COLOR_PRESSED, ColorToInt(cfg->ui_secondary));
        GuiSetStyle(LISTVIEW, BORDER_COLOR_FOCUSED, ColorToInt(cfg->ui_secondary));
        GuiSetStyle(LISTVIEW, BORDER_COLOR_PRESSED, ColorToInt(cfg->ui_secondary));

        GuiScrollPanel((Rectangle){ tm_x, tm_y + 65*cfg->ui_scale, tmMgrWindow.width, tmMgrWindow.height - 65*cfg->ui_scale }, NULL, contentRec, &tle_mgr_scroll, &viewRec);

        GuiSetStyle(DEFAULT, BORDER_COLOR_FOCUSED, oldFocusD);
        GuiSetStyle(DEFAULT, BORDER_COLOR_PRESSED, oldPressD);
        GuiSetStyle(LISTVIEW, BORDER_COLOR_FOCUSED, oldFocusL);
        GuiSetStyle(LISTVIEW, BORDER_COLOR_PRESSED, oldPressL);

        BeginScissorMode(viewRec.x, viewRec.y, viewRec.width, viewRec.height);
        float current_y = tm_y + 65*cfg->ui_scale + tle_mgr_scroll.y;

        // RETLECTOR
        Rectangle retlectorHead = { tm_x + 5*cfg->ui_scale + tle_mgr_scroll.x, current_y, viewRec.width - 10*cfg->ui_scale, 24*cfg->ui_scale };
        if (CheckCollisionPointRec(GetMousePosition(), retlectorHead) && CheckCollisionPointRec(GetMousePosition(), viewRec)) {
            DrawRectangleRec(retlectorHead, ApplyAlpha(cfg->ui_secondary, 0.4f));
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) retlector_expanded = !retlector_expanded;
        } else {
            DrawRectangleRec(retlectorHead, ApplyAlpha(cfg->ui_secondary, 0.15f));
        }
        DrawUIText(customFont, TextFormat("%s  RETLECTOR (Unrestricted)", retlector_expanded ? "v" : ">"), retlectorHead.x + 8*cfg->ui_scale, retlectorHead.y + 4*cfg->ui_scale, 16*cfg->ui_scale, cfg->ui_accent);
        current_y += 28 * cfg->ui_scale;

        if (retlector_expanded) {
            for (int i = 0; i < NUM_RETLECTOR_SOURCES; i++) {
                if (current_y + 25*cfg->ui_scale >= viewRec.y && current_y <= viewRec.y + viewRec.height) {
                    Rectangle cbRec = { tm_x + 15*cfg->ui_scale + tle_mgr_scroll.x, current_y + 4*cfg->ui_scale, 16*cfg->ui_scale, 16*cfg->ui_scale };
                    GuiCheckBox(cbRec, RETLECTOR_SOURCES[i].name, &retlector_selected[i]);
                }
                current_y += 25 * cfg->ui_scale;
            }
        }

        // CELESTRAK
        Rectangle celestrakHead = { tm_x + 5*cfg->ui_scale + tle_mgr_scroll.x, current_y, viewRec.width - 10*cfg->ui_scale, 24*cfg->ui_scale };
        if (CheckCollisionPointRec(GetMousePosition(), celestrakHead) && CheckCollisionPointRec(GetMousePosition(), viewRec)) {
            DrawRectangleRec(celestrakHead, ApplyAlpha(cfg->ui_secondary, 0.4f));
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) celestrak_expanded = !celestrak_expanded;
        } else {
            DrawRectangleRec(celestrakHead, ApplyAlpha(cfg->ui_secondary, 0.15f));
        }
        DrawUIText(customFont, TextFormat("%s  CELESTRAK (infrequent pulls only)", celestrak_expanded ? "v" : ">"), celestrakHead.x + 8*cfg->ui_scale, celestrakHead.y + 4*cfg->ui_scale, 16*cfg->ui_scale, cfg->ui_accent);
        current_y += 28 * cfg->ui_scale;

        if (celestrak_expanded) {
            for (int i = 0; i < 25; i++) {
                if (current_y + 25*cfg->ui_scale >= viewRec.y && current_y <= viewRec.y + viewRec.height) {
                    Rectangle cbRec = { tm_x + 15*cfg->ui_scale + tle_mgr_scroll.x, current_y + 4*cfg->ui_scale, 16*cfg->ui_scale, 16*cfg->ui_scale };
                    GuiCheckBox(cbRec, SOURCES[i].name, &celestrak_selected[i]);
                }
                current_y += 25 * cfg->ui_scale;
            }
        }

        // CUSTOM LIST
        Rectangle otherHead = { tm_x + 5*cfg->ui_scale + tle_mgr_scroll.x, current_y, viewRec.width - 10*cfg->ui_scale, 24*cfg->ui_scale };
        if (CheckCollisionPointRec(GetMousePosition(), otherHead) && CheckCollisionPointRec(GetMousePosition(), viewRec)) {
            DrawRectangleRec(otherHead, ApplyAlpha(cfg->ui_secondary, 0.4f));
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) other_expanded = !other_expanded;
        } else {
            DrawRectangleRec(otherHead, ApplyAlpha(cfg->ui_secondary, 0.15f));
        }
        DrawUIText(customFont, TextFormat("%s  CUSTOM SOURCES (settings.json)", other_expanded ? "v" : ">"), otherHead.x + 8*cfg->ui_scale, otherHead.y + 4*cfg->ui_scale, 16*cfg->ui_scale, cfg->ui_accent);
        current_y += 28 * cfg->ui_scale;

        if (other_expanded) {
            for (int i = 0; i < cfg->custom_tle_source_count; i++) {
                if (current_y + 25*cfg->ui_scale >= viewRec.y && current_y <= viewRec.y + viewRec.height) {
                    Rectangle cbRec = { tm_x + 15*cfg->ui_scale + tle_mgr_scroll.x, current_y + 4*cfg->ui_scale, 16*cfg->ui_scale, 16*cfg->ui_scale };
                    GuiCheckBox(cbRec, cfg->custom_tle_sources[i].name, &cfg->custom_tle_sources[i].selected);
                }
                current_y += 25 * cfg->ui_scale;
            }
        }
        EndScissorMode();
    }

    /* render satellite manager dialog */
    if (show_sat_mgr_dialog) {
        if (drag_sat_mgr) {
            sm_x = GetMousePosition().x - drag_sat_mgr_off.x;
            sm_y = GetMousePosition().y - drag_sat_mgr_off.y;
            SnapWindow(&sm_x, &sm_y, smWindow.width, smWindow.height, cfg);
        }
        
        if (GuiWindowBox(smWindow, "Satellite Manager")) show_sat_mgr_dialog = false;

        Rectangle searchBox = { sm_x + 10*cfg->ui_scale, sm_y + 35*cfg->ui_scale, smWindow.width - 90*cfg->ui_scale, 24*cfg->ui_scale };
        if (GuiTextBox(searchBox, sat_search_text, 64, edit_sat_search)) edit_sat_search = !edit_sat_search;

        Rectangle btnCheckAll = { sm_x + smWindow.width - 75*cfg->ui_scale, sm_y + 35*cfg->ui_scale, 30*cfg->ui_scale, 24*cfg->ui_scale };
        Rectangle btnUncheckAll = { sm_x + smWindow.width - 40*cfg->ui_scale, sm_y + 35*cfg->ui_scale, 30*cfg->ui_scale, 24*cfg->ui_scale };
        
        bool doCheckAll = GuiButton(btnCheckAll, "#80#");
        bool doUncheckAll = GuiButton(btnUncheckAll, "#79#");

        int filtered_indices[MAX_SATELLITES];
        int filtered_count = 0;
        for (int i = 0; i < sat_count; i++) {
            if (string_contains_ignore_case(satellites[i].name, sat_search_text)) {
                filtered_indices[filtered_count++] = i;
                if (doCheckAll) satellites[i].is_active = true;
                if (doUncheckAll) satellites[i].is_active = false;
            }
        }
        
        if (doCheckAll || doUncheckAll) {
            SaveSatSelection();
            if (show_passes_dialog) {
                if (multi_pass_mode) CalculatePasses(NULL, *ctx->current_epoch);
                else if (*ctx->selected_sat) CalculatePasses(*ctx->selected_sat, *ctx->current_epoch);
            }
        }

        Rectangle contentRec = { 0, 0, smWindow.width - 20*cfg->ui_scale, filtered_count * 25 * cfg->ui_scale };
        Rectangle viewRec = {0};

        int oldFocusD = GuiGetStyle(DEFAULT, BORDER_COLOR_FOCUSED);
        int oldPressD = GuiGetStyle(DEFAULT, BORDER_COLOR_PRESSED);
        int oldFocusL = GuiGetStyle(LISTVIEW, BORDER_COLOR_FOCUSED);
        int oldPressL = GuiGetStyle(LISTVIEW, BORDER_COLOR_PRESSED);
        GuiSetStyle(DEFAULT, BORDER_COLOR_FOCUSED, ColorToInt(cfg->ui_secondary));
        GuiSetStyle(DEFAULT, BORDER_COLOR_PRESSED, ColorToInt(cfg->ui_secondary));
        GuiSetStyle(LISTVIEW, BORDER_COLOR_FOCUSED, ColorToInt(cfg->ui_secondary));
        GuiSetStyle(LISTVIEW, BORDER_COLOR_PRESSED, ColorToInt(cfg->ui_secondary));

        if (filtered_count > 0) {
            Rectangle contentRec = { 0, 0, smWindow.width - 20*cfg->ui_scale, filtered_count * 25 * cfg->ui_scale };
            Rectangle viewRec = {0};

            int oldFocusD = GuiGetStyle(DEFAULT, BORDER_COLOR_FOCUSED);
            int oldPressD = GuiGetStyle(DEFAULT, BORDER_COLOR_PRESSED);
            int oldFocusL = GuiGetStyle(LISTVIEW, BORDER_COLOR_FOCUSED);
            int oldPressL = GuiGetStyle(LISTVIEW, BORDER_COLOR_PRESSED);
            GuiSetStyle(DEFAULT, BORDER_COLOR_FOCUSED, ColorToInt(cfg->ui_secondary));
            GuiSetStyle(DEFAULT, BORDER_COLOR_PRESSED, ColorToInt(cfg->ui_secondary));
            GuiSetStyle(LISTVIEW, BORDER_COLOR_FOCUSED, ColorToInt(cfg->ui_secondary));
            GuiSetStyle(LISTVIEW, BORDER_COLOR_PRESSED, ColorToInt(cfg->ui_secondary));

            GuiScrollPanel((Rectangle){ sm_x, sm_y + 70*cfg->ui_scale, smWindow.width, smWindow.height - 70*cfg->ui_scale }, NULL, contentRec, &sat_mgr_scroll, &viewRec);

            GuiSetStyle(DEFAULT, BORDER_COLOR_FOCUSED, oldFocusD);
            GuiSetStyle(DEFAULT, BORDER_COLOR_PRESSED, oldPressD);
            GuiSetStyle(LISTVIEW, BORDER_COLOR_FOCUSED, oldFocusL);
            GuiSetStyle(LISTVIEW, BORDER_COLOR_PRESSED, oldPressL);

            BeginScissorMode(viewRec.x, viewRec.y, viewRec.width, viewRec.height);
            for (int k = 0; k < filtered_count; k++) {
                int sat_idx = filtered_indices[k];
                float item_y = sm_y + 70*cfg->ui_scale + sat_mgr_scroll.y + k * 25 * cfg->ui_scale;
                if (item_y + 25*cfg->ui_scale < viewRec.y || item_y > viewRec.y + viewRec.height) continue;

                Rectangle cbRec = { sm_x + 10*cfg->ui_scale + sat_mgr_scroll.x, item_y + 4*cfg->ui_scale, 16*cfg->ui_scale, 16*cfg->ui_scale };
                Rectangle textRec = { sm_x + 35*cfg->ui_scale + sat_mgr_scroll.x, item_y, smWindow.width - 60*cfg->ui_scale, 25*cfg->ui_scale };

                bool was_active = satellites[sat_idx].is_active;
                GuiCheckBox(cbRec, "", &satellites[sat_idx].is_active);

                if (was_active != satellites[sat_idx].is_active) {
                    SaveSatSelection();
                    if (show_passes_dialog) {
                        if (multi_pass_mode) CalculatePasses(NULL, *ctx->current_epoch);
                        else if (*ctx->selected_sat) CalculatePasses(*ctx->selected_sat, *ctx->current_epoch);
                    }
                }

                bool isTargeted = (*ctx->selected_sat == &satellites[sat_idx]);
                bool isHovered = CheckCollisionPointRec(GetMousePosition(), textRec) && CheckCollisionPointRec(GetMousePosition(), viewRec);

                if (isTargeted) {
                    DrawRectangleLinesEx(textRec, 1.5f * cfg->ui_scale, cfg->ui_accent);
                } else if (isHovered) {
                    DrawRectangleLinesEx(textRec, 1.0f * cfg->ui_scale, ApplyAlpha(cfg->ui_secondary, 0.5f));
                }

                if (isHovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    if (isTargeted) *ctx->selected_sat = NULL;
                    else *ctx->selected_sat = &satellites[sat_idx];
                }

                DrawUIText(customFont, satellites[sat_idx].name, textRec.x + 5*cfg->ui_scale, textRec.y + 4*cfg->ui_scale, 16*cfg->ui_scale, isTargeted ? cfg->ui_accent : cfg->text_main);
            }
            EndScissorMode();
        } else {
            const char* empty_msg = (sat_count == 0) ? "No orbital data loaded." : "No satellites match search.";
            Vector2 msg_size = MeasureTextEx(customFont, empty_msg, 16*cfg->ui_scale, 1.0f);
            DrawUIText(customFont, empty_msg, sm_x + (smWindow.width - msg_size.x)/2.0f, sm_y + 180*cfg->ui_scale, 16*cfg->ui_scale, cfg->text_secondary);
            
            Rectangle btnImport = { sm_x + (smWindow.width - 200*cfg->ui_scale)/2.0f, sm_y + 220*cfg->ui_scale, 200*cfg->ui_scale, 30*cfg->ui_scale };
            if (GuiButton(btnImport, "Import orbital data")) {
                if (!show_tle_mgr_dialog) {
                    if (!opened_once_tle_mgr) {
                        FindSmartWindowPosition(400*cfg->ui_scale, 500*cfg->ui_scale, cfg, &tm_x, &tm_y);
                        opened_once_tle_mgr = true;
                    }
                    show_tle_mgr_dialog = true;
                }
            }
        }
    }
    /* render help dialog */
    if (show_help) {
        if (drag_help) {
            hw_x = GetMousePosition().x - drag_help_off.x;
            hw_y = GetMousePosition().y - drag_help_off.y;
            SnapWindow(&hw_x, &hw_y, helpWindow.width, helpWindow.height, cfg);
        }
        if (GuiWindowBox(helpWindow, "Help & Controls")) show_help = false;
        DrawUIText(customFont, *ctx->is_2d_view ? "Controls: RMB to pan, Scroll to zoom. 'M' switches to 3D. Space: Pause." : "Controls: RMB to orbit, Shift+RMB to pan. 'M' switches to 2D. Space: Pause.", hw_x + 10*cfg->ui_scale, hw_y + 35*cfg->ui_scale, 16*cfg->ui_scale, cfg->text_secondary);
        DrawUIText(customFont, "Time: '.' (Faster 2x), ',' (Slower 0.5x), '/' (1x Speed), 'Shift+/' (Reset)", hw_x + 10*cfg->ui_scale, hw_y + 65*cfg->ui_scale, 16*cfg->ui_scale, cfg->text_secondary);
        DrawUIText(customFont, TextFormat("UI Scale: '-' / '+' (%.1fx)", cfg->ui_scale), hw_x + 10*cfg->ui_scale, hw_y + 95*cfg->ui_scale, 16*cfg->ui_scale, cfg->text_secondary);
    }

    /* render settings dialog */
    if (show_settings) {
        if (IsKeyPressed(KEY_TAB)) {
            if (edit_hl_name) { edit_hl_name = false; edit_hl_lat = true; }
            else if (edit_hl_lat) { edit_hl_lat = false; edit_hl_lon = true; }
            else if (edit_hl_lon) { edit_hl_lon = false; edit_hl_alt = true; }
            else if (edit_hl_alt) { edit_hl_alt = false; edit_hl_name = true; }
        }

        if (drag_settings) {
            sw_x = GetMousePosition().x - drag_settings_off.x;
            sw_y = GetMousePosition().y - drag_settings_off.y;
            SnapWindow(&sw_x, &sw_y, settingsWindow.width, settingsWindow.height, cfg);
        }
        if (GuiWindowBox(settingsWindow, "Settings")) show_settings = false;
        
        float sy = sw_y + 40 * cfg->ui_scale;
        GuiCheckBox((Rectangle){ sw_x + 10 * cfg->ui_scale, sy, 20 * cfg->ui_scale, 20 * cfg->ui_scale }, "Show Statistics", &cfg->show_statistics); sy += 30 * cfg->ui_scale;
        GuiCheckBox((Rectangle){ sw_x + 10 * cfg->ui_scale, sy, 20 * cfg->ui_scale, 20 * cfg->ui_scale }, "Show Clouds", &cfg->show_clouds); sy += 30 * cfg->ui_scale;
        GuiCheckBox((Rectangle){ sw_x + 10 * cfg->ui_scale, sy, 20 * cfg->ui_scale, 20 * cfg->ui_scale }, "Night Lights", &cfg->show_night_lights); sy += 30 * cfg->ui_scale;
        GuiCheckBox((Rectangle){ sw_x + 10 * cfg->ui_scale, sy, 20 * cfg->ui_scale, 20 * cfg->ui_scale }, "Show Markers", &cfg->show_markers); sy += 35 * cfg->ui_scale;

        DrawLine(sw_x + 10 * cfg->ui_scale, sy, sw_x + settingsWindow.width - 10 * cfg->ui_scale, sy, cfg->ui_secondary);
        sy += 15 * cfg->ui_scale;

        GuiLabel((Rectangle){ sw_x + 10 * cfg->ui_scale, sy, 200 * cfg->ui_scale, 24 * cfg->ui_scale }, "Home Location:");
        sy += 25 * cfg->ui_scale;

        GuiLabel((Rectangle){ sw_x + 10 * cfg->ui_scale, sy, 40 * cfg->ui_scale, 24 * cfg->ui_scale }, "Name:");
        if (GuiTextBox((Rectangle){ sw_x + 60 * cfg->ui_scale, sy, 170 * cfg->ui_scale, 24 * cfg->ui_scale }, text_hl_name, 64, edit_hl_name)) edit_hl_name = !edit_hl_name;
        sy += 30 * cfg->ui_scale;

        GuiLabel((Rectangle){ sw_x + 10 * cfg->ui_scale, sy, 40 * cfg->ui_scale, 24 * cfg->ui_scale }, "Lat:");
        if (GuiTextBox((Rectangle){ sw_x + 60 * cfg->ui_scale, sy, 170 * cfg->ui_scale, 24 * cfg->ui_scale }, text_hl_lat, 32, edit_hl_lat)) edit_hl_lat = !edit_hl_lat;
        sy += 30 * cfg->ui_scale;

        GuiLabel((Rectangle){ sw_x + 10 * cfg->ui_scale, sy, 40 * cfg->ui_scale, 24 * cfg->ui_scale }, "Lon:");
        if (GuiTextBox((Rectangle){ sw_x + 60 * cfg->ui_scale, sy, 170 * cfg->ui_scale, 24 * cfg->ui_scale }, text_hl_lon, 32, edit_hl_lon)) edit_hl_lon = !edit_hl_lon;
        sy += 30 * cfg->ui_scale;

        GuiLabel((Rectangle){ sw_x + 10 * cfg->ui_scale, sy, 40 * cfg->ui_scale, 24 * cfg->ui_scale }, "Alt:");
        if (GuiTextBox((Rectangle){ sw_x + 60 * cfg->ui_scale, sy, 170 * cfg->ui_scale, 24 * cfg->ui_scale }, text_hl_alt, 32, edit_hl_alt)) edit_hl_alt = !edit_hl_alt;
        sy += 35 * cfg->ui_scale;

        if (GuiButton((Rectangle){ sw_x + 10 * cfg->ui_scale, sy, 220 * cfg->ui_scale, 28 * cfg->ui_scale }, *ctx->picking_home ? "Cancel Picking" : "Pick on Map")) {
            *ctx->picking_home = !*ctx->picking_home;
        }
        sy += 35 * cfg->ui_scale;

        if (GuiButton((Rectangle){ sw_x + 10 * cfg->ui_scale, sy, 220 * cfg->ui_scale, 28 * cfg->ui_scale }, "Save Settings")) {
            strncpy(home_location.name, text_hl_name, 63);
            home_location.lat = atof(text_hl_lat);
            home_location.lon = atof(text_hl_lon);
            home_location.alt = atof(text_hl_alt);
            SaveAppConfig("settings.json", cfg);
            if (show_passes_dialog) {
                if (multi_pass_mode) CalculatePasses(NULL, *ctx->current_epoch);
                else if (*ctx->selected_sat) CalculatePasses(*ctx->selected_sat, *ctx->current_epoch);
            }
        }
    }

    /* render time adjustment dialog */
    if (show_time_dialog) {
        if (IsKeyPressed(KEY_TAB)) {
            if (edit_year) { edit_year = false; edit_month = true; }
            else if (edit_month) { edit_month = false; edit_day = true; }
            else if (edit_day) { edit_day = false; edit_hour = true; }
            else if (edit_hour) { edit_hour = false; edit_min = true; }
            else if (edit_min) { edit_min = false; edit_sec = true; }
            else if (edit_sec) { edit_sec = false; edit_unix = true; }
            else if (edit_unix) { edit_unix = false; edit_year = true; }
            else { edit_year = true; } 
        }
        if (drag_time_dialog) {
            td_x = GetMousePosition().x - drag_time_off.x;
            td_y = GetMousePosition().y - drag_time_off.y;
            SnapWindow(&td_x, &td_y, timeWindow.width, timeWindow.height, cfg);
        }

        if (GuiWindowBox(timeWindow, "Set Date & Time (UTC)")) show_time_dialog = false;

        float cur_y = td_y + 35 * cfg->ui_scale;
        GuiLabel((Rectangle){ td_x + 15*cfg->ui_scale, cur_y, 100*cfg->ui_scale, 24*cfg->ui_scale }, "Date (Y-M-D):");
        cur_y += 25 * cfg->ui_scale;
        if (GuiTextBox((Rectangle){ td_x + 15*cfg->ui_scale, cur_y, 60*cfg->ui_scale, 28*cfg->ui_scale }, text_year, 8, edit_year)) edit_year = !edit_year;
        if (GuiTextBox((Rectangle){ td_x + 80*cfg->ui_scale, cur_y, 40*cfg->ui_scale, 28*cfg->ui_scale }, text_month, 4, edit_month)) edit_month = !edit_month;
        if (GuiTextBox((Rectangle){ td_x + 125*cfg->ui_scale, cur_y, 40*cfg->ui_scale, 28*cfg->ui_scale }, text_day, 4, edit_day)) edit_day = !edit_day;

        cur_y += 35 * cfg->ui_scale;
        GuiLabel((Rectangle){ td_x + 15*cfg->ui_scale, cur_y, 100*cfg->ui_scale, 24*cfg->ui_scale }, "Time (H:M:S):");
        cur_y += 25 * cfg->ui_scale;
        if (GuiTextBox((Rectangle){ td_x + 15*cfg->ui_scale, cur_y, 40*cfg->ui_scale, 28*cfg->ui_scale }, text_hour, 4, edit_hour)) edit_hour = !edit_hour;
        if (GuiTextBox((Rectangle){ td_x + 60*cfg->ui_scale, cur_y, 40*cfg->ui_scale, 28*cfg->ui_scale }, text_min, 4, edit_min)) edit_min = !edit_min;
        if (GuiTextBox((Rectangle){ td_x + 105*cfg->ui_scale, cur_y, 40*cfg->ui_scale, 28*cfg->ui_scale }, text_sec, 4, edit_sec)) edit_sec = !edit_sec;

        cur_y += 35 * cfg->ui_scale;
        if (GuiButton((Rectangle){ td_x + 15*cfg->ui_scale, cur_y, 222*cfg->ui_scale, 30*cfg->ui_scale }, "Apply Date/Time")) {
            *ctx->is_auto_warping = false;
            struct tm t = {0};
            t.tm_year = atoi(text_year) - 1900;
            t.tm_mon = atoi(text_month) - 1;
            t.tm_mday = atoi(text_day);
            t.tm_hour = atoi(text_hour);
            t.tm_min = atoi(text_min);
            t.tm_sec = atoi(text_sec);

            #ifdef _WIN32
                time_t unix_time = _mkgmtime(&t);
            #else
                time_t unix_time = timegm(&t);
            #endif

            if (unix_time != -1) {
                *ctx->current_epoch = unix_to_epoch((double)unix_time);
            }
            show_time_dialog = false;
        }

        cur_y += 40 * cfg->ui_scale;
        DrawLine(td_x + 15*cfg->ui_scale, cur_y, td_x + 237*cfg->ui_scale, cur_y, cfg->ui_secondary);

        cur_y += 10 * cfg->ui_scale;
        GuiLabel((Rectangle){ td_x + 15*cfg->ui_scale, cur_y, 150*cfg->ui_scale, 24*cfg->ui_scale }, "Unix Epoch:");
        cur_y += 25 * cfg->ui_scale;
        if (GuiTextBox((Rectangle){ td_x + 15*cfg->ui_scale, cur_y, 222*cfg->ui_scale, 28*cfg->ui_scale }, text_unix, 64, edit_unix)) edit_unix = !edit_unix;
        
        cur_y += 35 * cfg->ui_scale;
        if (GuiButton((Rectangle){ td_x + 15*cfg->ui_scale, cur_y, 222*cfg->ui_scale, 30*cfg->ui_scale }, "Apply Epoch")) {
            *ctx->is_auto_warping = false;
            double ep;
            if (sscanf(text_unix, "%lf", &ep) == 1) *ctx->current_epoch = unix_to_epoch(ep);
            show_time_dialog = false;
        }
    }

    /* render pass predictor dialog */
    if (show_passes_dialog) {
        if (drag_passes) {
            pd_x = GetMousePosition().x - drag_passes_off.x;
            pd_y = GetMousePosition().y - drag_passes_off.y;
            SnapWindow(&pd_x, &pd_y, passesWindow.width, passesWindow.height, cfg);
        }

        if (GuiWindowBox(passesWindow, "Upcoming Passes")) show_passes_dialog = false;

        Rectangle toggleRec = { passesWindow.x + 10*cfg->ui_scale, passesWindow.y + 30*cfg->ui_scale, passesWindow.width - 150*cfg->ui_scale, 24*cfg->ui_scale };
        if (GuiButton(toggleRec, multi_pass_mode ? "Mode: All Passes" : "Mode: Targeted only")) {
            multi_pass_mode = !multi_pass_mode;
            if (multi_pass_mode) CalculatePasses(NULL, *ctx->current_epoch);
            else if (*ctx->selected_sat) CalculatePasses(*ctx->selected_sat, *ctx->current_epoch);
            else { num_passes = 0; last_pass_calc_sat = NULL; }
        }

        GuiLabel((Rectangle){ passesWindow.x + passesWindow.width - 132*cfg->ui_scale, passesWindow.y + 30*cfg->ui_scale, 60*cfg->ui_scale, 24*cfg->ui_scale }, "Min Elv:");
        if (GuiTextBox((Rectangle){ passesWindow.x + passesWindow.width - 55*cfg->ui_scale, passesWindow.y + 30*cfg->ui_scale, 45*cfg->ui_scale, 24*cfg->ui_scale }, text_min_el, 8, edit_min_el)) edit_min_el = !edit_min_el;

        float min_el_threshold = atof(text_min_el);
        int valid_passes[MAX_PASSES];
        int valid_count = 0;
        for (int i = 0; i < num_passes; i++) {
            if (passes[i].max_el >= min_el_threshold) {
                valid_passes[valid_count++] = i;
            }
        }

        Rectangle contentRec = { 0, 0, passesWindow.width - 20*cfg->ui_scale, (valid_count == 0 ? 1 : valid_count) * 55 * cfg->ui_scale };
        Rectangle viewRec = {0};
        
        int oldFocusD = GuiGetStyle(DEFAULT, BORDER_COLOR_FOCUSED);
        int oldPressD = GuiGetStyle(DEFAULT, BORDER_COLOR_PRESSED);
        int oldFocusL = GuiGetStyle(LISTVIEW, BORDER_COLOR_FOCUSED);
        int oldPressL = GuiGetStyle(LISTVIEW, BORDER_COLOR_PRESSED);
        GuiSetStyle(DEFAULT, BORDER_COLOR_FOCUSED, ColorToInt(cfg->ui_secondary));
        GuiSetStyle(DEFAULT, BORDER_COLOR_PRESSED, ColorToInt(cfg->ui_secondary));
        GuiSetStyle(LISTVIEW, BORDER_COLOR_FOCUSED, ColorToInt(cfg->ui_secondary));
        GuiSetStyle(LISTVIEW, BORDER_COLOR_PRESSED, ColorToInt(cfg->ui_secondary));

        GuiScrollPanel((Rectangle){ passesWindow.x, passesWindow.y + 60*cfg->ui_scale, passesWindow.width, passesWindow.height - 60*cfg->ui_scale }, NULL, contentRec, &passes_scroll, &viewRec);

        GuiSetStyle(DEFAULT, BORDER_COLOR_FOCUSED, oldFocusD);
        GuiSetStyle(DEFAULT, BORDER_COLOR_PRESSED, oldPressD);
        GuiSetStyle(LISTVIEW, BORDER_COLOR_FOCUSED, oldFocusL);
        GuiSetStyle(LISTVIEW, BORDER_COLOR_PRESSED, oldPressL);

        BeginScissorMode(viewRec.x, viewRec.y, viewRec.width, viewRec.height);
        
        if (!multi_pass_mode && !*ctx->selected_sat) {
            DrawUIText(customFont, "No satellite targeted.", passesWindow.x + 20*cfg->ui_scale + passes_scroll.x, passesWindow.y + 70*cfg->ui_scale + passes_scroll.y, 16*cfg->ui_scale, cfg->text_main);
        } else if (valid_count == 0) {
            DrawUIText(customFont, "No passes meet your criteria.", passesWindow.x + 20*cfg->ui_scale + passes_scroll.x, passesWindow.y + 70*cfg->ui_scale + passes_scroll.y, 16*cfg->ui_scale, cfg->text_main);
        } else {
            for (int k = 0; k < valid_count; k++) {
                int i = valid_passes[k];
                float item_y = passesWindow.y + 65*cfg->ui_scale + passes_scroll.y + k * 55 * cfg->ui_scale;
                
                if (item_y + 55*cfg->ui_scale < viewRec.y || item_y > viewRec.y + viewRec.height) continue;

                Rectangle row = { passesWindow.x + 10*cfg->ui_scale + passes_scroll.x, item_y, contentRec.width - 10*cfg->ui_scale, 50*cfg->ui_scale };
                Rectangle rowBtn = row;
                
                bool isHovered = CheckCollisionPointRec(GetMousePosition(), rowBtn) && CheckCollisionPointRec(GetMousePosition(), viewRec);
                bool isSelected = (show_polar_dialog && passes[i].sat == locked_pass_sat && fabs(passes[i].aos_epoch - locked_pass_aos) < (1.0/86400.0));

                if (isHovered || isSelected) {
                    DrawRectangleLinesEx(rowBtn, 1.0f, cfg->ui_accent);
                    if (isHovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                        if (!show_polar_dialog) {
                            if (!opened_once_polar) {
                                FindSmartWindowPosition(300*cfg->ui_scale, 430*cfg->ui_scale, cfg, &pl_x, &pl_y);
                                opened_once_polar = true;
                            }
                            show_polar_dialog = true;
                        }
                        locked_pass_sat = passes[i].sat;
                        locked_pass_aos = passes[i].aos_epoch;
                        locked_pass_los = passes[i].los_epoch;
                        *ctx->selected_sat = passes[i].sat; 
                    }
                }

                char aos_str[16], los_str[16];
                epoch_to_time_str(passes[i].aos_epoch, aos_str);
                epoch_to_time_str(passes[i].los_epoch, los_str);
                
                GuiSetStyle(LABEL, TEXT_COLOR_NORMAL, ColorToInt(cfg->ui_accent));
                GuiLabel((Rectangle){ row.x + 10*cfg->ui_scale, row.y + 2*cfg->ui_scale, row.width - 20*cfg->ui_scale, 20*cfg->ui_scale }, passes[i].sat->name);

                char info_str[128];
                sprintf(info_str, "%s -> %s   Max: %.1fdeg", aos_str, los_str, passes[i].max_el);
                
                GuiSetStyle(LABEL, TEXT_COLOR_NORMAL, ColorToInt(cfg->text_main));
                GuiLabel((Rectangle){ row.x + 10*cfg->ui_scale, row.y + 20*cfg->ui_scale, row.width - 20*cfg->ui_scale, 20*cfg->ui_scale }, info_str);

                if (*ctx->current_epoch >= passes[i].aos_epoch && *ctx->current_epoch <= passes[i].los_epoch) {
                    float prog = (*ctx->current_epoch - passes[i].aos_epoch) / (passes[i].los_epoch - passes[i].aos_epoch);
                    Rectangle pb_bg = { rowBtn.x + 5*cfg->ui_scale, rowBtn.y + 42*cfg->ui_scale, rowBtn.width - 10*cfg->ui_scale, 4*cfg->ui_scale };
                    Rectangle pb_fg = { pb_bg.x, pb_bg.y, pb_bg.width * prog, pb_bg.height };
                    DrawRectangleRec(pb_bg, cfg->ui_secondary);
                    DrawRectangleRec(pb_fg, cfg->ui_accent);
                }
            }
        }
        EndScissorMode();
    }

    /* render polar tracking plot */
    if (show_polar_dialog) {
        if (drag_polar) {
            pl_x = GetMousePosition().x - drag_polar_off.x;
            pl_y = GetMousePosition().y - drag_polar_off.y;
            SnapWindow(&pl_x, &pl_y, polarWindow.width, polarWindow.height, cfg);
        }

        if (GuiWindowBox(polarWindow, "Polar Tracking Plot")) show_polar_dialog = false;

        if (selected_pass_idx >= 0 && selected_pass_idx < num_passes) {
            SatPass* p = &passes[selected_pass_idx];
            Satellite* p_sat = p->sat;

            float cx = pl_x + polarWindow.width/2;
            float cy = pl_y + 160 * cfg->ui_scale;
            float r_max = 100 * cfg->ui_scale;

            DrawCircleLines(cx, cy, r_max, cfg->ui_secondary); 
            DrawCircleLines(cx, cy, r_max * 0.666f, cfg->ui_secondary); 
            DrawCircleLines(cx, cy, r_max * 0.333f, cfg->ui_secondary); 
            DrawLine(cx - r_max, cy, cx + r_max, cy, cfg->ui_secondary);
            DrawLine(cx, cy - r_max, cx, cy + r_max, cfg->ui_secondary);
            
            DrawUIText(customFont, "N", cx - 5*cfg->ui_scale, cy - r_max - 20*cfg->ui_scale, 16*cfg->ui_scale, cfg->text_secondary);
            DrawUIText(customFont, "E", cx + r_max + 5*cfg->ui_scale, cy - 8*cfg->ui_scale, 16*cfg->ui_scale, cfg->text_secondary);
            DrawUIText(customFont, "S", cx - 5*cfg->ui_scale, cy + r_max + 5*cfg->ui_scale, 16*cfg->ui_scale, cfg->text_secondary);
            DrawUIText(customFont, "W", cx - r_max - 20*cfg->ui_scale, cy - 8*cfg->ui_scale, 16*cfg->ui_scale, cfg->text_secondary);

            for (int k = 0; k < p->num_pts - 1; k++) {
                float az1 = p->path_pts[k].x, el1 = p->path_pts[k].y;
                float az2 = p->path_pts[k+1].x, el2 = p->path_pts[k+1].y;
                float r1 = r_max * (90 - el1)/90.0f;
                float r2 = r_max * (90 - el2)/90.0f;
                Vector2 pt1 = { cx + r1 * sin(az1*DEG2RAD), cy - r1 * cos(az1*DEG2RAD) };
                Vector2 pt2 = { cx + r2 * sin(az2*DEG2RAD), cy - r2 * cos(az2*DEG2RAD) };
                
                Color lineCol = cfg->ui_accent;
                if (cfg->highlight_sunlit) {
                    double step = (p->los_epoch - p->aos_epoch) / 99.0;
                    double pt_epoch = p->aos_epoch + k * step;
                    Vector3 pos3d = calculate_position(p_sat, get_unix_from_epoch(pt_epoch));
                    Vector3 sun_dir = Vector3Normalize(calculate_sun_position(pt_epoch));
                    if (!is_sat_eclipsed(pos3d, sun_dir)) lineCol = cfg->sat_highlighted;
                    else lineCol = cfg->orbit_normal; 
                }

                DrawLineEx(pt1, pt2, 2.0f, lineCol);
            }

            if (*ctx->current_epoch >= p->aos_epoch && *ctx->current_epoch <= p->los_epoch) {
                double t_unix = get_unix_from_epoch(*ctx->current_epoch);
                double gmst = epoch_to_gmst(*ctx->current_epoch);
                double c_az, c_el;
                get_az_el(calculate_position(p_sat, t_unix), gmst, home_location.lat, home_location.lon, home_location.alt, &c_az, &c_el);
                
                float r_c = r_max * (90 - c_el)/90.0f;
                Vector2 pt_c = { cx + r_c * sin(c_az*DEG2RAD), cy - r_c * cos(c_az*DEG2RAD) };
                DrawCircleV(pt_c, 4.0f*cfg->ui_scale, RED);

                char c_info[128];
                sprintf(c_info, "Az: %05.1f deg  El: %04.1f deg", c_az, c_el);
                DrawUIText(customFont, c_info, pl_x + 20*cfg->ui_scale, pl_y + 290*cfg->ui_scale, 18*cfg->ui_scale, cfg->text_main);

                int sec_till_los = (int)((p->los_epoch - *ctx->current_epoch)*86400.0);
                char tl_info[64];
                sprintf(tl_info, "LOS in: %02d:%02d", sec_till_los/60, sec_till_los%60);
                DrawUIText(customFont, tl_info, pl_x + 20*cfg->ui_scale, pl_y + 315*cfg->ui_scale, 16*cfg->ui_scale, cfg->ui_accent);
            } else if (*ctx->current_epoch < p->aos_epoch) {
                int sec_till_aos = (int)((p->aos_epoch - *ctx->current_epoch)*86400.0);
                char tl_info[64];
                sprintf(tl_info, "AOS in: %02d:%02d:%02d", sec_till_aos/3600, (sec_till_aos%3600)/60, sec_till_aos%60);
                DrawUIText(customFont, tl_info, pl_x + 20*cfg->ui_scale, pl_y + 295*cfg->ui_scale, 16*cfg->ui_scale, cfg->text_secondary);
            } else {
                DrawUIText(customFont, "Pass Complete", pl_x + 20*cfg->ui_scale, pl_y + 295*cfg->ui_scale, 16*cfg->ui_scale, cfg->text_secondary);
            }

            if (GuiButton((Rectangle){ pl_x + 20*cfg->ui_scale, pl_y + 345*cfg->ui_scale, 260*cfg->ui_scale, 30*cfg->ui_scale }, "#134# Jump to AOS")) {
                *ctx->auto_warp_target = p->aos_epoch;
                *ctx->auto_warp_initial_diff = (*ctx->auto_warp_target - *ctx->current_epoch) * 86400.0;
                if (*ctx->auto_warp_initial_diff > 0.0) *ctx->is_auto_warping = true;
            }

            if (GuiButton((Rectangle){ pl_x + 20*cfg->ui_scale, pl_y + 385*cfg->ui_scale, 260*cfg->ui_scale, 30*cfg->ui_scale }, "#125# Doppler Shift Analysis")) {
                if (!show_doppler_dialog) {
                    if (!opened_once_doppler) {
                        FindSmartWindowPosition(320*cfg->ui_scale, 480*cfg->ui_scale, cfg, &dop_x, &dop_y);
                        opened_once_doppler = true;
                    }
                    show_doppler_dialog = true;
                }
            }
        } else {
            DrawUIText(customFont, "No valid pass selected.", pl_x + 20*cfg->ui_scale, pl_y + 40*cfg->ui_scale, 16*cfg->ui_scale, cfg->text_main);
        }
    }

    /* render doppler shift analysis dialog */
    if (show_doppler_dialog) {
        if (IsKeyPressed(KEY_TAB)) {
            if (edit_doppler_freq) { edit_doppler_freq = false; edit_doppler_res = true; }
            else if (edit_doppler_res) { edit_doppler_res = false; edit_doppler_file = true; }
            else if (edit_doppler_file) { edit_doppler_file = false; edit_doppler_freq = true; }
            else { edit_doppler_freq = true; } 
        }

        if (drag_doppler) {
            dop_x = GetMousePosition().x - drag_doppler_off.x;
            dop_y = GetMousePosition().y - drag_doppler_off.y;
            SnapWindow(&dop_x, &dop_y, dopplerWindow.width, dopplerWindow.height, cfg);
        }

        if (GuiWindowBox(dopplerWindow, "Doppler Shift Analysis")) show_doppler_dialog = false;

        if (selected_pass_idx >= 0 && selected_pass_idx < num_passes) {
            SatPass* p = &passes[selected_pass_idx];
            Satellite* d_sat = p->sat;

            float dy = dop_y + 35 * cfg->ui_scale;
            GuiLabel((Rectangle){ dop_x + 15*cfg->ui_scale, dy, 200*cfg->ui_scale, 24*cfg->ui_scale }, "Freq (Hz):");
            dy += 25 * cfg->ui_scale;
            if (GuiTextBox((Rectangle){ dop_x + 15*cfg->ui_scale, dy, 290*cfg->ui_scale, 28*cfg->ui_scale }, text_doppler_freq, 32, edit_doppler_freq)) edit_doppler_freq = !edit_doppler_freq;

            dy += 35 * cfg->ui_scale;
            GuiLabel((Rectangle){ dop_x + 15*cfg->ui_scale, dy, 200*cfg->ui_scale, 24*cfg->ui_scale }, "CSV Res (s):");
            dy += 25 * cfg->ui_scale;
            if (GuiTextBox((Rectangle){ dop_x + 15*cfg->ui_scale, dy, 290*cfg->ui_scale, 28*cfg->ui_scale }, text_doppler_res, 32, edit_doppler_res)) edit_doppler_res = !edit_doppler_res;

            dy += 35 * cfg->ui_scale;
            GuiLabel((Rectangle){ dop_x + 15*cfg->ui_scale, dy, 200*cfg->ui_scale, 24*cfg->ui_scale }, "Export:");
            dy += 25 * cfg->ui_scale;
            if (GuiTextBox((Rectangle){ dop_x + 15*cfg->ui_scale, dy, 290*cfg->ui_scale, 28*cfg->ui_scale }, text_doppler_file, 128, edit_doppler_file)) edit_doppler_file = !edit_doppler_file;

            dy += 35 * cfg->ui_scale;
            if (GuiButton((Rectangle){ dop_x + 15*cfg->ui_scale, dy, 290*cfg->ui_scale, 30*cfg->ui_scale }, "Export CSV")) {
                double base_freq = atof(text_doppler_freq);
                double res = atof(text_doppler_res);
                if (res <= 0.0) res = 1.0;
                double pass_dur = (p->los_epoch - p->aos_epoch) * 86400.0;
                
                FILE* fp = fopen(text_doppler_file, "w");
                if (fp) {
                    fprintf(fp, "Time(s),Frequency(Hz)\n");
                    int num_samples = (int)(pass_dur * res);
                    for (int k = 0; k <= num_samples; k++) {
                        double t_sec = k / res;
                        double ep = p->aos_epoch + t_sec / 86400.0;
                        double f = calculate_doppler_freq(d_sat, ep, home_location, base_freq);
                        fprintf(fp, "%.3f,%.3f\n", t_sec, f);
                    }
                    fclose(fp);
                }
            }

            dy += 45 * cfg->ui_scale;
            double base_freq = atof(text_doppler_freq);
            double pass_dur = (p->los_epoch - p->aos_epoch) * 86400.0;
            
            if (pass_dur > 0 && base_freq > 0) {
                float graph_x = dop_x + 75 * cfg->ui_scale;
                float graph_y = dy;
                float graph_w = dopplerWindow.width - 90 * cfg->ui_scale;
                float graph_h = dopplerWindow.height - (dy - dop_y) - 20 * cfg->ui_scale;
                
                DrawRectangleLines(graph_x, graph_y, graph_w, graph_h, cfg->ui_secondary);

                double min_f = base_freq * 2.0; 
                double max_f = 0.0;
                int plot_pts = (int)graph_w; 
                for (int k = 0; k <= plot_pts; k++) {
                    double t = p->aos_epoch + (k / (double)plot_pts) * (pass_dur / 86400.0);
                    double f = calculate_doppler_freq(d_sat, t, home_location, base_freq);
                    if (f < min_f) min_f = f;
                    if (f > max_f) max_f = f;
                }

                double d1 = max_f - base_freq;
                double d2 = min_f - base_freq;
                double max_abs_d = fmax(fabs(d1), fabs(d2));
                if (max_abs_d < 1.0) max_abs_d = 1.0;
                
                double max_d = max_abs_d * 1.1;
                double min_d = -max_abs_d * 1.1;
                
                DrawUIText(customFont, TextFormat("%+.0f Hz", max_d), dop_x + 5*cfg->ui_scale, graph_y, 14*cfg->ui_scale, cfg->text_main);
                DrawUIText(customFont, TextFormat("%+.0f Hz", min_d), dop_x + 5*cfg->ui_scale, graph_y + graph_h - 14*cfg->ui_scale, 14*cfg->ui_scale, cfg->text_main);
                DrawUIText(customFont, "0s", graph_x + 10*cfg->ui_scale, graph_y + graph_h + 5*cfg->ui_scale, 14*cfg->ui_scale, cfg->text_main);
                DrawUIText(customFont, TextFormat("%.0fs", pass_dur), graph_x + graph_w - 55*cfg->ui_scale, graph_y + graph_h + 5*cfg->ui_scale, 14*cfg->ui_scale, cfg->text_main);

                float zero_y = graph_y + graph_h / 2.0f;
                for (float x = graph_x; x < graph_x + graph_w; x += 8.0f) {
                    DrawLine(x, zero_y, x + 4.0f, zero_y, ApplyAlpha(cfg->text_secondary, 0.4f));
                }

                for (int s = 0; s <= pass_dur; s += 60) {
                    float lx = graph_x + (s / pass_dur) * graph_w;
                    for (float y = graph_y; y < graph_y + graph_h; y += 8.0f) {
                        DrawLine(lx, y, lx, y + 4.0f, ApplyAlpha(cfg->ui_secondary, 0.6f));
                    }
                }

                BeginScissorMode((int)graph_x, (int)graph_y, (int)graph_w, (int)graph_h);
                Vector2 prev_pt = {0};
                for (int k = 0; k <= plot_pts; k++) {
                    double t = p->aos_epoch + (k / (double)plot_pts) * (pass_dur / 86400.0);
                    double f = calculate_doppler_freq(d_sat, t, home_location, base_freq);
                    double delta = f - base_freq;
                    
                    float px = graph_x + k;
                    float py = graph_y + graph_h - (float)((delta - min_d) / (max_d - min_d)) * graph_h;
                    
                    if (k > 0) DrawLineEx(prev_pt, (Vector2){px, py}, 2.0f, cfg->ui_accent);
                    prev_pt = (Vector2){px, py};
                }

                if (*ctx->current_epoch >= p->aos_epoch && *ctx->current_epoch <= p->los_epoch) {
                    double t_offset = (*ctx->current_epoch - p->aos_epoch) * 86400.0;
                    float cx = graph_x + (t_offset / pass_dur) * graph_w;
                    double cur_f = calculate_doppler_freq(d_sat, *ctx->current_epoch, home_location, base_freq);
                    double c_delta = cur_f - base_freq;
                    float cy = graph_y + graph_h - (float)((c_delta - min_d) / (max_d - min_d)) * graph_h;
                    DrawCircleV((Vector2){cx, cy}, 4.0f*cfg->ui_scale, RED);
                }
                EndScissorMode();

                Rectangle chartRec = { graph_x, graph_y, graph_w, graph_h };
                if (CheckCollisionPointRec(GetMousePosition(), chartRec)) {
                    float mouseX = GetMousePosition().x;
                    float mouseY = GetMousePosition().y;
                    
                    double t_sec = ((mouseX - graph_x) / graph_w) * pass_dur;
                    double f_hz = calculate_doppler_freq(d_sat, p->aos_epoch + t_sec / 86400.0, home_location, base_freq);
                    double speed_of_light = 299792.458; 
                    double rel_speed = speed_of_light * (base_freq / f_hz - 1.0);
                    
                    float dot_y = graph_y + graph_h - (float)((f_hz - base_freq - min_d) / (max_d - min_d)) * graph_h;

                    DrawLine(mouseX, graph_y, mouseX, graph_y + graph_h, cfg->ui_accent);
                    DrawCircle(mouseX, dot_y, 4.0f * cfg->ui_scale, cfg->ui_accent);

                    char tooltip[128];
                    sprintf(tooltip, "%.0f Hz\n%.1f s\n%.3f km/s", f_hz, t_sec, rel_speed);
                    
                    Vector2 textSize = MeasureTextEx(customFont, tooltip, 14*cfg->ui_scale, 1.0f);
                    float tt_x = mouseX + 10*cfg->ui_scale;
                    float tt_y = mouseY;
                    
                    if (tt_x + textSize.x + 10*cfg->ui_scale > dop_x + dopplerWindow.width) {
                        tt_x = mouseX - textSize.x - 10*cfg->ui_scale;
                    }
                    
                    DrawRectangle(tt_x, tt_y, textSize.x + 10*cfg->ui_scale, textSize.y + 10*cfg->ui_scale, ApplyAlpha(cfg->ui_bg, 0.9f));
                    DrawRectangleLines(tt_x, tt_y, textSize.x + 10*cfg->ui_scale, textSize.y + 10*cfg->ui_scale, cfg->ui_secondary);
                    DrawUIText(customFont, tooltip, tt_x + 5*cfg->ui_scale, tt_y + 5*cfg->ui_scale, 14*cfg->ui_scale, cfg->text_main);
                }
            }
        } else {
            DrawUIText(customFont, "No valid pass selected.", dop_x + 20*cfg->ui_scale, dop_y + 60*cfg->ui_scale, 16*cfg->ui_scale, cfg->text_main);
        }
    }

    /* render stale tle warning dialog */
    if (show_tle_warning) {
        if (GuiWindowBox(tleWindow, "Warning")) show_tle_warning = false;
        DrawUIText(customFont, "Your TLEs are out of date.", tleWindow.x + 20*cfg->ui_scale, tleWindow.y + 45*cfg->ui_scale, 18*cfg->ui_scale, cfg->text_main);

        if (GuiButton((Rectangle){tleWindow.x + 20*cfg->ui_scale, tleWindow.y + 80*cfg->ui_scale, 120*cfg->ui_scale, 30*cfg->ui_scale}, "Sync")) {
            /* integration pending */
        }
        if (GuiButton((Rectangle){tleWindow.x + 160*cfg->ui_scale, tleWindow.y + 80*cfg->ui_scale, 120*cfg->ui_scale, 30*cfg->ui_scale}, "Ignore")) {
            show_tle_warning = false;
        }
    }

    /* render persistent bottom bar, stats, and tooltips */
    char top_text_render[128];
    sprintf(top_text_render, "%s", ctx->datetime_str);
    Vector2 dt_size = MeasureTextEx(customFont, top_text_render, 20*cfg->ui_scale, 1.0f);
    DrawUIText(customFont, top_text_render, btn_start_x - dt_size.x - 20*cfg->ui_scale, GetScreenHeight() - 35*cfg->ui_scale, 20*cfg->ui_scale, cfg->text_main);

    char speed_str[64];
    sprintf(speed_str, "Speed: %gx %s", *ctx->time_multiplier, (*ctx->time_multiplier == 0.0) ? "[PAUSED]" : "");
    DrawUIText(customFont, speed_str, btn_start_x + buttons_w + 20*cfg->ui_scale, GetScreenHeight() - 35*cfg->ui_scale, 20*cfg->ui_scale, cfg->text_main);

    Rectangle btnRecs[13] = { btnSet, btnHelp, btn2D3D, btnSatMgr, btnHideUnselected, btnPasses, btnTLEMgr, btnSunlit, btnRewind, btnPlayPause, btnFastForward, btnNow, btnClock };
    const char* tt_texts[13] = { "Settings", "Help & Controls", "Toggle 2D/3D View", "Satellite Manager", "Toggle Unselected Orbits", "Pass Predictor", "TLE Manager", "Highlight Sunlit Orbits", "Slower / Reverse", "Play / Pause", "Faster", "Real Time", "Set Date & Time" };
    
/* Evaluate window occlusion directly to avoid self-blocking buttons */
    bool over_dialog = false;
    if (show_help && CheckCollisionPointRec(GetMousePosition(), helpWindow)) over_dialog = true;
    if (show_settings && CheckCollisionPointRec(GetMousePosition(), settingsWindow)) over_dialog = true;
    if (show_time_dialog && CheckCollisionPointRec(GetMousePosition(), timeWindow)) over_dialog = true;
    if (show_passes_dialog && CheckCollisionPointRec(GetMousePosition(), passesWindow)) over_dialog = true;
    if (show_polar_dialog && CheckCollisionPointRec(GetMousePosition(), polarWindow)) over_dialog = true;
    if (show_doppler_dialog && CheckCollisionPointRec(GetMousePosition(), dopplerWindow)) over_dialog = true;
    if (show_sat_mgr_dialog && CheckCollisionPointRec(GetMousePosition(), smWindow)) over_dialog = true;
    if (show_tle_mgr_dialog && CheckCollisionPointRec(GetMousePosition(), tmMgrWindow)) over_dialog = true;
    if (show_tle_warning && CheckCollisionPointRec(GetMousePosition(), tleWindow)) over_dialog = true;

    /* tooltips~~ */
    for (int i = 0; i < 13; i++) {
        if (!over_dialog && CheckCollisionPointRec(GetMousePosition(), btnRecs[i])) {
            tt_hover[i] += GetFrameTime();
            if (tt_hover[i] > 0.3f) {
                Vector2 m = GetMousePosition();
                float tw = MeasureTextEx(customFont, tt_texts[i], 14*cfg->ui_scale, 1.0f).x + 12*cfg->ui_scale;
                
                float tt_x = m.x + 10*cfg->ui_scale;
                float tt_y = m.y + 15*cfg->ui_scale;
                
                /* Keep tooltip inside screen boundaries */
                if (tt_x + tw > GetScreenWidth()) tt_x = GetScreenWidth() - tw - 5*cfg->ui_scale;
                if (tt_y + 24*cfg->ui_scale > GetScreenHeight()) tt_y = m.y - 25*cfg->ui_scale;

                DrawRectangle(tt_x, tt_y, tw, 24*cfg->ui_scale, ApplyAlpha(cfg->ui_bg, 0.6f));
                DrawRectangleLines(tt_x, tt_y, tw, 24*cfg->ui_scale, cfg->ui_primary);
                DrawUIText(customFont, tt_texts[i], tt_x + 6*cfg->ui_scale, tt_y + 4*cfg->ui_scale, 14*cfg->ui_scale, cfg->text_main);
            }
        } else {
            tt_hover[i] = 0.0f;
        }
    }

    if (cfg->show_statistics) {
        DrawUIText(customFont, TextFormat("%3i FPS", GetFPS()), GetScreenWidth() - (90*cfg->ui_scale), 10*cfg->ui_scale, 20*cfg->ui_scale, cfg->ui_accent);
        DrawUIText(customFont, TextFormat("%i Sats", sat_count), GetScreenWidth() - (90*cfg->ui_scale), 34*cfg->ui_scale, 16*cfg->ui_scale, cfg->text_secondary);
    }

    if (*ctx->time_multiplier == 1.0 && fabs(*ctx->current_epoch - get_current_real_time_epoch()) < (5.0 / 86400.0) && !*ctx->is_auto_warping) {
        float blink_alpha = (sinf(GetTime() * 4.0f) * 0.5f + 0.5f);
        int oldStyle = GuiGetStyle(LABEL, TEXT_COLOR_NORMAL);
        GuiSetStyle(LABEL, TEXT_COLOR_NORMAL, ColorToInt(ApplyAlpha(cfg->ui_accent, blink_alpha)));
        
        float rt_w = 20*cfg->ui_scale + 5*cfg->ui_scale + MeasureTextEx(customFont, "REAL TIME", 16*cfg->ui_scale, 1.0f).x;
        float rt_x = (GetScreenWidth() - rt_w) / 2.0f;
        float rt_y = GetScreenHeight() - 65*cfg->ui_scale; 

        GuiLabel((Rectangle){ rt_x, rt_y - 2*cfg->ui_scale, 20*cfg->ui_scale, 20*cfg->ui_scale }, "#212#");
        GuiSetStyle(LABEL, TEXT_COLOR_NORMAL, oldStyle);

        DrawUIText(customFont, "REAL TIME", rt_x + 25*cfg->ui_scale, rt_y, 16*cfg->ui_scale, cfg->ui_accent);
    }
    /* exit dialog!! byessssssssssss*/
    if (show_exit_dialog) {
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), (Color){0, 0, 0, 150}); // Darken background
        
        float ew = 300 * cfg->ui_scale;
        float eh = 140 * cfg->ui_scale;
        Rectangle exitRec = { (GetScreenWidth() - ew) / 2.0f, (GetScreenHeight() - eh) / 2.0f, ew, eh };
        
        if (GuiWindowBox(exitRec, "Exit Application")) show_exit_dialog = false;
        
        DrawUIText(customFont, "Are you sure you want to exit?", exitRec.x + 25*cfg->ui_scale, exitRec.y + 45*cfg->ui_scale, 16*cfg->ui_scale, cfg->text_main);
        
        if (GuiButton((Rectangle){exitRec.x + 20*cfg->ui_scale, exitRec.y + 85*cfg->ui_scale, 120*cfg->ui_scale, 30*cfg->ui_scale}, "Yes")) {
            *ctx->exit_app = true;
        }
        if (GuiButton((Rectangle){exitRec.x + 160*cfg->ui_scale, exitRec.y + 85*cfg->ui_scale, 120*cfg->ui_scale, 30*cfg->ui_scale}, "No")) {
            show_exit_dialog = false;
        }
    }
}