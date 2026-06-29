#include "../globals.hpp"

static constexpr int LIST_X = 0;
static constexpr int LIST_W = 120;

static constexpr int ROW0_Y = 16;
static constexpr int ROW1_Y = 25;
static constexpr int ROW2_Y = 34;
static constexpr int ROW_H = 8;

static std::mutex gui_mtx;
static std::condition_variable gui_cv;
static bool gui_dirty = true;
void lcd_mark_dirty() {
    {
        std::lock_guard<std::mutex> lk(gui_mtx);
        gui_dirty = true;
    }
    gui_cv.notify_one();
}

static std::string fit(const std::string& s, size_t max) {
    if (s.size() <= max) return s;
    return s.substr(0, max);
}

static void render_help(g15canvas* c) {
	g15r_drawLine(canvas, G15_LCD_WIDTH-37, 14, G15_LCD_WIDTH, 14, G15_COLOR_BLACK);
	g15r_drawLine(canvas, G15_LCD_WIDTH-37, 14, G15_LCD_WIDTH-37, G15_LCD_HEIGHT, G15_COLOR_BLACK);
	g15r_renderString (canvas, (unsigned char *)"1:Default\0", 3, G15_TEXT_SMALL, G15_LCD_WIDTH-35, 0);
	g15r_renderString (canvas, (unsigned char *)"2:     Up\0", 4, G15_TEXT_SMALL, G15_LCD_WIDTH-35, 0);
	g15r_renderString (canvas, (unsigned char *)"3:   Down\0", 5, G15_TEXT_SMALL, G15_LCD_WIDTH-35, 0);
	g15r_renderString (canvas, (unsigned char *)"4:     OK\0", 6, G15_TEXT_SMALL, G15_LCD_WIDTH-35, 0);
}

static void render_list(g15canvas* c) {
    std::lock_guard<std::mutex> lk(gui_mtx);

    int total  = (int)profile_list.size();
    if (total == 0) return;

    int sel    = gui_select_idx;

    int idx0   = (sel - 1 + total) % total;
    int idx1   = sel;
    int idx2   = (sel + 1) % total;

    auto name0 = fit(profile_list[idx0], 18);
    auto name1 = fit(profile_list[idx1], 18);
    auto name2 = fit(profile_list[idx2], 18);

    g15r_renderString(c, (unsigned char*)name0.c_str(), 0, G15_TEXT_SMALL, LIST_X + 2, ROW0_Y);
    g15r_renderString(c, (unsigned char*)name1.c_str(), 0, G15_TEXT_SMALL, LIST_X + 2, ROW1_Y);
    g15r_renderString(c, (unsigned char*)name2.c_str(), 0, G15_TEXT_SMALL, LIST_X + 2, ROW2_Y);

    g15r_pixelReverseFill(c, LIST_X, ROW1_Y - 1, LIST_W - 1, ROW1_Y + ROW_H - 1,
                          0, G15_COLOR_BLACK);
}

static void render_header(g15canvas* c) {
    std::string hdr = "Current: " + fit(config_name, 16);
    g15r_renderString(c, (unsigned char*)hdr.c_str(), 0, G15_TEXT_SMALL, 2, 4);

    g15r_drawLine(c, 0, 14, 125, 14, G15_COLOR_BLACK);
}

static void render_full(g15canvas* c) {
    g15r_clearScreen(c, G15_COLOR_WHITE);
    if(macro_state != 0){
        switch (macro_state) {
            case 0:
                break;
            case 1:
                g15r_renderString(c, (unsigned char *)"Enter the macro", 0, G15_TEXT_MED, 2, 4);
                g15r_renderString(c, (unsigned char *)"Press G to confirm or MR to cancel", 0, G15_TEXT_MED, 2, 35);
                break;
            default:
                break;
        }
    }
    else {
        render_header(c);
        render_help(c);
        render_list(c);
    }
    g15_send(g15screen_fd, (char*)c->buffer, G15_BUFFER_LEN);
}

void gui_select_up() {
    {
        std::lock_guard<std::mutex> lk(gui_mtx);
        int total = (int)profile_list.size();
        if (total == 0) return;
        gui_select_idx = (gui_select_idx - 1 + total) % total;
        gui_dirty = true;
    }
    gui_cv.notify_one();
}

void gui_select_down() {
    {
        std::lock_guard<std::mutex> lk(gui_mtx);
        int total = (int)profile_list.size();
        if (total == 0) return;
        gui_select_idx = (gui_select_idx + 1) % total;
        gui_dirty = true;
    }
    gui_cv.notify_one();
}

void gui_select_default() {
    {
        std::lock_guard<std::mutex> lk(gui_mtx);
        gui_select_idx = 0;
        gui_dirty = true;
    }
    gui_cv.notify_one();
}

void gui_apply_selection() {
    std::string chosen;
    {
        std::lock_guard<std::mutex> lk(gui_mtx);
        if (profile_list.empty()) return;
        chosen = profile_list[gui_select_idx];
    }
    save_config(config_name);
    load_config(chosen);
    config_name = chosen;
    lcd_mark_dirty();
}

void lcd_thread() {

    render_full(canvas);

    while (running) {
        std::unique_lock<std::mutex> lk(gui_mtx);

        gui_cv.wait_for(lk, std::chrono::milliseconds(500),
                        [] { return gui_dirty || !running.load(); });

        if (!running) break;
        if (!gui_dirty) continue;

        gui_dirty = false;
        lk.unlock();

        render_full(canvas);
    }
}