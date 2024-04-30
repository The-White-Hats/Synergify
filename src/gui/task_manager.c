#include "gui.h"
#include "../ds/fib_heap.h"
#include "../ds/queue.h"
#include "../header.h"

gui_t gui;
static void *processes;

//================================= SIGNAL HANDLERS =================================//
//================================= INPUT HANDLERS ==================================//
//==================================== Draw GUI =====================================//
static void drawProcessesWithOrder(fib_heap_t *heap);
static void drawProcessesFromQueue(queue_t *queue);
//=================================== Clear DATA ====================================//

void initTaskManager(void *ready_queue)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    processes = ready_queue;
    gui.WIDTH = 1600;
    gui.SIDEBAR_WIDTH = 350;
    gui.HEIGHT = 900;
    gui.NAV_HEIGHT = 50;
    gui.background = (Color){25, 25, 25, 255};
    gui.process_sort = SORT_ID;
    InitWindow(gui.WIDTH, gui.HEIGHT, "Task Manager Simulator");

    // Load font
    const absolute_path[256];
    getAbsolutePath(absolute_path, "assets/fonts/Jersey15-Regular.ttf");
    gui.font = LoadFont(absolute_path);

    GUIPage *page = malloc(sizeof(GUIPage));
    initProcessesPage(page);

    SetTargetFPS(60);

    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
        // check input according to current page.
        checkMouseInput(page);

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
            DrawTextEx(gui.font, element->text, (Vector2){bounds.x + element->text_offset.x, bounds.y + element->text_offset.y}, element->font_size, 0, element->text_color);

        // Add more drawing commands as needed
    }

    // Draw Page Specific Functionalities
    switch (page->pageType)
    {
    case PAGE_PROCESSES:
        SchedulerConfig *schedulerConfig = getSchedulerConfigInstance();
        scheduling_algo selected_algo = schedulerConfig->selected_algorithm;
        if (selected_algo == RR)
            drawProcessesFromQueue((queue_t *)processes);
        else
            drawProcessesWithOrder((fib_heap_t *)processes);
        break;
    case PAGE_PERFORMANCE:
        break;
    default:
        break;
    }
}

static void drawProcess(PCB *element, int idx)
{
    int HEIGHT = 50, GAP = 10, font_size = 24;
    Color color = PRIMARY_COLOR;
    Rectangle bounds = {gui.SIDEBAR_WIDTH + 10, 150 + idx * HEIGHT + idx * GAP, gui.WIDTH - gui.SIDEBAR_WIDTH - 20, HEIGHT};
    DrawRectangleRounded(bounds, 0.2, 0, color);

    // Draw Process Info
    char str[10], name[20];

    int currentTime = getClk();

    // Draw Process ID
    sprintf(str, "%d", element->file_id);
    strcpy(name, "Process ");
    strcat(name, str);
    bounds.x += 30;
    DrawTextEx(gui.font, name, (Vector2){bounds.x, bounds.y + 10}, font_size, 0, WHITE);
    // Draw Process PID
    sprintf(str, "%d", element->fork_id);
    bounds.x += 170;
    DrawTextEx(gui.font, str, (Vector2){bounds.x, bounds.y + 10}, font_size, 0, WHITE);
    // Draw Process Arrival Time
    sprintf(str, "%d", element->arrival);
    bounds.x += 170;
    DrawTextEx(gui.font, str, (Vector2){bounds.x, bounds.y + 10}, font_size, 0, WHITE);
    // Draw Process Running Time
    sprintf(str, "%d", element->runtime);
    bounds.x += 170;
    DrawTextEx(gui.font, str, (Vector2){bounds.x, bounds.y + 10}, font_size, 0, WHITE);
    // Draw Process Remaining Time
    int value = (element->state == RUNNING ? 
        element->waiting_time : 
        element->waiting_time + currentTime - element->last_stop_time);
    sprintf(str, "%d", element->runtime - (currentTime - element->arrival - value));
    bounds.x += 170;
    DrawTextEx(gui.font, str, (Vector2){bounds.x, bounds.y + 10}, font_size, 0, WHITE);
    // Draw Process State
    bounds.x += 170;
    switch (element->state) {
        case RUNNING:
            DrawTextEx(gui.font, "RUNNING", (Vector2){bounds.x, bounds.y + 10}, font_size, 0, WHITE);
            break;
        case READY:
        default:
            DrawTextEx(gui.font, "READY", (Vector2){bounds.x, bounds.y + 10}, font_size, 0, WHITE);
            break;
    }
    // Draw Process Priority
    sprintf(str, "%d", element->priority);
    bounds.x += 170;
    DrawTextEx(gui.font, str, (Vector2){bounds.x, bounds.y + 10}, font_size, 0, WHITE);
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

static void drawProcessesWithOrder(fib_heap_t *heap)
{
    fib_heap_t *custom_heap = fib_heap_alloc();

    // Create custom ordered heap
    fib_heap_copy(heap, custom_heap, &selectKey);

    // Display custom ordered heap
    size_t i = 0;
    while (fib_heap_size(custom_heap) > 0 && i < 12)
    {
        drawProcess((PCB *)fib_heap_extract_min(custom_heap), i++);
    }
    fib_heap_free(custom_heap, 0);
}

static void drawProcessesFromQueue(queue_t *queue)
{
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

    // Display custom ordered heap
    size_t i = 0;
    while (fib_heap_size(custom_heap) > 0 && i < 12)
    {
        drawProcess((PCB *)fib_heap_extract_min(custom_heap), i++);
    }
    fib_heap_free(custom_heap, 0);
}

void clearPageResources(GUIPage *page)
{
    if (!page)
        return;
    if (page->elements != NULL)
        free(page->elements);
}