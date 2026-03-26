namespace TestMacros;

#include <stdio.h>
#include <raylib.h>

int32 main() {
    InitWindow(400, 300, c"Test C Macros");
    defer CloseWindow();

    SetTargetFPS(60);
    
    int32 frame = 0;
    
    while !WindowShouldClose() && frame < 120 {
        BeginDrawing();
        
        // Testar macros de cores do raylib
        if frame < 30 {
            ClearBackground(RED);
            DrawText(c"RED (macro)", 100, 130, 30, WHITE);
        } else if frame < 60 {
            ClearBackground(GREEN);
            DrawText(c"GREEN (macro)", 80, 130, 30, WHITE);
        } else if frame < 90 {
            ClearBackground(BLUE);
            DrawText(c"BLUE (macro)", 90, 130, 30, WHITE);
        } else {
            ClearBackground(YELLOW);
            DrawText(c"YELLOW (macro)", 70, 130, 30, BLACK);
        }
        
        EndDrawing();
        frame = frame + 1;
    }
    
    ret 0;
}
