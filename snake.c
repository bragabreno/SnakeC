#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <SDL2/SDL.h>
#include "glyphs.h"

#define WIN_X 500
#define WIN_Y 0

#define WIN_WIDTH 850
#define WIN_HEIGHT 850

#define CELL_SIZE 25
#define MAX_NUMBER_SEGMENTS 400

#define DELAY_TIME 50 // In milliseconds 

// Structures
typedef enum {
    UP, 
    DOWN, 
    RIGHT, 
    LEFT
} direction;

typedef struct {
    int x, y;
} object_coordinates;


// Rendering
void render_grid(SDL_Renderer* renderer, int cell_size) {
    SDL_SetRenderDrawColor(renderer, 0x8D, 0x8D, 0x8D, 255); 
    for (int i = 0; i <= WIN_WIDTH / cell_size; i++) {
        SDL_RenderDrawLine(renderer, i * cell_size, 2 * cell_size, i * cell_size, WIN_HEIGHT);
    }
    for (int j = 2; j <= WIN_HEIGHT / cell_size; j++) {
        SDL_RenderDrawLine(renderer, 0, j * cell_size, WIN_WIDTH, j * cell_size);
    }
}

void render_digit(SDL_Renderer* renderer, int glyph[5][5], int x, int y, int pixel_size) {
    SDL_Rect rect;
    for (int row = 0; row < 5; row++) {
        for (int col = 0; col < 5; col++) {
            if (glyph[row][col] == 1) {
                rect.x = x + col * pixel_size;
                rect.y = y + row * pixel_size;
                rect.w = pixel_size;
                rect.h = pixel_size;
                SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 255); 
                SDL_RenderFillRect(renderer, &rect);
            }
        }
    }
}

void render_snake(SDL_Renderer* renderer, object_coordinates* snake_segments, int number_segments, int seg_size) {
    for (int j = 0; j < number_segments; j++) {
        SDL_Rect segment = { snake_segments[j].x, snake_segments[j].y, seg_size, seg_size };
        SDL_SetRenderDrawColor(renderer, 0x29, 0xDA, 0x81, 255); 
        SDL_RenderFillRect(renderer, &segment);
    }
}

void render_apple(SDL_Renderer* renderer, int x, int y, int apple_size) {
    SDL_Rect apple = { x * CELL_SIZE, y * CELL_SIZE, apple_size, apple_size };
    SDL_SetRenderDrawColor(renderer, 0xF0, 0x29, 0x29, 255); 
    SDL_RenderFillRect(renderer, &apple);
}


// Checking collisions
bool has_collided_boundary(object_coordinates* head) {
    return (head->x < 0 || head->x >= WIN_WIDTH || head->y < 2 * CELL_SIZE || head->y >= WIN_HEIGHT);
}

bool has_collided_with_itself(object_coordinates* head, int number_segments) {
    for (int i = 1; i < number_segments; i++) {
        if (head->x == (head + i)->x && head->y == (head + i)->y)
            return true;
    }
    return false;
}

bool has_eaten_food(object_coordinates* head, object_coordinates* current_apple) {
    return (head->x == current_apple->x * CELL_SIZE && head->y == current_apple->y * CELL_SIZE);
}


// Snake and apple initialization and movement
void initialize_snake(object_coordinates* snake, int number_segments) {
    snake[0].x = 10 * CELL_SIZE;
    snake[0].y = 10 * CELL_SIZE;

    for (int i = 1; i < number_segments; i++) {
        snake[i].x = snake[0].x;
        snake[i].y = snake[0].y - i * CELL_SIZE; 
    }
}

void move_snake(object_coordinates* snake_segment, int current_number_segments, direction snake_direction) { 
    for (int i = current_number_segments - 1; i > 0; i--) {
        snake_segment[i] = snake_segment[i - 1];
    }

    switch (snake_direction) {
        case UP: 
            snake_segment[0].y -= CELL_SIZE; 
            break;
        case DOWN: 
            snake_segment[0].y += CELL_SIZE; 
            break;
        case LEFT: 
            snake_segment[0].x -= CELL_SIZE; 
            break;
        case RIGHT: 
            snake_segment[0].x += CELL_SIZE; 
            break;
    }
}

void generate_apple(object_coordinates* apple) {
    apple->x = (rand() % (WIN_WIDTH / CELL_SIZE - 2)) + 1;
    apple->y = (rand() % (WIN_HEIGHT / CELL_SIZE - 2)) + 2;
}


// Managing the score's array increment logic
void increment_score_digits(int* score_digits) {
    if ((++score_digits[0]) == 10) {
        score_digits[0] = 0;
        score_digits[1]++;
        if ((score_digits[1] + 1) == 10) {
            score_digits[1] = 0;
            score_digits[2]++;
        }
    }
}


// End-game and display score
void game_over(int* score) {
    printf("GAME OVER! Your score was: ");
    for (int i = 2; i >= 0; i--) {
        printf("%d", score[i]);
    }
    printf(".\n");
}


int main() {
    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;

    object_coordinates apple;
    direction snake_direction = DOWN; 
    object_coordinates snake[MAX_NUMBER_SEGMENTS];

    int current_number_segments = 4;
    int score[3] = {0, 0, 0}; 
    
    srand(time(NULL));

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL couldn't initialize. SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

    window = SDL_CreateWindow("Snake", WIN_X, WIN_Y, WIN_WIDTH, WIN_HEIGHT, SDL_WINDOW_BORDERLESS);
    if (!window) {
        fprintf(stderr, "Window couldn't be created! SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        fprintf(stderr, "Renderer couldn't be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    initialize_snake(snake, current_number_segments);
    generate_apple(&apple);

    // Main loop
    bool quit = false;
    SDL_Event event;

    while (!quit) {

        // Handle events
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    quit = true;
                    break;
                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym) {
                        case SDLK_ESCAPE:
                            quit = true;
                            break;
                        case SDLK_UP:
                            if (snake_direction != DOWN) {
                                snake_direction = UP;
                            }
                            break;
                        case SDLK_DOWN:
                            if (snake_direction != UP) {
                                snake_direction = DOWN;
                            }
                            break;
                        case SDLK_LEFT:
                            if (snake_direction != RIGHT) {
                                snake_direction = LEFT;
                            }
                            break;
                        case SDLK_RIGHT:
                            if (snake_direction != LEFT) {
                                snake_direction = RIGHT;
                            }
                            break;
                    }
                    break;
            }
        }

        move_snake(snake, current_number_segments, snake_direction);

        // Check collisions
        if (has_collided_boundary(&snake[0])) {
            game_over(score);
            break;
        }

        if (has_collided_with_itself(&snake[0], current_number_segments)) {
            game_over(score);
            break;
        }

        if (has_eaten_food(&snake[0], &apple)) {
            generate_apple(&apple);
            increment_score_digits(score);
            current_number_segments += 1;
            snake[current_number_segments - 1] = snake[current_number_segments - 2];
        }

        // Rendering
        SDL_SetRenderDrawColor(renderer, 0x35, 0x35, 0x36, 255);
        SDL_RenderClear(renderer);

        render_snake(renderer, snake, current_number_segments, CELL_SIZE);
        render_digit(renderer, glyphs[score[2]], WIN_WIDTH / 2 - CELL_SIZE, CELL_SIZE, CELL_SIZE / 5);
        render_digit(renderer, glyphs[score[1]], WIN_WIDTH / 2, CELL_SIZE, CELL_SIZE / 5);
        render_digit(renderer, glyphs[score[0]], WIN_WIDTH / 2 + CELL_SIZE, CELL_SIZE, CELL_SIZE / 5);
        render_apple(renderer, apple.x, apple.y, CELL_SIZE);
        render_grid(renderer, CELL_SIZE);

        SDL_RenderPresent(renderer);
        SDL_Delay(DELAY_TIME);
    }

    // Cleanup
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

