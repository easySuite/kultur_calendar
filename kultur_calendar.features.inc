<?php
/**
 * @file
 * kultur_calendar.features.inc
 */

/**
 * Implements hook_ctools_plugin_api().
 */
function kultur_calendar_ctools_plugin_api($module = NULL, $api = NULL) {
  if ($module == "strongarm" && $api == "strongarm") {
    return array("version" => "1");
  }
}
