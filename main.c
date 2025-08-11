
#pragma GCC diagnostic ignored "-Wsign-conversion"

#define SDL_MAIN_HANDLED

#include <SDL2/SDL.h>
#include <SDL2/SDL_gpu.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "dbg_macros.c"
#include "match.h"
#include "inttypes.h"
#include "array.c"
#include "todo.c"
#include "mark_unused.h"
#include "move.c"

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

// Can't pull out Comp##name into a macro because the language server is too weak for my shit

#define DEFINE_COMP(name, ...) \
    const int Comp##name##_type_id = __COUNTER__; \
    typedef struct { \
        Comp comp; \
        __VA_ARGS__ \
    } Comp##name; \
    typedef void name;
    // ^^ for autocomplete

#define COMP_INIT(name, comp_name, ...) \
    void Comp##name##_init(Comp##name *comp_name, __VA_ARGS__)


//  Do NOT use __COUNTER__ between COMP_TYPE_IDS_START and COMP_TYPE_IDS_END!!!
const int COMP_TYPE_IDS_START = __COUNTER__;

DEFINE_COMP(Foo, int x, y, z;)

COMP_INIT(Foo, f, int x, int y, int z) {
    f->x = x;
    f->y = y;
    f->z = z;
}

DEFINE_COMP(
    Bar, 
    int x, y, z;
)

COMP_INIT(Bar, b, int x, int y, int z) {
    b->x = x;
    b->y = y;
    b->z = z;
}

const int COMP_TYPE_IDS_END = __COUNTER__;


typedef u16 comp_idx_t;

typedef struct CompArray {
    int type_id;
    size_t _id_to_idx_len;
    comp_idx_t *id_to_idx; // the more generally used name is the 'sparse array' but thats a stupid name
    Comp **comps; // indirect so we can do inheritance
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

void ECS_remove_id(int id);

void ECS_remove_comp(int id, Comp *comp);

int Node_find_child(Node *parent, Node *child);

void Node_remove_child(Node *parent, Node *child);

void Node_remove_idx(Node *parent, int idx);

#define Comp_new(type_name, ...) \
    ((Comp *)({ \
        Comp##type_name *__c = calloc(sizeof(Comp##type_name), 1); \
        __c->comp.type_id = Comp##type_name##_type_id; \
        Comp##type_name##_init(__c, __VA_ARGS__); \
        __c; \
    }))

void CompArray_print_dbg(CompArray *comp_arr);

Node *Node_get_child(Node *parent, int idx);

static inline bool is_valid_type_id(int type_id);

static inline bool is_valid_id(int id);

Comp *CompArray_get(CompArray *comp_arr, int id);

static inline int type_id_to_ecs_idx(int type_id);

Node *get_root();

void Node_add_child(Node *node, Node *child);

void ECS_add_comp(Comp *comp, int id);

Comp *_ECS_get_comp(Node *node, int type_id);

#define ECS_get_comp(node, type) ((Comp##type *)_ECS_get_comp(node, Comp##type##_type_id))

void ECS_add_comp_arr(CompArray *comp_arr);

void ECS_init(void);

int Node_child_count(Node *node);

Node *_Node_new(Comp **comps, int count);

#define Node_new(...) \
    ({ \
        Comp *__arr[] = { __VA_ARGS__ }; \
        _Node_new(__arr, sizeof(__arr) / sizeof(Comp *)); \
    })

#define Node_new_empty() _Node_new(NULL, 0)

void _Node_free(Node *node);

#define Node_free(node) _Node_free(move(node))

void CompArray_expand_to_fit_id(CompArray *comp_array, int id);

void CompArray_add(CompArray *comp_arr, Comp *comp, int id);

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

void start(void);

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

// #TICK
void tick(UNUSED double delta) {

}


// #RENDER
void render(UNUSED double delta) {

}

int _max_id = 0; // if an id is equal to or bigger than _max_id, it is invalid.

int get_next_id(void) {
    static comp_idx_t _id = 1; // reserve 0
    int new_id = _id++;
    _max_id = new_id + 1;
    return new_id;
}

#define COMP_ARRAY_SPARSE_SIZE 100

#define COMP_ARRAY_IDX_NONE -1

CompArray *_CompArray_new(int type_id) {
    CompArray *comp_arr = malloc(sizeof(CompArray));
    *comp_arr = (CompArray){
        .type_id = type_id,
        .id_to_idx = malloc(sizeof(comp_idx_t) * COMP_ARRAY_SPARSE_SIZE),
        ._id_to_idx_len = COMP_ARRAY_SPARSE_SIZE,
        .comps = array(Comp *, 4)
    };

    for (int i = 0; i < comp_arr->_id_to_idx_len; i++) {
        comp_arr->id_to_idx[i] = COMP_ARRAY_IDX_NONE;
    }

    return comp_arr;
}



void CompArray_free(CompArray *comp_arr) {
    free(comp_arr->id_to_idx);
    array_free(comp_arr->comps);
    free(comp_arr);
}

void CompArray_add(CompArray *comp_arr, Comp *comp, int id) {
    assert(is_valid_id(id));
    CompArray_expand_to_fit_id(comp_arr, id);
    if (comp_arr->id_to_idx[id]) err_exit(
        "Can't add comp! id '%d' is already taken!", id
    );
    array_append(comp_arr->comps, comp);
    comp_arr->id_to_idx[id] = (comp_idx_t)(array_length(comp_arr->comps) - 1);
    print_dbg("ID: %d, id_to_idx[ID]: %d", id, comp_arr->id_to_idx[id]);
}

void CompArray_expand_to_fit_id(CompArray *comp_array, int id) {
    if (comp_array->_id_to_idx_len >= id) return;

    size_t old_len = comp_array->_id_to_idx_len;

    while (comp_array->_id_to_idx_len < id) comp_array->_id_to_idx_len *= 2;

    comp_idx_t *new_arr = calloc(sizeof(comp_idx_t), comp_array->_id_to_idx_len);

    memcpy(new_arr, comp_array->id_to_idx, old_len);

    free(comp_array->id_to_idx);

    comp_array->id_to_idx = new_arr;
}


Node *_Node_new(Comp **comps, int count) {

    print_dbg("Count: %d", count);

    Node *node = malloc(sizeof(Node));
    *node = (Node) {
        .children = array(Node *, 2),
        .id = get_next_id(),
        .parent = NULL,
        .render = NULL,
        .tick = NULL
    };

    for (int i = 0; i < count; i++) {
        ECS_add_comp(comps[i], node->id);
    }

    return node;
}

void _Node_free(Node *node) {
    for (int i = 0; i < Node_child_count(node); i++) {
        _Node_free(node->children[i]);
    }
    array_free(node->children);
    
    
}

int Node_child_count(Node *node) {
    return array_length(node->children);
}

void ECS_init(void) {
    _ecs = calloc(sizeof(ECS), 1);
    _ecs->comp_arrays = array(CompArray *, 2);

    for (int type_id = COMP_TYPE_IDS_START + 1; type_id < COMP_TYPE_IDS_END; type_id++) {
        array_append(_ecs->comp_arrays, _CompArray_new(type_id));
    }

    print_dbg("Initialized ECS!");
}

void ECS_add_comp_arr(CompArray *comp_arr) {
    array_append(_ecs->comp_arrays, comp_arr);
}

// #START
void start() {
    _root = Node_new_empty();
    Node_add_child(_root, Node_new(Comp_new(Foo, 1, 2, 3), Comp_new(Bar, 4, 5, 6)));
    Node_add_child(_root, Node_new(Comp_new(Foo, 1, 2, 3), Comp_new(Bar, 4, 5, 6)));
    Node_add_child(_root, Node_new(Comp_new(Foo, 1, 2, 3), Comp_new(Bar, 4, 5, 6)));
    Node_add_child(_root, Node_new(Comp_new(Foo, 1, 2, 3), Comp_new(Bar, 4, 5, 6)));



    CompFoo *c = ECS_get_comp(_root->children[2], Foo);

    CompArray_print_dbg(_ecs->comp_arrays[0]);
    assert(c->x == 1);
    print_dbg("yay!");
    
}

void ECS_add_comp(Comp *comp, int id) {
    CompArray *comp_arr = _ecs->comp_arrays[type_id_to_ecs_idx(comp->type_id)];
    CompArray_add(comp_arr, comp, id);
}

// Will detach the child from their previous parent
void Node_add_child(Node *node, Node *child) {

    assert(node);
    assert(node->children);
    assert(child);

    array_append(node->children, child);

    Node_remove_child(node, child);
    child->parent = node;
}

Node *get_root() {
    return _root;
}

// Converts a component type id into the index of its component array in the ECS
static inline int type_id_to_ecs_idx(int type_id) {
    assert(is_valid_type_id(type_id));

    return type_id - COMP_TYPE_IDS_START - 1;
}

static inline bool is_valid_id(int id) {
    return id > 0 && id < _max_id;
}

Comp *_ECS_get_comp(Node *node, int type_id) {
    assert(is_valid_type_id(type_id));
    assert(node);
    
    CompArray *comp_arr = _ecs->comp_arrays[type_id_to_ecs_idx(type_id)];

    CompArray_print_dbg(comp_arr);

    return CompArray_get(comp_arr, node->id);
}

Comp *CompArray_get(CompArray *comp_arr, int id) {
    assert(is_valid_id(id));
    assert(comp_arr);

    int idx = comp_arr->id_to_idx[id];

    if (idx == -1)

    return comp_arr->comps[comp_arr->id_to_idx[id]];
}

static inline bool is_valid_type_id(int type_id) {
    return type_id > COMP_TYPE_IDS_START && type_id < COMP_TYPE_IDS_END;
}


Node *Node_get_child(Node *parent, int idx) {
    assert(parent);
    assert(idx >= 0 && idx < array_length(parent->children));

    return parent->children[idx];
}

void CompArray_print_dbg(CompArray *comp_arr) {

    assert(comp_arr);

    printf("CompArray at <%p> \n", comp_arr);
    printf("Type ID: %d (index = %d) \n", comp_arr->type_id, type_id_to_ecs_idx(comp_arr->type_id));
    printf("Id map: \n");
    for (int i = 0; i < comp_arr->_id_to_idx_len; i++) {
        printf("\t#%d: %d \n", i, comp_arr->id_to_idx[i]);
    }
    printf("Component array: \n");
    for (int i = 0; i < array_length(comp_arr->comps); i++) {
        printf("\t <%p> \n", comp_arr->comps[i]);
    }
    
}

void Node_remove_idx(Node *parent, int idx) {
    assert(parent);
    assert(idx >= 0 && idx < array_length(parent->children));

    Node *child = parent->children[idx];
    array_remove(parent->children, idx);
    child->parent = NULL;
}

void Node_remove_child(Node *parent, Node *child) {
    assert(parent);
    assert(child);
    assert(child->parent == parent);

    Node_remove_idx(parent, Node_find_child(parent, child));
    child->parent = NULL;
}

int Node_find_child(Node *parent, Node *child) {
    assert(parent);

    for (int i = 0; i < Node_child_count(parent); i++) {
        if (Node_get_child(parent, i) == child) return i;
    }
    return -1;
}

void ECS_remove_comp(int id, Comp *comp) {
    assert(is_valid_id(id));
    assert(comp);
    
    CompArray *comp_arr = _ecs->comp_arrays[type_id_to_ecs_idx(comp->type_id)];
    
    // swap for O(1) removal
    int idx = comp_arr->id_to_idx[id];

    if (idx == COMP_ARRAY_IDX_NONE) {
        print_dbg("Component with id=%d and type_id=%d (start=%d) doesn't exist! not removing.", id, comp->type_id, COMP_TYPE_IDS_START);
        return;
    }

    int comp_count = array_length(comp_arr->comps);

    int last_idx = comp_count - 1;
    Comp *temp = comp_arr->comps[idx];
    comp_arr->comps[idx] = comp_arr->comps[last_idx];
    comp_arr->comps[last_idx] = temp;
    comp_arr->id_to_idx[id] = 0;
    for (int i = 0; i < comp_arr->_id_to_idx_len; i++) {
        if (comp_arr->id_to_idx[i] == last_idx) {
            comp_arr->id_to_idx[i] = idx;
            break;
        }
    }

    array_remove(comp_arr->comps, comp_count - 1);
}

void ECS_remove_id(int id) {
    for (int i = COMP_TYPE_IDS_START + 1; i < COMP_TYPE_IDS_END; i++) {
        ECS_remove_comp(id, CompArray_get(_ecs->comp_arrays[type_id_to_ecs_idx(i)], id));
    }
}

// #END