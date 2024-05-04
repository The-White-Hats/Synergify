#pragma once
#include <stdio.h>
#include <stdlib.h>
#include "raylib.h"
#include "raymath.h"

#define BACKGROUND_COLOR (Color){ 8, 2, 3, 255 }
#define PRIMARY_COLOR (Color){ 116, 56, 194, 255 }
#define SECONDARY_COLOR (Color){ 66, 31, 112, 255 }
#define ACCENT_COLOR (Color) { 190, 140, 255, 255 }
#define TEXT_COLOR (Color){ 247, 245, 250, 255 }
#define TEXT_LIGHT_COLOR (Color){ 251, 250, 253, 255 }

typedef enum {
  PAGE_WELCOME,
  PAGE_PROCESSES,
  PAGE_PERFORMANCE,
  PAGE_MONITOR,
  PAGE_OUTPUT,
  PAGE_COUNT
} PAGE;

typedef enum {
  SORT_ID,
  SORT_PID,
  SORT_ARRIVAL_TIME,
  SORT_RUNNING_TIME,
  SORT_REMAINING_TIME,
  SORT_STATE,
  SORT_PRIORITY,
  SORT_COUNT
} TABLE_SORT;

typedef struct {
  int WIDTH, SIDEBAR_WIDTH;
  int HEIGHT, NAV_HEIGHT;
  Color background;
  PAGE page;
  Font font;
  TABLE_SORT process_sort;
} gui_t;

typedef enum {
  ELEMENT_IMAGE,
  ELEMENT_RECTANGLE,
  ELEMENT_ROUNDED_RECTANGLE,
  ELEMENT_CIRCLE,
  ELEMENT_TEXT,
  ELEMENT_LINE,
  ELEMENT_COUNT
    // Add more properties as needed
} ElementType;

typedef struct {
  Rectangle bounds;
  ElementType type;
  const char *text;
  Vector2 text_offset;
  Color color, text_color;
  int font_size;

  bool interactive;
  bool hover;
  bool active;
  void *onClick;
    
    // Add more properties as needed
} GUIElement;

typedef struct {
    PAGE pageType;
    GUIElement *elements; // Dynamic array of GUI elements for this page
    int num_of_elements; // Number of elements in the array
} GUIPage;

extern gui_t gui;

void initTaskManager(void *ready_queue);

void initProcessesPage(GUIPage *page);

void initPerformancePage(GUIPage *page);

void initMonitorPage(GUIPage *page);

void clearPageResources(GUIPage *page);

void checkMouseInput(GUIPage *page);

void drawPage(GUIPage *page);