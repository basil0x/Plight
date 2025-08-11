#include <SDL2/SDL_events.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_video.h>
#include <iostream>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <math.h>
#include <cmath>
#include <string>
#include <thread>
#include <vector>
#include <string.h>
#include <bits/stdc++.h> 

#define WIDTH 900
#define HEIGHT 600
#define COLOR_WHITE 0xffffffff
#define COLOR_BLACK 0x00000000
#define COLOR_GREY 0x808080

struct Vec2{
  double oa;
  double ob;
  double a;
  double b;
};

Vec2 MakeVec2(double x1, double y1, double x2, double y2){
  return (Vec2) {x1,y1,x2-x1,y2-y1};
}

int Orientation(Vec2 v1,Vec2 v2){
  int z = v1.a * v2.b - v2.a * v1.b;
  if (z<0) return -1;
  if (z>0) return 1;
  return 0;
}

// FIXME: Segmentation error
void SetPixel(SDL_Surface* surface, int x, int y, Uint32 color) {
    if (SDL_MUSTLOCK(surface)) {
        SDL_LockSurface(surface);
    }
    
    if (x>=0 && x<=WIDTH && y>=0 && y<=HEIGHT)
    { 
      Uint32* pixels = (Uint32*)surface->pixels;
      pixels[(y * WIDTH) + x] = color;
    }

    if (SDL_MUSTLOCK(surface)) {
        SDL_UnlockSurface(surface);
    }
}

// TODO: Clean this up please
std::pair<Vec2, Vec2> findOuterVectors(const std::vector<Vec2>& vectors) {
    std::pair<Vec2, Vec2> outerVectors;
    Uint32 frameStart = SDL_GetTicks();
    // Iterate over all pairs of vectors
    for (size_t i = 0; i < vectors.size(); ++i) {
        for (size_t j = i + 1; j < vectors.size(); ++j) {
            const Vec2& v1 = vectors[i];
            const Vec2& v2 = vectors[j];

            bool isOuter = true;
            // Compare the orientation of the remaining vectors with respect to the line v1-v2
            for (size_t k = 0; k < vectors.size(); ++k) {
                if (k != i && k != j) {
                    // Create a vector between v1 and v2 and check orientation for vector k

                    int orientl = Orientation(v1, vectors[k]);
                    int orientr = Orientation(v2, vectors[k]);
                    int orient = Orientation(v1, v2);

                    if (orientl != 1 && orient == 1) {  // If the vector is not on the same side
                        isOuter = false;
                        break;
                    }
                    if (orientl != -1 && orient == -1) {  // If the vector is not on the same side
                        isOuter = false;
                        break;
                    }
                    if (orientr != -1 && orient == 1) {  // If the vector is not on the same side
                        isOuter = false;
                        break;
                    } 
                    if (orientr != 1 && orient == -1) {  // If the vector is not on the same side
                        isOuter = false;
                        break;
                    }
                }
            }

            // If a pair of vectors satisfy the condition, set them as outer vectors
            if (isOuter) {
                outerVectors = {v1, v2};
                break;
            }
        }
    }
    
    std::cout <<"Vectors took " << SDL_GetTicks() - frameStart << " ms\n";
    return outerVectors;
}

struct Circle{
  double x;
  double y;
  double r;
};

void FillCircle(SDL_Surface *surface,struct Circle circle,Uint32 Color)
{
  double r_sq = pow(circle.r,2);
  double disttance_sq;
  for(double x=circle.x-circle.r;x<=circle.x+circle.r;x++)
    for(double y=circle.y-circle.r;y<=circle.y+circle.r;y++)
      if(pow(x-circle.x,2)+pow(y-circle.y,2)<r_sq)
      {
        SetPixel(surface, x, y, Color);
      }
}
// FIXME: Segfault
void FillRect(SDL_Surface *surface,SDL_Rect rect,int intensity,struct Circle light,Uint32 Color)
{
  for(double x=rect.x;x<=rect.x+rect.w;x++)
    {
      for(double y=rect.y;y<=rect.y+rect.h;y++)
      {
        int dx = x - light.x;
        int dy = y - light.y;
        float distance = std::sqrt(dx * dx + dy * dy);
        if (distance <= intensity)
        SetPixel(surface, x, y, Color);
      }
    }
}
unsigned int calculateColorBasedOnDistance(int x, int y, int lightX, int lightY, int maxDistance) {
    int dx = x - lightX;
    int dy = y - lightY;
    float distance = std::sqrt(dx * dx + dy * dy);
    float intensityFactor = std::max(0.0f, 1.0f - (distance / maxDistance));
    if (distance > maxDistance) intensityFactor = 0.0f;

    unsigned int r = (COLOR_WHITE >> 24) & 0xFF;
    unsigned int g = (COLOR_WHITE >> 16) & 0xFF;
    unsigned int b = (COLOR_WHITE >> 8) & 0xFF;

    r = static_cast<unsigned int>(r * intensityFactor);
    g = static_cast<unsigned int>(g * intensityFactor);
    b = static_cast<unsigned int>(b * intensityFactor);

    unsigned int dimmedColor = (r << 24) | (g << 16) | (b << 8) | 0x00;

    return dimmedColor;
}

/* TODO:  rewrite */
void FillRectShadow(SDL_Surface *surface,struct Circle light,SDL_Rect rect)
{
  Uint32 frameStart = SDL_GetTicks();
  Vec2 v1 = MakeVec2(light.x,light.y, rect.x, rect.y); 
  Vec2 v2 = MakeVec2(light.x,light.y, rect.x, rect.y+rect.h); 
  Vec2 v3 = MakeVec2(light.x,light.y, rect.x+rect.w, rect.y); 
  Vec2 v4 = MakeVec2(light.x,light.y, rect.x+rect.w, rect.y+rect.h); 
  
  //Figure out which vectors are outer ones
  
  std::vector<Vec2> vectors{v1,v2,v3,v4};
  
  std::pair<Vec2,Vec2> outer = findOuterVectors(vectors);
  // TODO Set up propper rendering (SDL_Renderer); Performance issues are here
  // TODO: Multithreading
  auto f = [=](int x1, int y1, int x2, int y2)
  { 
  try {
  for(int x = x1;x<x2;x++)
    for(int y = y1;y<y2;y++)
    {
      Vec2 v = MakeVec2(light.x, light.y, x, y);
      Vec2 orv = MakeVec2(outer.first.oa+outer.first.a,
                          outer.first.ob+outer.first.b,
                          outer.second.oa+outer.second.a,
                          outer.second.ob+outer.second.b);
      Vec2 orl = MakeVec2(outer.first.oa+outer.first.a,
                          outer.first.ob+outer.first.b,
                          x,
                          y);
       
      if (Orientation(outer.first, v) == 1 &&
          Orientation(outer.second, v) == -1 && 
          Orientation(outer.first, outer.second) == 1 * -Orientation(orv,orl))
            { 
              SetPixel(surface, x, y, COLOR_BLACK);
            }

      else if (Orientation(outer.first, v) == -1 &&
               Orientation(outer.second, v) == 1 &&
               Orientation(outer.first, outer.second) == -1 * Orientation(orv,orl))
            {
              SetPixel(surface, x, y, COLOR_BLACK);
            }
      else {
        int intensity = 400;
        Uint32 Color = calculateColorBasedOnDistance(x, y, light.x, light.y, intensity);
        SetPixel(surface, x, y, Color);
      }
       
    }
    }catch (const std::exception& e) {
        std::cerr << "Exception caught in thread: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "Unknown exception caught in thread!" << std::endl;
    }
  };

  int num_threads = std::thread::hardware_concurrency();
  std::vector<std::thread> threads;
  int chunk_width = WIDTH / num_threads;

  for (int i = 0; i < num_threads; ++i) {
     int startX = i * chunk_width;
     int endX = (i == num_threads - 1) ? WIDTH : (i + 1) * chunk_width;
     threads.push_back(std::thread(f, startX, 0, endX, HEIGHT));
  }

  for (auto& t : threads) {
     t.join();
  }
  
  std::cout << "Draw Rectangle Shadow took " << SDL_GetTicks() - frameStart << " ms\n";


}



int main()
{
  SDL_Init(SDL_INIT_VIDEO);
  SDL_Window*  window = SDL_CreateWindow("Raytracing",
      SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,WIDTH,HEIGHT,0);

  SDL_Surface *surface = SDL_GetWindowSurface(window);
  SDL_Rect rect = (SDL_Rect) {200,200,200,100};
  SDL_Rect rect2 = (SDL_Rect) {500,400,200,100};
  SDL_Rect erase_rect = (SDL_Rect) {0,0,WIDTH,HEIGHT}; 
  //SDL_FillRect(surface, &rect, COLOR_WHITE);

  SDL_UpdateWindowSurface(window);
  
  bool running = true;
  int msec;
  Uint32 frameStart;
  SDL_Event event;
  struct Circle circle = {0,0,20};
  while (running)
  {
    frameStart = SDL_GetTicks();
    int px = circle.x,py = circle.y;
    while (SDL_PollEvent(&event))
    {
      if (event.type == SDL_QUIT) running = false;
      if (event.type == SDL_MOUSEMOTION && event.motion.state)
      {
        circle.x = event.motion.x;
        circle.y = event.motion.y;
      }

    }
    /* TODO: Figure out this shit*/
    try {
    if (circle.x != px || circle.y != py)
    { 
    SDL_FillRect(surface, &erase_rect , COLOR_GREY);
    //FillCircle(surface,circle,COLOR_WHITE);
    //FillRectShadow(surface, circle, rect2);
    //SDL_FillRect(surface, &rect2 , COLOR_WHITE);

    FillRectShadow(surface, circle, rect);
    FillRect(surface, rect , 400 ,circle ,COLOR_WHITE);
    FillCircle(surface,circle,COLOR_WHITE);
    }
    SDL_UpdateWindowSurface(window);
    
    double fps;
    msec = SDL_GetTicks() - frameStart;
	  if(msec > 0)
		  fps = 1000.0 / (double) msec;
    
    std::cout << "Simulation time: " << msec << " ms\n";
    SDL_SetWindowTitle(window, std::to_string((int)fps).c_str());
    } catch (const std::exception& e) {
        std::cerr << "Exception caught in thread: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "Unknown exception caught in thread!" << std::endl;
    }
    
  }



}
