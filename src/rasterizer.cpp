#include "rasterizer.h"
#include "texture.h"

using namespace std;

namespace CGL {

RasterizerImp::RasterizerImp(PixelSampleMethod psm, LevelSampleMethod lsm,
                                       size_t width, size_t height,
                                       unsigned int sample_rate) {
  this->psm = psm;
  this->lsm = lsm;
  this->width = width;
  this->height = height;
  this->sample_rate = sample_rate;
    
  supersample_buffer.resize(width * height * sample_rate, Color::White);

}

// Used by rasterize_point and rasterize_line
void RasterizerImp::fill_pixel(size_t x, size_t y, Color c) {
  // TODO: Task 2: You might need to this function to fix points and lines (such as the black rectangle border in test4.svg)
  // NOTE: You are not required to implement proper supersampling for points and lines
  // It is sufficient to use the same color for all supersamples of a pixel for points and lines (not triangles)

  rgb_framebuffer_target[3 * (y * width + x)    ] = (unsigned char)(c.r * 255);
  rgb_framebuffer_target[3 * (y * width + x) + 1] = (unsigned char)(c.g * 255);
  rgb_framebuffer_target[3 * (y * width + x) + 2] = (unsigned char)(c.b * 255);
}

// Optional helper function to add a sample to the supersample_buffer
void RasterizerImp::fill_supersample(size_t x, size_t y, size_t s, Color c) {
  // TODO: Task 2: You may want to implement this function. Hint: our solution uses one line
    supersample_buffer[sample_rate*(y * width + x) + s] = c;
};

// Rasterize a point: simple example to help you start familiarizing
// yourself with the starter code.
//
void RasterizerImp::rasterize_point(float x, float y, Color color) {
  // fill in the nearest pixel
  int sx = (int)floor(x);
  int sy = (int)floor(y);

  // check bounds
  if (sx < 0 || sx >= width) return;
  if (sy < 0 || sy >= height) return;

//  fill_pixel(sx, sy, color);
    for (int i =0;i< sample_rate;i++){
        fill_supersample(sx, sy, i, color);
    }
  
  return;
}

// Rasterize a line.
void RasterizerImp::rasterize_line(float x0, float y0,
  float x1, float y1,
  Color color) {
  if (x0 > x1) {
    swap(x0, x1); swap(y0, y1);
  }

  float pt[] = { x0,y0 };
  float m = (y1 - y0) / (x1 - x0);
  float dpt[] = { 1,m };
  int steep = abs(m) > 1;
  if (steep) {
    dpt[0] = x1 == x0 ? 0 : 1 / abs(m);
    dpt[1] = x1 == x0 ? (y1 - y0) / abs(y1 - y0) : m / abs(m);
  }

  while (floor(pt[0]) <= floor(x1) && abs(pt[1] - y0) <= abs(y1 - y0)) {
    rasterize_point(pt[0], pt[1], color);
    pt[0] += dpt[0]; pt[1] += dpt[1];
  }
}

// Rasterize a triangle.
void RasterizerImp::rasterize_triangle(float x0, float y0,
                                       float x1, float y1,
                                       float x2, float y2,
                                       Color color) {
// TODO: Task 1: Implement basic triangle rasterization here, no supersampling
// TODO: Task 2: Update to implement super-sampled rasterization

//    sample_rate = 9;
//    supersample_buffer.resize(width * height * sample_rate, Color::White);
    
//    [0, 1]
//    [0.25, 0.75]
    
    //task 2: ietrate through (x,y) within framebuffer with step size 0.5/sqrt(sample_rate)
    //fill the color to the frame buffer then compute the average value at "sample_rate" a time
    float sqt = sqrt(sample_rate);
    for (int x = min(min(x0, x1),x2); x < max(max(x0,x1),x2); x += 1 ){
        for(int y = min(min(y0, y1),y2); y < max(max(y0,y1),y2); y+= 1){
            int s = 0;
            float sx = x + 1/(2*sqt);
            float sy = y + 1/(2*sqt);
            for(int i =0; i < sqt; i++){
                for(int j = 0; j < sqt; j++){
                    
                    float l0 = -(sx-x0)*(y1-y0) + (sy-y0)*(x1-x0);
                    float l1 = -(sx-x1)*(y2-y1) + (sy-y1)*(x2-x1);
                    float l2 = -(sx-x2)*(y0-y2) + (sy-y2)*(x0-x2);
                    // clockwise or counter-clockwise
                    if ((l0 >= 0 && l1 >= 0 && l2 >=0) || (-l0 >= 0 && -l1 >= 0 && -l2 >= 0)){
//                        fill_pixel(x, y, color);
                        fill_supersample(x,y,s,color);
                    }
                s+=1;
                sy += 1/sqt;
                }
                sx += 1/sqt;
            }
        }
    }

}


void RasterizerImp::rasterize_interpolated_color_triangle(float x0, float y0, Color c0,
                                                          float x1, float y1, Color c1,
                                                          float x2, float y2, Color c2)
{
  // TODO: Task 4: Rasterize the triangle, calculating barycentric coordinates and using them to interpolate vertex colors across the triangle
  // Hint: You can reuse code from rasterize_triangle
    for (int x = 0; x < width; x += 1 ){
        for(int y = 0; y < height; y+= 1){
            int s = 0;
            float sx = x + 1/(2*sqrt(sample_rate));
            float sy = y + 1/(2*sqrt(sample_rate));
            for(int i =0; i < sqrt(sample_rate); i++){
                for(int j = 0; j < sqrt(sample_rate); j++){
                    
                    float alpha = (-(sx - x1)*(y2-y1)+(sy-y1)*(x2-x1))/(-(x0-x1)*(y2-y1)+(y0-y1)*(x2-x1));
                    float beta = (-(sx - x2)*(y0-y2)+(sy-y2)*(x0-x2))/(-(x1-x2)*(y0-y2)+(y1-y2)*(x0-x2));
                    float gama = 1-alpha-beta;
                    // clockwise or counter-clockwise
                    float l0 = -(sx-x0)*(y1-y0) + (sy-y0)*(x1-x0);
                    float l1 = -(sx-x1)*(y2-y1) + (sy-y1)*(x2-x1);
                    float l2 = -(sx-x2)*(y0-y2) + (sy-y2)*(x0-x2);
                    if ((l0 >= 0 && l1 >= 0 && l2 >=0) || (-l0 >= 0 && -l1 >= 0 && -l2 >= 0)){
//                        fill_pixel(x, y, color);
                        float r = alpha*c0.r + beta*c1.r + gama*c2.r;
                        float g = alpha*c0.g + beta*c1.g + gama*c2.g;
                        float b = alpha*c0.b + beta*c1.b + gama*c2.b;
                        fill_supersample(x,y,s,Color(r,g,b));
                    }
                s+=1;
                sy += 1/sqrt(sample_rate);
                }
                sx += 1/sqrt(sample_rate);
            }
        }
    }


}

float _alpha(float x0, float y0,float x1, float y1, float x2, float y2, float sx, float sy){
    return (-(sx - x1)*(y2-y1)+(sy-y1)*(x2-x1))/(-(x0-x1)*(y2-y1)+(y0-y1)*(x2-x1));
}
float _beta(float x0, float y0,float x1, float y1, float x2, float y2, float sx, float sy){
    return (-(sx - x2)*(y0-y2)+(sy-y2)*(x0-x2))/(-(x1-x2)*(y0-y2)+(y1-y2)*(x0-x2));
}

void RasterizerImp::rasterize_textured_triangle(float x0, float y0, float u0, float v0,
    float x1, float y1, float u1, float v1,
    float x2, float y2, float u2, float v2,
    Texture& tex)
{
    // TODO: Task 5: Fill in the SampleParams struct and pass it to the tex.sample function.
    // TODO: Task 6: Set the correct barycentric differentials in the SampleParams struct.
    // Hint: You can reuse code from rasterize_triangle/rasterize_interpolated_color_triangle
    
    float sqt = sqrt(sample_rate);
        for (int x = min(min(x0, x1),x2); x < max(max(x0,x1),x2); x += 1 ){
            for(int y = min(min(y0, y1),y2); y < max(max(y0,y1),y2); y+= 1){
                int s = 0;
                float sx = x + 1/(2*sqt);
                float sy = y + 1/(2*sqt);
                for(int i =0; i < sqt; i++){
                    for(int j = 0; j < sqt; j++){
                        float l0 = -(sx-x0)*(y1-y0) + (sy-y0)*(x1-x0);
                        float l1 = -(sx-x1)*(y2-y1) + (sy-y1)*(x2-x1);
                        float l2 = -(sx-x2)*(y0-y2) + (sy-y2)*(x0-x2);
                        // clockwise or counter-clockwise
                        if ((l0 >= 0 && l1 >= 0 && l2 >=0) || (-l0 >= 0 && -l1 >= 0 && -l2 >= 0)){
                            float alpha = _alpha(x0,y0,x1,y1,x2,y2,sx,sy);
                            float beta = _beta(x0,y0,x1,y1,x2,y2,sx,sy);
                            float gama = 1-alpha-beta;

                            float tu = alpha*u0 + beta*u1+ gama*u2;
                            float tv = alpha*v0 + beta*v1+ gama*v2;
                            
                            float alpha2 = _alpha(x0,y0,x1,y1,x2,y2,sx+(1/sqt),sy);
                            float beta2 = _beta(x0,y0,x1,y1,x2,y2,sx+(1/sqt),sy);
                            float gama2 = 1-alpha2-beta2;
                            
                            float tu_x = alpha2*u0 + beta2*u1+ gama2*u2;
                            float tv_x = alpha2*v0 + beta2*v1+ gama2*v2;
                            
                            float alpha3 = _alpha(x0,y0,x1,y1,x2,y2,sx,sy+(1/sqt));
                            float beta3 = _beta(x0,y0,x1,y1,x2,y2,sx,sy+(1/sqt));
                            float gama3 = 1-alpha3-beta3;
                            
                            float tu_y = alpha3*u0 + beta3*u1+ gama3*u2;
                            float tv_y = alpha3*v0 + beta3*v1+ gama3*v2;
                            
                            
                            //    Vector2D p_uv;
                            //    Vector2D p_dx_uv, p_dy_uv;
                            //    PixelSampleMethod psm;
                            //    LevelSampleMethod lsm;
                            SampleParams sp;
                            sp.p_uv = Vector2D(tu,tv);
                            sp.p_dx_uv = Vector2D(tu_x,tv_x);
                            sp.p_dy_uv = Vector2D(tu_y,tv_y);
                            sp.psm = psm;
                            sp.lsm = lsm;
                            Color c = tex.sample(sp);
                            fill_supersample(x,y,s,c);
                        }
                    s+=1;
                    sy += 1/sqt;
                    }
                    sx += 1/sqt;
                }
            }
        }
}

void RasterizerImp::set_sample_rate(unsigned int rate) {
  // TODO: Task 2: You may want to update this function for supersampling support
  
  this->sample_rate = rate;
  clear_buffers();

}


void RasterizerImp::set_framebuffer_target( unsigned char* rgb_framebuffer,
                                                size_t width, size_t height )
{
  // TODO: Task 2: You may want to update this function for supersampling support

  this->width = width;
  this->height = height;
  this->rgb_framebuffer_target = rgb_framebuffer;
    clear_buffers();
}


void RasterizerImp::clear_buffers() {
  // TODO: Task 2: You may want to update this function for supersampling support
  // Hint: With supersampling, you have an additional buffer to take care of
  supersample_buffer.resize(0);
  supersample_buffer.resize(width * height * sample_rate, Color::White);
  std::fill(rgb_framebuffer_target, rgb_framebuffer_target + 3 * width * height, 255);

}


// This function is called at the end of rasterizing all elements of the
// SVG file.  If you use a supersample buffer to rasterize SVG elements
// for antialising, you could use this call to fill the target framebuffer
// pixels from the supersample buffer data.
//
void RasterizerImp::resolve_to_framebuffer() {
  // TODO: Task 2: You will likely want to update this function for supersampling support

    for (int x = 0; x < width; x += 1 ){
        for (int y = 0; y< height; y+= 1){
            float r = 0;
            float g = 0;
            float b = 0;

            for (int s = 0; s < sample_rate;s++){
                size_t index = sample_rate * (y * width + x) + s;
                Color c = supersample_buffer[index];
                r += c.r;
                g += c.g;
                b += c.b;
                
            }
 
            fill_pixel(x,y,Color(r/sample_rate,g/sample_rate,b/sample_rate));
        }
    }
}

Rasterizer::~Rasterizer() { }


}// CGL
