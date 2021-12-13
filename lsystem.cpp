#include <algorithm>
#include <cmath>
#include <iostream>
#include <fstream>
#include <tuple>
#include <vector>
#include <stack>
#include <map>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <glad/glad.h>

using namespace std;

#define WIDTH 1920
#define HEIGHT 1080
// #define WIDTH 3440
// #define HEIGHT 1440

#define ANGLE_DELTA M_PI / 6
#define FORWARD_DELTA 100
#define ZOOM_FACTOR 1.8

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

void start_window_renderer_and_gl(SDL_Window **window, SDL_Renderer **renderer, SDL_GLContext *gl_context, size_t width, size_t height) {
	// create the window, renderer and gl context
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		std::cerr << "SDL2 video subsystem couldn't be initialized. Error: " << SDL_GetError() << std::endl;
		exit(1);
	}

	// window
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
	*window = SDL_CreateWindow(
		"L-Systems Explorer",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		WIDTH, HEIGHT, window_flags
	);

	// renderer
	*renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_ACCELERATED);
	if (*renderer == nullptr) {
		std::cerr << "SDL2 Renderer couldn't be created. Error: "
				<< SDL_GetError()
				<< std::endl;
		exit(1);
	}

	// create GL context
	*gl_context = SDL_GL_CreateContext(*window);
	// Load GL extensions using glad
	if (!gladLoadGLLoader((GLADloadproc) SDL_GL_GetProcAddress)) {
		std::cerr << "Failed to initialize the OpenGL context." << std::endl;
		exit(1);
	}
	// std::cout << "OpenGL version loaded: " << GLVersion.major << "." << GLVersion.minor << std::endl;
}

unsigned int compile_shader(unsigned int shader_type, const std::string& shader_source) {
	unsigned int shader_id = glCreateShader(shader_type);

	const char* c_source = shader_source.c_str();
	glShaderSource(shader_id, 1, &c_source, nullptr);
	glCompileShader(shader_id);

	GLint result;
	glGetShaderiv(shader_id, GL_COMPILE_STATUS, &result);

	if (result == GL_FALSE) {
		int length;
		glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &length);

		GLchar* strInfoLog = new GLchar[length + 1];
		glGetShaderInfoLog(shader_id, length, &length, strInfoLog);

		fprintf(stderr, "Compilation error in shader: %s\n", strInfoLog);
		delete[] strInfoLog;
	}

	return shader_id;
}

unsigned int load_shaders(const std::string& vertexShaderFile, const std::string& fragmentShaderFile) {
	std::ifstream is_vs(vertexShaderFile);
	const std::string f_vs((std::istreambuf_iterator<char>(is_vs)), std::istreambuf_iterator<char>());

	std::ifstream is_fs(fragmentShaderFile);
	const std::string f_fs((std::istreambuf_iterator<char>(is_fs)), std::istreambuf_iterator<char>());

	unsigned int id = glCreateProgram();

	unsigned int vs = compile_shader(GL_VERTEX_SHADER, f_vs);
	unsigned int fs = compile_shader(GL_FRAGMENT_SHADER, f_fs);

	glAttachShader(id, vs);
	glAttachShader(id, fs);

	glLinkProgram(id);
	glValidateProgram(id);

	glDeleteShader(vs);
	glDeleteShader(fs);

	return id;
}

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

vector<float> generate_lines(string instructions, double angle_delta, double forward_distance, double angle) {
    // run thrugh the insturction string one character at a time and run the character as an insturction
    // returns a flat array of lines serialized in order x1, y1, x2, y2
    vector<float> out_buffer;
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
                double new_x = x + forward_distance*sin(angle);
                double new_y = y - forward_distance*cos(angle);

                out_buffer.push_back(+x);
                out_buffer.push_back(-y);
                out_buffer.push_back(+new_x);
                out_buffer.push_back(-new_y);

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

void populate_orthographic_projection_matrix(float screen_width, float screen_height, float transform[16]) {
    float width = screen_width;
    float height = screen_height;
    float left = -width;
    float right = width * 1;
    float top = height * 1;
    float bottom = -height;
    float near = 0;
    float far = 100;

    transform[0] = (2 / (right - left));
    transform[5] = (2 / (top - bottom));
    transform[10] = -2 / (far - near);
    transform[3] = - (right + left) / (right - left);
    transform[7] = - (top + bottom) / (top - bottom);
    transform[11] = -(far + near) / (far - near);
    transform[15] = 1;
}

int main(int argc, char* argv[]) {
	const Uint8 *keyboard = NULL;
	SDL_Window *window = NULL;
	SDL_Renderer *renderer = NULL;
	SDL_GLContext gl_context;

	// start window and gl context
	start_window_renderer_and_gl(&window, &renderer, &gl_context, WIDTH, HEIGHT);
    SDL_GL_MakeCurrent(window, gl_context);

	// set initial gl state
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetSwapInterval(0); // 1 -> vsync : 0 -> NO vsync 

    //line smoothing
    glEnable( GL_LINE_SMOOTH );
    glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// blur and AA
	glEnable(GL_MULTISAMPLE);

	// load global shader
	unsigned int program_id = load_shaders("main.vert", "main.frag");
	glUseProgram(program_id);

	vector<Lsystem> fractals = {
		{ // hexperiment
			{"+", "-", "[", "]"},
			"F",
			{{"F", "F++F++F++F++F++F-F"}},
			M_PI / 6
		},
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
		},
	};

	string lsystem_instruction = "";
	vector<float> lsystem_lines;

	// runtime parameters
	size_t num_iterations = 2;
	double forward_distance = 20;
	double offset_angle = M_PI;
	size_t fractal_index = 0;

	bool is_done = false;
	bool should_draw = true;
	bool should_generate = true;

	int screen_offset_x = 0;
	int screen_offset_y = 0;
	float zoom = 1.0;

	unsigned int vbo, vao;
	while (!is_done) {
		if (should_generate) {
			// regenerate the instruction string and cachend lines buffer
			Lsystem cur = fractals[fractal_index];
			lsystem_instruction = generate_lsystem(cur.axiom, cur.rules, cur.constants, num_iterations);
			lsystem_lines = generate_lines(lsystem_instruction, cur.angle, forward_distance, offset_angle);
			// cout << lsystem_lines.size() << endl;

			glGenVertexArrays(1, &vao);
			glGenBuffers(1, &vbo);
			glBindVertexArray(vao);

			glBindBuffer(GL_ARRAY_BUFFER, vbo);
			glBufferData(GL_ARRAY_BUFFER, sizeof(float)*lsystem_lines.size(), &lsystem_lines[0], GL_STATIC_DRAW);

			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);

			glBindBuffer(GL_ARRAY_BUFFER, 0); 
			glBindVertexArray(0); 

			should_generate = false;
		}
		if (should_draw) {
			// re-draw the fractal
			glClear(GL_COLOR_BUFFER_BIT);

			float transform[16] = {0.0};
			populate_orthographic_projection_matrix((float)WIDTH, (float)HEIGHT, transform);
			glUniformMatrix4fv(glGetUniformLocation(program_id, "transform"), 1, GL_FALSE, transform);
			glUniform2f(glGetUniformLocation(program_id, "offset"), (float)screen_offset_x, (float)screen_offset_y); 
			glUniform1f(glGetUniformLocation(program_id, "angle"), offset_angle); 
			glUniform1f(glGetUniformLocation(program_id, "zoom"), zoom); 

			glBindVertexArray(vao);
			glDrawArrays(GL_LINES, 0, lsystem_lines.size());
			glBindVertexArray(0);

			SDL_GL_SwapWindow(window);
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
						case SDLK_a: screen_offset_x += FORWARD_DELTA; should_draw = true; break;
						case SDLK_d: screen_offset_x -= FORWARD_DELTA; should_draw = true; break;
						case SDLK_w: screen_offset_y -= FORWARD_DELTA; should_draw = true; break;
						case SDLK_s: screen_offset_y += FORWARD_DELTA; should_draw = true; break;
						// zoom
						case SDLK_x: zoom /= ZOOM_FACTOR; should_draw = true; break;
						case SDLK_c: zoom *= ZOOM_FACTOR; should_draw = true; break;
						// rotation
						case SDLK_e: offset_angle += ANGLE_DELTA; should_draw = true; break;
						case SDLK_q: offset_angle -= ANGLE_DELTA; should_draw = true; break;
						// reset
						case SDLK_r: screen_offset_x = 0; screen_offset_y = 0; offset_angle = 0; should_draw = true; break;
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

	// cleanup
	glDeleteProgram(program_id);
	if (renderer) {
		SDL_DestroyRenderer(renderer);
	}
	if (window) {
		SDL_DestroyWindow(window);
	}
	SDL_GL_DeleteContext(gl_context);
    SDL_Quit();
    return 0;
}
