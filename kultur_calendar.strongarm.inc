<?php
/**
 * @file
 * kultur_calendar.strongarm.inc
 */

/**
 * Implements hook_strongarm().
 */
function kultur_calendar_strongarm() {
  $export = array();

  $strongarm = new stdClass();
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'date_format_kulturoen_day_heading';
  $strongarm->value = 'l d.m.Y';
  $export['date_format_kulturoen_day_heading'] = $strongarm;

  $strongarm = new stdClass();
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'date_format_kultur_time';
  $strongarm->value = 'G:i';
  $export['date_format_kultur_time'] = $strongarm;

  return $export;
}
