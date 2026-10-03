#pragma once

struct ButtonEdge {
    bool held = false;
    bool pressed = false;
    bool released = false;
};

struct InputFrame {
    bool connected = false;

    ButtonEdge left, right, up, down;
    ButtonEdge cross, circle, square, triangle;
    ButtonEdge l1, r1, l2, r2;
    ButtonEdge start, select, l3, r3;

    // Normalized -1..+1, center = 0.
    float left_x = 0.0f;
    float left_y = 0.0f;
    float right_x = 0.0f;
    float right_y = 0.0f;
};
