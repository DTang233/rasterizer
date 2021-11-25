#include "texture.h"
#include "CGL/color.h"

#include <algorithm>
#include <cmath>

namespace CGL {

Color lerp(float t, Color c0,Color c1){
    Color c = (1-t)*c0 + t*c1;
    return c;
    
}

Color Texture::sample(const SampleParams &sp) {
  // TODO: Task 6: Fill this in.
  // return magenta for invalid level
    Color c;
    float level;
    int maxlevel = mipmap.size();
//    bool cond;
    if (sp.lsm == L_LINEAR){
        level = get_level(sp);
        float clevel = ceil(level);
        float flevel = floor(level);
        
        if (flevel > maxlevel){
            level = maxlevel;
        }
        if (clevel > maxlevel){
            clevel = maxlevel;
        }
        
        Color c1 = sample_bilinear(sp.p_uv,clevel);
        Color c2 = sample_bilinear(sp.p_uv,flevel);
        
        float t = (level - flevel)/level;

        c = lerp(t, c1, c2);
 
        return c;
        
    }
    if (sp.lsm == L_ZERO){
        level = 0;

    }
    else if (sp.lsm == L_NEAREST){

        level = (int)floor(get_level(sp));
//        cout << level;
        
        //TODO: why level< 0??
//        if (level < 0) {
//            level = 0;
//        }
        if (level > maxlevel){
            level = maxlevel;
        }
        

    }

    
    
    if (sp.psm == P_NEAREST){

        c = sample_nearest(sp.p_uv,level);

    }
    else if(sp.psm == P_LINEAR){

        c = sample_bilinear(sp.p_uv,level);
    }

  return c;
}

float Texture::get_level(const SampleParams &sp) {
  // TODO: Task 6: Fill this in.
//    Vector2D p_uv;
//    Vector2D p_dx_uv, p_dy_uv;
//    PixelSampleMethod psm;
//    LevelSampleMethod lsm;

    //sp.p_dx_uv - sp.p_uv = (du/dx, dv/dx)
    Vector2D duv_x = sp.p_dx_uv - sp.p_uv;
    //sp.p_dy_uv - sp.p_uv = (du/dy, dv/dy)
    Vector2D duv_y = sp.p_dy_uv - sp.p_uv;
    
    float dudx = duv_x.x*width;
    float dvdx = duv_x.y*height;
    
    float dudy = duv_y.x*width;
    float dvdy = duv_y.y*height;

    
    
    float var1 = sqrt(dudx*dudx + dvdx*dvdx);
    float var2 = sqrt(dudy*dudy + dvdy*dvdy);
//    cout << var1;
//    cout << var2;
    float level = log2(max(var1,var2));

//    cout << level;
//    if (level < 0){
//        cout << duv_x;
////        cout << width;
//        cout << max(var1,var2);
//        cout << "!!!!!!!!!!";
//    }
    
    
  return level;
}

Color MipLevel::get_texel(int tx, int ty) {
 
//    cout << tx * 3 + ty * width * 3;
  return Color(&texels[tx * 3 + ty * width * 3]);
}

Color Texture::sample_nearest(Vector2D uv, int level) {
  // TODO: Task 5: Fill this in.
  // return magenta for invalid level
//  cout <<mipmap[level].get_texel(uv.x,uv.y);
    
//    cout << level;
  float mul_x = (width-1)/pow(2, level);
  float mul_y = (height-1)/pow(2, level);
  return mipmap[level].get_texel((int)floor(uv.x* mul_x),(int)floor(uv.y*mul_y));
}



Color Texture::sample_bilinear(Vector2D uv, int level) {
  // TODO: Task 5: Fill this in.
  // return magenta for invalid level
    float mul_x = (width-1)/pow(2, level);
    float mul_y = (height-1)/pow(2, level);
    float s_x = uv.x*mul_x;
    float s_y = uv.y*mul_y;
    
    float flx = floor(s_x);
    float fly = floor(s_y);
    float clx = ceil(s_x);
    float cly = ceil(s_y);
    
    Vector2D u00 = Vector2D(flx,fly);
    Vector2D u01 = Vector2D(flx,cly);
    Vector2D u11 = Vector2D(clx,cly);
    Vector2D u10 = Vector2D(clx,fly);
    
    
    float s = (s_x - u00.x)/(u10.x-u00.x);
    float t = (s_y - u00.y)/(u11.y-u10.y);

    Color c00 = mipmap[level].get_texel(u00.x,u00.y);
//    cout << c00;
    Color c10 = mipmap[level].get_texel(u10.x,u10.y);
    Color c01 = mipmap[level].get_texel(u01.x,u01.y);
    Color c11 = mipmap[level].get_texel(u11.x,u11.y);

//    float d = u10.x-u00.x;
    Color c00_c10 = lerp(s, c00, c10);
    Color c01_c11 = lerp(s, c01, c11);
    
//    float d1 = u11.y - u10.y;
    
    Color res = lerp(t,c00_c10,c01_c11);
    
    
    return res;
}

/****************************************************************************/

// Helpers

inline void uint8_to_float(float dst[3], unsigned char *src) {
  uint8_t *src_uint8 = (uint8_t *)src;
  dst[0] = src_uint8[0] / 255.f;
  dst[1] = src_uint8[1] / 255.f;
  dst[2] = src_uint8[2] / 255.f;
}

inline void float_to_uint8(unsigned char *dst, float src[3]) {
  uint8_t *dst_uint8 = (uint8_t *)dst;
  dst_uint8[0] = (uint8_t)(255.f * max(0.0f, min(1.0f, src[0])));
  dst_uint8[1] = (uint8_t)(255.f * max(0.0f, min(1.0f, src[1])));
  dst_uint8[2] = (uint8_t)(255.f * max(0.0f, min(1.0f, src[2])));
}

void Texture::generate_mips(int startLevel) {

  // make sure there's a valid texture
  if (startLevel >= mipmap.size()) {
    std::cerr << "Invalid start level";
  }

  // allocate sublevels
  int baseWidth = mipmap[startLevel].width;
  int baseHeight = mipmap[startLevel].height;
  int numSubLevels = (int)(log2f((float)max(baseWidth, baseHeight)));

  numSubLevels = min(numSubLevels, kMaxMipLevels - startLevel - 1);
  mipmap.resize(startLevel + numSubLevels + 1);

  int width = baseWidth;
  int height = baseHeight;
  for (int i = 1; i <= numSubLevels; i++) {

    MipLevel &level = mipmap[startLevel + i];

    // handle odd size texture by rounding down
    width = max(1, width / 2);
    // assert (width > 0);
    height = max(1, height / 2);
    // assert (height > 0);

    level.width = width;
    level.height = height;
    level.texels = vector<unsigned char>(3 * width * height);
  }

  // create mips
  int subLevels = numSubLevels - (startLevel + 1);
  for (int mipLevel = startLevel + 1; mipLevel < startLevel + subLevels + 1;
       mipLevel++) {

    MipLevel &prevLevel = mipmap[mipLevel - 1];
    MipLevel &currLevel = mipmap[mipLevel];

    int prevLevelPitch = prevLevel.width * 3; // 32 bit RGB
    int currLevelPitch = currLevel.width * 3; // 32 bit RGB

    unsigned char *prevLevelMem;
    unsigned char *currLevelMem;

    currLevelMem = (unsigned char *)&currLevel.texels[0];
    prevLevelMem = (unsigned char *)&prevLevel.texels[0];

    float wDecimal, wNorm, wWeight[3];
    int wSupport;
    float hDecimal, hNorm, hWeight[3];
    int hSupport;

    float result[3];
    float input[3];

    // conditional differentiates no rounding case from round down case
    if (prevLevel.width & 1) {
      wSupport = 3;
      wDecimal = 1.0f / (float)currLevel.width;
    } else {
      wSupport = 2;
      wDecimal = 0.0f;
    }

    // conditional differentiates no rounding case from round down case
    if (prevLevel.height & 1) {
      hSupport = 3;
      hDecimal = 1.0f / (float)currLevel.height;
    } else {
      hSupport = 2;
      hDecimal = 0.0f;
    }

    wNorm = 1.0f / (2.0f + wDecimal);
    hNorm = 1.0f / (2.0f + hDecimal);

    // case 1: reduction only in horizontal size (vertical size is 1)
    if (currLevel.height == prevLevel.height) {
      // assert (currLevel.height == 1);

      for (int i = 0; i < currLevel.width; i++) {
        wWeight[0] = wNorm * (1.0f - wDecimal * i);
        wWeight[1] = wNorm * 1.0f;
        wWeight[2] = wNorm * wDecimal * (i + 1);

        result[0] = result[1] = result[2] = 0.0f;

        for (int ii = 0; ii < wSupport; ii++) {
          uint8_to_float(input, prevLevelMem + 3 * (2 * i + ii));
          result[0] += wWeight[ii] * input[0];
          result[1] += wWeight[ii] * input[1];
          result[2] += wWeight[ii] * input[2];
        }

        // convert back to format of the texture
        float_to_uint8(currLevelMem + (3 * i), result);
      }

      // case 2: reduction only in vertical size (horizontal size is 1)
    } else if (currLevel.width == prevLevel.width) {
      // assert (currLevel.width == 1);

      for (int j = 0; j < currLevel.height; j++) {
        hWeight[0] = hNorm * (1.0f - hDecimal * j);
        hWeight[1] = hNorm;
        hWeight[2] = hNorm * hDecimal * (j + 1);

        result[0] = result[1] = result[2] = 0.0f;
        for (int jj = 0; jj < hSupport; jj++) {
          uint8_to_float(input, prevLevelMem + prevLevelPitch * (2 * j + jj));
          result[0] += hWeight[jj] * input[0];
          result[1] += hWeight[jj] * input[1];
          result[2] += hWeight[jj] * input[2];
        }

        // convert back to format of the texture
        float_to_uint8(currLevelMem + (currLevelPitch * j), result);
      }

      // case 3: reduction in both horizontal and vertical size
    } else {

      for (int j = 0; j < currLevel.height; j++) {
        hWeight[0] = hNorm * (1.0f - hDecimal * j);
        hWeight[1] = hNorm;
        hWeight[2] = hNorm * hDecimal * (j + 1);

        for (int i = 0; i < currLevel.width; i++) {
          wWeight[0] = wNorm * (1.0f - wDecimal * i);
          wWeight[1] = wNorm * 1.0f;
          wWeight[2] = wNorm * wDecimal * (i + 1);

          result[0] = result[1] = result[2] = 0.0f;

          // convolve source image with a trapezoidal filter.
          // in the case of no rounding this is just a box filter of width 2.
          // in the general case, the support region is 3x3.
          for (int jj = 0; jj < hSupport; jj++)
            for (int ii = 0; ii < wSupport; ii++) {
              float weight = hWeight[jj] * wWeight[ii];
              uint8_to_float(input, prevLevelMem +
                                        prevLevelPitch * (2 * j + jj) +
                                        3 * (2 * i + ii));
              result[0] += weight * input[0];
              result[1] += weight * input[1];
              result[2] += weight * input[2];
            }

          // convert back to format of the texture
          float_to_uint8(currLevelMem + currLevelPitch * j + 3 * i, result);
        }
      }
    }
  }
}

} // namespace CGL
