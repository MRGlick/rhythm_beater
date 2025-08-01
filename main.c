
#define SDL_MAIN_HANDLED

#include <SDL2/SDL.h>
#include <SDL2/SDL_gpu.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "err_macros.c"
#include "match.h"
#include "inttypes.h"
#include "array.c"
#include "todo.c"

typedef struct Node {
    void (*render)(struct Node *, double);
    void (*tick)(struct Node *, double);

    int id;
    struct Node *parent;
    struct Node **children;
} Node;

typedef struct Comp {
    int type_id;
    void (*render)(struct Node *, double);
    void (*tick)(struct Node *, double);
} Comp;


#define DEFINE_COMP(name, ...) \
    const int Comp##name##_type_id = __COUNTER__; \
    typedef struct { \
        Comp comp; \
        __VA_ARGS__ \
    } Comp##name;

//  Do NOT use __COUNTER__ between COMP_TYPE_IDS_START and COMP_TYPE_IDS_END!!!

const int COMP_TYPE_IDS_START = __COUNTER__;

DEFINE_COMP(
    Foo, 
    int x, y, z;
)

DEFINE_COMP(
    Bar, 
    int x, y, z;
)

DEFINE_COMP(
    Baz, 
    int x, y, z;
)

DEFINE_COMP(
    Faz, 
    int x, y, z;
)

const int COMP_TYPE_IDS_END = __COUNTER__;


typedef u16 comp_idx_t;

typedef struct CompArray {
    int type_id;
    int _id_to_idx_len;
    comp_idx_t *id_to_idx; // the more generally used name is the 'sparse array' but thats a stupid name
    Comp *comps;
} CompArray;

typedef struct ECS {
    CompArray **comp_arrays;
} ECS;


GPU_Target *screen = NULL;
Node *_root = NULL;
ECS *_ecs = NULL;


#define SDL_INIT_FLAGS SDL_INIT_AUDIO | SDL_INIT_VIDEO
#define ERRBUF_LEN 1024

#define WINDOW_WIDTH 500
#define WINDOW_HEIGHT 500
#define FPS 60

// #FUNCS

#define Comp_new(type) ({type *comp = malloc(sizeof(type)); })

void ECS_add_comp(Comp *comp);

void ECS_add(CompArray *comp_arr);

void ECS_init(void);

int Node_child_count(Node *node);

Node *_Node_new(Comp *comps, int count);

#define Node_new(comp_arr_literal) ({Comp __arr[] = comp_arr_literal; _Node_new(__arr, sizeof(__arr) / sizeof(Comp)); })

#define Node_new_empty() _Node_new(NULL, 0)

void Node_free(Node *node);

void CompArray_expand_to_fit_id(CompArray *comp_array, int id);

void CompArray_add_comp(CompArray *comp_arr, Comp *comp, int id);

void CompArray_free(CompArray *comp_arr);

CompArray *_CompArray_new(int type_id);

#define CompArray_new(comp_type) _CompArray_new(comp_type##_type_id)

int get_next_id(void);

void on_input(SDL_Event *e);

void on_key_pressed(SDL_Keycode key);

void on_key_released(SDL_Keycode key);

bool is_key_pressed(SDL_Scancode scancode);

void render(double delta);

void tick(double delta);

void start();

int main() {

    char errbuf[ERRBUF_LEN] = {0};

    if (SDL_Init(SDL_INIT_FLAGS)) err_exit(
        "SDL Couldn't init! error: '%s'", SDL_GetErrorMsg(errbuf, ERRBUF_LEN)
    );

    screen = GPU_Init(WINDOW_WIDTH, WINDOW_HEIGHT, GPU_DEFAULT_INIT_FLAGS);
    if (!screen) err_exit(
        "GPU Couldn't init! error: '%s'", GPU_GetErrorString(GPU_PopErrorCode().error)
    );

    ECS_init();

    start();

    u64 time = SDL_GetTicks64();
    double frame_timer = 0;

    while (true) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            on_input(&e);
        }
        
        u64 delta_mili = SDL_GetTicks64() - time;
        time = SDL_GetTicks64();

        double delta = (double)delta_mili / 1000;
        frame_timer += delta;

        if (frame_timer > 1.0/FPS) {
            tick(delta);
            render(delta);
        }
        
    }


    getchar();
}

void on_input(SDL_Event *e) {
    match(e->type) {
        case (SDL_QUIT) {
            exit(EXIT_SUCCESS);
        }
        case (SDL_KEYDOWN) {
            on_key_pressed(e->key.keysym.sym);
        }
        case (SDL_KEYUP) {
            on_key_released(e->key.keysym.sym);
        }
    }
}


void on_key_pressed(SDL_Keycode key) {
    match (key) {

    }
}

void on_key_released(SDL_Keycode key) {
    match (key) {
        
    }
}

bool is_key_pressed(SDL_Scancode scancode) {

    assert(scancode >= 0 && scancode < SDL_NUM_SCANCODES);

    return SDL_GetKeyboardState(NULL)[scancode];
}

void tick(double delta) {
    
}

void render(double delta) {

}

int _max_id = 0;

int get_next_id(void) {
    static comp_idx_t _id = 1; // reserve 0
    return _max_id = _id++;
}

#define COMP_ARRAY_SPARSE_SIZE 100

CompArray *_CompArray_new(int type_id) {
    CompArray *comp_arr = malloc(sizeof(CompArray));
    *comp_arr = (CompArray){
        .type_id = type_id,
        .id_to_idx = calloc(sizeof(comp_idx_t), COMP_ARRAY_SPARSE_SIZE),
        ._id_to_idx_len = COMP_ARRAY_SPARSE_SIZE,
        .comps = array(Comp, 4)
    };

    return comp_arr;
}



void CompArray_free(CompArray *comp_arr) {
    free(comp_arr->id_to_idx);
    array_free(comp_arr->comps);
    free(comp_arr);
}

void CompArray_add_comp(CompArray *comp_arr, Comp *comp, int id) {
    assert(id < _max_id);
    CompArray_expand_to_fit_id(comp_arr, id);
    if (comp_arr->id_to_idx[id]) err_exit(
        "Can't add comp! id '%d' is already taken!", id
    );
    array_append(comp_arr->comps, *comp);
    comp_arr->id_to_idx[id] = array_length(comp_arr->comps) - 1;
}

void CompArray_expand_to_fit_id(CompArray *comp_array, int id) {
    if (comp_array->_id_to_idx_len >= id) return;

    int old_len = comp_array->_id_to_idx_len;

    while (comp_array->_id_to_idx_len < id) comp_array->_id_to_idx_len *= 2;

    comp_idx_t *new_arr = calloc(sizeof(comp_idx_t), comp_array->_id_to_idx_len);

    memcpy(new_arr, comp_array->id_to_idx, old_len);

    free(comp_array->id_to_idx);

    comp_array->id_to_idx = new_arr;
}


Node *_Node_new(Comp *comps, int count) {
    Node *node = malloc(sizeof(Node));
    *node = (Node) {
        .children = array(Node *, 2),
        .id = get_next_id(),
        .parent = NULL,
        .render = NULL,
        .tick = NULL
    };

    for (int i = 0; i < count; i++) {
        ECS_add_comp(&comps[i]);
    }

    return node;
}

void Node_free(Node *node) {
    for (int i = 0; i < Node_child_count(node); i++) {
        Node_free(node->children[i]);
    }
    array_free(node->children);
    
    print_todo("ECS component deletion when deleting a node");
}

int Node_child_count(Node *node) {
    return array_length(node->children);
}

void ECS_init(void) {
    _ecs = malloc(sizeof(ECS));
    _ecs->comp_arrays = array(CompArray *, 2);

    ECS_add(CompArray_new(CompFoo));
    ECS_add(CompArray_new(CompBar));
    ECS_add(CompArray_new(CompBaz));
    ECS_add(CompArray_new(CompFaz));
}

void ECS_add(CompArray *comp_arr) {
    array_append(_ecs->comp_arrays, comp_arr);
}

void start() {
    _root = Node_new_empty();
    
}

void ECS_add_comp(Comp *comp) {

}

// #END