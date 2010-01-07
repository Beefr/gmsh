Point(1) = {0, 0, 0, .1};
Point(2) = {.6, 0, 0, .1};
Point(3) = {.6, .2, 0, .1};
Point(4) = {2.4, .2, 0, .1};
Point(5) = {2.4, 1, 0, .1};
Point(6) = {0, 1, 0, .1};
Line(1) = {6, 5};
Line(2) = {5, 4};
Line(3) = {4, 3};
Line(4) = {3, 2};
Line(5) = {2, 1};
Line(6) = {1, 6};
Line Loop(7) = {2, 3, 4, 5, 6, 1};
Plane Surface(8) = {7};
Physical Line("Walls") = {1, 3, 4, 5};
Physical Line("LeftRight") = {2, 6};
Physical Surface("Body") = {8};
