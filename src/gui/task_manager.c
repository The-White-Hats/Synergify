#include "gui.h"
#include "../ds/fib_heap.h"
#include "../ds/queue.h"
#include "../header.h"

#define GRAPH_SIZE 8
#define GRAPH_PADDING 3
#define PLOT_GRAPH_SIZE 17
#define PLOT_PADDING 5

gui_t gui;
static void *processes;

//================================= SIGNAL HANDLERS =================================//
//================================= INPUT HANDLERS ==================================//
//============================= TASK MANAGER FUNCTIONS ==============================//
static fib_heap_t *copyProcessesFromReadyQUeue();
static fib_heap_t *copyProcessesWithOrder(fib_heap_t *heap);
static fib_heap_t *copyProcessesFromQueue(queue_t *queue);
static int selectKey(void *data);
static void updateData();
//==================================== Draw GUI =====================================//
static void drawProcessesWithOrder(fib_heap_t *custom_heap);
static void drawProcess(PCB *custom_heap, int idx, int offsetY);
static void drawPerformanceGraphs(fib_heap_t *custom_heap);
//=================================== Clear DATA ====================================//

//================================ Global Variables =================================//
int processes_num[GRAPH_SIZE] = {[0 ... GRAPH_SIZE - 1] = -1 };
int cpu_state[GRAPH_SIZE] = {[0 ... GRAPH_SIZE - 1] = -1};
int cpu_util[PLOT_GRAPH_SIZE] = {[0 ... PLOT_GRAPH_SIZE - 1] = -1 };
static int graph_idx = 0, plot_idx = 0;

void initTaskManager(void *ready_queue)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    processes = ready_queue;
    gui.WIDTH = 1600;
    gui.SIDEBAR_WIDTH = 350;
    gui.HEIGHT = 900;
    gui.NAV_HEIGHT = 50;
    gui.background = BACKGROUND_COLOR;
    gui.process_sort = SORT_ID;
    InitWindow(gui.WIDTH, gui.HEIGHT, "Task Manager Simulator");

    // Load font
    const absolute_path[256];
    getAbsolutePath(absolute_path, "assets/fonts/Jersey15-Regular.ttf");
    gui.font = LoadFont(absolute_path);

    GUIPage *page = malloc(sizeof(GUIPage));
    initProcessesPage(page);

    SetTargetFPS(60);

    int prev_time = -1;

    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
        // check input according to current page.
        checkMouseInput(page);

        int current_time = getClk();
        if (current_time != prev_time) {
            prev_time = current_time;
            updateData();
        }


        BeginDrawing();
        // draw the current page.
        drawPage(page);

        EndDrawing();
    }

    // De-Initialization
    clearPageResources(page);
    free(page);
    //--------------------------------------------------------------------------------------
    CloseWindow(); // Close window and OpenGL context
    //--------------------------------------------------------------------------------------
}

void checkMouseInput(GUIPage *page)
{
    Vector2 mousePosition;
    bool clicked, collided = false;
    if (!page)
        return;
    // Check click
    mousePosition = GetMousePosition();
    clicked = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    for (int i = 0; i < page->num_of_elements; i++)
    {
        // Check each GUI element based on its properties
        GUIElement *element = &(page->elements[i]);
        if (!element || !(element->interactive))
            continue;
        switch (element->type)
        {
        case ELEMENT_IMAGE:
            break;
        case ELEMENT_CIRCLE:
            break;
        case ELEMENT_RECTANGLE:
        case ELEMENT_ROUNDED_RECTANGLE:
            collided = CheckCollisionPointRec(mousePosition, element->bounds);
            break;
        case ELEMENT_TEXT:
            break;
        default:
            break;
        }
        if (collided)
        {
            if (clicked)
            {
                void (*onClickFunc)(GUIPage *) = (void (*)(GUIPage *))element->onClick;
                return onClickFunc(page);
            }
            element->hover = true;
            break;
        }
        else if (element->hover)
            element->hover = false;
        // Add more checking commands as needed
    }

    // Change mouse style to a pointer for better User Experience
    SetMouseCursor((collided ? MOUSE_CURSOR_POINTING_HAND : MOUSE_CURSOR_DEFAULT));
}

static int selectKey(void *data)
{
    PCB *element = (PCB *)data;
    switch (gui.process_sort)
    {
    case SORT_RUNNING_TIME:
        return element->runtime;
    case SORT_ARRIVAL_TIME:
        return element->arrival;
    case SORT_STATE:
        return element->state;
    case SORT_PID:
        return element->fork_id;
    case SORT_ID:
        return element->file_id;
    case SORT_PRIORITY:
        return element->priority;
    case SORT_REMAINING_TIME:
        int currentTime = getClk();
        int value = (element->state == RUNNING ? 
        element->waiting_time : 
        element->waiting_time + currentTime - element->last_stop_time);
        return element->runtime - (currentTime - element->arrival - value);
    default:
        break;
    }
    return 0;
}

static fib_heap_t *copyProcessesFromReadyQUeue() {
    SchedulerConfig *schedulerConfig = getSchedulerConfigInstance();
    scheduling_algo selected_algo = schedulerConfig->selected_algorithm;
    if (selected_algo == RR)
        return copyProcessesFromQueue((queue_t *)processes);
    else
        return copyProcessesWithOrder((fib_heap_t *)processes);

}


static fib_heap_t *copyProcessesWithOrder(fib_heap_t *heap) {
    fib_heap_t *custom_heap = fib_heap_alloc();

    // Create custom ordered heap
    fib_heap_copy(heap, custom_heap, &selectKey);

    return custom_heap;
}

static fib_heap_t *copyProcessesFromQueue(queue_t *queue) {
    queue_t *temp_queue = create_queue();
    fib_heap_t *custom_heap = fib_heap_alloc();

    // Create custom ordered heap
    queue_copy(queue, temp_queue);

    // Create custom ordered heap
    while (!is_queue_empty(temp_queue))
    {
        void *element = dequeue(temp_queue);
        fib_heap_insert(custom_heap, element, selectKey(element));
    }
    queue_free(temp_queue, 0);
    return custom_heap;
}

void drawPage(GUIPage *page)
{
    if (!page)
        return;
    // Draw background, etc.
    ClearBackground(gui.background);
    // Draw GUI elements
    for (int i = 0; i < page->num_of_elements; i++)
    {
        // Draw each GUI element based on its properties
        GUIElement *element = &(page->elements[i]);
        Color color = element->color;
        Rectangle bounds = element->bounds;
        color = element->hover ? SECONDARY_COLOR : color;
        switch (element->type)
        {
        case ELEMENT_IMAGE:
            break;
        case ELEMENT_CIRCLE:
            break;
        case ELEMENT_RECTANGLE:
            DrawRectangleRec(bounds, color);
            break;
        case ELEMENT_ROUNDED_RECTANGLE:
            DrawRectangleRounded(bounds, 0.2, 0, color);
            break;
        case ELEMENT_TEXT:
            break;
        case ELEMENT_LINE:
            DrawLine(bounds.x, bounds.y, bounds.x + bounds.width, bounds.y + bounds.height, color);
            break;
        default:
            break;
        }
        if (element->text != NULL)
            DrawTextEx(gui.font, element->text, 
                    (Vector2){bounds.x + element->text_offset.x, bounds.y + element->text_offset.y},
                    element->font_size, 0,
                    TEXT_COLOR);

        // Add more drawing commands as needed
    }

    // Draw time step
    char timeStep[6];
    sprintf(timeStep, "%d", getClk());
    DrawTextEx(gui.font, timeStep, (Vector2){gui.WIDTH - 100, 40}, 30, 0, TEXT_COLOR);

    // Draw Page Specific Functionalities
    fib_heap_t *custom_heap = copyProcessesFromReadyQUeue();
    switch (page->pageType)
    {
    case PAGE_PROCESSES:
        drawProcessesWithOrder(custom_heap);
        break;
    case PAGE_PERFORMANCE:
        drawPerformanceGraphs(custom_heap);
        break;
    default:
        break;
    }
}

static void drawProcess(PCB *element, int idx, int offsetY)
{
    int HEIGHT = 60, GAP = 10, font_size = 24, textOffsetY = 18, textOffsetX = 170;
    Color color = PRIMARY_COLOR;
    Rectangle bounds = { gui.SIDEBAR_WIDTH + 16, 160 + idx * HEIGHT + idx * GAP - offsetY, gui.WIDTH - gui.SIDEBAR_WIDTH - 32, HEIGHT};
    DrawRectangleRounded(bounds, 0.2, 2, color);

    // Draw Process Info
    char str[10], name[20];

    int currentTime = getClk();

    // Draw Process ID
    sprintf(str, "%d", element->file_id);
    strcpy(name, "Process ");
    strcat(name, str);
    bounds.x += 30;
    DrawTextEx(gui.font, name, (Vector2){bounds.x, bounds.y + textOffsetY}, font_size, 0, TEXT_COLOR);
    // Draw Process PID
    sprintf(str, "%d", element->fork_id);
    bounds.x += textOffsetX;
    DrawTextEx(gui.font, str, (Vector2){bounds.x, bounds.y + textOffsetY}, font_size, 0, TEXT_COLOR);
    // Draw Process Arrival Time
    sprintf(str, "%d", element->arrival);
    bounds.x += textOffsetX;
    DrawTextEx(gui.font, str, (Vector2){bounds.x, bounds.y + textOffsetY}, font_size, 0, TEXT_COLOR);
    // Draw Process Running Time
    sprintf(str, "%d", element->runtime);
    bounds.x += textOffsetX;
    DrawTextEx(gui.font, str, (Vector2){bounds.x, bounds.y + textOffsetY}, font_size, 0, TEXT_COLOR);
    // Draw Process Remaining Time
    int value = (element->state == RUNNING ? 
        element->waiting_time : 
        element->waiting_time + currentTime - element->last_stop_time);
    sprintf(str, "%d", element->runtime - (currentTime - element->arrival - value));
    bounds.x += textOffsetX;
    DrawTextEx(gui.font, str, (Vector2){bounds.x, bounds.y + textOffsetY}, font_size, 0, TEXT_COLOR);
    // Draw Process State
    bounds.x += textOffsetX;
    switch (element->state) {
        case RUNNING:
            DrawTextEx(gui.font, "RUNNING", (Vector2){bounds.x, bounds.y + textOffsetY}, font_size, 0, TEXT_COLOR);
            break;
        case READY:
        default:
            DrawTextEx(gui.font, "READY", (Vector2){bounds.x, bounds.y + textOffsetY}, font_size, 0, TEXT_COLOR);
            break;
    }
    // Draw Process Priority
    sprintf(str, "%d", element->priority);
    bounds.x += textOffsetX;
    DrawTextEx(gui.font, str, (Vector2){bounds.x, bounds.y + textOffsetY}, font_size, 0, TEXT_COLOR);
}

static void drawProcessesWithOrder(fib_heap_t *custom_heap)
{
    static int scrollY = 0;
    BeginScissorMode(gui.SIDEBAR_WIDTH, 150, gui.WIDTH - gui.SIDEBAR_WIDTH, gui.HEIGHT - 180);
    // Display custom ordered heap
    size_t i = 0;
    while (fib_heap_size(custom_heap) > 0)
    {
        drawProcess((PCB *)fib_heap_extract_min(custom_heap), i++, scrollY);
    }
    scrollY += GetMouseWheelMove() * 20;
    scrollY = Clamp(scrollY, 0, i * 70);
    EndScissorMode();
    fib_heap_free(custom_heap, 0);
}

static void updateData() {
    static int cpu_runtime = 0;
    int current_time = getClk();
    fib_heap_t *custom_heap = copyProcessesFromReadyQUeue();

    // Calculate performance
    int num_of_processes = fib_heap_size(custom_heap);
    bool idle = true;
    while (fib_heap_size(custom_heap) > 0)
    {
        const PCB *element = fib_heap_extract_min(custom_heap);
        if (element->state == RUNNING) idle = false;
    }
    cpu_runtime += (!idle ? 1 : 0);

    // Update graphs stats
    processes_num[graph_idx] = num_of_processes;
    cpu_state[graph_idx] = (!idle ? 1 : 0);
    cpu_util[plot_idx] = (current_time == 0 ? 0 : cpu_runtime * 100 / current_time);
    // Update indexes
    graph_idx = (graph_idx + 1) % GRAPH_SIZE;
    plot_idx = (plot_idx + 1) % PLOT_GRAPH_SIZE;
    
    fib_heap_free(custom_heap, 0);
}

static void drawGraph(int values[GRAPH_SIZE], int originX, int originY, int mx) {
    int x_plot[GRAPH_SIZE];
    int j = 0;
    int i = graph_idx;

    do {
        if (values[i] != -1) {
            if (values[i] > mx) mx = values[i];
            x_plot[j++] = values[i];
        }
        i = (i + 1) % GRAPH_SIZE;
    } while(i != graph_idx);
    // Draw axis
    int width = 30, gap = 12;
    int axisYLength = 270, axisXLength = (width + gap) * (GRAPH_SIZE + GRAPH_PADDING);
    DrawLine(originX, originY, originX, originY - axisYLength - 10, TEXT_COLOR);
    DrawLine(originX, originY, originX + axisXLength, originY, TEXT_COLOR);

    // Draw y plots
    char str[6];
    sprintf(str, "%d", mx);
    DrawTextEx(gui.font, str, (Vector2){originX - 30, originY - axisYLength - 5}, 20, 0, TEXT_COLOR);
    DrawLine(originX, originY - axisYLength, originX - 4, originY - axisYLength, TEXT_COLOR);

    // Draw x plots
    int current_time = getClk() - j + 1;
    for (int i = 0, xOffset = originX; i < (GRAPH_SIZE + GRAPH_PADDING); i++) {
        // Draw time
        sprintf(str, "%d", current_time);
        DrawTextEx(gui.font, str, (Vector2){xOffset + 5, originY + 10}, 20, 0, TEXT_COLOR);

        current_time++;
        xOffset += width + gap;
    }

    // Draw Graph
    Rectangle bounds = {originX, originY, width, 0};
    Color temp = PRIMARY_COLOR;
    for (int i = 0; i < j; i++) {
        // Draw rectangle
        int height = x_plot[i] * axisYLength / mx;
        bounds.y = originY - height;
        bounds.height = height;
        if (i == j - 1) temp = ACCENT_COLOR;
        DrawRectangleRounded(bounds, 0.2, 0, temp);
        bounds.height -= height / 2;
        bounds.y += height / 2;
        DrawRectangleRec(bounds, temp);

        bounds.x += width + gap;
    }

}

static void drawPlotGraph(int values[PLOT_GRAPH_SIZE], int originX, int originY) {
    // Draw axis
    int radius = 5, gap = 50;
    int axisYLength = 270, axisXLength = (radius + gap) * (PLOT_GRAPH_SIZE + PLOT_PADDING);
    DrawLine(originX, originY, originX, originY - axisYLength - 10, TEXT_COLOR);
    DrawLine(originX, originY, originX + axisXLength, originY, TEXT_COLOR);

    // Draw y plots
    DrawTextEx(gui.font, "100", (Vector2){originX - 30, originY - axisYLength - 5}, 20, 0, TEXT_COLOR);
    DrawLine(originX, originY - axisYLength, originX - 4, originY - axisYLength, TEXT_COLOR);

    int x_plot[PLOT_GRAPH_SIZE];
    int j = 0;
    int i = plot_idx;

    do {
        if (values[i] != -1) {
            x_plot[j++] = (values[i] * axisYLength / 100);
        }
        i = (i + 1) % PLOT_GRAPH_SIZE;
    } while(i != plot_idx);

    // Draw x plots
    char str[6];
    int current_time = getClk() - j + 1;
    for (int i = 0, xOffset = originX; i < (PLOT_GRAPH_SIZE + PLOT_PADDING); i++) {
        // Draw time
        sprintf(str, "%d", current_time);
        DrawTextEx(gui.font, str, (Vector2){xOffset, originY + 10}, 20, 0, TEXT_COLOR);

        current_time++;
        xOffset += radius + gap;
    }

    // Draw Graph
    Color temp = PRIMARY_COLOR;
    for (int i = 0, xOffset = originX; i < j; i++) {
        // Draw circle
        if (i == j - 1) temp = ACCENT_COLOR;
        DrawCircle(xOffset, originY - x_plot[i], radius, temp);

        // Draw line
        if (i < j - 1) {
            DrawLine(xOffset, originY - x_plot[i], xOffset + radius + gap, originY - x_plot[i + 1], PRIMARY_COLOR);
        }

        xOffset += radius + gap;
    }

}

static void drawPerformanceGraphs(fib_heap_t *custom_heap) {
    DrawTextEx(gui.font, "Process Number", (Vector2){gui.SIDEBAR_WIDTH + 50, gui.NAV_HEIGHT + 80}, 20, 0, TEXT_COLOR);
    drawGraph(processes_num, gui.SIDEBAR_WIDTH + 50, gui.NAV_HEIGHT + 400, 5);
    DrawTextEx(gui.font, "CPU State", (Vector2){gui.SIDEBAR_WIDTH + 700, gui.NAV_HEIGHT + 80}, 20, 0, TEXT_COLOR);
    drawGraph(cpu_state, gui.SIDEBAR_WIDTH + 700, gui.NAV_HEIGHT + 400, 1);
    DrawTextEx(gui.font, "CPU Utilization", (Vector2){gui.SIDEBAR_WIDTH + 50, gui.NAV_HEIGHT + 460}, 20, 0, TEXT_COLOR);
    drawPlotGraph(cpu_util, gui.SIDEBAR_WIDTH + 50, gui.HEIGHT - 80);
}

void clearPageResources(GUIPage *page)
{
    if (!page)
        return;
    if (page->elements != NULL)
        free(page->elements);
}