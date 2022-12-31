#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <queue>
#include <iostream>
#include <set>
#include <stack>
#include <string>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "third_party/stb/stb_image.h"

using Color = std::array<uint8_t, 3>;
using Pos = std::array<int, 2>;

struct TubePos {
    int bottom_i;
    int delta_i;
    int mid_j;
};

using Tube = std::array<uint8_t, 4>;

struct State {
    std::vector<Tube> tubes;
    int last_state_id;
    int from;
    int to;
};

int main() {
    std::string file_path = "screenshots/100.png";
    int iw, ih, in;
    uint8_t *idata = stbi_load(file_path.c_str(), &iw, &ih, &in, 0);
    std::cout << iw << " " << ih << " " << in << std::endl;

    auto get_color = [=](Pos pos) {
        uint8_t *p = &idata[(pos[0] * iw + pos[1]) * in];
        return Color{p[0], p[1], p[2]};
    };
    auto set_color = [=](Pos pos, Color color) {
        uint8_t *p = &idata[(pos[0] * iw + pos[1]) * in];
        p[0] = color[0];
        p[1] = color[1];
        p[2] = color[2];
    };
    auto to_hex = [](int x) -> std::string {
        if (x >= 16) {
            std::cout << "not implemented to_hex(x) where x >= 16" << std::endl;
            exit(1);
        }
        if (x < 10)
            return std::to_string(x);
        else
            return std::string(1, 'a' + x - 10);
    };

    std::vector<TubePos> tubes_pos;
    for (int i = 0; i < ih; i++) {
        for (int j = 0; j < iw; j++) {
            static Color const gray = {192, 192, 192};
            if (get_color({i, j}) == gray) {
                int num_gray = 0;
                Pos min_pos{i, j}, max_pos{i, j};
                std::stack<Pos> st;
                st.push({i, j});
                while (!st.empty()) {
                    Pos cur = st.top();
                    st.pop();
                    if (get_color(cur) == gray) {
                        num_gray++;
                        min_pos[0] = std::min(min_pos[0], cur[0]);
                        min_pos[1] = std::min(min_pos[1], cur[1]);
                        max_pos[0] = std::max(max_pos[0], cur[0]);
                        max_pos[1] = std::max(max_pos[1], cur[1]);
                        set_color(cur, {0, 0, 0});
                        static int const dx[4] = {1, -1, 0, 0};
                        static int const dy[4] = {0, 0, 1, -1};
                        for (int dir = 0; dir < 4; dir++) {
                            st.push({cur[0] + dx[dir], cur[1] + dy[dir]});
                        }
                    }
                }
                if (num_gray > 100) {
                    // std::cout << num_gray << std::endl;
                    // std::cout << min_pos[0] << " " << min_pos[1] << " " << max_pos[0] << " " << max_pos[1] << std::endl;
                    TubePos tube_pos;
                    tube_pos.mid_j = (min_pos[1] + max_pos[1]) / 2;
                    tube_pos.delta_i = (int)((max_pos[0] - min_pos[0]) / 4.5);
                    tube_pos.bottom_i = max_pos[0] - tube_pos.delta_i / 2;
                    tubes_pos.push_back(tube_pos);
                }
            }
        }
    }

    std::vector<Color> colors;
    std::vector<Tube> tubes;
    colors.push_back({255, 255, 255});
    for (auto const &tube_pos : tubes_pos) {
        Tube tube;
        for (int i = 0; i < 4; i++) {
            Color color = get_color({tube_pos.bottom_i - i * tube_pos.delta_i, tube_pos.mid_j});
            if (color[0] > 200 && color[1] > 200 && color[2] > 200)
                color = {255, 255, 255};
            // std::cout << (int)color[0] << " " << (int)color[1] << " " << (int)color[2] << " " << std::endl;
            if (std::find(colors.begin(), colors.end(), color) == colors.end())
                colors.push_back(color);
            int color_idx = std::find(colors.begin(), colors.end(), color) - colors.begin();
            tube[i] = color_idx;
        }
        tubes.push_back(tube);
    }
    // std::cout << colors.size() << std::endl;
    if (colors.size() != tubes_pos.size() - 1) {
        std::cout << "failed to get color" << std::endl;
        exit(1);
    }

    int n = tubes.size();
    std::cout << n << std::endl;
    auto show = [=](std::vector<Tube> const &tubes) {
        for (int i = 3; i >= 0; i--) {
            for (int j = 0; j < n; j++) {
                std::cout << to_hex(tubes[j][i]) << " ";
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    };
    show(tubes);

    std::vector<State> states;
    State state;
    state.tubes = tubes;
    state.last_state_id = -1;
    state.from = -1;
    state.to = -1;
    states.push_back(state);
    std::queue<int> q;
    q.push(0);
    std::set<std::vector<Tube> > checked;
    while (!q.empty()) {
        int state_id = q.front();
        q.pop();
        std::vector<Tube> const cur_tubes = states[state_id].tubes;
        std::cout << states.size() << " " << q.size() << std::endl;
        if (std::all_of(cur_tubes.begin(), cur_tubes.end(), [](Tube const &tube) {
                    return tube[0] == tube[1] && tube[0] == tube[2] && tube[0] == tube[3];
                })) {
            std::cout << "Win" << std::endl;
            std::vector<int> win_steps;
            auto friendly_tube_name = [n](int tube_id) {
                if (tube_id < n / 2) {
                    return "1-" + std::to_string(tube_id + 1);
                } else {
                    return "2-" + std::to_string(tube_id - n / 2 + 1);
                }
            };
            while (state_id != -1) {
                win_steps.push_back(state_id);
                show(states[state_id].tubes);
                std::cout << friendly_tube_name(states[state_id].from) << " -> " << friendly_tube_name(states[state_id].to) << std::endl;
                std::cout << std::endl;
                state_id = states[state_id].last_state_id;
            }
            win_steps.pop_back();
            std::reverse(win_steps.begin(), win_steps.end());
            for (int state_id : win_steps)
                std::cout << friendly_tube_name(states[state_id].from) << " -> " << friendly_tube_name(states[state_id].to) << std::endl;
            break;
        }
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                if (i == j)
                    continue;
                int num_i = 4;
                int num_j = 4;
                while (num_i > 0 && cur_tubes[i][num_i - 1] == 0)
                    num_i--;
                while (num_j > 0 && cur_tubes[j][num_j - 1] == 0)
                    num_j--;
                if (num_i == 0)  // empty
                    continue;
                if (num_j == 4)  // full
                    continue;
                if (num_j != 0 && cur_tubes[j][num_j - 1] != cur_tubes[i][num_i - 1])  // color differ
                    continue;
                int color = cur_tubes[i][num_i - 1];
                int n_i = 1;
                while (n_i < num_i && num_j + n_i < 4 && cur_tubes[i][num_i - n_i - 1] == color)
                    n_i++;
                // std::cout << i << " " << j << " " << num_i << " " << num_j << " " << n_i << std::endl;
                if (num_i < 0 || num_i > 4 || num_j < 0 || num_j > 4 || n_i > num_i || num_j + n_i > 4) {
                    std::cout << "error num" << std::endl;
                    exit(1);
                }
                
                std::vector<Tube> new_tubes = cur_tubes;
                for (int k = 0; k < n_i; k++) {
                    new_tubes[i][num_i - 1 - k] = 0;
                    new_tubes[j][num_j + k] = color;
                }
                std::vector<Tube> sorted_new_tubes = new_tubes;
                std::sort(sorted_new_tubes.begin(), sorted_new_tubes.end());
                if (checked.count(sorted_new_tubes))
                    continue;
                checked.insert(sorted_new_tubes);
                // show(new_tubes);
                State state;
                state.tubes = new_tubes;
                state.last_state_id = state_id;
                state.from = i;
                state.to = j;
                states.push_back(state);
                q.push(states.size() - 1);
            }
        }
    }
}
