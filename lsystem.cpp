#include <algorithm>
#include <cmath>
#include <iostream>
#include <tuple>
#include <vector>
#include <stack>
#include <map>

#define SDL_MAIN_HANDLED
#include <SDL.h>
// #include <glad/glad.h>

using namespace std;

#define WIDTH 1920
#define HEIGHT 1080
// #define WIDTH 3440
// #define HEIGHT 1440

struct Lsystem {
    // grammar alphabet subset that does not have production rules
    vector<string> constants;
    // starting string
    string axiom;
    // replacement lookup map
    map<string, string> rules;
    // angle magnitude for "+" and "-"
    double angle;
};

string run_step(map<string, string> rules, string step, vector<string> constants) {
    // run single grammar generation step
    string out;
    for (size_t i = 0; i < step.size(); i++) {
        string current = string(1, step[i]);
        if (find(constants.begin(), constants.end(), current) != constants.end())
            out.append(current);
        else
            out.append(rules[current]);
    }
    return out;
}

string generate_lsystem(string axiom, map<string, string> rules, vector<string> constants, size_t num_iterations) {
    // helper for running the desired number of iterations of an L-system given the starting axiom
    string next_step = axiom;
    for (size_t i = 0; i < num_iterations; i++) {
        // cout << i << " " << next_step << endl;
        next_step = run_step(rules, next_step, constants);
    }
    return next_step;
}

vector<int> generate_lines(string instructions, double angle_delta, double forward_distance, double angle) {
    // run thrugh the insturction string one character at a time and run the character as an insturction
    // returns a flat array of lines serialized in order x1, y1, x2, y2
    vector<int> out_buffer;
    double x = 0;
    double y = 0;

    stack<tuple<double, double, double>> saved_position;
    for (size_t i = 0; i < instructions.size(); i++) {
        switch(instructions[i]) {
            case 'X':
                break;
            // move forward
            case 'F': {
                // extend (x, y) with (0, forward_distance) and rotate to by the angle
                double new_x = x - forward_distance*sin(angle);
                double new_y = y + forward_distance*cos(angle);
                out_buffer.push_back((int)round(x));
                out_buffer.push_back((int)round(y));
                out_buffer.push_back((int)round(new_x));
                out_buffer.push_back((int)round(new_y));
                y = new_y;
                x = new_x;
                break;
            }
            // update angle
            case '-':
                angle += angle_delta;
                break;
            case '+':
                angle -= angle_delta;
                break;
            // push/pop position and angle stack
            case '[':
                saved_position.push(tuple<double, double, double>(x, y, angle));
                break;
            case ']':
                if (!saved_position.empty()) {
                    double pop_x, pop_y, pop_angle;
                    tie(pop_x, pop_y, pop_angle) = saved_position.top();
                    x = pop_x;
                    y = pop_y;
                    angle = pop_angle;
                    saved_position.pop();
                }
                break;
            default: break;
        }
    }
    return out_buffer;
}

void draw(SDL_Renderer *renderer, vector<int> line_buffer, int offset_x, int offset_y) {
    // render all lines in buffer throught SDL
    for (size_t i = 0; i < line_buffer.size()/4; i++) {
        // update colour
        double percentage = (double)i / (double)line_buffer.size();
        double lerp_size = 0xCC;
        int col = (int)round(percentage*lerp_size + 0xFF - lerp_size);
        SDL_SetRenderDrawColor(renderer, 0, col, col, SDL_ALPHA_OPAQUE);

        int x1 = line_buffer[i*4 + 0] + offset_x + WIDTH  / 2;
        int y1 = line_buffer[i*4 + 1] + offset_y + HEIGHT / 2;
        int x2 = line_buffer[i*4 + 2] + offset_x + WIDTH  / 2;
        int y2 = line_buffer[i*4 + 3] + offset_y + HEIGHT / 2;

        // draw line
        if ((x1 > 0 && x1 < WIDTH && y1 > 0 && y1 < HEIGHT) ||
            (x2 > 0 && x2 < WIDTH && y2 > 0 && y2 < HEIGHT)) {
            SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
        }
    }
}


int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) == 0) {
        SDL_Window* window = NULL;
        SDL_Renderer* renderer = NULL;
        const Uint8* keyboard = NULL;

        vector<Lsystem> fractals = {
            { // pretty tree
                {"+", "-", "[", "]"},
                "X",
                { {"X", "F+[[X]-X]-F[-FX]+X"}, {"F", "FF"}},
                M_PI / 4
            },
            { // conifer
                {"+", "-", "[", "]", "F"},
                "Y",
                {{"X", "X[-FFF][+FFF]FX"}, {"Y", "YFX[+Y][-Y]"}},
                25.7 *M_PI/180
            },
            { // prong bush
                {"+", "-", "[", "]"},
                "F",
                {{"F", "FF+[+F-F-F]-[-F+F+F]"} },
                22.5*M_PI/180
            },
            { // hilbert
                {"+", "-", "[", "]", "F"},
                "X",
                {{"X", "-YF+XFX+FY-"}, {"Y", "+XF-YFY-FX+"}},
                M_PI/2
            },
            { // tile
                {"+", "-", "[", "]"},
                "F+F+F+F",
                {{"F", "FF+F-F+F+FF"}},
                M_PI/2
            }
        };

        string lsystem_instruction = "";
        vector<int> lsystem_lines;

        // runtime parameters
        size_t num_iterations = 2;
        double forward_distance = 10;
        double offset_angle = M_PI;
        size_t fractal_index = 0;

        if (SDL_CreateWindowAndRenderer(WIDTH, HEIGHT, 0, &window, &renderer) == 0) {
            bool is_done = false;
            bool should_draw = true;
            bool should_generate = true;

            int screen_offset_x = 0;
            int screen_offset_y = 0;

            while (!is_done) {
                if (should_generate) {
                    // regenerate the instruction string and cachend lines buffer
                    Lsystem cur = fractals[fractal_index];
                    lsystem_instruction = generate_lsystem(cur.axiom, cur.rules, cur.constants, num_iterations);
                    lsystem_lines = generate_lines(lsystem_instruction, cur.angle, forward_distance, offset_angle);
                    // cout << lsystem_lines.size() << endl;
                    should_generate = false;
                }
                if (should_draw) {
                    // re-draw the fractal
                    SDL_SetRenderDrawColor(renderer, 0, 0x10, 0, SDL_ALPHA_OPAQUE);

                    SDL_RenderClear(renderer);
                    SDL_SetRenderDrawColor(renderer, 0, 0xF0, 0, SDL_ALPHA_OPAQUE);
                    draw(renderer, lsystem_lines, screen_offset_x, screen_offset_y);
                    SDL_RenderPresent(renderer);
                    should_draw = false;
                }
                SDL_Event event;
                while (SDL_PollEvent(&event)) {
                    switch (event.type) {
                        case SDL_KEYDOWN:
                            SDL_PumpEvents();
                            keyboard = SDL_GetKeyboardState(NULL);

                            switch (event.key.keysym.sym) {
                                case SDLK_ESCAPE: is_done = true; break;
                                // movement
                                case SDLK_a: screen_offset_x += 100; should_draw = true; break;
                                case SDLK_d: screen_offset_x -= 100; should_draw = true; break;
                                case SDLK_w: screen_offset_y += 100; should_draw = true; break;
                                case SDLK_s: screen_offset_y -= 100; should_draw = true; break;
                                // rotation
                                case SDLK_e: offset_angle += M_PI/4; should_generate = true; should_draw = true; break;
                                case SDLK_q: offset_angle -= M_PI/4; should_generate = true; should_draw = true; break;
                                // reset
                                case SDLK_r: screen_offset_x = 0; screen_offset_y = 0; should_draw = true; break;
                                // cycle through fractals
                                case SDLK_f: fractal_index = (fractal_index + 1) % fractals.size(); should_generate = true; should_draw = true; break;
                                // set number of iterations
                                case SDLK_1: num_iterations = 1; should_generate = true; should_draw = true; break;
                                case SDLK_2: num_iterations = 2; should_generate = true; should_draw = true; break;
                                case SDLK_3: num_iterations = 3; should_generate = true; should_draw = true; break;
                                case SDLK_4: num_iterations = 4; should_generate = true; should_draw = true; break;
                                case SDLK_5: num_iterations = 5; should_generate = true; should_draw = true; break;
                                case SDLK_6: num_iterations = 6; should_generate = true; should_draw = true; break;
                                case SDLK_7: num_iterations = 7; should_generate = true; should_draw = true; break;
                                case SDLK_8: num_iterations = 8; should_generate = true; should_draw = true; break;
                                case SDLK_9: num_iterations = 9; should_generate = true; should_draw = true; break;
                                default: break;
                            }
                            break;
                        case SDL_QUIT: is_done = true; break;
                        default: break;
                    }
                }
            }
        }

        if (renderer) {
            SDL_DestroyRenderer(renderer);
        }
        if (window) {
            SDL_DestroyWindow(window);
        }
    }
    SDL_Quit();
    return 0;
}
