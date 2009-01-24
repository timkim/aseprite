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

#include "jinete/jinete.h"

#include "commands/commands.h"
#include "core/app.h"
#include "modules/sprites.h"
#include "raster/sprite.h"

static void cmd_exit_execute(const char *argument)
{
  Sprite *sprite = get_first_sprite();
  Sprite *clipboard = get_clipboard_sprite();

  while (sprite) {
    /* check if this sprite is modified */

    if (sprite_is_modified(sprite) &&
	(!clipboard || sprite->id != clipboard->id)) {
      if (jalert(_("Warning<<There are sprites with changes.<<Do you want quit anyway?||&Yes||&No")) != 1) {
	return;
      }
      break;
    }
    sprite = get_next_sprite(sprite);
  }

  /* close the window */
  jwindow_close(app_get_top_window(), 0);
}

Command cmd_exit = {
  CMD_EXIT,
  NULL,
  NULL,
  cmd_exit_execute,
  NULL
};