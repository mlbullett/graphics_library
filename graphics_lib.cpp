#include <GL/gl.h>
#include <GL/glut.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <deque>
#include <math.h>
#include <ctime>
#include <cmath>
#include <algorithm>
#include <set>

//Defines
// min max functions
#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))

// Typedefs
// rgb color struct
typedef struct color
{
  int32_t red;
  int32_t green;
  int32_t blue;
};

// Image coordinate types
typedef std::pair<int, int> TImageCoordPair;
typedef std::deque<TImageCoordPair> TImageCoordList;

// Struct for filled polygon edges
typedef struct Edge
{
  int yMax;
  float dx, slope;
  struct Edge *next;
} Edge;

// Test params
int winw = 1000;
int winh = 1000;
int alphaChannel1 = 255;
int alphaChannel2 = 128;
color pixelColor1 = {255, 255, 255};
color pixelColor2 = {255, 0, 0};
int lineWidth = 1;
uint32_t linePattern = 0xFFF00FFFU;
std::deque<uint32_t> fillPattern = {0x00000000U, 0x00F00F00U, 0x00F00F00U, 0x00F00F00U, 0x00F00F00U, 0x0FFFFFF0U, 0x0FFFFFF0U, 0x0FFFFFF0U, 0x0FFFFFF0U, 0x0FFFFFF0U, 0x0FFFFFF0U, 0x0FFFFFF0U, 0x0FFFFFF0U, 0x00FFFF00U, 0x00FFFF00U, 0x00FFFF00U, 0x00FFFF00U, 0x000FF000U, 0x000FF000U, 0x000FF000U, 0x000FF000U, 0x000FF000U, 0x000FF000U, 0x00000000U};

// Used to rotate through pattern and return pixel flag
int GetAndRotatePixelFlag(uint32_t *pattern)
{
  int rc = *pattern & 1;
  *pattern >>= 1;

  if (rc)
    *pattern |= 0x80000000UL;
  else
    *pattern &= 0x7FFFFFFFUL;

  return rc;
}

// Interface to get canvas size
int GetCanvasSize(int *x, int *y)
{
  if (winw > 0 && winh > 0)
  {
    *x = winw;
    *y = winh;
    return 0;
  }

  *x = *y = -1;
  return -1;
}

// Interface to draw pixels
void DrawPixel(int x, int y, color col = pixelColor1, int alpha = alphaChannel1)
{
  glBegin(GL_POINTS);
  glColor4ub(col.red, col.green, col.blue, alpha);
  glVertex2i(x, y);
  glEnd();
}

// Subprocess that draws a basic 1px thick line using addition fixed point with precalculations implementation of EFLA
void DrawBasicLine(int x1, int y1, int x2, int y2, int omitEndpoints, uint32_t pattern = 0xFFFFFFFFU)
{
  bool yLonger = false;
  int shortLen = y2 - y1;
  int longLen = x2 - x1;
  int pixelFlag;

  if (abs(shortLen) > abs(longLen))
  {
    int swap = shortLen;
    shortLen = longLen;
    longLen = swap;
    yLonger = true;
  }

  // Precalculation of incremental step
  int decInc = longLen == 0 ? 0 : (shortLen << 16) / longLen;

  // Handling if line is taller than wide
  if (yLonger)
  {
    // Handling for right to left direction
    if (longLen > 0)
    {
      longLen += y1;
      for (int j = 0x8000 + (x1 << 16); y1 <= longLen; ++y1)
      {
        pixelFlag = GetAndRotatePixelFlag(&pattern);
        if (pixelFlag)
        {
          DrawPixel(j >> 16, y1, pixelColor1, alphaChannel1);
        }
        j += decInc;
      }
      return;
    }

    // Handling for left to right direction
    longLen += y1;
    for (int j = 0x8000 + (x1 << 16); y1 >= longLen; --y1)
    {
      pixelFlag = GetAndRotatePixelFlag(&pattern);
      if (pixelFlag)
      {
        DrawPixel(j >> 16, y1, pixelColor1, alphaChannel1);
      }
      j -= decInc;
    }
    return;
  }

  // Handling for right to left direction
  if (longLen > 0)
  {
    longLen += x1;
    for (int j = 0x8000 + (y1 << 16); x1 <= longLen; ++x1)
    {
      pixelFlag = GetAndRotatePixelFlag(&pattern);
      if (pixelFlag)
      {
        DrawPixel(x1, j >> 16, pixelColor1, alphaChannel1);
      }
      j += decInc;
    }
    return;
  }

  // Handling for left to right direction
  longLen += x1;
  for (int j = 0x8000 + (y1 << 16); x1 >= longLen; --x1)
  {
    pixelFlag = GetAndRotatePixelFlag(&pattern);
    if (pixelFlag)
    {
      DrawPixel(x1, j >> 16, pixelColor1, alphaChannel1);
    }
    j -= decInc;
  }
}

// Interface for drawing lines
void DrawLine(int x1, int y1, int x2, int y2, int omitEndpoints = 0)
{
  int dx = x2 - x1;
  int dy = y2 - y1;

  // horizontal line
  if (dx != 0 && dy == 0)
  {
    // Handling line width
    for (int i = 1; i <= lineWidth; i++)
    {
      // Odd goes under
      if (i & 1)
        DrawBasicLine(x1, y1 - ((i - 1) >> 1), x2, y2 - ((i - 1) >> 1), omitEndpoints, linePattern);
      // Even goes over
      else
        DrawBasicLine(x1, y1 + (i >> 1), x2, y2 + (i >> 1), omitEndpoints, linePattern);
    }
  }
  // vertical line
  else if (dx == 0 && dy != 0)
  {
    // Handling line width
    for (int i = 1; i <= lineWidth; i++)
    {
      // Odd goes under
      if (i & 1)
      {
        // DrawBasicLine(x1 + (i/2), y1, x2 + (i/2), y2, omitEndpoints, linePattern);
        DrawBasicLine(x1 - ((i - 1) >> 1), y1, x2 - ((i - 1) >> 1), y2, omitEndpoints, linePattern);
      }
      // Even goes over
      else
      {
        // DrawBasicLine(x1 - ((i - 1)/2), y1, x2 - ((i - 1)/2), y2, omitEndpoints, linePattern);
        DrawBasicLine(x1 + (i >> 1), y1, x2 + (i >> 1), y2, omitEndpoints, linePattern);
      }
    }
  }
  // Handling left to right direction
  else if (dx > 0)
  {
    // Handling top to bottom direction
    if (dy > 0)
    {
      // Handling line width
      for (int i = 1; i <= lineWidth; i++)
      {
        // Odd goes under
        if (i & 1)
          DrawBasicLine(x1, y1 + ((i - 1) >> 1), x2 - ((i - 1) >> 1), y2, omitEndpoints, linePattern);
        // Even goes over
        else
          DrawBasicLine(x1 + (i >> 1), y1, x2, y2 - (i >> 1), omitEndpoints, linePattern);
      }
    }
    // Handling bottom to top direction
    else
    {
      // Handling line width
      for (int i = 1; i <= lineWidth; i++)
      {
        // Odd goes under
        if (i & 1)
          DrawBasicLine(x1, y1 - ((i - 1) >> 1), x2 - ((i - 1) >> 1), y2, omitEndpoints, linePattern);
        // Even goes over
        else
          DrawBasicLine(x1 + (i >> 1), y1, x2, y2 + (i >> 1), omitEndpoints, linePattern);
      }
    }
  }
  // Handling right to left direction
  else
  {
    // Handling top to bottom direction
    if (dy > 0)
    {
      // Handling line width
      for (int i = 1; i <= lineWidth; i++)
      {
        // Odd goes under
        if (i & 1)
          DrawBasicLine(x1 - ((i - 1) >> 1), y1, x2, y2 - ((i - 1) >> 1), omitEndpoints, linePattern);
        // Even goes over
        else
          DrawBasicLine(x1, y1 + (i >> 1), x2 + (i >> 1), y2, omitEndpoints, linePattern);
      }
    }
    // Handling bottom to top direction
    else
    {
      // Check in case of point
      if (dy != 0)
      {
        // Handling line width
        for (int i = 1; i <= lineWidth; i++)
        {
          // Odd goes under
          if (i & 1)
            DrawBasicLine(x1 - ((i - 1) >> 1), y1, x2, y2 + ((i - 1) >> 1), omitEndpoints, linePattern);
          // Even goes over
          else
            DrawBasicLine(x1, y1 - (i >> 1), x2 + (i >> 1), y2, omitEndpoints, linePattern);
        }
      }
    }
  }
}

// Interface to draw empty rectangles
void DrawRect(int x1, int y1, int x2, int y2)
{
  // Top side
  DrawLine(x1, y1, x2, y1, 1);
  // Right side w/ hacky corner pixel fix
  DrawLine(x2, y1, x2, y2 + 1, 0);
  // Left side
  DrawLine(x1, y1, x1, y2, 0);
  // Bottom side w/ hacky corner pixel fix
  DrawLine(x1, y2, x2 + 1, y2, 1);
}

// Interface to draw filled rectangles
void DrawBox(int x1, int y1, int x2, int y2)
{
  // Correction for width of line
  int widthCor = (lineWidth >> 1);
  int dx1;
  int dx2 = max(x1, x2) - widthCor;
  int dy1 = min(y1, y2) + widthCor + 1;
  int dy2 = max(y1, y2) - widthCor;
  int pixelFlag;
  std::deque<uint32_t>::iterator patternIter;
  uint32_t tempPattern;

  patternIter = fillPattern.begin();
  // Scanline loop
  for (; dy1 <= dy2; dy1++)
  {
    // Use temp pattern to avoid gradual drift of original line pattern
    tempPattern = *patternIter;
    dx1 = min(x1, x2) + widthCor + 1;

    for (; dx1 <= dx2; dx1++)
    {
      pixelFlag = GetAndRotatePixelFlag(&*patternIter);
      if (pixelFlag)
        DrawPixel(dx1, dy1, pixelColor2, alphaChannel2);
    }

    // Reset current line of fill pattern to avoid gradual drift
    *patternIter = tempPattern;
    // Loop fill pattern
    if (patternIter != fillPattern.end())
      patternIter++;
    else
      patternIter = fillPattern.begin();
  }

  // Draw outline
  DrawRect(x1, y1, x2, y2);
}

// Interface to draw unfilled polygons
void DrawPoly(TImageCoordList *coordList)
{
  int x1, y1, x2, y2;
  bool endPoints = true;

  // Iterate through vertexes
  for (TImageCoordList::iterator iter = coordList->begin(); iter != coordList->end(); iter++)
  {
    x1 = iter->first;
    y1 = iter->second;
    if (++iter == coordList->end())
      break;
    else
    {
      x2 = iter->first;
      y2 = iter->second;
    }
    iter--;
    DrawLine(x1, y1, x2, y2, endPoints);
    endPoints = !endPoints;
  }
}

// Subprocess to construct, allocate, and initialize edge for polygon fill
Edge *createEdge(TImageCoordPair lower, TImageCoordPair upper, int yComp)
{
  Edge *newEdge;
  newEdge = (Edge *)malloc(sizeof(Edge));
  newEdge->dx = (float)lower.first;
  newEdge->slope = (float)(upper.first - lower.first) / (upper.second - lower.second);
  if (upper.second < yComp)
    newEdge->yMax = upper.second - 1;
  else
    newEdge->yMax = upper.second;
  newEdge->next = NULL;
  return newEdge;
}

// Subprocess that inserts edge into edge list
void insertEdge(Edge **ptrHead, Edge *newEdge)
{
  while (*ptrHead != NULL)
  {
    if ((*ptrHead)->dx > newEdge->dx)
      break;

    ptrHead = &((*ptrHead)->next);
  }

  newEdge->next = *ptrHead;
  *ptrHead = newEdge;
}

// Subprocess to initializes edge table from polygon coordinate list
void initEdgeTable(Edge **edgeTable, TImageCoordList *coordList)
{
  int y1, y2, yPrev, yNext;
  TImageCoordList::iterator current = coordList->end();
  current--;
  current--;
  yPrev = current->second;
  current++;
  TImageCoordList::iterator next = coordList->begin();
  TImageCoordList::iterator next2 = coordList->begin();
  next2++;

  while (next != coordList->end())
  {
    y1 = current->second;
    y2 = next->second;

    yNext = next2->second;

    if (++next2 == coordList->end())
      next2 = coordList->begin();

    if (y1 <= y2)
    {
      insertEdge(&edgeTable[y1], createEdge(*current, *next, yNext));
    }
    else
    {
      insertEdge(&edgeTable[y2], createEdge(*next, *current, yPrev));
    }

    DrawLine(current->first, y1, next->first, y2);

    yPrev = y1;
    current = next;
    next++;
  }
}

// Subprocess that inserts to active edge list
void insertActiveList(Edge **ptrHead, Edge **activeList)
{
  Edge *ptr = NULL;
  Edge *current = *ptrHead;

  while (current != NULL)
  {
    ptr = current->next;
    insertEdge(activeList, current);
    current = ptr;
  }
}

// Subprocess that scan-fills given line from active list
void scanFill(int scan, Edge *activeList, u_int32_t *pattern)
{
  int x, startx, endx, off, pixelFlag, count = 0;
  Edge *current = activeList, *next;
  uint32_t tempPattern = *pattern;

  while (current->next != NULL)
  {
    count++;
    if (current->yMax == scan)
      count++;

    next = current->next;

    for (off = 0; off < (int)current->dx % 32; off++)
    {
      pixelFlag = GetAndRotatePixelFlag(pattern);
    }
    if (count & 1)
    {
      startx = current->dx + (lineWidth >> 2) + 1;
      endx = next->dx - ((lineWidth - 1) >> 2) + 1;
      for (x = startx; x < endx; x++)
      {
        pixelFlag = GetAndRotatePixelFlag(pattern);
        if (pixelFlag)
          DrawPixel(x, scan, pixelColor2, alphaChannel2);
      }
    }
    *pattern = tempPattern;
    current = current->next;
  }
}

// Subprocess that updates active edge list values with each scan line
void updateActiveList(int scan, Edge **activeList)
{
  Edge *current, *next, *prev = NULL;

  for (current = *activeList; current != NULL; current = next)
  {
    next = current->next;

    if (current->yMax > scan)
    {
      current->dx += current->slope;
      prev = current;
      continue;
    }

    if (prev != NULL)
      prev->next = next;
    else
      *activeList = next;

    free(current);
  }
}

// Subprocess that resorts active edge list
void resortActiveList(Edge **activeList)
{
  Edge *current = *activeList, *next;
  *activeList = NULL;
  while (current != NULL)
  {
    next = current->next;
    insertEdge(activeList, current);
    current = next;
  }
}

// Interface to draw filled closed polygons
void DrawFilledPoly(TImageCoordList *coordList)
{
  int i, scan, canvasX, canvasY;
  if (GetCanvasSize(&canvasX, &canvasY))
    return;

  Edge **edgeTable = (Edge **)malloc(sizeof(Edge *) * canvasY);

  std::deque<uint32_t>::iterator patternIter;
  patternIter = fillPattern.begin();
  uint32_t tempPattern;

  for (i = 0; i < canvasY; i++)
  {
    edgeTable[i] = NULL;
  }

  initEdgeTable(edgeTable, coordList);

  Edge *activeList = (Edge *)malloc(sizeof(Edge *));
  activeList = NULL;

  // Scan line loop
  for (scan = 0; scan < canvasY; scan++)
  {
    tempPattern = *patternIter;
    insertActiveList(&edgeTable[scan], &activeList);
    if (activeList != NULL)
    {
      scanFill(scan, activeList, &*patternIter);
      updateActiveList(scan, &activeList);
      resortActiveList(&activeList);
    }

    // Reset current line of fill pattern to avoid gradual drift
    *patternIter = tempPattern;
    // Loop fill pattern
    if (patternIter != fillPattern.end())
      patternIter++;
    else
      patternIter = fillPattern.begin();
  }

  // Finally free the list heads (the vertices are freed during the scan line process)
  free(edgeTable);
  free(activeList);
}

// Subprocess that draws a basic 1px thick full ellipse using midpoint algorithm
void DrawBasicEllipse(int x, int y, int rx, int ry)
{
  int a = 2 * rx;
  int b = 2 * ry;
  int b1 = b & 1;
  long long dx = 4 * (1 - a) * b * b;
  long long dy = 4 * (b1 + 1) * a * a;
  long long err = dx + dy + b1 * a * a;
  long long err2;

  int x0 = x - rx;
  int y0 = y - ry + (b + 1) / 2;
  int x1 = x + rx;
  int y1 = y0;
  a *= 8 * a;
  b1 = 8 * b * b;

  int pixelFlag;
  uint32_t pattern = linePattern;

  do
  {
    pixelFlag = GetAndRotatePixelFlag(&pattern);
    if (pixelFlag)
    {
      DrawPixel(x1, y0, pixelColor1, alphaChannel1);
      DrawPixel(x0, y0, pixelColor1, alphaChannel1);
      DrawPixel(x0, y1, pixelColor1, alphaChannel1);
      DrawPixel(x1, y1, pixelColor1, alphaChannel1);
    }
    err2 = 2 * err;
    if (err2 <= dy)
    {
      y0++;
      y1--;
      dy += a;
      err += dy;
    }
    if (err2 >= dx || 2 * err > dy)
    {
      x0++;
      x1--;
      dx += b1;
      err += dx;
    }
  } while (x0 <= x1);

  while (y0 - y1 < b)
  {
    pixelFlag = GetAndRotatePixelFlag(&pattern);
    if (pixelFlag)
    {
      DrawPixel(x0 - 1, y0, pixelColor1, alphaChannel1);
      DrawPixel(x1 + 1, y0, pixelColor1, alphaChannel1);
      DrawPixel(x0 - 1, y1, pixelColor1, alphaChannel1);
      DrawPixel(x1 + 1, y1, pixelColor1, alphaChannel1);
    }
    y0++;
    y1--;
  }
}

// Subprocess that draws a basic 1px thick elliptical arc using slower trigonometric algorithm in the clockwise direction
void DrawPartialEllipse(int x, int y, int rx, int ry, int a1, int a2)
{
  int dx, dy, lx, ly, a;
  int pixelFlag;
  uint32_t pattern = linePattern;

  lx = rx * cos(a1 * M_PI / 180);
  ly = ry * sin(a1 * M_PI / 180);

  for (a = a1 + 1; a <= a2; a++)
  {
    dx = rx * cos(a * M_PI / 180.00);
    dy = ry * sin(a * M_PI / 180.00);
    pixelFlag = GetAndRotatePixelFlag(&pattern);
    if (pixelFlag)
      DrawBasicLine(x + lx, y + ly, x + dx, y + dy, 1);
    lx = dx;
    ly = dy;
  }
}

// Interface to draw empty full ellipses and ellipse sectors in the clockwise direction
void DrawEllipse(int x, int y, int rx, int ry, int a1 = -1, int a2 = -1, int radii = 0)
{
  // Handling for full ellipse
  if ((a1 < 0 && a2 < 0) || (a1 == a2))
  {
    // Handling line width
    for (int i = 1; i <= lineWidth; i++)
    {
      // Odd goes under
      if (i & 1)
        DrawBasicEllipse(x, y, rx - ((i - 1) >> 1), ry - ((i - 1) >> 1));
      // Even goes over
      else
        DrawBasicEllipse(x, y, rx + (i >> 1), ry + (i >> 1));
    }
  }
  else
  // Handling for elliptical arcs and sectors
  {
    int ta1 = max(a1, 0);
    int ta2 = min(a2, 360);
    int ta3 = 0;

    // Break down into 2 arcs if a1 > a2 to preserve directionality
    if (ta1 > ta2)
    {
      ta3 = ta1;
      ta1 = 0;
    }

    // Handling line width
    for (int i = 1; i <= lineWidth; i++)
    {
      // Odd goes under
      if (i & 1)
        DrawPartialEllipse(x, y, rx - ((i - 1) >> 1), ry - ((i - 1) >> 1), ta1, ta2);
      // Even goes over
      else
        DrawPartialEllipse(x, y, rx + (i >> 1), ry + (i >> 1), ta1, ta2);
    }

    // Draw second part of arc if it loops past start
    if (ta3)
    {
      // Handling line width
      for (int i = 1; i <= lineWidth; i++)
      {
        // Odd goes under
        if (i & 1)
          DrawPartialEllipse(x, y, rx - ((i - 1) >> 1), ry - ((i - 1) >> 1), ta3, 360);
        // Even goes over
        else
          DrawPartialEllipse(x, y, rx + (i >> 1), ry + (i >> 1), ta3, 360);
      }
    }

    // Draw radius lines to close sector
    if (radii)
    {
      DrawLine(x, y, x + cos(a1 * M_PI / 180) * rx, y + sin(a1 * M_PI / 180) * ry);
      DrawLine(x, y, x + cos(a2 * M_PI / 180) * rx, y + sin(a2 * M_PI / 180) * ry);
    }
  }
}

// Subprocess that draws a filled ellipse or ellipse sector
void DrawBasicPie(int x, int y, int rx, int ry, int a1, int a2)
{
  int scanx, scany, yy;
  long rx2 = rx * rx;
  long ry2 = ry * ry;
  long rxry = rx2 * ry2;
  long p;
  int a = a2 - a1;
  double ra1 = a1 * M_PI / 180;
  double ra2 = a2 * M_PI / 180;
  int a1x = rx * cos(ra1);
  int a1y = ry * sin(ra1);
  int a2x = rx * cos(ra2);
  int a2y = ry * sin(ra2);

  int pixelFlag;
  std::deque<uint32_t>::iterator patternIter;
  patternIter = fillPattern.begin();
  uint32_t tempPattern;

  // Handling for sectors wider than 180°
  if (a < 180)
  {
    for (scany = -ry; scany <= ry; scany++)
    {
      // Use temp pattern to avoid gradual drift of original line pattern
      tempPattern = *patternIter;
      yy = scany * scany;
      for (scanx = -rx; scanx <= rx; scanx++)
      {
        pixelFlag = GetAndRotatePixelFlag(&tempPattern);
        p = scanx * scanx * ry2 + yy * rx2;
        if (pixelFlag && p < rxry)
          if (scanx * a1y - scany * a1x <= 0 && scanx * a2y - scany * a2x > 0)
            DrawPixel(x + scanx, y + scany, pixelColor2, alphaChannel2);
      }

      // Reset current line of fill pattern to avoid gradual drift
      *patternIter = tempPattern;
      // Loop fill pattern
      if (patternIter != fillPattern.end())
        patternIter++;
      else
        patternIter = fillPattern.begin();
    }
  }
  // Handling for smaller sectors
  else
  {
    for (scany = -ry; scany <= ry; scany++)
    {
      tempPattern = *patternIter;
      yy = scany * scany;
      for (scanx = -rx; scanx <= rx; scanx++)
      {
        pixelFlag = GetAndRotatePixelFlag(&*patternIter);
        p = scanx * scanx * ry2 + yy * rx2;
        if (pixelFlag && p < rxry)
          if (scanx * a1y - scany * a1x <= 0 || scanx * a2y - scany * a2x > 0)
            DrawPixel(x + scanx, y + scany, pixelColor2, alphaChannel2);
      }

      // Reset current line of fill pattern to avoid gradual drift
      *patternIter = tempPattern;
      // Loop fill pattern
      if (patternIter != fillPattern.end())
        patternIter++;
      else
        patternIter = fillPattern.begin();
    }
  }
}

// Interface for pies and pie sectors
void DrawPie(int x, int y, int rx, int ry, int a1 = -1, int a2 = -1)
{
  // Correction for width of line
  int widthCor = (lineWidth >> 1);
  int ta1 = max(a1, 0);
  int ta2 = a2 < 0 ? 360 : min(a2, 360);
  int ta3 = 0;

  // Break down into 2 sectors if a1 > a2 to preserve directionality
  if (ta1 > ta2)
  {
    ta3 = ta1;
    ta1 = 0;
  }

  DrawBasicPie(x, y, rx - widthCor, ry - widthCor, ta1, ta2);

  // Draw second part of arc if it loops past start
  if (ta3)
    DrawBasicPie(x, y, rx - widthCor, ry - widthCor, ta3, 360);

  // Draw outline of pie
  DrawEllipse(x, y, rx, ry, a1, a2, 1);
}

// Function to test drawing
void draw()
{
  // Testing lines
  DrawLine(250, 250, 0, 0, 1);
  DrawLine(250, 250, 125, 0, 1);
  DrawLine(250, 250, 250, 0, 1);
  DrawLine(250, 250, 375, 0, 1);
  DrawLine(250, 250, 500, 0, 1);
  DrawLine(250, 250, 500, 125, 1);
  DrawLine(250, 250, 500, 250, 1);
  DrawLine(250, 250, 500, 375, 1);
  DrawLine(250, 250, 500, 500, 1);
  DrawLine(250, 250, 375, 500, 1);
  DrawLine(250, 250, 250, 500, 1);
  DrawLine(250, 250, 125, 500, 1);
  DrawLine(250, 250, 0, 500, 1);
  DrawLine(250, 250, 0, 375, 1);
  DrawLine(250, 250, 0, 250, 1);
  DrawLine(250, 250, 0, 125, 1);

  // Testing boxes
  DrawRect(600, 100, 950, 200);
  DrawBox(600, 300, 700, 500);
  DrawBox(950, 500, 750, 300);

  // Testing ellipses
  DrawEllipse(100, 680, 150, 100, 0, 90, 1);
  DrawEllipse(100, 680, 100, 100, 90, 180, 0);
  DrawEllipse(100, 680, 100, 150, 180, 270, 1);
  DrawEllipse(100, 680, 150, 150, 270, 360, 0);
  DrawEllipse(100, 680, 100, 100);
  DrawEllipse(100, 680, 100, 50);
  DrawEllipse(100, 680, 50, 100);

  // Testing pies
  DrawPie(360, 890, 150, 100, 0, 90);
  DrawPie(350, 890, 100, 100, 90, 180);
  DrawPie(350, 880, 100, 150, 180, 270);
  DrawPie(360, 880, 150, 150, 270, 360);
  DrawPie(100, 900, 100, 100);
  DrawPie(360, 600, 100, 50);

  // Testing polygons
  TImageCoordList coords1;
  coords1.push_back(std::make_pair(550, 550));
  coords1.push_back(std::make_pair(550, 750));
  coords1.push_back(std::make_pair(650, 650));
  coords1.push_back(std::make_pair(750, 750));
  coords1.push_back(std::make_pair(850, 650));
  coords1.push_back(std::make_pair(950, 750));
  coords1.push_back(std::make_pair(950, 550));
  DrawPoly(&coords1);

  TImageCoordList coords2;
  coords2.push_back(std::make_pair(550, 850));
  coords2.push_back(std::make_pair(650, 950));
  coords2.push_back(std::make_pair(850, 750));
  coords2.push_back(std::make_pair(950, 850));
  coords2.push_back(std::make_pair(850, 950));
  coords2.push_back(std::make_pair(650, 750));
  DrawFilledPoly(&coords2);

  glutSwapBuffers();
}

void init()
{
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glClear(GL_COLOR_BUFFER_BIT);
  glMatrixMode(GL_PROJECTION);
  gluOrtho2D(0, winw, winh, 0);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glPointSize(0.0);
}

void keyHandler(unsigned char key, int x, int y)
{
  switch (key)
  {
  case 27:
  case * "q":
  case * "Q":
    exit(0);
    break;
  }
  glutPostRedisplay();
}

int main(int argc, char **argv)
{
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);
  glutInitWindowSize(winw, winh);
  glutInitWindowPosition(0, 0);
  glutCreateWindow("floating");
  init();
  glutDisplayFunc(draw);
  glutKeyboardFunc(keyHandler);
  glutMainLoop();

  return 0;
}