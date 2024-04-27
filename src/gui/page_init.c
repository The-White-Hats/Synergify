#include "gui.h"

#define SIDEBAR_ELEMENTS 4
#define HEADER_ELEMENTS 2

static void initSidebar(GUIPage *page, int *offset) {
  // Page is already initialized.
  Rectangle bounds = { 10, 100, gui.SIDEBAR_WIDTH - 20, 70 };
  int gap = 20;
  const char *text[] = {"Processes", "Performance", "Monitor"};
  void (*ptrInitPages[])(GUIPage *) = {initProcessesPage, initPerformancePage, initPerformancePage};

  // Initialize each GUI element
  // Initialize the background
  page->elements[*offset].bounds = (Rectangle){ 0, 0, gui.SIDEBAR_WIDTH, gui.HEIGHT };
  page->elements[*offset].type = ELEMENT_RECTANGLE;
  page->elements[*offset].color = PRIMARY_COLOR;
  page->elements[*offset].text = NULL;
  page->elements[*offset].active = false;
  page->elements[*offset].hover = false;
  page->elements[*offset].interactive = false;
  page->elements[*offset].onClick = NULL;

  // Initialize the navigation buttons
  for (int i = (*offset) + 1; i < SIDEBAR_ELEMENTS; i++) {
    page->elements[i].bounds = bounds;
    page->elements[i].type = ELEMENT_ROUNDED_RECTANGLE;
    page->elements[i].color = PRIMARY_COLOR;
    page->elements[i].text = text[i - 1];
    page->elements[i].text_color = WHITE;
    page->elements[i].text_offset = (Vector2){ 25, 25};
    page->elements[i].font_size = 26;
    page->elements[i].active = false;
    page->elements[i].hover = false;
    page->elements[i].interactive = true;
    page->elements[i].onClick = (void *)ptrInitPages[i - 1];

    bounds.y += bounds.height + gap;
  }

  *offset += SIDEBAR_ELEMENTS;
}

static void initHeader(GUIPage *page, int *offset, const char *header) {
  Rectangle bounds = { gui.SIDEBAR_WIDTH, 0, gui.WIDTH - gui.SIDEBAR_WIDTH, 100};

  // Initialize each GUI element
  // Initialize the background
  page->elements[*offset].bounds = bounds;
  page->elements[*offset].type = ELEMENT_RECTANGLE;
  page->elements[*offset].color = BACKGROUND_COLOR;
  page->elements[*offset].text = header;
  page->elements[*offset].text_color = WHITE;
  page->elements[*offset].text_offset = (Vector2){ 25, 40};
  page->elements[*offset].font_size = 30;
  page->elements[*offset].active = false;
  page->elements[*offset].hover = false;
  page->elements[*offset].interactive = false;
  page->elements[*offset].onClick = NULL;

  bounds.y = 100;
  bounds.height = 1;
  page->elements[*offset + 1].bounds = bounds;
  page->elements[*offset + 1].type = ELEMENT_LINE;
  page->elements[*offset + 1].color = SECONDARY_COLOR;
  page->elements[*offset + 1].text = NULL;
  page->elements[*offset + 1].active = false;
  page->elements[*offset + 1].hover = false;
  page->elements[*offset + 1].interactive = false;
  page->elements[*offset + 1].onClick = NULL;

  *offset += HEADER_ELEMENTS;

}

static void orderByID(GUIPage *page) {
  (void *)page;
  gui.process_sort = SORT_ID;
  int offset = SIDEBAR_ELEMENTS + HEADER_ELEMENTS;
  for (int i = offset; i < offset + SORT_COUNT; i++)
    page->elements[i].color = BACKGROUND_COLOR;
  page->elements[offset].color = PRIMARY_COLOR;
}

static void orderByPID(GUIPage *page) {
  (void *)page;
  gui.process_sort = SORT_PID;
  int offset = SIDEBAR_ELEMENTS + HEADER_ELEMENTS;
  for (int i = offset; i < offset + SORT_COUNT; i++)
    page->elements[i].color = BACKGROUND_COLOR;
  page->elements[offset + 1].color = PRIMARY_COLOR;
}

static void orderByArrival(GUIPage *page) {
  (void *)page;
  gui.process_sort = SORT_ARRIVAL_TIME;
  int offset = SIDEBAR_ELEMENTS + HEADER_ELEMENTS;
  for (int i = offset; i < offset + SORT_COUNT; i++)
    page->elements[i].color = BACKGROUND_COLOR;
  page->elements[offset + 2].color = PRIMARY_COLOR;
}

static void orderByRunning(GUIPage *page) {
  (void *)page;
  gui.process_sort = SORT_RUNNING_TIME;
  int offset = SIDEBAR_ELEMENTS + HEADER_ELEMENTS;
  for (int i = offset; i < offset + SORT_COUNT; i++)
    page->elements[i].color = BACKGROUND_COLOR;
  page->elements[offset + 3].color = PRIMARY_COLOR;
}

static void orderByRemainingTime(GUIPage *page) {
  (void *)page;
  gui.process_sort = SORT_REMAINING_TIME;
  int offset = SIDEBAR_ELEMENTS + HEADER_ELEMENTS;
  for (int i = offset; i < offset + SORT_COUNT; i++)
    page->elements[i].color = BACKGROUND_COLOR;
  page->elements[offset + 4].color = PRIMARY_COLOR;
}

static void orderByState(GUIPage *page) {
  (void *)page;
  gui.process_sort = SORT_STATE;
  int offset = SIDEBAR_ELEMENTS + HEADER_ELEMENTS;
  for (int i = offset; i < offset + SORT_COUNT; i++)
    page->elements[i].color = BACKGROUND_COLOR;
  page->elements[offset + 5].color = PRIMARY_COLOR;
}

static void orderByPriority(GUIPage *page) {
  (void *)page;
  gui.process_sort = SORT_PRIORITY;
  int offset = SIDEBAR_ELEMENTS + HEADER_ELEMENTS;
  for (int i = offset; i < offset + SORT_COUNT; i++)
    page->elements[i].color = BACKGROUND_COLOR;
  page->elements[offset + 6].color = PRIMARY_COLOR;
}

void initProcessesPage(GUIPage *page) {
  if (page->pageType == PAGE_PROCESSES)  return;
  int idx = 0;

  clearPageResources(page);
  page->pageType = PAGE_PROCESSES;
  page->num_of_elements = SIDEBAR_ELEMENTS + HEADER_ELEMENTS + SORT_COUNT + 0;

  page->elements = malloc(sizeof(GUIElement)*page->num_of_elements);

  // Initialize each GUI element
  // 1. Create Sidebar
  initSidebar(page, &idx);
  page->elements[1].color = SECONDARY_COLOR;

  // 2. Create Main
  initHeader(page, &idx, "Processes");

  // 3. Create table headers
  void (*ptrOrderBy[SORT_COUNT])(GUIPage *) = {orderByID, orderByPID, orderByArrival, orderByRunning, orderByRemainingTime, orderByState, orderByPriority};
  const char *headerNames[SORT_COUNT] = { "ID", "PID", "Arrival Time", "Running Time", "Remaining Time", "State", "Priority" };
  Rectangle bounds = { gui.SIDEBAR_WIDTH + 20, 105, 200, 40 };
  for (int i = 0; i < SORT_COUNT; i++, idx++) {
    page->elements[idx].bounds = bounds;
    page->elements[idx].type = ELEMENT_RECTANGLE;
    page->elements[idx].color = BACKGROUND_COLOR;
    page->elements[idx].text = headerNames[i];
    page->elements[idx].text_color = WHITE;
    page->elements[idx].text_offset = (Vector2){ 20, 15};
    page->elements[idx].font_size = 20;
    page->elements[idx].active = false;
    page->elements[idx].hover = false;
    page->elements[idx].interactive = true;
    page->elements[idx].onClick = (void *)ptrOrderBy[i];

    bounds.x += 170;
  }
  orderByID(page);
  
}

void initPerformancePage(GUIPage *page) {
  if (page->pageType == PAGE_PERFORMANCE) return;
  int idx = 0;
  clearPageResources(page);
  page->pageType = PAGE_PERFORMANCE;
  page->num_of_elements = SIDEBAR_ELEMENTS + HEADER_ELEMENTS + 0;

  page->elements = malloc(sizeof(GUIElement)*page->num_of_elements);

  // Initialize each GUI element
  // 1. Create Sidebar
  initSidebar(page, &idx);
  page->elements[2].color = SECONDARY_COLOR;

  // 2. Create Main
  initHeader(page, &idx, "Performance");

}