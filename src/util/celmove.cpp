/* ASE - Allegro Sprite Editor
 * Copyright (C) 2001-2009  David Capello
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "config.h"

#include <assert.h>
#include <jinete/jlist.h>

#include "console/console.h"
#include "core/app.h"
#include "core/color.h"
#include "core/core.h"
#include "modules/gui.h"
#include "modules/sprites.h"
#include "raster/blend.h"
#include "raster/cel.h"
#include "raster/image.h"
#include "raster/layer.h"
#include "raster/sprite.h"
#include "raster/stock.h"
#include "raster/undo.h"
#include "util/celmove.h"
#include "util/functions.h"

/* these variables indicate what cel to move (and the current_sprite
   frame indicates to where move it) */
static Layer *src_layer = NULL;	/* TODO warning not thread safe */
static Layer *dst_layer = NULL;
static int src_frame = 0;
static int dst_frame = 0;

void set_frame_to_handle(Layer *_src_layer, int _src_frame,
			 Layer *_dst_layer, int _dst_frame)
{
  src_layer = _src_layer;
  src_frame = _src_frame;
  dst_layer = _dst_layer;
  dst_frame = _dst_frame;
}

void move_cel()
{
  Sprite *sprite = current_sprite;
  Cel *src_cel, *dst_cel;

  assert(src_layer != NULL);
  assert(dst_layer != NULL);
  assert(src_frame >= 0 && src_frame < sprite->frames);
  assert(dst_frame >= 0 && dst_frame < sprite->frames);

  if (layer_is_background(src_layer)) {
    copy_cel();
    return;
  }

  src_cel = layer_get_cel(src_layer, src_frame);
  dst_cel = layer_get_cel(dst_layer, dst_frame);

  if (undo_is_enabled(sprite->undo)) {
    undo_set_label(sprite->undo, "Move Cel");
    undo_open(sprite->undo);

    undo_set_layer(sprite->undo, sprite);
    undo_int(sprite->undo, sprite, &sprite->frame);
  }

  sprite_set_layer(sprite, dst_layer);
  sprite_set_frame(sprite, dst_frame);

  /* remove the 'dst_cel' (if it exists) because it must be
     replaced with 'src_cel' */
  if ((dst_cel != NULL) && (!layer_is_background(dst_layer) || src_cel != NULL))
    RemoveCel(dst_layer, dst_cel);

  /* move the cel in the same layer */
  if (src_cel != NULL) {
    if (src_layer == dst_layer) {
      if (undo_is_enabled(sprite->undo))
	undo_int(sprite->undo, (GfxObj *)src_cel, &src_cel->frame);

      src_cel->frame = dst_frame;
    }
    /* move the cel in different layers */
    else {
      if (undo_is_enabled(sprite->undo))
	undo_remove_cel(sprite->undo, src_layer, src_cel);
      layer_remove_cel(src_layer, src_cel);

      src_cel->frame = dst_frame;

      /* if we are moving a cel from a transparent layer to the
	 background layer, we have to clear the background of the
	 image */
      if (!layer_is_background(src_layer) &&
	  layer_is_background(dst_layer)) {
	Image *src_image = stock_get_image(sprite->stock, src_cel->image);
	Image *dst_image = image_crop(src_image,
				      -src_cel->x,
				      -src_cel->y,
				      sprite->w,
				      sprite->h, 0);

	if (undo_is_enabled(sprite->undo)) {
	  undo_replace_image(sprite->undo, sprite->stock, src_cel->image);
	  undo_int(sprite->undo, (GfxObj *)src_cel, &src_cel->x);
	  undo_int(sprite->undo, (GfxObj *)src_cel, &src_cel->y);
	  undo_int(sprite->undo, (GfxObj *)src_cel, &src_cel->opacity);
	}

	image_clear(dst_image, app_get_color_to_clear_layer(dst_layer));
	image_merge(dst_image, src_image, src_cel->x, src_cel->y, 255, BLEND_MODE_NORMAL);

	src_cel->x = 0;
	src_cel->y = 0;
	src_cel->opacity = 255;

	stock_replace_image(sprite->stock, src_cel->image, dst_image);
	image_free(src_image);
      }
      
      if (undo_is_enabled(sprite->undo))
	undo_add_cel(sprite->undo, dst_layer, src_cel);
      layer_add_cel(dst_layer, src_cel);
    }
  }

  if (undo_is_enabled(sprite->undo))
    undo_close(sprite->undo);

  set_frame_to_handle(NULL, 0, NULL, 0);
}

void copy_cel()
{
  Sprite *sprite = current_sprite;
  Cel *src_cel, *dst_cel;

  assert(src_layer != NULL);
  assert(dst_layer != NULL);
  assert(src_frame >= 0 && src_frame < sprite->frames);
  assert(dst_frame >= 0 && dst_frame < sprite->frames);

  src_cel = layer_get_cel(src_layer, src_frame);
  dst_cel = layer_get_cel(dst_layer, dst_frame);

  if (undo_is_enabled(sprite->undo)) {
    undo_set_label(sprite->undo, "Move Cel");
    undo_open(sprite->undo);

    undo_set_layer(sprite->undo, sprite);
    undo_int(sprite->undo, sprite, &sprite->frame);
  }

  sprite_set_layer(sprite, dst_layer);
  sprite_set_frame(sprite, dst_frame);

  /* remove the 'dst_cel' (if it exists) because it must be
     replaced with 'src_cel' */
  if ((dst_cel != NULL) && (!layer_is_background(dst_layer) || src_cel != NULL))
    RemoveCel(dst_layer, dst_cel);

  /* move the cel in the same layer */
  if (src_cel != NULL) {
    Image *src_image = stock_get_image(sprite->stock, src_cel->image);
    Image *dst_image;
    int image_index;
    int dst_cel_x;
    int dst_cel_y;
    int dst_cel_opacity;

    /* if we are moving a cel from a transparent layer to the
       background layer, we have to clear the background of the
       image */
    if (!layer_is_background(src_layer) &&
	layer_is_background(dst_layer)) {
      dst_image = image_crop(src_image,
			     -src_cel->x,
			     -src_cel->y,
			     sprite->w,
			     sprite->h, 0);

      image_clear(dst_image, app_get_color_to_clear_layer(dst_layer));
      image_merge(dst_image, src_image, src_cel->x, src_cel->y, 255, BLEND_MODE_NORMAL);

      dst_cel_x = 0;
      dst_cel_y = 0;
      dst_cel_opacity = 255;
    }
    else {
      dst_image = image_new_copy(src_image);
      dst_cel_x = src_cel->x;
      dst_cel_y = src_cel->y;
      dst_cel_opacity = src_cel->opacity;
    }

    /* add the image in the stock */
    image_index = stock_add_image(sprite->stock, dst_image);
    if (undo_is_enabled(sprite->undo))
      undo_add_image(sprite->undo, sprite->stock, image_index);

    /* create the new cel */
    dst_cel = cel_new(dst_frame, image_index);
    cel_set_position(dst_cel, dst_cel_x, dst_cel_y);
    cel_set_opacity(dst_cel, dst_cel_opacity);

    if (undo_is_enabled(sprite->undo))
      undo_add_cel(sprite->undo, dst_layer, dst_cel);
    layer_add_cel(dst_layer, dst_cel);
  }

  if (undo_is_enabled(sprite->undo))
    undo_close(sprite->undo);

  set_frame_to_handle(NULL, 0, NULL, 0);
}