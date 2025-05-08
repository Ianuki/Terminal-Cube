#include <iostream>
#include <vector>
#include <string>
#include <sys/ioctl.h>
#include <unistd.h>
#include <math.h>
#include <bits/stdc++.h>
#include <ncurses.h>

class vector3 {
    public:
        float x, y, z;

        vector3(float _x, float _y, float _z) :
        x(_x), y(_y), z(_z) {}
};

class surface_3d {
    public:
        vector3 position;
        vector3 relative_position;
        vector3 rotation;
        vector3 relative_rotation;
        std::string texture;
        int color_id = 1;

        surface_3d(
            vector3 _position,
            vector3 _relative_position,
            vector3 _rotation,
            vector3 _relative_rotation,
            std::string _texture,
            int _color_id
        ) :
        position(_position),
        relative_position(_relative_position),
        rotation(_rotation),
        relative_rotation(_relative_rotation),
        texture(_texture),
        color_id(_color_id) {}
};

enum cube_material {
    DEFAULT,
    GRASS,
    DIRT,
    STONE,
    WATER
};

float focal_length = 120;
int resolution = 45; 
int cube_size = 1000;
std::vector<surface_3d> surfaces;
vector3 camera_position(
    0,
    0,
    -5000
);

vector3 rotate_x(vector3 v, float rotation) {
    return vector3(
        v.x,
        v.y * cos(rotation) + v.z * -sin(rotation),
        v.y * sin(rotation) + v.z * cos(rotation)
    );
}

vector3 rotate_y(vector3 v, float rotation) {
    return vector3(
        v.x * cos(rotation) + sin(rotation) * v.z,
        v.y,
        v.x * -sin(rotation) + cos(rotation) * v.z
    );
}

vector3 rotate_z(vector3 v, float rotation) {
    return vector3(
        v.x * cos(rotation) + sin(rotation) * v.z,
        v.y,
        v.x * -sin(rotation) + cos(rotation) * v.z
    );
}

vector3 calculate_surface_relative_position(surface_3d surface) {
    vector3 relative_position = surface.relative_position;
    relative_position = rotate_x(relative_position, surface.relative_rotation.x);
    relative_position = rotate_y(relative_position, surface.relative_rotation.y);
    relative_position = rotate_x(relative_position, surface.rotation.x);
    relative_position = rotate_y(relative_position, surface.rotation.y);

    return relative_position;
}

void draw_surface(surface_3d surface, int screen_width, int screen_height) {
    for (float i = 0; i < resolution + 1; i++) {
        for (float j = 0; j < resolution + 1; j++) {
            vector3 vertex(
                (cube_size / resolution) * i - cube_size / 2,
                (cube_size / resolution) * j - cube_size / 2, 
                cube_size / 2
            );
            vertex = rotate_x(vertex, surface.relative_rotation.x);
            vertex = rotate_y(vertex, surface.relative_rotation.y);
            vertex = rotate_x(vertex, surface.rotation.x);
            vertex = rotate_y(vertex, surface.rotation.y);
            
            attron(COLOR_PAIR(surface.color_id));
            mvaddstr(
                vertex.y * focal_length / (vertex.z - camera_position.z) + screen_height / 2,
                (vertex.x * focal_length / (vertex.z - camera_position.z)) * 2 + screen_width / 2,
                surface.texture.c_str()
            );
        }
    }
}

bool surface_sort_comp(surface_3d a, surface_3d b) {
    vector3 a_relative_position = calculate_surface_relative_position(a);
    vector3 b_relative_position = calculate_surface_relative_position(b);

    return a_relative_position.z < b_relative_position.z;
}

void draw_all_surfaces(int width, int height) {
    std::sort(surfaces.begin(), surfaces.end(), surface_sort_comp);

    for (surface_3d surface : surfaces) {
        draw_surface(surface, width, height);
    }
}

int main(int argc, char ** argv) {
    struct winsize w; ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	int height = w.ws_row;
	int width = w.ws_col;

    initscr();			
  	noecho();
    curs_set(0);

    if (!has_colors()) { std::cout << "Your terminal does not support colors!\n"; return 1; }
    start_color();
    init_pair(cube_material::GRASS, COLOR_GREEN, COLOR_BLACK);
    init_pair(cube_material::DIRT, COLOR_RED, COLOR_BLACK);
    init_pair(cube_material::STONE, COLOR_WHITE, COLOR_BLACK);
    init_pair(cube_material::WATER, COLOR_BLUE, COLOR_BLACK);
    init_pair(5, COLOR_YELLOW, COLOR_BLACK);
    init_pair(6, COLOR_MAGENTA, COLOR_BLACK);

    int elapsed_time = 0;
    while (true) {
        winsize w; ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
        height = w.ws_row;
	    width = w.ws_col;

        surfaces.clear();
        erase();

        for (int i = 0; i < 4; i++) {
            std::string texture = "";
            if (i == 0) 
                texture = "@@";
            else if (i == 1)
                texture = "##";
            else if (i == 2)
                texture = "&&";
            else if (i == 3) {
                texture = "++";
            }

            surfaces.push_back(surface_3d(
                vector3(0, 0, 0),
                vector3(0, 0, -cube_size / 2),
                vector3(elapsed_time * M_PI / 45, elapsed_time * M_PI / 180, 0),
                vector3(0, i * 90 * M_PI / 180, 0),
                texture,
                i+1
            ));
        }

        surfaces.push_back(surface_3d(
            vector3(0, 0, 0),
            vector3(0, 0, -cube_size / 2),
            vector3(elapsed_time * M_PI / 45, elapsed_time * M_PI / 180, 0),
            vector3(90 * M_PI / 180, 0, 0),
            "TT",
            5
        ));

        surfaces.push_back(surface_3d(
            vector3(0, 0, 0),
            vector3(0, 0, -cube_size / 2),
            vector3(elapsed_time * M_PI / 45, elapsed_time * M_PI / 180, 0),
            vector3(-90 * M_PI / 180, 0, 0),
            "%%",
            6
        ));

        draw_all_surfaces(width, height);
        usleep(10000);
        refresh();
        elapsed_time++;
    }

    endwin();
}
