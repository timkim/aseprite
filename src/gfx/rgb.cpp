// ASEPRITE gfx library
// Copyright (C) 2001-2013  David Capello
//
// This source file is distributed under a BSD-like license, please
// read LICENSE.txt for more information.

#include "gfx/rgb.h"
#include "gfx/hsv.h"
#include <cmath>

using namespace gfx;
using namespace std;

// Reference: http://en.wikipedia.org/wiki/HSL_and_HSV
Rgb::Rgb(const Hsv& hsv)
{
  double chroma = hsv.value() * hsv.saturation();
  double hue_prime = hsv.hue() / 60.0;
  double x = chroma * (1.0 - fabs(fmod(hue_prime, 2.0) - 1.0));
  double r, g, b;

  r = g = b = 0.0;

  switch (int(hue_prime)) {

    case 6:
    case 0:
      r = chroma;
      g = x;
      break;
    case 1:
      r = x;
      g = chroma;
      break;

    case 2:
      g = chroma;
      b = x;
      break;
    case 3:
      g = x;
      b = chroma;
      break;

    case 4:
      b = chroma;
      r = x;
      break;
    case 5:
      b = x;
      r = chroma;
      break;
  }

  double m = hsv.value() - chroma;
  r += m;
  g += m;
  b += m;

  m_red   = int(r*255.0+0.5);
  m_green = int(g*255.0+0.5);
  m_blue  = int(b*255.0+0.5);
}

int Rgb::maxComponent() const
{
  if (m_red > m_green)
    return (m_red > m_blue) ? m_red: m_blue;
  else
    return (m_green > m_blue) ? m_green: m_blue;
}

int Rgb::minComponent() const
{
  if (m_red < m_green)
    return (m_red < m_blue) ? m_red: m_blue;
  else
    return (m_green < m_blue) ? m_green: m_blue;
}
