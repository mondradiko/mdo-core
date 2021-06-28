/** @file display.h
 */

#pragma once

struct display_poll_t
{
  /**
   * Time elapsed this frame, in seconds.
   */
  double dt;

  /**
   * Non-zero if the world simulation should continue this frame.
   */
  int should_run;

  /**
   * Non-zero if the renderer should draw this frame.
   */
  int should_render;

  /**
   * Non-zero if this display should exit.
   */
  int should_exit;
};
