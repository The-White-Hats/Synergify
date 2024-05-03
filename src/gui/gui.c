#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#include "pthread.h" // POSIX style threads management

#include <stdatomic.h> // C11 atomic data types

#include <time.h> // Required for: clock()
#include "../header.h"

#define PATH_SIZE 256

//------------------------------------------Loading Bar variables and functions------------------------------------------

// Using C11 atomics for synchronization
// NOTE: A plain bool (or any plain data type for that matter) can't be used for inter-thread synchronization
static atomic_bool dataLoaded = false;  // Data Loaded completion indicator
static void *LoadDataThread(void *arg); // Loading data thread function declaration
static atomic_int dataProgress = 0;     // Data progress accumulator
static void loadingBar();

pthread_t threadId = {0}; // Loading data thread id

enum
{
    STATE_WAITING,
    STATE_LOADING,
    STATE_FINISHED
} state = STATE_WAITING;
int framesCounter = 0;

//------------------------------------------------------------------------------------------------------------------------

static void WelcomeScreen(void);                             // Welcome screen
static int CustomButton(Rectangle bounds, const char *text); // Custom button function
static bool inputScreen();                                   // Input screen function
static int autoType(const char *text, int current_frame, int max_frame_count, int x, int y, int font_size, Color color);
void run_process_generator();
void read_input_file();
void read_log_file();
void get_cnt_of_log();
void draw_line(int x, int y, int direction, int start_idx);
void draw_image(int x, int y);
void ouput_image(Texture2D background);
// Global variables

//----------------------------------------------- Screen variables--------------------------------------------------------
const int screenWidth = 1067;
const int screenHeight = 600;
int height = screenHeight / 2 - 80;
//------------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------input variables-----------------------------------------

char quantumSize[32] = "\0"; // Buffer to store user input for quantum size
int algoChoice = 0;          // Variable to store user choice for algorithm
const char *algoOptions = "Non-preemptive Highest Priority First;Shortest Remaining time Next;Round Robin";
int quantum = 1; // Initial value
int totalTime = 5;
int number_of_logs = 0;
int max_height = 0;
char filePath[512] = {0}; // Buffer to store the file path

//------------------------------------------------------------------------------------------------------------------------

typedef struct process_block_s
{
    int time;
    int id;
    int arrival;
    int runtime;
    char status[20];
    int remaining_time;
    int waiting_time;
    int TA;
    float WTA;
} process_block_t;

process_block_t *processes;

void draw_block(int x, int y, process_block_t process);

#define BLOCK_WIDTH 150
#define BLOCK_HEIGHT 100

int main(void)
{
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(screenWidth, screenHeight, "Process Scheduler Simulation");
    SetTargetFPS(60);

    char absolute_path[PATH_SIZE];
    getAbsolutePath(absolute_path, "assets/images/neon.png");
    Texture2D background = LoadTexture(absolute_path);
    Vector2 offset = {0.0f, 0.0f};
    BeginDrawing();
    // Draw the background image
    DrawTexture(background, 0, 0, WHITE);
    ClearBackground(RAYWHITE);
    EndDrawing();

    WelcomeScreen();

    bool buttonPressed = false; // Flag to check if the button has been pressed
    bool once = true;

    while (!WindowShouldClose())
    {
        int wheelMove = GetMouseWheelMove();
        offset.y += wheelMove * 40.0f; // Adjust the multiplier as needed

        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawTexture(background, 0, 0, WHITE);

        if (!buttonPressed)
            buttonPressed = inputScreen();
        else if (state != STATE_FINISHED)
        {
            if (once)
            {
                once = false;
                read_input_file();
                run_process_generator();
            }
            loadingBar();
        }
        else
        {
            if (!once)
            {
                once = true;
                get_cnt_of_log();
                read_log_file();
            }
            BeginMode2D((Camera2D){.offset = offset, .target = (Vector2){0.0f, 0.0f}, .rotation = 0.0f, .zoom = 1.0f});
            draw_image(50, 50);
            EndMode2D();
        }
        EndDrawing();
    }
    ouput_image(background);
    UnloadTexture(background);

    CloseWindow(); // Close window and OpenGL context

    return 0;
}

void run_process_generator()
{
    // Get path to the process.out
    char *args[5];
    char absolute_path[PATH_SIZE];
    getAbsolutePath(absolute_path, "process_generator.out");

    args[0] = absolute_path;
    args[1] = (char *)malloc(12);
    args[2] = (char *)malloc(12);
    sprintf(args[1], "%d", algoChoice + 1);
    sprintf(args[2], "%d", quantum);
    args[3] = filePath;
    args[4] = NULL;
    int pid = fork();
    if (pid == -1)
    {
        perror("Couldn't fork a process in scheduler");
        exit(EXIT_FAILURE);
    }
    else if (pid == 0)
    {
        execvp(args[0], args);
        perror("Couldn't use execvp");
        exit(EXIT_FAILURE);
    }
}

static void WelcomeScreen()
{

    char dev_absolute_path[4][PATH_SIZE];
    getAbsolutePath(dev_absolute_path[0], "assets/images/Ahmed Alaa.png");
    getAbsolutePath(dev_absolute_path[1], "assets/images/Akram.png");
    getAbsolutePath(dev_absolute_path[2], "assets/images/mo2.png");
    getAbsolutePath(dev_absolute_path[3], "assets/images/marwan.png");
    Texture2D developers[] = {LoadTexture(dev_absolute_path[0]),
                              LoadTexture(dev_absolute_path[1]),
                              LoadTexture(dev_absolute_path[2]),
                              LoadTexture(dev_absolute_path[3])};
    const char *welcomeText = "Welcome to";
    const char *projectName = "Synergify";
    const char *description[] = {"This is a Process Scheduler Simulation", "Enhance your understanding of the process scheduling algorithms", "Avialable algorithms: Non-preemptive Highest Priority First, Shortest Remaining time Next, Round Robin", "Task manager to track the progress of the processes", "Image Created at the end to show information about Processes", "Enjoy!"};

    int maxFrameCount = 80;             // The number of frames over which the text will be typed
    int maxFrameCount_description = 60; // The number of frames over which the text will be typed
    int frameCount_description = 0;     // The current frame count
    int frameCount = 0;                 // The current frame count
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
                UnloadTexture(developers[i]); // Unload the textures
            return;
        }

        BeginDrawing();
        if (welcomeNumChars < strlen(welcomeText))
        {
            welcomeNumChars = autoType(welcomeText, frameCount, maxFrameCount, screenWidth / 2 - 260, 50, 45, WHITE);
            frameCount = (frameCount + 1) % (maxFrameCount + 1);
        }
        else if (projectNumChars < strlen(projectName))
        {
            projectNumChars = autoType(projectName, frameCount, maxFrameCount, screenWidth / 2 + 20, 40, 55, MAGENTA);
            frameCount = (frameCount + 1) % (maxFrameCount + 1);
        }
        else if (descriptionIdx < 6)
        {

            if (descriptionNumChars < strlen(description[descriptionIdx]))
                descriptionNumChars = autoType(description[descriptionIdx], frameCount_description, maxFrameCount_description, screenWidth / 2 - 300, height + 50 * descriptionIdx - 80, 15, PURPLE);
            else
            {
                descriptionNumChars = 0;
                descriptionIdx++;
                frameCount_description = 0;
            }

            frameCount_description = (frameCount_description + 1) % (maxFrameCount_description + 1);
        }

        //---------------------------------------------------------Developers Images---------------------------------------------------------

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
        }
        if (hoverFlag)
            SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
        else
            SetMouseCursor(MOUSE_CURSOR_ARROW);

        //----------------------------------------------------------------------------------------------------------------------------

        EndDrawing();

        // Increase the frame count
    }
}

// Loading data thread function definition
static void *LoadDataThread(void *arg)
{
    int timeCounter = 0;        // Time counted in ms
    clock_t prevTime = clock(); // Previous time

    // We simulate data loading with a time counter for 5 seconds
    while (timeCounter < 1000 * totalTime)
    {
        clock_t currentTime = clock() - prevTime;
        timeCounter = currentTime * 1000 / CLOCKS_PER_SEC;

        // We accumulate time over a global variable to be used in
        // main thread as a progress bar
        atomic_store_explicit(&dataProgress, timeCounter / (2 * totalTime), memory_order_relaxed);
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

static void loadingBar()
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
    default:
        break;
    }

    int loadingHeight = height + 50;
    switch (state)
    {
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

static int autoType(const char *text, int current_frame, int max_frame_count, int x, int y, int font_size, Color color)
{
    int numChars = 0;
    // Calculate the number of characters to draw
    numChars = strlen(text) * current_frame / max_frame_count;
    // Draw the text
    char buffer[256];
    strncpy(buffer, text, numChars);
    buffer[numChars] = '\0'; // Null-terminate the string
    DrawText(buffer, x, y, font_size, color);
    return numChars;
}

bool once = false;
FILE *fp = NULL;
static bool inputScreen()
{
    GuiComboBox((Rectangle){screenWidth / 2 - 110, height, 250, 30}, algoOptions, &algoChoice);
    int buttonPressed = CustomButton((Rectangle){screenWidth / 2 - 110, height + ((algoChoice == 2) ? 190 : 110), 250, 30}, "Continue");
    if (GuiButton((Rectangle){screenWidth / 2 - 110, height + ((algoChoice == 2) ? 150 : 60), 250, 30}, "Choose File"))
    {
        // Open file dialog using zenity
        fp = popen("zenity --file-selection", "r");
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

    DrawText("Choose An Algorithm", screenWidth / 2 - 210, height - 100, 45, MAGENTA);
    if (algoChoice == 2)
    {
        // Spinner
        DrawText("Quantum", screenWidth / 2 - 110, height + 86, 15, MAGENTA); // Change the font size and color as needed
        GuiSpinner((Rectangle){screenWidth / 2 - 40, height + 80, 180, 30}, "", &quantum, 1, 10000, false);
    }
    if ((buttonPressed || once) && fp == NULL)
    {
        DrawText("Please select a file to continue", screenWidth / 2 - 60, height + ((algoChoice == 2) ? 225 : 150), 10, WHITE); // Change the font size and color as needed
        once = true;
        return false;
    }
    return buttonPressed;
}
void read_input_file()
{
    FILE *input_file;
    input_file = fopen(filePath, "r");
    if (!input_file)
    {
        printf("\nCould not open file processes.txt!!\n");
        exit(-1);
    }
    int cnt = 0;
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), input_file))
    {
        if (buffer[0] == '#')
            continue;
        int id, arrival, runtime, priority;
        sscanf(buffer, "%d %d %d %d", &id, &arrival, &runtime, &priority);
        totalTime += runtime + ((cnt == 1) ? arrival : 0);
        cnt++;
    }
    printf("Total Time: %d \n", totalTime);
    fclose(input_file);
}

int is_file_empty(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Failed to open file: %s\n", filename);
        return 1; // Return -1 to indicate an error
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fclose(file);

    return size == 0;
}

void get_cnt_of_log()
{
    while(is_file_empty("/home/marwan/mywork/Synergify/scheduler.perf"));
    FILE *file = fopen("/home/marwan/mywork/Synergify/scheduler.log", "r");
    if (file == NULL)
    {
        printf("Error opening file.\n");
        exit(1);
    }
    char buffer[256];
    while (number_of_logs == 0)
    {
        while (fgets(buffer, sizeof(buffer), file))
        {
            number_of_logs++;
        }
    }
    fclose(file);
}
void read_log_file()
{
    FILE *file = fopen("/home/marwan/mywork/Synergify/scheduler.log", "r");
    if (!file)
    {
        printf("Error opening file.\n");
        exit(1);
    }
    int idx = 0;
    processes = (process_block_t *)malloc(number_of_logs * sizeof(process_block_t));
    process_block_t block;
    char buffer[256];
    char status_buffer[20]; // Temporary buffer for reading status
    while (fgets(buffer, sizeof(buffer), file))
    {
        if (sscanf(buffer, "At time %d process %d %s arr %d total %d remain %d wait %d TA %d WTA %f",
                   &block.time, &block.id, status_buffer, &block.arrival, &block.runtime, &block.remaining_time, &block.waiting_time, &block.TA, &block.WTA) == 9)
        {
            strcpy(block.status, status_buffer); // Copy status from temporary buffer
        }
        else
        {
            sscanf(buffer, "At time %d process %d %s arr %d total %d remain %d wait %d",
                   &block.time, &block.id, status_buffer, &block.arrival, &block.runtime, &block.remaining_time, &block.waiting_time);
            block.TA = 0;
            block.WTA = 0.0f;
            strcpy(block.status, status_buffer); // Copy status from temporary buffer
        }
        processes[idx++] = block;
    }
    fclose(file);
}
void draw_block(int x, int y, process_block_t process)
{
    int distance = 70;
    Rectangle rec = {x, y, BLOCK_WIDTH, BLOCK_HEIGHT};
    float roundness = 0.2f;
    int segments = 5;
    int separate = 90;
    int space_after_line = 45;
    int space_after_text = 15;
    int text_size = 10;
    Color text_color = VIOLET;
    Color inside = CLITERAL(Color){167, 203, 254, 255};
    Color border = CLITERAL(Color){4, 79, 206, 255};
    Color main_text = CLITERAL(Color){97, 15, 108, 255};
    Color time_color = CLITERAL(Color){251, 90, 255, 255};
    Color Stopped = CLITERAL(Color){221, 98, 12, 255};
    Color Resumed = CLITERAL(Color){180, 153, 8, 255};
    Color status_color = strcmp(process.status, "started") == 0 ? DARKGREEN : strcmp(process.status, "finished") == 0 ? RED
                                                                          : strcmp(process.status, "resumed") == 0    ? Resumed
                                                                                                                      : Stopped;

    DrawRectangleRounded(rec, roundness, segments, inside);
    DrawRectangleRoundedLines(rec, roundness, segments, border);

    DrawText(TextFormat("Time: %d", process.time), x + 10, y - 20, 15, time_color);

    DrawText(TextFormat("Process %d", process.id), x + 10, y + 10, 15, main_text);
    DrawLine(x, y + 30, x + BLOCK_WIDTH, y + 30, border);

    DrawText(process.status, x + 100, y + 13, 10, status_color);

    DrawText(TextFormat("Arrival: %d", process.arrival), x + 10, y + space_after_line, text_size, text_color);
    DrawText(TextFormat("Total: %d", process.runtime), x + 10, y + space_after_line + space_after_text, text_size, text_color);
    DrawText(TextFormat("Remaining: %d", process.remaining_time), x + 10, y + space_after_line + 2 * space_after_text, text_size, text_color);

    DrawText(TextFormat("Waiting: %d", process.waiting_time), x + separate, y + space_after_line, text_size, text_color);
    if (strcmp(process.status, "finished") == 0)
    {
        DrawText(TextFormat("TA: %d", process.TA), x + separate, y + space_after_line + space_after_text, text_size, text_color);
        DrawText(TextFormat("WTA: %.2f", process.WTA), x + separate, y + space_after_line + 2 * space_after_text, text_size, text_color);
    }
}

void DrawHorizontalArrow(Vector2 startPos, float length, float arrowSize, int direction, Color color)
{
    Vector2 endPos = {startPos.x + direction * length, startPos.y};

    // Draw the horizontal line
    DrawLineV(startPos, endPos, color);

    // Draw the arrowhead
    Vector2 arrow1 = {endPos.x - direction * arrowSize, endPos.y - direction * arrowSize / 2};
    Vector2 arrow2 = {endPos.x - direction * arrowSize, endPos.y + direction * arrowSize / 2};
    DrawTriangle(endPos, arrow1, arrow2, color);
}

void DrawVerticalArrow(Vector2 startPos, float length, float arrowSize, int direction, Color color)
{
    Vector2 endPos = {startPos.x, startPos.y + direction * length};

    // Draw the vertical line
    DrawLineV(startPos, endPos, color);

    // Draw the arrowhead
    Vector2 arrow1 = {endPos.x + direction * arrowSize / 2, endPos.y - direction * arrowSize};
    Vector2 arrow2 = {endPos.x - direction * arrowSize / 2, endPos.y - direction * arrowSize};
    DrawTriangle(endPos, arrow1, arrow2, color);
}

void draw_line(int x, int y, int direction, int start_idx) // 1 to right -1 to left
{
    Color color = CLITERAL(Color){121, 171, 255, 255};
    Vector2 verticalStartPos = {200, 100};
    float lineLength = 50;
    float arrowSize = 10;
    for (int i = 0; i < 5; i++)
    {
        draw_block(x + direction * (i * 200 + ((direction == -1) ? BLOCK_WIDTH : 0)), y, processes[start_idx++]);
        if (start_idx >= number_of_logs)
            break;

        if (i < 4)
        {
            Vector2 startPos = {x + direction * (i * 200 + BLOCK_WIDTH), y + BLOCK_HEIGHT / 2};
            DrawHorizontalArrow(startPos, lineLength, arrowSize, direction, color);
        }
        else
        {
            Vector2 startPos = {x + direction * (i * 200 + BLOCK_WIDTH / 2), y + BLOCK_HEIGHT};
            DrawVerticalArrow(startPos, lineLength, arrowSize, 1, color);
        }
    }
}

void draw_image(int x, int y)
{
    int direction = 1;
    int start_x = x;
    for (int i = 0; i < number_of_logs; i += 5)
    {
        if (direction == 1)
            x = start_x;
        else
            x = screenWidth - start_x - 17;
        draw_line(x, y, direction, i);
        y += BLOCK_HEIGHT + 50;
        direction *= -1;
    }
    max_height = y;
}

void ouput_image(Texture2D background)
{
    int idx = 1;
    Vector2 offset = {0.0f, 0.0f};
    for (; offset.y > -1 * max_height; offset.y -= (screenHeight))
    {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawTexture(background, 0, 0, WHITE);
        BeginMode2D((Camera2D){.offset = offset, .target = (Vector2){0.0f, 0.0f}, .rotation = 0.0f, .zoom = 1.0f});
        draw_image(50, 50);
        EndMode2D();
        EndDrawing();
        TakeScreenshot(TextFormat("Scheduler_log_%d.png", idx++));
    }
}