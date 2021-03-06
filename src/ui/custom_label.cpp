// ASEPRITE gui library
// Copyright (C) 2001-2013  David Capello
//
// This source file is distributed under a BSD-like license, please
// read LICENSE.txt for more information.


#include "config.h"

#include "ui/custom_label.h"

namespace ui {

CustomLabel::CustomLabel(const char *text)
  : Label(text)
{
}

bool CustomLabel::onProcessMessage(Message* msg)
{
  return Label::onProcessMessage(msg);
}

} // namespace ui
