#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#include "pthread.h" // POSIX style threads management

#include <stdatomic.h> // C11 atomic data types

#include <time.h> // Required for: clock()

#include <stdio.h> // Required for: FILE, fopen(), fclose(), fgets(), popen(), pclose()
#include <unistd.h> // Include this for the sleep function


#define GRID_SIZE 100

// Using C11 atomics for synchronization
// NOTE: A plain bool (or any plain data type for that matter) can't be used for inter-thread synchronization
static atomic_bool dataLoaded = false;  // Data Loaded completion indicator
static void *LoadDataThread(void *arg); // Loading data thread function declaration
static void WelcomeScreen(void);        // Welcome screen function declaration
static int CustomButton(Rectangle bounds, const char *text);

static atomic_int dataProgress = 0; // Data progress accumulator

const int screenWidth = 1067;
const int screenHeight = 600;
int height = screenHeight / 2 - 80;

int main(void)
{
    // Initialization

    pthread_t threadId = {0}; // Loading data thread id

    enum
    {
        STATE_WAITING,
        STATE_LOADING,
        STATE_FINISHED
    } state = STATE_WAITING;
    int framesCounter = 0;

    InitWindow(screenWidth, screenHeight, "Process Scheduler Simulation");
    SetTargetFPS(60);

    Texture2D background = LoadTexture("./assets/images/neon.png");

    BeginDrawing();
    // Draw the background image
    DrawTexture(background, 0, 0, WHITE);
    ClearBackground(RAYWHITE);
    EndDrawing();

    char quantumSize[32] = "\0"; // Buffer to store user input for quantum size
    int algoChoice = 2;          // Variable to store user choice for algorithm
    const char *algoOptions = "Non-preemptive Highest Priority First;Shortest Remaining time Next;Round Robin";
    int quantum = 1; // Initial value


    char filePath[512] = {0}; // Buffer to store the file path

    WelcomeScreen();

    bool buttonPressed = false; // Flag to check if the button has been pressed

   usleep(300000);

    while (!WindowShouldClose())
    {

        // Draw

        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawTexture(background, 0, 0, WHITE);

        // Update
        if (!buttonPressed)
        {
            GuiComboBox((Rectangle){screenWidth / 2 - 110, height, 250, 30}, algoOptions, &algoChoice);
            buttonPressed = CustomButton((Rectangle){screenWidth / 2 - 110, height + ((algoChoice == 2) ? 190 : 110), 250, 30}, "Continue");
            if (GuiButton((Rectangle){screenWidth / 2 - 110, height + ((algoChoice == 2) ?150:60), 250, 30}, "Choose File"))
            {
                // Open file dialog using zenity
                FILE *fp = popen("zenity --file-selection", "r");
                if (fp != NULL)
                {
                    fgets(filePath, sizeof(filePath), fp);
                    pclose(fp);
                    // Remove trailing newline character
                    size_t len = strlen(filePath);
                    if (len > 0 && filePath[len - 1] == '\n')
                    {
                        filePath[len - 1] = '\0';
                    }
                    // Print selected file path
                    printf("Selected file: %s\n", filePath);
                }
            }
        }
        else
        {
            switch (state)
            {
            case STATE_WAITING:
            {
                int error = pthread_create(&threadId, NULL, &LoadDataThread, NULL);
                if (error != 0)
                    TraceLog(LOG_ERROR, "Error creating loading thread");
                else
                    TraceLog(LOG_INFO, "Loading thread initialized successfully");

                state = STATE_LOADING;
            }
            break;
            case STATE_LOADING:
            {
                framesCounter++;
                if (atomic_load_explicit(&dataLoaded, memory_order_relaxed))
                {
                    framesCounter = 0;
                    int error = pthread_join(threadId, NULL);
                    if (error != 0)
                        TraceLog(LOG_ERROR, "Error joining loading thread");
                    else
                        TraceLog(LOG_INFO, "Loading thread terminated successfully");

                    state = STATE_FINISHED;
                }
            }
            break;
            case STATE_FINISHED:
            {
                if (IsKeyPressed(KEY_ENTER))
                {
                    // Reset everything to launch again
                    atomic_store_explicit(&dataLoaded, false, memory_order_relaxed);
                    atomic_store_explicit(&dataProgress, 0, memory_order_relaxed);
                    state = STATE_WAITING;
                }
            }
            break;
            default:
                break;
            }
        }

        if (!buttonPressed)
        {
            DrawText("Choose An Algorithm", screenWidth / 2 - 210, height - 100, 45, MAGENTA);
            if (algoChoice == 2)
            {
                // Spinner
                DrawText("Quantum", screenWidth / 2 - 110, height + 86, 15, MAGENTA); // Change the font size and color as needed
                GuiSpinner((Rectangle){screenWidth / 2 - 40, height + 80, 180, 30}, "", &quantum, 1, 10000, false);
            }
        }
        else
        {
            int loadingHeight = height + 50;
            switch (state)
            {
            case STATE_WAITING:
                // DrawText("PRESS ENTER to START LOADING DATA", 150, 170, 20, DARKGRAY);
                break;
            case STATE_LOADING:
            {
                DrawRectangle(280, loadingHeight, atomic_load_explicit(&dataProgress, memory_order_relaxed), 60, SKYBLUE);
                if ((framesCounter / 15) % 2)
                    DrawText("LOADING DATA...", 380, loadingHeight + 10, 40, DARKBLUE);
            }
            break;
            case STATE_FINISHED:
            {
                DrawRectangle(280, loadingHeight, 500, 60, LIME);
                DrawText("DATA LOADED!", 380, loadingHeight + 10, 40, GREEN);
            }
            break;
            default:
                break;
            }

            DrawRectangleLines(280, loadingHeight, 500, 60, DARKGRAY);
        }
        EndDrawing();
    }
    UnloadTexture(background);

    CloseWindow(); // Close window and OpenGL context

    return 0;
}

static void WelcomeScreen()
{

    Texture2D developers[] = {LoadTexture("./assets/images/Ahmed Alaa.png"), LoadTexture("./assets/images/Akram.png"), LoadTexture("./assets/images/mo2.png"), LoadTexture("./assets/images/marwan.png")};
    const char *welcomeText = "Welcome to";
    const char *projectName = "Synergify";
    const char *description[] = {"This is a Process Scheduler Simulation", "Enhance your understanding of the process scheduling algorithms", "Avialable algorithms: Non-preemptive Highest Priority First, Shortest Remaining time Next, Round Robin", "Task manager to track the progress of the processes", "Image Created at the end to show information about Processes", "Enjoy!"};

    int maxFrameCount = 80; // The number of frames over which the text will be typed
    int maxFrameCount_description = 60;
    int frameCount_description = 0; // The current frame count
    int frameCount = 0;             // The current frame count
    int welcomeNumChars = 0;
    int projectNumChars = 0;
    int descriptionNumChars = 0;
    int descriptionIdx = 0;
    int startX = 325;
    int imageWidth = 80;
    int gap = 50;
    bool buttonPressed = false;

    while (!WindowShouldClose())
    {
        // update
        buttonPressed = CustomButton((Rectangle){screenWidth / 2 - 50, height + 220, 100, 30}, "START!");

        if (buttonPressed)
        {
            for (int i = 0; i < 4; i++)
                UnloadTexture(developers[i]);
            return;
        }

        BeginDrawing();
        if (welcomeNumChars < strlen(welcomeText))
        {
            // Calculate the number of characters to draw
            welcomeNumChars = strlen(welcomeText) * frameCount / maxFrameCount;
            // Draw the text
            char buffer[256];
            strncpy(buffer, welcomeText, welcomeNumChars);
            buffer[welcomeNumChars] = '\0'; // Null-terminate the string
            DrawText(buffer, screenWidth / 2 - 260, 50, 45, WHITE);
        }
        else if (projectNumChars < strlen(projectName))
        {
            // Calculate the number of characters to draw
            projectNumChars = strlen(projectName) * frameCount / maxFrameCount;
            // Draw the text
            char buffer[256];
            strncpy(buffer, projectName, projectNumChars);
            buffer[projectNumChars] = '\0'; // Null-terminate the string
            DrawText(buffer, screenWidth / 2 + 20, 40, 55, MAGENTA);
        }
        else if (descriptionIdx < 6)
        {

            if (descriptionNumChars < strlen(description[descriptionIdx]))
            {
                // Calculate the number of characters to draw
                descriptionNumChars = strlen(description[descriptionIdx]) * frameCount_description / maxFrameCount_description;
                // Draw the text
                char buffer[256];
                strncpy(buffer, description[descriptionIdx], descriptionNumChars);
                buffer[descriptionNumChars] = '\0'; // Null-terminate the string
                DrawText(buffer, screenWidth / 2 - 300, height + 50 * descriptionIdx - 80, 15, PURPLE);
            }
            else
            {
                descriptionNumChars = 0;
                descriptionIdx++;
            }

            frameCount_description = (frameCount_description + 1) % (maxFrameCount_description + 1);
        }

        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 1000; j++)
                DrawCircleLines(startX + gap * i + imageWidth * i + 40, height + 280 + 40, imageWidth / 2 + (float)j / 1000, BLACK);
            DrawTexture(developers[i], startX + gap * i + imageWidth * i, height + 280, WHITE);
        }
        bool hoverFlag = false;
        for (int i = 0; i < 4; i++)
        {
            // Check if the mouse is over the image
            Vector2 mousePos = GetMousePosition();
            if (CheckCollisionPointCircle(mousePos, (Vector2){startX + gap * i + imageWidth * i + 40, height + 280 + 40}, imageWidth / 2))
            {
                // If the left mouse button is pressed, open the URL
                hoverFlag = true;

                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                {
                    if (i == 0)
                        system("xdg-open https://github.com/Ahmed-Aladdiin");
                    else if (i == 1)
                        system("xdg-open https://github.com/akramhany");
                    else if (i == 2)
                        system("xdg-open https://github.com/Mo2Hefny");
                    else if (i == 3)
                        system("xdg-open https://github.com/marwan2232004");
                }
            }
            else
                SetMouseCursor(MOUSE_CURSOR_ARROW);
        }
        if (hoverFlag)
            SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
        else
            SetMouseCursor(MOUSE_CURSOR_ARROW);

        EndDrawing();

        // Increase the frame count
        frameCount = (frameCount + 1) % (maxFrameCount + 1);
    }
}

// Loading data thread function definition
static void *LoadDataThread(void *arg)
{
    int timeCounter = 0;        // Time counted in ms
    clock_t prevTime = clock(); // Previous time

    // We simulate data loading with a time counter for 5 seconds
    while (timeCounter < 5000)
    {
        clock_t currentTime = clock() - prevTime;
        timeCounter = currentTime * 1000 / CLOCKS_PER_SEC;

        // We accumulate time over a global variable to be used in
        // main thread as a progress bar
        atomic_store_explicit(&dataProgress, timeCounter / 10, memory_order_relaxed);
    }

    // When data has finished loading, we set global variable
    atomic_store_explicit(&dataLoaded, true, memory_order_relaxed);

    return NULL;
}

static int CustomButton(Rectangle bounds, const char *text)
{
    int result = 0;
    GuiState state = guiState;

    // Update control
    //--------------------------------------------------------------------
    if ((state != STATE_DISABLED) && !guiLocked && !guiSliderDragging)
    {
        Vector2 mousePoint = GetMousePosition();

        // Check button state
        if (CheckCollisionPointRec(mousePoint, bounds))
        {
            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
                state = STATE_PRESSED;
            else
                state = STATE_FOCUSED;

            if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
                result = 1;
        }
    }
    //--------------------------------------------------------------------

    // Draw control
    //--------------------------------------------------------------------
    GuiDrawRectangle(bounds, GuiGetStyle(BUTTON, BORDER_WIDTH), PURPLE, VIOLET);
    GuiDrawText(text, GetTextBounds(BUTTON, bounds), GuiGetStyle(BUTTON, TEXT_ALIGNMENT), WHITE);

    if (state == STATE_FOCUSED)
        GuiTooltip(bounds);
    //------------------------------------------------------------------

    return result; // Button pressed: result = 1
}