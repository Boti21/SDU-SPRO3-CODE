#include <stdio.h>
#include <stdbool.h>

typedef struct {
    char x;
    char y;
} point;

typedef struct {
    bool blocked;
    point relative_point;
} edge;

typedef struct {
    point coords;
    edge up; // Naming in progress...
    edge right;
    edge down;
    edge left;
} node;

// Example of fully declaring every single point
node test = {
    .coords.x = 1,
    .coords.y = 1,

    .up.blocked = true,
    .up.relative_point.x = 0,
    .up.relative_point.y = 0,

    .right.blocked = false,
    .right.relative_point.x = 1,
    .right.relative_point.y = 0,

    .down.blocked = true,
    .down.relative_point.x = 0,
    .down.relative_point.y = 0,

    .left.blocked = true,
    .left.relative_point.x = 0,
    .left.relative_point.y = 0,
};

// Example of only declaring the points which deviate from 0
node test2 = {0};
test2 = {
    .coords.x = 2,
    .coords.y = 1,

    .up.blocked = false,
    .up.relative_point.x = 0,
    .up.relative_point.y = 1,

    .right.blocked = false,
    .right.relative_point.x = 1,
    .right.relative_point.y = 0,

    .down.blocked = true,
    .down.relative_point.x = 0,
    .down.relative_point.y = 0,

    .left.blocked = false,
    .left.relative_point.x = -1,
    .left.relative_point.y = 0,
};

node grid[3][4] = {0};

int main(void) {

    printf("Hey\n");

    grid[1][1] = test;

    return 0;
}