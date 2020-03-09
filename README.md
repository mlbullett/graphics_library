# graphics_library

A small and efficient c++ graphics library using opengl.
Built mostly as a learning/experimental project.

Were it a class, interfaces are meant to be:

- DrawLine
- DrawRect
- DrawBox
- DrawPoly
- DrawFilledPoly
- DrawEllipse
- DrawPie

Code is annotated with implementation details, should run pretty fast.
Needs to be compiled with -lGL -lGLU and -lglut flags.
i.e: "g++ -o graphics_test graphics_lib.cpp -lGL -lGLU -lglut"
